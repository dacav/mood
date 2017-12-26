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

#include <libmoodio/server.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct moodio_server
{
    struct moodio_server_params params;
    struct event* ev_connect;
    enum {
        ACCEPT_CONTINUE,
        ACCEPT_PAUSE,
    } state;
};

static void handle_connection(int, short, void*);

moodio_server_t moodio_server_new(const struct moodio_server_params* params)
{
    moodio_server_t server = calloc(1, sizeof(struct moodio_server));
    if (server == NULL) {
        perror("calloc");
        goto err0;
    }

    memcpy(&server->params, params, sizeof(struct moodio_server_params));
    server->state = ACCEPT_CONTINUE;

    errno = 0;
    server->ev_connect = event_new(
        server->params.event_base,
        server->params.socket,
        EV_READ,
        handle_connection,
        (void*)server
    );
    if (server->ev_connect == NULL) {
        perror("event_new");
        goto err1;
    }

    errno = 0;
    if (event_add(server->ev_connect, NULL) == -1) {
        perror("event_add");
        goto err2;
    }

    return server;

  err2:
    event_free(server->ev_connect);
  err1:
    free(server);
  err0:
    return NULL;
}

void moodio_server_pause_accepting(moodio_server_t server)
{
    if (server->state == ACCEPT_PAUSE) return;
    event_del(server->ev_connect);
    server->state = ACCEPT_PAUSE;
}

void moodio_server_resume_accepting(moodio_server_t server)
{
    if (server->state == ACCEPT_CONTINUE) return;
    event_add(server->ev_connect, NULL);
    server->state = ACCEPT_CONTINUE;
}

void* moodio_server_get_context(moodio_server_t server)
{
    return server->params.user_context;
}

void moodio_server_delete(moodio_server_t server)
{
    event_free(server->ev_connect);
    close(server->params.socket);
    free(server);
}

static void handle_connection(int sock, short ev, void* arg)
{
    int clsock = accept(sock, NULL, NULL);
    if (clsock == -1) {
        perror("accept");
        return;
    }

    moodio_server_t server = (moodio_server_t) arg;
    server->params.on_accepted(server, clsock);

    if (server->state == ACCEPT_CONTINUE) {
        event_add(server->ev_connect, NULL);
    }
}
