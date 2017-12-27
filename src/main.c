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

#include <libmoodio/setup.h>
#include <libmoodio/session.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

static moodio_session_t on_accepted(moodio_setup_t setup, int clsock);
static void on_deliver(moodio_session_t, uint8_t *, size_t);
static void on_send_done(moodio_session_t);
static void on_end_of_stream(moodio_session_t);
static void on_error(moodio_session_t, char* op, int errno_val);
static void on_deleted(moodio_session_t);

int main (int argc, char **argv)
{
    struct moodio_setup_conf setup_conf = {
        .ip_bind_address = "::1",
        .tcp_port = 7070,
        .backlog = 3,
        .reuse_addr = true,
        .max_sessions = 8,
        .user_context = NULL,
        .on_accepted = on_accepted
    };
    moodio_setup_t setup = moodio_setup_new(&setup_conf);
    if (!setup) {
        return EXIT_FAILURE;
    }

    errno = 0;
    if (event_base_dispatch(moodio_setup_get_event_base(setup)) == -1) {
        perror("event_base_dispatch");
        return EXIT_FAILURE;
    }

    moodio_setup_del(setup);

    return EXIT_SUCCESS;
}

static moodio_session_t on_accepted(moodio_setup_t setup, int clsock)
{
    struct moodio_session_params params = {
        .event_base = moodio_setup_get_event_base(setup),
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

    moodio_session_t session = moodio_session_new(&params);
    if (!session) return NULL;

    fprintf(stderr, "%p: created\n", (void*)session);

    moodio_session_sched_recv(session);
    return session;
}

static void on_deliver(moodio_session_t session, uint8_t *data, size_t len)
{
    fprintf(stderr, "%p: delivered %zu bytes\n",
        (void*)session, len
    );

    if (moodio_session_send_bytes(session, data, len) == -1) {
        fprintf(stderr, "session_send_bytes: %s\n", strerror(errno));
        moodio_session_sched_delete(session);
    }
}

static void on_send_done(moodio_session_t session)
{
    fprintf(stderr, "%p: sent.\n", (void*)session);
    if (moodio_session_sched_recv(session) == -1) {
        fprintf(stderr, "session_sched_recv: %s\n", strerror(errno));
        moodio_session_sched_delete(session);
    }
}

static void on_end_of_stream(moodio_session_t session)
{
    fprintf(stderr, "%p: end_of_stream\n", (void*)session);
    moodio_session_sched_delete(session);
}

static void on_error(moodio_session_t session, char* op, int errno_val)
{
    fprintf(stderr, "%p: error %s while %s\n",
        (void*)session, strerror(errno_val), op
    );
    moodio_session_sched_delete(session);
}

static void on_deleted(moodio_session_t session)
{
    fprintf(stderr, "%p: deleted\n", (void*)session);
    moodio_setup_t setup = (moodio_setup_t) moodio_session_get_context(session);
    moodio_setup_notify_session_termination(setup, session);
}
