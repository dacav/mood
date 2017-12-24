#include "tcpsock.h"
#include "error.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

int tcpsock_serve(const char* bind_addr,
                  in_port_t port,
                  unsigned backlog)
{
    struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(port),
        .sin6_flowinfo = 0,
        .sin6_scope_id = 0,
    };

    if (inet_pton(AF_INET6, bind_addr, &addr.sin6_addr) == 0) {
        fprintf(stderr, "inet_pton(AF_INET6, %s, ...): %s\n",
            bind_addr,
            strerror(errno)
        );
        return -1;
    }

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sock);
    }

    if (listen(sock, backlog) == -1) {
        perror("listen");
        close(sock);
    }

    return sock;
}
