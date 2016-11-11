#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <xcb/xcb.h>

#include "util.h"

static xcb_connection_t *conn;

static void usage(char *);

enum {
    LIST_ACTIVE = 1 << 0,
    LIST_CONNECTED = 1 << 1
};

static void
usage(char *name) {
    fprintf(stderr, "usage: %s [-h] [-a]\n", name);
    exit(1);
}

int
main(int argc, char **argv) {
    monitor_t *monitors;
    monitor_t m;
    int num_monitors;
    int mask = 0;

    if (
        argc >= 2 &&
        (strncmp(argv[1], "-h", 2) == 0 ||
         strncmp(argv[1], "-a", 2) != 0)
    ) {
        usage(argv[0]);

        return 1;
    }

    init_xcb(&conn);

    num_monitors = get_randr_monitors(conn, &monitors);

    switch (argc) {
        case 1: mask |= LIST_ACTIVE; break;
        case 2: mask |= LIST_CONNECTED; break;
        default:
            kill_xcb(&conn);
            usage(argv[0]);
    }

    for (int i=0; i<num_monitors; i++) {
        m = monitors[i];
        if (
            (m.active == 1 && mask & LIST_ACTIVE) ||
            (m.connected == 1 && mask & LIST_CONNECTED)
        ) {
            printf("%s\n", m.name);
        }
    }

    kill_xcb(&conn);

    return 0;
}

