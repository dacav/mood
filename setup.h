#pragma once

#include <event2/event.h>
#include <netinet/in.h>

struct setup_conf
{
    /* Bind address (IPv6 or IPv4-mapped-on-IPv6). */
    const char* ip_bind_address;

    /* TCP port. */
    in_port_t tcp_port;

    /* Listen backlog */ 
    unsigned backlog;
};

typedef struct setup* setup_t;

setup_t setup_new(const struct setup_conf* conf);
struct event_base* setup_get_event_base(setup_t);

void setup_del(setup_t);
