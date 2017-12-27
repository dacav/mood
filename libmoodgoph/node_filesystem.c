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

#include <libmoodgoph/node_filesystem.h>
#include <string.h>
#include <stdlib.h>

static void serve_cb(moodgoph_node_t, moodgoph_request_t);
static void delete_cb(moodgoph_node_t);

struct fs_info
{
    char* base_dir;
};

moodgoph_node_t moodgoph_node_filesystem_new(const char* name,
                                             const char* base_dir)
{
    struct fs_info* info = malloc(sizeof(struct fs_info));
    if (!info) {
        return NULL;
    }

    const size_t len = strlen(base_dir) + 1;
    info->base_dir = malloc(sizeof(char) * len);
    if (!info->base_dir) {
        free(info);
        return NULL;
    }
    memcpy(info->base_dir, base_dir, len);

    struct moodgoph_node_params params = {
        .name = name,
        .context = info,
        .serve_cb = serve_cb,
        .delete_cb = delete_cb
    };

    moodgoph_node_t node = moodgoph_node_new(&params);
    if (node == NULL) {
        free(info->base_dir);
        free(info);
    }
    return node;
}

static void serve_cb(moodgoph_node_t node, moodgoph_request_t request)
{
}

static void delete_cb(moodgoph_node_t node)
{
    struct fs_info* info = (struct fs_info*) moodgoph_node_get_context(node);
    free(info->base_dir);
    free(info);
}
