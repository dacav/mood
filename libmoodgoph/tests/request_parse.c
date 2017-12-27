#include <libmoodgoph/request.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_only_separators();
static void test_only_separators_no_nil();
static void test_empty_query();
static void test_empty_tail();

int main(int argc, char** argv)
{
    test_only_separators();
    test_only_separators_no_nil();
    test_empty_query();
    test_empty_tail();
    return 0;
}

static void test_only_separators()
{
    char test_string[20] = "//////";
    moodgoph_request_t request = moodgoph_request_new(test_string, sizeof(test_string));
    assert(moodgoph_request_next_token(request) == NULL);
    moodgoph_request_delete(request);
}

static void test_only_separators_no_nil()
{
    const char* token;
    moodgoph_request_t request;

    {
        char test_string[7] = {'a', 'b', 'c', '/', 'd', 'e', '\0'};
        request = moodgoph_request_new(test_string, 7);
        token = moodgoph_request_next_token(request);
        assert(strcmp(token, "abc") == 0);
        token = moodgoph_request_next_token(request);
        assert(strcmp(token, "de") == 0);
        token = moodgoph_request_next_token(request);
        assert(token == NULL);
        moodgoph_request_delete(request);
    }
    {
        char test_string[7] = {'a', 'b', 'c', '/', 'd', 'e', '\0'};
        request = moodgoph_request_new(test_string, 6);
        token = moodgoph_request_next_token(request);
        assert(strcmp(token, "abc") == 0);
        token = moodgoph_request_next_token(request);
        assert(strcmp(token, "d") == 0);
        token = moodgoph_request_next_token(request);
        assert(token == NULL);
        moodgoph_request_delete(request);
    }
}

static void test_empty_query()
{
    char nothing = '\0';
    moodgoph_request_t request = moodgoph_request_new(&nothing, sizeof(char));
    moodgoph_request_delete(request);
}

static void test_empty_tail()
{
    char text[] = "/foo/bar////";
    const char* token;

    moodgoph_request_t request = moodgoph_request_new(text, sizeof(text));

    token = moodgoph_request_next_token(request);
    assert(strcmp(token, "foo") == 0);

    token = moodgoph_request_next_token(request);
    assert(strcmp(token, "bar") == 0);

    token = moodgoph_request_next_token(request);
    assert(token == NULL);

    moodgoph_request_delete(request);
}
