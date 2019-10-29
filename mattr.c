#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <err.h>

#include "util.h"

static xcb_connection_t *conn;

static void
usage(char *name) {
    fprintf(stderr, "usage: %s [-h] [-c] [nwhxyds] <wid|mid>\n", name);
    exit(1);
}

int
main (int argc, char *argv[]) {
    int cnt, ret = 0;
    size_t i;
    monitor_t monitor;

    if (argc < 2 || (strncmp(argv[1], "-h", 2) == 0)) {
        usage(argv[0]);

        goto end;
    }

    init_xcb(&conn);

    if (argc == 2) {
        monitor = get_monitor(conn, argv[1]);
        ret = monitor.active == 1 ? 0 : 1;

        goto end;
    }

    if (argc == 3 && strcmp(argv[1], "-c") == 0) {

        monitor = get_monitor(conn, argv[2]);
        ret = monitor.connected == 1 ? 0 : 1;

        goto end;
    }

    for (cnt=2; argv[cnt]; cnt++) {
        monitor = get_monitor(conn, argv[cnt]);

        // no monitor found
        if (monitor.name == NULL) {
            errx(1, "%s: no such monitor", argv[cnt]);
        }

        for (i=0; i<strlen(argv[1]); i++) {
            switch (argv[1][i]) {
                case 'i': /* wmutils compat */
                case 'n':
                    printf("%s", monitor.name);
                    break;
                case 'w':
                    printf("%d", monitor.width);
                    break;
                case 'h':
                    printf("%d", monitor.height);
                    break;
                case 'x':
                    printf("%d", monitor.x);
                    break;
                case 'y':
                    printf("%d", monitor.y);
                    break;
                case 'd':
                    printf("%d", monitor.num);
                    break;

                // Return the ID of the window/monitor that's passed as the
                // last argument. It's useful only if the argument is a window
                // ID so we can continue piping/chaining of commands. If it's a
                // monitor ID, it's identical to `i` and `n`.
                case 's':
                    printf("%s", argv[cnt]);
                    break;
                default: kill_xcb(&conn); usage(argv[0]);
            }
            putc(i+1 < strlen(argv[1]) ? ' ' : '\n', stdout);
        }
    }

end:
    kill_xcb(&conn);

    return ret;
}

