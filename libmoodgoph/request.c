#include <libmoodgoph/request.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char* SEPARATORS = "/";

struct moodgoph_request
{
    char* seek_start;
    char* next;
};

static size_t sanitize_string(char* str, size_t len);
static void goto_next(moodgoph_request_t request);

moodgoph_request_t moodgoph_request_new(char* query, size_t querylen)
{
    moodgoph_request_t request = malloc(sizeof(struct moodgoph_request));
    if (request == NULL) {
        return NULL;
    }

    (void)sanitize_string(query, querylen);
    request->seek_start = query;
    goto_next(request);

    return request;
}

const char* moodgoph_request_next_token(moodgoph_request_t request)
{
    const char* next = request->next;
    if (next) {
        goto_next(request);
    }
    return next;
}

void moodgoph_request_delete(moodgoph_request_t request)
{
    free(request);
}

/* Given a string and its length, make sure that it ends with '\0', possibly
 * forcing it as last byte (and overwriting its value).
 *
 * Returns the length of the string, excluding the '\0' terminator, as
 * strlen() would.
 */
static size_t sanitize_string(char* str, size_t len)
{
    /* This could be implemented with strnlen, as soon as the compiler will
     * have it without special compilation flags. */
    for (size_t i = 0; i < len; i ++) {
        if (str[i] == '\0') {
            return i;
        }
    }
    str[len - 1] = '\0';
    return len - 1;
}

static void goto_next(moodgoph_request_t request)
{
    request->next = NULL;
    if (request->seek_start == NULL) {
        /* This request was completely analysed */
        return;
    }

    const size_t skip = strspn(request->seek_start, SEPARATORS);
    if (request->seek_start[skip] == '\0') {
        /* We reached the end of the query without finding the next token. */
        return;
    }

    char* start = request->seek_start + skip;
    const size_t token_len = strcspn(start, SEPARATORS);

    /* The token length must be positive, or we would have reached the end
     * of the string when counting how much to skip. */
    assert(token_len > 0);

    if (start[token_len] == '\0') {
        /* This is the last token before the end of the string. We are done. */
        request->seek_start = NULL;
    }
    else {
        start[token_len] = '\0';
        request->seek_start = start + token_len + 1;
    }
    request->next = start;
}
