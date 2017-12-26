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

#include <event2/event.h>
#include <netinet/in.h>

#include "session.h"

typedef struct moodio_setup* moodio_setup_t;

/* Invoked when a new inbound connection is accepted.
 *
 * param: the setup object;
 * param: the new client socket;
 * returns: the new session object.
 */
typedef moodio_session_t (*moodio_setup_on_accepted_cb_t)(moodio_setup_t,
                                                          int clsock);

struct moodio_setup_conf
{
    /* Bind address (IPv6 or IPv4-mapped-on-IPv6). */
    const char* ip_bind_address;

    /* TCP port. */
    in_port_t tcp_port;

    /* Listen backlog */
    unsigned backlog;

    size_t max_sessions;

    void* user_context;
    moodio_setup_on_accepted_cb_t on_accepted;
};

moodio_setup_t moodio_setup_new(const struct moodio_setup_conf* conf);

void* moodio_setup_get_user_context(moodio_setup_t);

struct event_base* moodio_setup_get_event_base(moodio_setup_t);

void moodio_setup_notify_session_termination(moodio_setup_t, moodio_session_t);

void moodio_setup_del(moodio_setup_t);
