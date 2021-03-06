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

#include <libmoodio/session.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

struct send_data_buffer
{
    uint8_t *bytes;
    off_t offset;
    size_t to_send;
};

struct send_data
{
    enum
    {
        WM_IDLE = 0,
        WM_BUFFER,
    } mode;

    union
    {
        struct send_data_buffer buffer;
    } params;
};

struct recv_data
{
    uint8_t *buffer;
};

struct moodio_session
{
    struct moodio_session_params params;

    struct event* ev_recv;
    struct event* ev_send;
    struct event* ev_destroy;

    struct recv_data recv_data;
    struct send_data send_data;
};

static int check_params(const struct moodio_session_params* params);
static int schedule_send(moodio_session_t);
static void handle_recv(int, short, void*);
static void handle_send(int, short, void*);
static void handle_destroy(int, short, void*);
static void try_recv(moodio_session_t);
static void try_send(moodio_session_t);

moodio_session_t moodio_session_new(const struct moodio_session_params* params)
{
    if (check_params(params) == -1) {
        fprintf(stderr, "aborting session creation\n");
        return NULL;
    }

    moodio_session_t session = calloc(1, sizeof(struct moodio_session));
    if (session == NULL) {
        perror("calloc");
        return NULL;
    }

    memcpy(&session->params, params, sizeof(struct moodio_session_params));

    errno = 0;
    session->ev_recv = event_new(
        session->params.event_base,
        session->params.socket,
        EV_READ | EV_TIMEOUT,
        handle_recv,
        (void*)session
    );
    if (session->ev_recv == NULL) {
        perror("event_new");
        goto err0;
    }

    errno = 0;
    session->ev_send = event_new(
        session->params.event_base,
        session->params.socket,
        EV_WRITE | EV_TIMEOUT,
        handle_send,
        (void*)session
    );
    if (session->ev_send == NULL) {
        perror("event_new");
        goto err1;
    }

    errno = 0;
    session->ev_destroy = evtimer_new(
        session->params.event_base,
        handle_destroy,
        (void*)session
    );
    if (session->ev_destroy == NULL) {
        perror("evtimer_new");
        goto err2;
    }

    errno = 0;
    if (session->params.on_deliver) {
        session->recv_data.buffer = malloc(
            (1 + session->params.recv_buffer_size) * sizeof(uint8_t)
        );
        if (session->recv_data.buffer == NULL) {
            perror("malloc");
            goto err3;
        }

        /* Ensuring there's always a '\0' at the end of the string. Even
         * though no assumption is made on the payload, the terminator
         * guarantees a correct string handling */
        session->recv_data.buffer[session->params.recv_buffer_size] = '\0';
    }

    return session;

  /* currently unused 
  err4: 
    free(session->recv_data.buffer);
  */
  err3:
    event_free(session->ev_destroy);
  err2:
    event_free(session->ev_send);
  err1:
    event_free(session->ev_recv);
  err0:
    free(session);
    return NULL;
}

void* moodio_session_get_context(moodio_session_t session)
{
    return session->params.user_context;
}

int moodio_session_send_buffer(moodio_session_t session,
                               void* data,
                               size_t size)
{
    if (session->params.socket == -1) {
        errno = ENOTCONN;
        return -1;
    }

    if (session->send_data.mode != WM_IDLE) {
        errno = EBUSY;
        return -1;
    }

    errno = 0;
    session->send_data.mode = WM_BUFFER;
    session->send_data.params.buffer.bytes = (uint8_t *)data;
    session->send_data.params.buffer.offset = 0;
    session->send_data.params.buffer.to_send = size;

    return schedule_send(session);
}

int moodio_session_send_bytes(moodio_session_t session,
                              const void* bytes,
                              size_t len)
{
    char* copy = malloc(len);
    if (copy == NULL) {
        return -1;
    }
    memcpy(copy, bytes, len);
    int ret = moodio_session_send_buffer(session, copy, len);
    if (ret == -1) {
        free(copy);
    }
    return ret;
}

int moodio_session_send_string(moodio_session_t session, const char* str)
{
    return moodio_session_send_bytes(session, str, strlen(str) + 1);
}

int moodio_session_sched_recv(moodio_session_t session)
{
    if (session->params.socket == -1) {
        errno = ENOTCONN;
        return -1;
    }

    errno = 0;
    const int ret = event_add(
        session->ev_recv,
        &session->params.recv_timeout
    );
    if (ret == -1) {
        perror("sched_recv: event_add");
    }
    return ret;
}

int moodio_session_sched_delete(moodio_session_t session)
{
    if (session->params.socket == -1) {
        errno = EINVAL;
        return -1;
    }

    errno = 0;
    const struct timeval now = {0, 0};
    const int ret = event_add(session->ev_destroy, &now);
    if (ret == -1) {
        perror("sched_delete: event_add");
    }
    return ret;
}

static int schedule_send(moodio_session_t session)
{
    errno = 0;
    const int ret = event_add(
        session->ev_send,
        &session->params.send_timeout
    );
    if (ret == -1) {
        perror("sched_send: event_add");
    }
    return ret;
}

static int check_params(const struct moodio_session_params* params)
{
    /* -- 1. Static checks, just assertions -- */
    assert(params->socket != -1);
    assert(params->on_error);
    assert(
        !params->on_deliver
        || (params->on_end_of_stream && params->recv_buffer_size)
    );

    /* -- 2. Runtime-dependent situation checks -- */
    if (fcntl(params->socket, F_GETFL) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

static void handle_recv(int socket, short what, void* arg)
{
    moodio_session_t session = (moodio_session_t)arg;
    if (what == EV_TIMEOUT) {
        session->params.on_error(session, "recv", ETIMEDOUT);
    }
    else {
        try_recv(session);
    }
}

static void handle_send(int socket, short what, void* arg)
{
    moodio_session_t session = (moodio_session_t)arg;
    if (what == EV_TIMEOUT) {
        session->params.on_error(session, "send", ETIMEDOUT);
    }
    else {
        try_send(session);
    }
}

static void handle_destroy(int socket, short ev, void* arg)
{
    moodio_session_t session = (moodio_session_t)arg;

    close(session->params.socket);
    free(session->recv_data.buffer);
    event_free(session->ev_destroy);
    event_free(session->ev_send);
    event_free(session->ev_recv);

    if (session->params.on_deleted) {
        session->params.socket = -1;    /* Flags destruction */
        session->params.on_deleted(session);
    }

    struct send_data_buffer* buffer = &session->send_data.params.buffer;
    if (buffer->bytes) {
        free(buffer->bytes);
        buffer->bytes = NULL;
    }
    free(session);
}

static void try_recv(moodio_session_t session)
{
    const size_t recvbuf_size = session->params.recv_buffer_size;
    uint8_t * const recvbuf = session->recv_data.buffer;
    const ssize_t recv_size = recv(
        session->params.socket,
        recvbuf,
        recvbuf_size,
        0
    );

    if (recv_size == -1) {
        session->params.on_error(session, "recv", errno);
    }
    else if (recv_size == 0) {
        session->params.on_end_of_stream(session);
    }
    else {
        session->params.on_deliver(session, recvbuf, recv_size);

        /* Not exhaustive, but could detect some exceeding of boundaries.
         * We always want the terminator to be in place. */
        assert(recvbuf[recvbuf_size] == '\0');
    }
}

static void try_send_buffer(moodio_session_t session)
{
    const int sock = session->params.socket;
    struct send_data_buffer* buffer = &session->send_data.params.buffer;

    ssize_t sent = send(
        sock,
        buffer->bytes + buffer->offset,
        buffer->to_send,
        MSG_NOSIGNAL
    );

    if (sent == -1) {
        session->params.on_error(session, "send", errno);
    }
    else if (sent < buffer->to_send) {
        buffer->to_send -= sent;
        buffer->offset += sent;

        if (schedule_send(session) == -1) {
            session->params.on_error(session, "schedule_send", errno);
        }
    }
    else {
        free(buffer->bytes);
        buffer->bytes = NULL;
        session->send_data.mode = WM_IDLE;
        session->params.on_send_done(session);
    }
}

static void try_send(moodio_session_t session)
{
    switch (session->send_data.mode) {
        case WM_IDLE:
            assert(0);  /* by construction */
        case WM_BUFFER:
            try_send_buffer(session);
    }
}
