#include <multitoonlib.h>
#include <mtlib_linux.h>
#include <xdo.h>

extern mtlib_iset_t linux_iset;
extern mtlib_iset_t windows_iset;
extern mtlib_iset_t mac_iset;

mtlib_session_t *mtlib_init(void) {
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

void mtlib_shutdown(mtlib_session_t *session) {
        if (session && session->iset && session->iset->shutdown)
                session->iset->shutdown(session);
}

uint64_t mtlib_select_window(mtlib_session_t *session) {
        if (session && session->iset && session->iset->select_window)
                return session->iset->select_window(session);
        return 0;
}

