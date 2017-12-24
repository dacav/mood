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

typedef struct server* server_t;

typedef void (*server_accepted_cb_t)(server_t, int clientfd);

struct server_params
{
    struct event_base* event_base;
    int socket;
    server_accepted_cb_t on_accepted;
    void *user_context;
};

server_t server_new(const struct server_params* params);
void server_pause_accepting(server_t);
void server_resume_accepting(server_t);
void* server_get_context(server_t);
void server_delete(server_t);
