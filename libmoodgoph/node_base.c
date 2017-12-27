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

#include <libmoodgoph/node_base.h>

#include <stdlib.h>
#include <string.h>

struct moodgoph_node
{
    char* name;
    void* context;
    moodgoph_node_serve_cb_t serve_cb;
};

moodgoph_node_t moodgoph_node_new(const char* name,
                                  void* context,
                                  moodgoph_node_serve_cb_t serve_cb)
{
    moodgoph_node_t node = malloc(sizeof(struct moodgoph_node));
    if (!node) {
        return NULL;
    }

    const size_t len = 1 + strlen(name);
    node->name = malloc(sizeof(char) * len);
    if (!node->name) {
        free(node);
        return NULL;
    }

    node->context = context;
    node->serve_cb = serve_cb;

    memcpy(node->name, name, len);
    return node;
}

void* moodgoph_node_get_context(moodgoph_node_t node)
{
    return node->context;
}

void moodgoph_node_serve(moodgoph_node_t node, moodgoph_request_t request)
{
    node->serve_cb(node, request);
}

void moodgoph_node_delete(moodgoph_node_t node)
{
    free(node->name);
    free(node);
}
