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

#include <libmoodio/sigwrap.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct moodio_sigwrap
{
    struct event* event;
    moodio_sigwrap_t next;
    struct moodio_sigwrap_setting setting;
};

static int set_event(struct event_base* event_base,
                     moodio_sigwrap_t sigwrap,
                     int signal);
static void handler(int fd, short ev, void* arg);

moodio_sigwrap_t moodio_sigwrap_new(struct event_base *event_base,
                                    const struct moodio_sigwrap_setting* setting)
{
    errno = 0;
    if (!setting->callback) {
        errno = EINVAL;
        return NULL;
    }

    moodio_sigwrap_t sigwrap = calloc(1, sizeof(struct moodio_sigwrap));
    if (sigwrap == NULL) {
        perror("calloc");
        return NULL;
    }

    for (;;) {
        if (set_event(event_base, sigwrap, setting->signal) == -1) {
            moodio_sigwrap_del(sigwrap);
            return NULL;
        }
        memcpy(&sigwrap->setting, setting, sizeof(struct moodio_sigwrap_setting));

        setting ++;
        if (!setting->callback) break;  /* Guard object reached */

        moodio_sigwrap_t prev = calloc(1, sizeof(struct moodio_sigwrap));
        if (!prev) {
            perror("calloc");
            moodio_sigwrap_del(sigwrap);
            return NULL;
        }

        prev->next = sigwrap;
        sigwrap = prev;
    }

    return sigwrap;
}

void moodio_sigwrap_del(moodio_sigwrap_t sigwrap)
{
    while (sigwrap) {
        event_free(sigwrap->event);
        moodio_sigwrap_t next = sigwrap->next;
        free(sigwrap);
        sigwrap = next;
    }
}

static int set_event(struct event_base* event_base,
                     moodio_sigwrap_t sigwrap,
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
    moodio_sigwrap_t sigwrap = (moodio_sigwrap_t)arg;
    sigwrap->setting.callback(
        sigwrap->setting.signal,
        sigwrap->setting.user_context
    );
}
