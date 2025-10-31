#include <X11/X.h>
#include <X11/Xlib.h>
#include <mtlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <xdo.h>

typedef struct {
        xdo_t *xdo;
} linux_data_t;

static mtlib_iset_t linux_iset;

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
        if (!session)
                return;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        xdo_send_keysequence_window_down(ldata->xdo, (Window)w, key, 0);
}

static void linux_set_key_up(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session)
                return;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        xdo_send_keysequence_window_up(ldata->xdo, (Window)w, key, 0);
}

static void linux_send_key(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session)
                return;
        linux_data_t *ldata = (linux_data_t *)session->platform_data;
        xdo_send_keysequence_window(ldata->xdo, (Window)w, key, 0);
}

static mtlib_iset_t linux_iset = {
    .init = linux_init,
    .shutdown = linux_shutdown,
    .select_window = linux_select_window,
    .set_key_down = linux_set_key_down,
    .set_key_up = linux_set_key_up,
    .send_key = linux_send_key,
};

mtlib_iset_t *mtlib_linux_get_iset(void) { return &linux_iset; }

const char *mtlib_normalize_key_linux(const char *key)
{
        if (strcmp(key, "arrow_up") == 0)
                return "Up";
        if (strcmp(key, "arrow_down") == 0)
                return "Down";
        if (strcmp(key, "arrow_left") == 0)
                return "Left";
        if (strcmp(key, "arrow_right") == 0)
                return "Right";
        return "";
}

