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

#include <signal.h>
#include <event2/event.h>

typedef struct moodio_sigwrap* moodio_sigwrap_t;

typedef void (*moodio_sigwrap_cb_t)(int signal, void* user_context);

struct moodio_sigwrap_setting
{
    int signal;
    moodio_sigwrap_cb_t callback;
    void* user_context;
};

/*
 * param: the base libevent object
 * param: array of signals to register. Must be terminated with a guard
 *        object having callback=NULL
 */
moodio_sigwrap_t moodio_sigwrap_new(struct event_base *event_base,
                                    const struct moodio_sigwrap_setting*);

void moodio_sigwrap_del(moodio_sigwrap_t);
