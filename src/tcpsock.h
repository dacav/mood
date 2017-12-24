#pragma once

#include <netinet/in.h>

int tcpsock_serve(const char* bind_addr,
                  in_port_t port,
                  unsigned backlog);

