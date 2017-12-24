#pragma once

#include <event2/event.h>

typedef struct server* server_t;

typedef void (*server_accepted_cb_t)(server_t, int clientfd);

struct server_params
{
    struct event_base* event_base;
    int socket;
    server_accepted_cb_t on_accepted;
    void *user_context;
};

server_t server_new(const struct server_params* params);
void server_pause_accepting(server_t);
void server_resume_accepting(server_t);
void* server_get_context(server_t);
void server_delete(server_t);
