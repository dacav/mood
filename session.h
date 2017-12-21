#pragma once

#include <sys/time.h>
#include <stdint.h>
#include <event2/event.h>

typedef struct session* session_t;

typedef void (*session_deliver_cb_t)(session_t, uint8_t *, size_t);
typedef void (*session_send_done_cb_t)(session_t);
typedef void (*session_end_of_stream_cb_t)(session_t);
typedef void (*session_error_cb_t)(session_t, char* op, int errno_val);

struct session_params
{
    struct event_base* event_base;
    int socket;
    size_t recv_buffer_size;

    session_deliver_cb_t on_deliver;
    session_send_done_cb_t on_send_done;
    session_end_of_stream_cb_t on_end_of_stream;
    session_error_cb_t on_error;

    struct timeval send_timeout;
    struct timeval recv_timeout;

    void *user_context;
};

session_t session_new(const struct session_params* params);

void* session_get_context(session_t);

int session_send_buffer(session_t, const void*, size_t);
int session_send_file(session_t, int fd);

int session_sched_recv(session_t);

int session_sched_delete(session_t);
