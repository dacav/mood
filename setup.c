#include "setup.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "tcpsock.h"
#include "server.h"
#include "session.h"
#include "signal_wrapper.h"

#define MAX_ACTIVE_SESSIONS 2

struct setup
{
    struct event_base *event_base;
    server_t server;
    sigwrap_t sigwrap;

    size_t active_sessons;
    session_t sessions[MAX_ACTIVE_SESSIONS];
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

    errno = 0;
    setup->event_base = event_base_new();
    if (!setup->event_base) {
        perror("event_base_new");
        goto err1;
    }

    if (setup_server(setup, conf) == -1) {
        goto err2;
    }

    if (setup_signals(setup) == -1) {
        goto err3;
    }
    return setup;

  err3:
    server_delete(setup->server);
  err2:
    event_base_free(setup->event_base);
  err1:
    free(setup);
  err0:
    return NULL;
}

struct event_base* setup_get_event_base(setup_t setup)
{
    return setup->event_base;
}

void setup_del(setup_t setup)
{
    sigwrap_del(setup->sigwrap);
    server_delete(setup->server);
    event_base_free(setup->event_base);
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
    fprintf(stderr, "flip off! (clsock=%i)\n", clsock);
    close(clsock);
}

static void handle_termination_by_signal(int signal, void* arg)
{
    setup_t setup = (setup_t)arg;
    shutdown_sessions(setup);
    event_base_loopexit(setup->event_base, NULL);
}

static void shutdown_sessions(setup_t setup)
{
    for (size_t i = 0; i < setup->active_sessons; i ++) {
        session_sched_delete(setup->sessions[i]);
    }
    setup->active_sessons = 0;
}
