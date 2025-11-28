#include <X11/X.h>
#include <X11/Xlib.h>
#include <mtlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <uthash.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xdo.h>

#define PID_PROP "_NET_WM_PID"

typedef struct {
        xdo_t *xdo;
} linux_data_t;

static mtlib_iset_t linux_iset;
static const char *mtlib_normalize_key_linux(const char *key);
static Window find_window_with_pid(Display *dpy, Window root, int pid);

static mtlib_session_t *linux_init(void)
{
        mtlib_session_t *session = malloc(sizeof(mtlib_session_t));
        linux_data_t *linux_data = malloc(sizeof(linux_data_t));
        linux_data->xdo = xdo_new(NULL);
        session->platform_data = linux_data;
        session->iset = &linux_iset;
        return session;
}

static void linux_shutdown(mtlib_session_t *session)
{
        if (!session)
                return;
        linux_data_t *linux_data = (linux_data_t *)session->platform_data;
        if (linux_data->xdo)
                xdo_free(linux_data->xdo);
        free(linux_data);
        free(session);
}

static Window linux_select_window(mtlib_session_t *session)
{
        if (!session)
                return 0;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        Window w = 0;
        int ret = xdo_select_window_with_click(ldata->xdo, &w);
        if (ret != XDO_SUCCESS) {
                return 0;
        }

        return w;
}

static void linux_set_key_down(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session || !key)
                return;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        const char *x_key = mtlib_normalize_key_linux(key);
        if (x_key == NULL)
                return;
        xdo_send_keysequence_window_down(ldata->xdo, (Window)w, x_key, 0);
}

static void linux_set_key_up(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session || !key)
                return;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        const char *x_key = mtlib_normalize_key_linux(key);
        if (x_key == NULL)
                return;
        xdo_send_keysequence_window_up(ldata->xdo, (Window)w, x_key, 0);
}

static void linux_send_key(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session)
                return;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        xdo_send_keysequence_window(ldata->xdo, (Window)w, key, 0);
}

static Window linux_get_window_from_pid(mtlib_session_t *session, int pid)
{
        if (!session)
                return -1;

        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        Display *dpy = ldata->xdo->xdpy;

        Window root = DefaultRootWindow(dpy);
        Window w = find_window_with_pid(dpy, root, pid);

        return w;
}

static mtlib_iset_t linux_iset = {
    .init = linux_init,
    .shutdown = linux_shutdown,
    .select_window = linux_select_window,
    .set_key_down = linux_set_key_down,
    .set_key_up = linux_set_key_up,
    .send_key = linux_send_key,
    .get_window_from_pid = linux_get_window_from_pid,
};

mtlib_iset_t *mtlib_linux_get_iset(void) { return &linux_iset; }

static Window find_window_with_pid(Display *dpy, Window root, int pid)
{
        Atom atom = XInternAtom(dpy, PID_PROP, True);
        if (!atom)
                return 0;

        Window parent;
        Window *children;
        unsigned int nchildren;

        if (!XQueryTree(dpy, root, &root, &parent, &children, &nchildren))
                return 0;

        for (unsigned int i = 0; i < nchildren; i++) {
                Atom actual;
                int format;
                unsigned long nitems, bytes_after;
                unsigned char *prop = 0;

                int status = XGetWindowProperty(
                    dpy, children[i], atom, 0, 1, False, AnyPropertyType,
                    &actual, &format, &nitems, &bytes_after, &prop);

                if (status == Success && prop != 0) {
                        unsigned long win_pid = *(unsigned long *)prop;
                        XFree(prop);

                        if (win_pid == pid) {
                                Window result = children[i];
                                XFree(children);
                                return (Window)result;
                        }
                }

                Window child_result =
                    find_window_with_pid(dpy, children[i], pid);
                if (child_result != 0) {
                        XFree(children);
                        return (Window)child_result;
                }
        }

        if (children)
                XFree(children);

        return 0;
}

typedef struct {
        const char *keysym;
        const char *panda3d;
} KeyMapEntry;

static const KeyMapEntry keyMap[] = {
    {"Left", "arrow_left"},
    {"Up", "arrow_up"},
    {"Down", "arrow_down"},
    {"Right", "arrow_right"},

    {"Shift_L", "shift"},
    {"Shift_R", "shift"},
    {"Control_L", "control"},
    {"Control_R", "control"},
    {"Alt_L", "alt"},
    {"Alt_R", "alt"},
    {"Caps_Lock", "caps_lock"},
    {"Num_Lock", "num_lock"},

    {"space", "space"},
    {"Space", "space"},

    {"BackSpace", "backspace"},
    {"Tab", "tab"},
    {"Return", "enter"},
    {"Enter", "enter"},
    {"Delete", "delete"},
    {"Insert", "insert"},
    {"Home", "home"},
    {"End", "end"},
    {"Page_Up", "page_up"},
    {"Page_Down", "page_down"},
    {"Escape", "escape"},
    {"Scroll_Lock", "scroll_lock"},
    {"Print", "print_screen"},

    {"F1", "f1"},
    {"F2", "f2"},
    {"F3", "f3"},
    {"F4", "f4"},
    {"F5", "f5"},
    {"F6", "f6"},
    {"F7", "f7"},
    {"F8", "f8"},
    {"F9", "f9"},
    {"F10", "f10"},
    {"F11", "f11"},
    {"F12", "f12"},
};

static const size_t keyMapSize = sizeof(keyMap) / sizeof(keyMap[0]);

static const char *mtlib_normalize_key_linux(const char *panda3d)
{
        if (strlen(panda3d) == 1)
                return panda3d;
        for (size_t i = 0; i < keyMapSize; i++) {
                if (strcmp(keyMap[i].panda3d, panda3d) == 0) {
                        return keyMap[i].keysym;
                }
        }
        return NULL;
}

