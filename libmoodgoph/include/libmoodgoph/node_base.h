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

#include <libmoodgoph/request.h>

typedef struct moodgoph_node* moodgoph_node_t;

typedef void (*moodgoph_node_serve_cb_t)(moodgoph_node_t, moodgoph_request_t);

moodgoph_node_t moodgoph_node_new(const char* name,
                                  void* context,
                                  moodgoph_node_serve_cb_t serve_cb);

void* moodgoph_node_get_context(moodgoph_node_t);

void moodgoph_node_serve(moodgoph_node_t, moodgoph_request_t);

void moodgoph_node_delete(moodgoph_node_t);
