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

typedef struct moodio_server* moodio_server_t;

typedef void (*server_accepted_cb_t)(moodio_server_t, int clientfd);

struct moodio_server_params
{
    struct event_base* event_base;
    int socket;
    server_accepted_cb_t on_accepted;
    void *user_context;
};

moodio_server_t moodio_server_new(const struct moodio_server_params* params);
void moodio_server_pause_accepting(moodio_server_t);
void moodio_server_resume_accepting(moodio_server_t);
void* moodio_server_get_context(moodio_server_t);
void moodio_server_delete(moodio_server_t);
