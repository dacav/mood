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

#pragma once

#include <sys/time.h>
#include <stdint.h>
#include <event2/event.h>

typedef struct session* session_t;

typedef void (*session_deliver_cb_t)(session_t, uint8_t *, size_t);
typedef void (*session_send_done_cb_t)(session_t);
typedef void (*session_end_of_stream_cb_t)(session_t);
typedef void (*session_error_cb_t)(session_t, char* op, int errno_val);
typedef void (*session_deleted_cb_t)(session_t);

struct session_params
{
    struct event_base* event_base;
    int socket;
    size_t recv_buffer_size;

    session_deliver_cb_t on_deliver;
    session_send_done_cb_t on_send_done;
    session_end_of_stream_cb_t on_end_of_stream;
    session_error_cb_t on_error;
    session_deleted_cb_t on_deleted;    /* optional */

    struct timeval send_timeout;
    struct timeval recv_timeout;

    void *user_context;
};

session_t session_new(const struct session_params* params);

void* session_get_context(session_t);

/* Send a buffer.
 *
 * The buffer is assumed to be dynamically allocated (via malloc),
 * and its ownership is considered transferred to the session.
 *
 * Returns 0 on success and -1 on error. In case of error, errno is set, and
 * the ownership of the input buffer is not taken in: the calling code is in
 * charge of buffer deallocation.
 * 
 * Possible values for errno:
 *  - ENOTCONN: The session has been terminated;
 *  - EBUSY: A previously scheduled send operation is still in progress.
 *  - Different values could be assigned by the underlying libevent calls.
 */
int session_send_buffer(session_t, void*, size_t);

/* Send an array of bytes.
 *
 * The calling code retains ownership on the provided array of bytes. An
 * internal copy of it is made for internal use.
 *
 * Returns 0 on success and -1 on error. In case of error, errno is set.
 *
 * Possible values for errno:
 *  - The values from session_send_buffer
 *  - The values from malloc (used internally);
 */
int session_send_bytes(session_t, const void*, size_t);

/* Send a null-terminated string.
 *
 * The calling code retains ownership on the string. An internal copy of it
 * is made for internal use.
 *
 * Returns 0 on success and -1 on error. In case of error, errno is set with
 * the same errno values as in session_send_bytes.
 */
int session_send_string(session_t, const char*);

/* Schedule a data reception.
 *
 * Returns 0 on success and -1 on error. In case of error, errno is set.
 *
 * Possible values for errno:
 *  - ENOTCONN: The session has been terminated;
 *  - Different values could be assigned by the underlying libevent calls.
 */
int session_sched_recv(session_t);

int session_sched_delete(session_t);
