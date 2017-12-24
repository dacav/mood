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

typedef struct sigwrap* sigwrap_t;

typedef void (*sigwrap_cb_t)(int signal, void* user_context);

struct sigwrap_setting
{
    int signal;
    sigwrap_cb_t callback;
    void* user_context;
};

/*
 * param event_base: the base libevent object
 * param sigwrap_setting: array of signals to register. Must be terminated
 *                        with a guard object having callback=NULL
 */
sigwrap_t sigwrap_new(struct event_base *event_base,
                      const struct sigwrap_setting*);

void sigwrap_del(sigwrap_t);
