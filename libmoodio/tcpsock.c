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

#include <libmoodio/tcpsock.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

int moodio_tcpsock_serve(const char* bind_addr,
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
