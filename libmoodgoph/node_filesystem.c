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

#include <stdio.h>

static void serve_cb(moodgoph_node_t, moodgoph_request_t);
static void delete_cb(moodgoph_node_t);
static const char* secure_join_path(const char* base_path,
                                    moodgoph_request_t request,
                                    char* buffer,
                                    size_t buffer_size);
static int bad_path_component(const char*);

struct fs_info
{
    char* base_dir;
    size_t base_dir_strlen;
};

moodgoph_node_t moodgoph_node_filesystem_new(const char* name,
                                             const char* base_dir)
{
    struct fs_info* info = malloc(sizeof(struct fs_info));
    if (!info) {
        return NULL;
    }

    info->base_dir_strlen = strlen(base_dir);
    const size_t len = info->base_dir_strlen + 1;
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
    struct fs_info* info = moodgoph_node_get_context(node);
    const size_t pathmaxlen = info->base_dir_strlen
                            + moodgoph_request_get_query_len(request)
                            + 1;
    char* buffer = malloc(pathmaxlen);
    if (!buffer) {
        /* TODO: send internal error */
        return;
    }

    const char* path = secure_join_path(
        info->base_dir, request, buffer, pathmaxlen
    );
    if (path == NULL) {
        /* TODO: send bad request (security) */
    }
    else {
        /* TODO: send response */
    }

    free(buffer);
}

static void delete_cb(moodgoph_node_t node)
{
    struct fs_info* info = (struct fs_info*) moodgoph_node_get_context(node);
    free(info->base_dir);
    free(info);
}

static const char* secure_join_path(const char* base_path,
                                    moodgoph_request_t request,
                                    char* buffer,
                                    size_t buffer_size)
{
    if (buffer_size < 1) {
        return NULL;
    }

    const size_t last = buffer_size - 1;
    size_t cursor = 0;
    const char* token;
    while ((token = moodgoph_request_next_token(request)) != NULL) {
        if (bad_path_component(token)) return NULL;

        while (cursor < last && *token != '\0') {
            buffer[cursor++] = *token++;
        }

        if (cursor == last) return NULL;
        buffer[cursor++] = '/';
        if (cursor == last) return NULL;
    }
    buffer[--cursor] = '\0';

    return buffer;
}

static int bad_path_component(const char* name)
{
    /* TODO: check bad path and return non-zero upon error */
    return 0;
}
