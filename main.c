#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcpsock.h"
#include "server.h"
#include "session.h"

#define MAX_ACTIVE_SESSIONS 2

struct global_context
{
    struct event_base *event_base;
    server_t server;

    size_t active_sessons;
    session_t sessions[MAX_ACTIVE_SESSIONS];
};

static void on_accepted(server_t server, int clsock);
static void glc_shutdown(struct global_context* glc);
static void handle_termination_by_signal(int, short, void*);

int main (int argc, char **argv)
{
    int exit_status = EXIT_FAILURE;

    if (argc < 2) {
        fprintf(stderr, "usage %s <bindaddr>\n", argv[0]);
        goto term0;
    }

    int server_sock = tcpsock_serve(argv[1], 7070, 2);
    if (server_sock == -1) {
        goto term0;
    }

    struct global_context global_context;
    memset(&global_context, 0, sizeof(struct global_context));

    errno = 0;
    global_context.event_base = event_base_new();
    if (global_context.event_base == NULL) {
        perror("event_base_new");
        goto term1;
    }

    struct server_params server_params = {
        .socket = server_sock,
        .event_base = global_context.event_base,
        .on_accepted = on_accepted,
        .user_context = &global_context,
    };
    global_context.server = server_new(&server_params);
    if (global_context.server == NULL) {
        goto term2;
    }

    struct event* ev_sigint = evsignal_new(
        global_context.event_base,
        SIGINT,
        handle_termination_by_signal,
        &global_context
    );
    if (ev_sigint == NULL) {
        perror("evsignal_new");
        goto term3;
    }

    errno = 0;
    if (evsignal_add(ev_sigint, NULL) == -1) {
        perror("evsignal_add");
        goto term3;
    }

    errno = 0;
    if (event_base_dispatch(global_context.event_base) == -1) {
        perror("event_base_dispatch");
    }

  term3:
    server_delete(global_context.server);
  term2:
    event_base_free(global_context.event_base);
  term1:
    close(server_sock);
  term0:
    return exit_status;
}

static void glc_shutdown(struct global_context* glc)
{
    fprintf(stderr, "glc_shutdown\n");
    for (size_t i = 0; i < glc->active_sessons; i ++) {
        session_sched_delete(glc->sessions[i]);
    }
}

static void on_accepted(server_t server, int clsock)
{
    fprintf(stderr, "flip off! (clsock=%i)\n", clsock);
    close(clsock);
}

static void handle_termination_by_signal(int fd, short ev, void* arg)
{
    struct global_context *glc = (struct global_context*)arg;
    glc_shutdown(glc);
    event_base_loopexit(glc->event_base, NULL);
}
