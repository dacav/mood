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

#include <libmoodgoph/response.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

static const size_t LIST_ARRAY_BASE_SIZE = 10;
static const size_t LIST_ARRAY_GROW_RATIO = 2;
static const char* LIST_ITEM_FORMAT = "%c%s\t%s\t%s\t%hu\r\n";

struct list_data
{
    size_t array_size;
    char** lines;
    size_t lines_count;
};

struct moodgoph_response
{
    enum {
        TYPE_UNDEFINED = 0,
        TYPE_LIST,
        TYPE_ERROR,
        TYPE_BYTES,
    } type;
    union {
        struct list_data list;
    } data;
};

static void list_data_init(struct list_data* list_data);
static void list_data_free(struct list_data* list_data);
static int list_data_append(struct list_data* list_data,
                            const struct moodgoph_response_list_item* item);
static size_t list_item_repr_length(const struct moodgoph_response_list_item*);

moodgoph_response_t moodgoph_response_new()
{
    moodgoph_response_t response = malloc(sizeof(struct moodgoph_response));
    if (!response) {
        return NULL;
    }

    memset(response, 0, sizeof(struct moodgoph_response));
    return response;
}

int moodgoph_response_list_append(moodgoph_response_t response,
                                  const struct moodgoph_response_list_item* item)
{
    assert(response->type == TYPE_UNDEFINED || response->type == TYPE_LIST);
    switch (response->type) {
        case TYPE_UNDEFINED:
            response->type = TYPE_LIST;
            list_data_init(&response->data.list);
        case TYPE_LIST:
            break;
        default:
            abort();
    }

    int error = list_data_append(&response->data.list, item);
    if (error) {
        list_data_free(&response->data.list);
        response->type = TYPE_UNDEFINED;
    }
    return error;
}

void moodgoph_response_delete(moodgoph_response_t response)
{
    switch (response->type)
    {
        case TYPE_LIST:
            list_data_free(&response->data.list);
            break;
        case TYPE_ERROR:
        case TYPE_BYTES:
        case TYPE_UNDEFINED:
            break;
    }
    free(response);
}

static void list_data_init(struct list_data* list_data)
{
    memset(list_data, 0, sizeof(struct list_data));
}

static void list_data_free(struct list_data* list_data)
{
    for (size_t i = 0; i < list_data->lines_count; i ++) {
        free(list_data->lines[i]);
        list_data->lines[i] = NULL;
    }
    free(list_data->lines);
}

static int list_data_append(struct list_data* list_data,
                            const struct moodgoph_response_list_item* item)
{
    if (list_data->array_size == 0) {
        list_data->lines = malloc(LIST_ARRAY_BASE_SIZE * sizeof(char *));
        list_data->array_size = LIST_ARRAY_BASE_SIZE;
    }

    const size_t line_len = list_item_repr_length(item);
    char* line = malloc(list_item_repr_length(item) * sizeof(char));
    if (!line) {
        return -1;
    }

    snprintf(line, line_len, LIST_ITEM_FORMAT,
        item->type,
        item->display_string,
        item->selector,
        item->hostname,
        item->port
    );
    list_data->lines[list_data->lines_count++] = line;

    if (list_data->lines_count == list_data->array_size) {
        const size_t new_size = list_data->array_size * LIST_ARRAY_GROW_RATIO;
        char** resized = realloc(list_data->lines, new_size * sizeof(char *));
        if (!resized) {
            return -1;
        }
        list_data->lines = resized;
        list_data->array_size = new_size;
    }
    list_data->lines[list_data->lines_count] = NULL;

    return 0;
}

static size_t list_item_repr_length(const struct moodgoph_response_list_item* item)
{
    char byte = 0;
    return 1 + snprintf(&byte, 0, LIST_ITEM_FORMAT,
        item->type,
        item->display_string,
        item->selector,
        item->hostname,
        item->port
    );
}
