#include "multitoonlib.h"
#include <X11/X.h>
#include <stdio.h>


int main(void) {
        printf("Starting Main\n");
        mtlib_session_t *session = mtlib_init();
        if (!session) {
                fprintf(stderr, "Failed to initialize mt_lib\n");
        }

        printf("Click on xwayland window");

        Window w = session->iset->select_window(session);

        if (w == 0) {
                printf("window select failed\n");
        } else {
                printf("window selected: 0x%lx\n", w);
        }

        mtlib_shutdown(session);

        return 0;
}

