#include "sigwrap.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct sigwrap
{
    struct event* event;
    sigwrap_t next;
    struct sigwrap_setting setting;
};

static int set_event(struct event_base* event_base,
                     sigwrap_t sigwrap,
                     int signal);
static void handler(int fd, short ev, void* arg);

sigwrap_t sigwrap_new(struct event_base *event_base,
                      const struct sigwrap_setting* setting)
{
    errno = 0;
    if (!setting->callback) {
        errno = EINVAL;
        return NULL;
    }

    sigwrap_t sigwrap = calloc(1, sizeof(struct sigwrap));
    if (sigwrap == NULL) {
        perror("calloc");
        return NULL;
    }

    for (;;) {
        if (set_event(event_base, sigwrap, setting->signal) == -1) {
            sigwrap_del(sigwrap);
            return NULL;
        }
        memcpy(&sigwrap->setting, setting, sizeof(struct sigwrap_setting));

        setting ++;
        if (!setting->callback) break;  /* Guard object reached */

        sigwrap_t prev = calloc(1, sizeof(struct sigwrap));
        if (!prev) {
            perror("calloc");
            sigwrap_del(sigwrap);
            return NULL;
        }

        prev->next = sigwrap;
        sigwrap = prev;
    }

    return sigwrap;
}

void sigwrap_del(sigwrap_t sigwrap)
{
    while (sigwrap) {
        event_free(sigwrap->event);
        sigwrap_t next = sigwrap->next;
        free(sigwrap);
        sigwrap = next;
    }
}

static int set_event(struct event_base* event_base,
                     sigwrap_t sigwrap,
                     int signal)
{
    errno = 0;
    struct event* event = evsignal_new(event_base, signal, handler, sigwrap);
    if (event == NULL) {
        perror("evsigwrap_new");
        return -1;
    }
    errno = 0;
    if (evsignal_add(event, NULL) == -1) {
        perror("evsigwrap_add");
        evsignal_del(event);
        return -1;
    }
    sigwrap->event = event;
    return 0;
}

static void handler(int fd, short ev, void* arg)
{
    sigwrap_t sigwrap = (sigwrap_t)arg;
    sigwrap->setting.callback(
        sigwrap->setting.signal,
        sigwrap->setting.user_context
    );
}
