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

#include "setup.h"

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "tcpsock.h"
#include "server.h"
#include "session.h"
#include "sigwrap.h"

struct session_slot
{
    session_t session;
};

struct setup
{
    struct event_base *event_base;
    server_t server;
    sigwrap_t sigwrap;

    enum { READY, SHUTDOWN } state;

    void* user_context;
    setup_on_accepted_cb_t on_accepted;

    size_t max_sessions;
    size_t active_sessons;
    struct session_slot *session_slots;
};

/* Subtasks of setup */
static int setup_server(setup_t, const struct setup_conf*);
static int setup_signals(setup_t);

/* Callbacks */
static void on_accepted(server_t server, int clsock);
static void handle_termination_by_signal(int signal, void* arg);
static void shutdown_sessions(setup_t);

setup_t setup_new(const struct setup_conf* conf)
{
    setup_t setup = calloc(1, sizeof(struct setup));
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
    server_delete(setup->server);
  err3:
    event_base_free(setup->event_base);
  err2:
    free(setup->session_slots);
  err1:
    free(setup);
  err0:
    return NULL;
}

void* setup_get_user_context(setup_t setup)
{
    return setup->user_context;
}

struct event_base* setup_get_event_base(setup_t setup)
{
    return setup->event_base;
}

void setup_notify_session_termination(setup_t setup, session_t session)
{
    for (unsigned i = 0; i < setup->active_sessons; i ++) {
        if (setup->session_slots[i].session == session) {
            setup->active_sessons --;
            memcpy(
                &setup->session_slots[i],
                &setup->session_slots[setup->active_sessons],
                sizeof(struct session_slot)
            );
            server_resume_accepting(setup->server);
            break;
        }
    }
}

void setup_del(setup_t setup)
{
    sigwrap_del(setup->sigwrap);
    server_delete(setup->server);
    event_base_free(setup->event_base);
    free(setup->session_slots);
    free(setup);
}

static int setup_server(setup_t setup, const struct setup_conf* conf)
{
    int server_sock = tcpsock_serve(
        conf->ip_bind_address,
        conf->tcp_port,
        conf->backlog
    );
    if (server_sock == -1) return -1;

    struct server_params server_params = {
        .socket = server_sock,
        .event_base = setup->event_base,
        .on_accepted = on_accepted,
        .user_context = setup,
    };
    setup->server = server_new(&server_params);
    if (!setup->server) {
        close(server_sock);
        return -1;
    }
    return 0;
}

static int setup_signals(setup_t setup)
{
    struct sigwrap_setting signals[] = {
        {SIGINT, handle_termination_by_signal, setup},
        {SIGTERM, handle_termination_by_signal, setup},
        {-1, NULL, NULL}
    };
    setup->sigwrap = sigwrap_new(setup->event_base, signals);
    return setup->sigwrap ? 0 : -1;
}

static void on_accepted(server_t server, int clsock)
{
    setup_t setup = server_get_context(server);

    if (setup->state == SHUTDOWN) {
        close(clsock);
        return;
    }

    session_t new_session = setup->on_accepted(setup, clsock);
    if (new_session == NULL) {
        close(clsock);
        return;
    }

    setup->session_slots[setup->active_sessons ++].session = new_session;
    if (setup->active_sessons == setup->max_sessions) {
        server_pause_accepting(setup->server);
    }
    else assert(setup->active_sessons < setup->max_sessions);
}

static void handle_termination_by_signal(int signal, void* arg)
{
    setup_t setup = (setup_t)arg;
    shutdown_sessions(setup);
    event_base_loopexit(setup->event_base, NULL);
}

static void shutdown_sessions(setup_t setup)
{
    setup->state = SHUTDOWN;
    for (size_t i = 0; i < setup->active_sessons; i ++) {
        session_t session = setup->session_slots[i].session;
        session_sched_delete(session);
    }
    setup->active_sessons = 0;
}
