#pragma once

#include <signal.h>
#include <event2/event.h>

typedef struct sigwrap* sigwrap_t;

typedef void (*sigwrap_cb_t)(int signal, void* user_context);

struct sigwrap_setting
{
    int signal;
    sigwrap_cb_t callback;
    void* user_context;
};

/*
 * param event_base: the base libevent object
 * param sigwrap_setting: array of signals to register. Must be terminated
 *                        with a guard object having callback=NULL
 */
sigwrap_t sigwrap_new(struct event_base *event_base,
                      const struct sigwrap_setting*);

void sigwrap_del(sigwrap_t);
