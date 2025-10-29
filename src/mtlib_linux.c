#include <X11/X.h>
#include <multitoonlib.h>
#include <stdlib.h>
#include <xdo.h>

typedef struct {
        xdo_t *xdo;
} linux_data_t;

static mtlib_iset_t linux_iset;

static mtlib_session_t *linux_init(void) {
        mtlib_session_t *session = malloc(sizeof(mtlib_session_t));
        linux_data_t *linux_data = malloc(sizeof(linux_data_t));
        linux_data->xdo = xdo_new(NULL);
        session->platform_data = linux_data;
        session->iset = &linux_iset;
        return session;
}

static void linux_shutdown(mtlib_session_t *session) {
        if (!session) return;
        linux_data_t *linux_data = (linux_data_t*)session->platform_data;
        if (linux_data->xdo) xdo_free(linux_data->xdo);
        free(linux_data);
        free(session);
}

static uint64_t linux_select_window(mtlib_session_t *session) {
        if (!session) return 0;

        linux_data_t *ldata = (linux_data_t*)session->platform_data;
        xdo_t *xdo = ldata->xdo;

        Window w = 0;
        int ret = xdo_select_window_with_click(xdo, &w);

        if (ret != XDO_SUCCESS) {
                return 0;
        }

        return w;
}


static mtlib_iset_t linux_iset = {
        .init = linux_init,
        .shutdown = linux_shutdown,
        .select_window = linux_select_window,
};

mtlib_iset_t *mtlib_linux_get_iset(void) { return &linux_iset; }

