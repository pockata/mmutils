typedef struct monitor_t {
    char *name;
    int x, y, width, height, active, connected, primary;
    int num;
} monitor_t;

xcb_visualid_t get_visual (void);

void init_xcb(xcb_connection_t **);

void kill_xcb(xcb_connection_t **);

int startsWith(const char *, const char *);

int get_randr_monitors (xcb_connection_t *, monitor_t **);

monitor_t get_monitor(xcb_connection_t *, char *);

monitor_t get_monitor_by_window_id(xcb_connection_t *, xcb_window_t);

monitor_t get_monitor_by_name(xcb_connection_t *, char *);

xcb_window_t get_focused_win(xcb_connection_t *);

