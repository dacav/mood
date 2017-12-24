#include "setup.h"
#include "session.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>

static session_t on_accepted(setup_t setup, int clsock);
static void on_deliver(session_t, uint8_t *, size_t);
static void on_send_done(session_t);
static void on_end_of_stream(session_t);
static void on_error(session_t, char* op, int errno_val);
static void on_deleted(session_t);

int main (int argc, char **argv)
{
    struct setup_conf setup_conf = {
        .ip_bind_address = "::1",
        .tcp_port = 7070,
        .backlog = 3,
        .max_sessions = 8,
        .user_context = NULL,
        .on_accepted = on_accepted
    };
    setup_t setup = setup_new(&setup_conf);
    if (!setup) {
        return EXIT_FAILURE;
    }

    errno = 0;
    if (event_base_dispatch(setup_get_event_base(setup)) == -1) {
        perror("event_base_dispatch");
        return EXIT_FAILURE;
    }

    setup_del(setup);

    return EXIT_SUCCESS;
}

static session_t on_accepted(setup_t setup, int clsock)
{
    struct session_params params = {
        .event_base = setup_get_event_base(setup),
        .socket = clsock,
        .recv_buffer_size = 512,
        .on_deliver = on_deliver,
        .on_send_done = on_send_done,
        .on_end_of_stream = on_end_of_stream,
        .on_error = on_error,
        .on_deleted = on_deleted,

        .send_timeout = {
            .tv_sec = 1000,
            .tv_usec = 0
        },
        .recv_timeout = {
            .tv_sec = 1000,
            .tv_usec = 0
        },
        .user_context = setup
    };

    session_t session = session_new(&params);
    if (!session) return NULL;

    fprintf(stderr, "%p: created\n", session);

    session_sched_recv(session);
    return session;
}

static void on_deliver(session_t session, uint8_t *data, size_t len)
{
    fprintf(stderr, "%p: delivered %zu bytes\n",
        session, len
    );

    if (session_send_bytes(session, data, len) == -1) {
        fprintf(stderr, "session_send_buffer: %s\n", strerror(errno));
        session_sched_delete(session);
    }
}

static void on_send_done(session_t session)
{
    fprintf(stderr, "%p: sent.\n", session);
    if (session_sched_recv(session) == -1) {
        fprintf(stderr, "session_sched_recv: %s\n", strerror(errno));
        session_sched_delete(session);
    }
}

static void on_end_of_stream(session_t session)
{
    fprintf(stderr, "%p: end_of_stream\n", session);
    session_sched_delete(session);
}

static void on_error(session_t session, char* op, int errno_val)
{
    fprintf(stderr, "%p: error %s while %s\n",
        session, strerror(errno_val), op
    );
    session_sched_delete(session);
}

static void on_deleted(session_t session)
{
    fprintf(stderr, "%p: deleted\n");
    setup_t setup = (setup_t) session_get_context(session);
    setup_notify_session_termination(setup, session);
}
