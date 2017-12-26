/*  This file is part of Mood.
 *
 *  Mood is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  Mood is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mood.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <libmoodio/server.h>
#include <libmoodio/session.h>
#include <libmoodio/setup.h>
#include <libmoodio/sigwrap.h>
#include <libmoodio/tcpsock.h>

struct session_slot
{
    moodio_session_t session;
};

struct moodio_setup
{
    struct event_base *event_base;
    moodio_server_t server;
    moodio_sigwrap_t sigwrap;

    enum { READY, SHUTDOWN } state;

    void* user_context;
    moodio_setup_on_accepted_cb_t on_accepted;

    size_t max_sessions;
    size_t active_sessons;
    struct session_slot *session_slots;
};

/* Subtasks of setup */
static int setup_server(moodio_setup_t,
                        const struct moodio_setup_conf*);
static int setup_signals(moodio_setup_t);

/* Callbacks */
static void on_accepted(moodio_server_t server, int clsock);
static void handle_termination_by_signal(int signal, void* arg);
static void shutdown_sessions(moodio_setup_t);

moodio_setup_t moodio_setup_new(const struct moodio_setup_conf* conf)
{
    moodio_setup_t setup = calloc(1, sizeof(struct moodio_setup));
    if (!setup) {
        perror("calloc");
        goto err0;
    }

    setup->session_slots = calloc(conf->max_sessions, sizeof(struct session_slot));
    if (!setup->session_slots) {
        perror("calloc");
        goto err1;
    }

    errno = 0;
    setup->event_base = event_base_new();
    if (!setup->event_base) {
        perror("event_base_new");
        goto err2;
    }

    if (setup_server(setup, conf) == -1) {
        goto err3;
    }

    if (setup_signals(setup) == -1) {
        goto err4;
    }

    setup->state = READY;
    setup->user_context = conf->user_context;
    setup->on_accepted = conf->on_accepted;
    setup->max_sessions = conf->max_sessions;
    return setup;

  err4:
    moodio_server_delete(setup->server);
  err3:
    event_base_free(setup->event_base);
  err2:
    free(setup->session_slots);
  err1:
    free(setup);
  err0:
    return NULL;
}

void* moodio_setup_get_user_context(moodio_setup_t setup)
{
    return setup->user_context;
}

struct event_base* moodio_setup_get_event_base(moodio_setup_t setup)
{
    return setup->event_base;
}

void moodio_setup_notify_session_termination(moodio_setup_t setup,
                                             moodio_session_t session)
{
    for (unsigned i = 0; i < setup->active_sessons; i ++) {
        if (setup->session_slots[i].session == session) {
            setup->active_sessons --;
            memcpy(
                &setup->session_slots[i],
                &setup->session_slots[setup->active_sessons],
                sizeof(struct session_slot)
            );
            moodio_server_resume_accepting(setup->server);
            break;
        }
    }
}

void moodio_setup_del(moodio_setup_t setup)
{
    moodio_sigwrap_del(setup->sigwrap);
    moodio_server_delete(setup->server);
    event_base_free(setup->event_base);
    free(setup->session_slots);
    free(setup);
}

static int setup_server(moodio_setup_t setup,
                        const struct moodio_setup_conf* conf)
{
    int server_sock = moodio_tcpsock_serve(
        conf->ip_bind_address,
        conf->tcp_port,
        conf->backlog
    );
    if (server_sock == -1) return -1;

    struct moodio_server_params server_params = {
        .socket = server_sock,
        .event_base = setup->event_base,
        .on_accepted = on_accepted,
        .user_context = setup,
    };
    setup->server = moodio_server_new(&server_params);
    if (!setup->server) {
        close(server_sock);
        return -1;
    }
    return 0;
}

static int setup_signals(moodio_setup_t setup)
{
    struct moodio_sigwrap_setting signals[] = {
        {SIGINT, handle_termination_by_signal, setup},
        {SIGTERM, handle_termination_by_signal, setup},
        {-1, NULL, NULL}
    };
    setup->sigwrap = moodio_sigwrap_new(setup->event_base, signals);
    return setup->sigwrap ? 0 : -1;
}

static void on_accepted(moodio_server_t server, int clsock)
{
    moodio_setup_t setup = moodio_server_get_context(server);

    if (setup->state == SHUTDOWN) {
        close(clsock);
        return;
    }

    moodio_session_t new_session = setup->on_accepted(setup, clsock);
    if (new_session == NULL) {
        close(clsock);
        return;
    }

    setup->session_slots[setup->active_sessons ++].session = new_session;
    if (setup->active_sessons == setup->max_sessions) {
        moodio_server_pause_accepting(setup->server);
    }
    else assert(setup->active_sessons < setup->max_sessions);
}

static void handle_termination_by_signal(int signal, void* arg)
{
    moodio_setup_t setup = (moodio_setup_t)arg;
    shutdown_sessions(setup);
    event_base_loopexit(setup->event_base, NULL);
}

static void shutdown_sessions(moodio_setup_t setup)
{
    setup->state = SHUTDOWN;
    for (size_t i = 0; i < setup->active_sessons; i ++) {
        moodio_session_t session = setup->session_slots[i].session;
        moodio_session_sched_delete(session);
    }
    setup->active_sessons = 0;
}
