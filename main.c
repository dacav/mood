#include "setup.h"

#include <stdlib.h>
#include <errno.h>

int main (int argc, char **argv)
{
    struct setup_conf setup_conf = {
        .ip_bind_address = "::1",
        .tcp_port = 7070,
        .backlog = 3,
    };
    setup_t setup = setup_new(&setup_conf);
    if (!setup) {
        return EXIT_FAILURE;
    }

    errno = 0;
    if (event_base_dispatch(setup_get_event_base(setup)) == -1) {
        perror("event_base_dispatch");
        return EXIT_FAILURE;
    }

    setup_del(setup);

    return EXIT_SUCCESS;
}

