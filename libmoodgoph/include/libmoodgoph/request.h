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

#include <stddef.h>

typedef struct moodgoph_request* moodgoph_request_t;

/* Create a request given a query string.
 *
 * The procedure has side effects on the query string, which is marked with
 * '\0' characters and kept as backing memory for the returned
 * moodgoph_request_t object. Until the object is destroyed with
 * moodgoph_request_delete, the string must be not touched. The string
 * however is not owned by the object: moodgoph_request_delete will not
 * free() the string.
 */
moodgoph_request_t moodgoph_request_new(char* query, size_t querylen);

const char* moodgoph_request_next_token(moodgoph_request_t request);

size_t moodgoph_request_get_query_len(moodgoph_request_t request);

void moodgoph_request_delete(moodgoph_request_t);
