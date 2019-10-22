#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <err.h>

#include "util.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

void
init_xcb(xcb_connection_t **con) {
    *con = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(*con))
        errx(1, "unable connect to the X server");
}

void
kill_xcb(xcb_connection_t **con) {
    if (*con)
        xcb_disconnect(*con);
}

int
get_randr_monitors (xcb_connection_t *conn, monitor_t **monitors) {
    int num_outputs = 0;

    xcb_randr_get_screen_resources_current_reply_t *rres_reply;
    xcb_randr_get_output_primary_reply_t *primary_monitor;
    xcb_randr_output_t *outputs;

    int i, j, num, valid = 0;
    xcb_screen_t *scr;

    // Grab infos from the first screen
    scr = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    rres_reply = xcb_randr_get_screen_resources_current_reply(
        conn,
        xcb_randr_get_screen_resources_current(conn, scr->root),
        NULL
    );

    if (!rres_reply) {
        fprintf(stderr, "Failed to get current randr screen resources\n");
        return num_outputs;
    }

    num = xcb_randr_get_screen_resources_current_outputs_length(rres_reply);
    outputs = xcb_randr_get_screen_resources_current_outputs(rres_reply);

    // There should be at least one output
    if (num < 1) {
        free(rres_reply);
        return num_outputs;
    }

    monitor_t mons[num];

    // get the primary monitor, if available
    primary_monitor = xcb_randr_get_output_primary_reply (
        conn,
        xcb_randr_get_output_primary(conn, scr->root),
        NULL
    );

    // Get all outputs
    for (i = 0; i < num; i++) {
        xcb_randr_get_output_info_reply_t *oi_reply;
        xcb_randr_get_crtc_info_reply_t *ci_reply;

        oi_reply = xcb_randr_get_output_info_reply(
            conn,
            xcb_randr_get_output_info(conn, outputs[i], XCB_CURRENT_TIME),
            NULL
        );

        // Output disconnected?
        if (!oi_reply || oi_reply->connection != XCB_RANDR_CONNECTION_CONNECTED) {
            free(oi_reply);
            mons[i].width = 0;
            mons[i].connected = 0;
            mons[i].active = 0;
            continue;
        }

        int namelen;
        uint8_t *str;
        char *name;

        namelen = xcb_randr_get_output_info_name_length(oi_reply);
        name = malloc(namelen+1);

        if (!name) {
            fprintf(stderr, "Failed to allocate randr output name\n");
            exit(EXIT_FAILURE);
        }

        str = xcb_randr_get_output_info_name(oi_reply);
        memcpy(name, str, namelen);
        name[namelen] = '\0';

        ci_reply = xcb_randr_get_crtc_info_reply(
            conn,
            xcb_randr_get_crtc_info(conn, oi_reply->crtc, XCB_CURRENT_TIME),
            NULL
        );

        // If output is active (connected to crtc)
        if (ci_reply) {
            int is_primary = 0;

            if (primary_monitor != NULL) {
                is_primary = (int) primary_monitor->output == outputs[i];
            }

            // TODO: Avoid the boolean trap
            mons[i] = (monitor_t) {
                name,
                ci_reply->x,
                ci_reply->y,
                ci_reply->width,
                ci_reply->height,
                1,
                1,
                is_primary
            };
        }
        else {
            // Otherwise mark it as 'not active'
            // TODO: Avoid the boolean trap
            mons[i] = (monitor_t) { name, 0, 0, 0, 0, 0, 1, 0 };
        }

        free(oi_reply);
        free(ci_reply);

        valid++;
    }

    free(rres_reply);
    free(primary_monitor);

    // Check for clones and inactive outputs
    for (i = 0; i < num; i++) {
        if (mons[i].width == 0)
            continue;

        for (j = 0; j < num; j++) {
            // Does I contain J ?

            if (i != j && mons[j].width) {
                if (mons[j].x >= mons[i].x && mons[j].x + mons[j].width <= mons[i].x + mons[i].width &&
                    mons[j].y >= mons[i].y && mons[j].y + mons[j].height <= mons[i].y + mons[i].height) {
                    mons[j].width = 0;
                    valid--;
                }
            }
        }
    }

    num_outputs = valid;

    if (valid < 1) {
        fprintf(stderr, "No usable RandR output found\n");
        return num_outputs;
    }

    *monitors = (monitor_t *) malloc(sizeof(monitor_t) * valid);

    if (*monitors == NULL) {
        fprintf(stderr, "Failed to allocate monitors array\n");
        exit(EXIT_FAILURE);
    }

    for (i = j = 0; i < num && j < valid; i++) {
        if (mons[i].connected == 1) {
            (*monitors)[j++] = mons[i];
        }
    }

    return num_outputs;
}

monitor_t
get_monitor(xcb_connection_t *conn, char str[]) {

    xcb_window_t win = 0;
    monitor_t monitor;

    if (startsWith("0x", str)) {
        win = strtoul(str, NULL, 16);
        monitor = get_monitor_by_window_id(conn, win);
    }
    else {
        monitor = get_monitor_by_name(conn, str);
    }

    return monitor;
}

monitor_t
get_monitor_by_window_id(xcb_connection_t *conn, xcb_window_t pfw) {

    monitor_t *monitors;
    int num_monitors = get_randr_monitors(conn, &monitors);

    xcb_get_geometry_reply_t *w;
    monitor_t m;
    monitor_t current_monitor;

    unsigned int intersect, current_intersect;
    int top, bottom, left, right;

    w = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, pfw), NULL);

    // TODO: Avoid the boolean trap
    current_monitor = (monitor_t) { NULL, 0, 0, 0, 0, 0, 0, 0 };
    current_intersect = 0;

    int inactive_monitors = 0;

    for (int i=0; i<num_monitors; i++) {
        m = monitors[i];

        if (!m.active) {
            inactive_monitors++;
            continue;
        }

        top = max(w->y, m.y);
        left = max(w->x, m.x);
        bottom = min(w->y + w->height, m.y + m.height);
        right = min(w->x + w->width, m.x + m.width);

        if (bottom >= top && right >= left) {
            intersect = (bottom - top) * (right - left);
            if (intersect >= current_intersect) {
                // set the monitor num to match xrandr screen number. the count
                // excludes inactive monitors
                m.num = i - inactive_monitors;
                current_intersect = intersect;
                current_monitor = m;
            }
        }
    }

    return current_monitor;
}

monitor_t
get_monitor_by_name(xcb_connection_t *conn, char *name) {

    monitor_t *monitors;
    int num_monitors = get_randr_monitors(conn, &monitors);

    monitor_t m;

    int inactive_monitors = 0;
    for (int i=0; i<num_monitors; i++) {
        m = monitors[i];

        if (!m.active) {
            inactive_monitors++;
            continue;
        }

        if (strcmp(m.name, name) == 0) {
            // set the monitor num to match xrandr screen number. the count
            // excludes inactive monitors
            m.num = i - inactive_monitors;
            return m;
        }
    }

    // TODO: Avoid the boolean trap
    return (monitor_t) { NULL, 0, 0, 0, 0, 0, 0, 0, 0 };
}

xcb_window_t
get_focused_win(xcb_connection_t *conn) {
    xcb_window_t w = 0;

    xcb_get_input_focus_cookie_t  ct;
    xcb_get_input_focus_reply_t  *r;

    ct = xcb_get_input_focus(conn);
    r = xcb_get_input_focus_reply(conn, ct, NULL);

    if (r == NULL)
        errx(1, "failed to get focused window");

    w = r->focus;
    free(r);
    return w;
}

int
startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

