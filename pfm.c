#include <stdio.h>
#include <string.h>
#include <err.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>

#include "util.h"

static xcb_connection_t *conn;
static xcb_screen_t *scr;

// from wmutils' util.c
void
get_screen(xcb_connection_t *con, xcb_screen_t **scr) {
    *scr = xcb_setup_roots_iterator(xcb_get_setup(con)).data;
    if (*scr == NULL)
        errx(1, "unable to retrieve screen informations");
}

void
get_cursor_position(xcb_connection_t *conn, uint32_t win, int *x, int *y) {

    xcb_query_pointer_reply_t *r;
    xcb_query_pointer_cookie_t c;

    c = xcb_query_pointer(conn, win);
    r = xcb_query_pointer_reply(conn, c, NULL);

    if (r == NULL)
        errx(1, "cannot retrieve pointer position");

    if (r->child != XCB_NONE) {
        *x = r->win_x;
        *y = r->win_y;
    }
    else {
        *x = r->root_x;
        *y = r->root_y;
    }
}

void
get_pointer_monitor(xcb_connection_t *conn, int x, int y) {

    monitor_t *monitors;
    monitor_t m;
    int num_monitors;

    num_monitors = get_randr_monitors(conn, &monitors);

    for (int i=0; i<num_monitors; i++) {
        m = monitors[i];
        if (
            x >= m.x && x <= m.x + m.width &&
            y >= m.y && y <= m.y + m.height
        ) {
            printf("%s\n", m.name);

            return;
        }
    }

    errx(1, "pointer not in visible monitor");
}

int
main(int argc, char **argv) {

    int x = 0;
    int y = 0;

    init_xcb(&conn);

    get_screen(conn, &scr);

    get_cursor_position(conn, scr->root, &x, &y);

    get_pointer_monitor(conn, x, y);

    kill_xcb(&conn);

    return 0;
}

