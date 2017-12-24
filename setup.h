#pragma once

#include <event2/event.h>
#include <netinet/in.h>

#include "session.h"

typedef struct setup* setup_t;

/* Invoked when a new inbound connection is accepted.
 *
 * param: the setup object;
 * param: the new client socket;
 * returns: the new session object.
 */
typedef session_t (*setup_on_accepted_cb_t)(setup_t, int clsock);

struct setup_conf
{
    /* Bind address (IPv6 or IPv4-mapped-on-IPv6). */
    const char* ip_bind_address;

    /* TCP port. */
    in_port_t tcp_port;

    /* Listen backlog */
    unsigned backlog;

    void* user_context;
    setup_on_accepted_cb_t on_accepted;
};

setup_t setup_new(const struct setup_conf* conf);

void* setup_get_user_context(setup_t);

struct event_base* setup_get_event_base(setup_t);

void setup_notify_session_termination(setup_t, session_t);

void setup_del(setup_t);
