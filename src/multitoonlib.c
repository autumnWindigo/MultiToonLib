#include <X11/Xlib.h>
#include <mtlib_linux.h>
#include <multitoonlib.h>
#include <string.h>
#include <xdo.h>

extern mtlib_iset_t linux_iset;
extern mtlib_iset_t windows_iset;
extern mtlib_iset_t mac_iset;

mtlib_session_t *mtlib_init(void)
{
#if defined(__linux__)
        return mtlib_linux_get_iset()->init();
#elif defined(_WIN32)
        return windows_iset.init();
#elif defined(__APPLE__)
        return mac_iset.init();
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

void mtlib_set_key_down(mtlib_session_t *session, Window w, char *key)
{
        if (session && session->iset && session->iset->set_key_down)
                session->iset->set_key_down(session, w, key);
}

void mtlib_set_key_up(mtlib_session_t *session, Window w, char *key)
{
        if (session && session->iset && session->iset->set_key_up)
                session->iset->set_key_up(session, w, key);
}

void mtlib_send_key(mtlib_session_t *session, Window w, char *key)
{
        if (session && session->iset && session->iset->send_key)
                session->iset->send_key(session, w, key);
}

const char *mtlib_normalize_key(const char *key)
{
#ifdef __linux__
        if (strcmp(key, "arrow_up") == 0) return "Up";
        if (strcmp(key, "arrow_down") == 0) return "Down";
        if (strcmp(key, "arrow_left") == 0) return "Left";
        if (strcmp(key, "arrow_right") == 0) return "Right";

#endif

        return key;
}

