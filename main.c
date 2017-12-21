#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tcpsock.h"
#include "error.h"

/* accept */
#include <sys/types.h>
#include <sys/socket.h>

/* epoll */
#include <sys/epoll.h>

int main (int argc, char **argv)
{
    int sock = tcpsock_serve(argv[1], 7070, 2);
    if (sock == -1) {
        return EXIT_FAILURE;
    }

    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        close(sock);
        return EXIT_FAILURE;
    }

    struct epoll_event accept_ev;
    accept_ev.events = EPOLLIN;
    accept_ev.data.fd = sock;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &accept_ev);

    const size_t nevents = 10;
    struct epoll_event events[nevents];
    for (;;) {
        const int nevents = epoll_wait(epollfd, events, nevents, -1);
        if (nevents == -1) {
            perror("epoll_wait");
            break;
        }

        for (size_t i = 0; i < nevents; i ++) {
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}
