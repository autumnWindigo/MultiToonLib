#include "mtlib_macos.h"
#include <X11/X.h>
#include <mtlib_linux.h>
#include <mtlib_windows.h>
#include <mtlib.h>
#include <stdint.h>
#include <sys/types.h>

extern mtlib_iset_t linux_iset;
extern mtlib_iset_t windows_iset;
extern mtlib_iset_t mac_iset;

mtlib_session_t *mtlib_init(void)
{
#if defined(_WIN32)
        return mtlib_windows_get_iset()->init();
#elif defined(__APPLE__)
        return mtlib_macos_get_iset()->init();
#elif defined(__linux__)
        return mtlib_linux_get_iset()->init();
#else
        return NULL;
#endif
}

void mtlib_shutdown(mtlib_session_t *session)
{
        if (session && session->iset && session->iset->shutdown)
                session->iset->shutdown(session);
}

uint64_t mtlib_select_window(mtlib_session_t *session)
{
        if (session && session->iset && session->iset->select_window)
                return session->iset->select_window(session);
        return 0;
}

void mtlib_set_key_down(mtlib_session_t *session, uint64_t w, char *key)
{
        if (session && session->iset && session->iset->set_key_down)
                session->iset->set_key_down(session, w, key);
}

void mtlib_set_key_up(mtlib_session_t *session, uint64_t w, char *key)
{
        if (session && session->iset && session->iset->set_key_up)
                session->iset->set_key_up(session, w, key);
}

void mtlib_send_key(mtlib_session_t *session, uint64_t w, char *key)
{
        if (session && session->iset && session->iset->send_key)
                session->iset->send_key(session, w, key);
}

uint64_t mtlib_get_window_from_pid(mtlib_session_t *session, int pid) {
        if (session && session->iset && session->iset->get_window_from_pid)
				return session->iset->get_window_from_pid(session, pid);
		return -1;
}

