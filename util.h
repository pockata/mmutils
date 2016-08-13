typedef struct monitor_t {
    char *name;
    int x, y, width, height;
} monitor_t;

xcb_visualid_t get_visual (void);

void init_xcb(xcb_connection_t **);

void kill_xcb(xcb_connection_t **);

int startsWith(const char *, const char *);

int get_randr_monitors (xcb_connection_t *, monitor_t **);

monitor_t get_monitor_by_window_id(xcb_connection_t *, xcb_window_t pfw);

monitor_t get_monitor_by_name(xcb_connection_t *, char *name);

xcb_window_t get_focused_win(xcb_connection_t *);

