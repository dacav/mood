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

void moodgoph_request_delete(moodgoph_request_t);
