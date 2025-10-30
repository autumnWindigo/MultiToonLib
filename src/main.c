#include "multitoonlib.h"
#include <X11/X.h>
#include <stdlib.h>
#include <xdo.h>

typedef struct {
        xdo_t *xdo;
} linux_data_t;

int main(int argc, char *argv[])
{
        char *key = "a";

        mtlib_session_t *x = mtlib_init();
        linux_data_t *linux_data = (linux_data_t *)x->platform_data;

        Window w = mtlib_select_window(x);

        mtlib_set_key_down(x, w, key);

        // Hold for a moment
        sleep(1);

        // Send key up
        mtlib_set_key_up(x, w, key);

        if (linux_data->xdo)
                xdo_free(linux_data->xdo);
        free(linux_data);
        free(x);

        return 0;
}

