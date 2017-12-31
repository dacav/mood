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

#include <stdint.h>

typedef struct moodgoph_response* moodgoph_response_t;

moodgoph_response_t moodgoph_response_new();

struct moodgoph_response_list_item
{
    enum {
        MOODGOPH_RESPONSE_FILE          = '0',
        MOODGOPH_RESPONSE_DIRECTORY     = '1',
        MOODGOPH_RESPONSE_ERROR         = '3',
        MOODGOPH_RESPONSE_INDEX_SEARCH  = '7',
        MOODGOPH_RESPONSE_TELNET        = '8',
        MOODGOPH_RESPONSE_BINARY        = '9',
        MOODGOPH_RESPONSE_GIF           = 'g',
        MOODGOPH_RESPONSE_IMAGE         = 'I',
    } type;
    const char* display_string;
    const char* selector;
    const char* hostname;
    uint16_t port;
};

int moodgoph_response_list_append(moodgoph_response_t response,
                                  const struct moodgoph_response_list_item*);

void moodgoph_response_delete(moodgoph_response_t);
