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
    moodgoph_node_delete_cb_t delete_cb;
};

moodgoph_node_t moodgoph_node_new(const struct moodgoph_node_params* params)
{
    moodgoph_node_t node = malloc(sizeof(struct moodgoph_node));
    if (!node) {
        return NULL;
    }

    const size_t len = 1 + strlen(params->name);
    char* name_copy = malloc(sizeof(char) * len);
    if (!name_copy) {
        free(node);
        return NULL;
    }
    memcpy(name_copy, params->name, len);
    node->name = name_copy;
    node->context = params->context;
    node->serve_cb = params->serve_cb;
    node->delete_cb = params->delete_cb;

    return node;
}

void* moodgoph_node_get_context(moodgoph_node_t node)
{
    return node->context;
}

const char* moodgoph_node_get_name(moodgoph_node_t node)
{
    return node->name;
}

void moodgoph_node_serve(moodgoph_node_t node, moodgoph_request_t request)
{
    node->serve_cb(node, request);
}

void moodgoph_node_delete(moodgoph_node_t node)
{
    if (node->delete_cb) {
        node->delete_cb(node);
    }
    free(node->name);
    free(node);
}
