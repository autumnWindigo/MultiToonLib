#include "handleapi.h"
#include "minwindef.h"
#include "synchapi.h"
#include "windef.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <windows.h>

#include "mtlib.h"

typedef struct {
        HWND hwnd;
} windows_data_t;

static mtlib_iset_t windows_iset;

static HANDLE clickEvent = NULL;
static HWND selectedWindow = NULL;
static HHOOK mouseHook = NULL;

static WORD mtlib_normalize_key_windows(const char *key);

static mtlib_session_t *windows_init(void)
{
        mtlib_session_t *session = malloc(sizeof(mtlib_session_t));
        windows_data_t *wdata = malloc(sizeof(windows_data_t));
        session->platform_data = wdata;
        session->iset = &windows_iset;
        return session;
}

static void windows_shutdown(mtlib_session_t *session)
{
        if (!session)
                return;
        windows_data_t *wdata = (windows_data_t *)session->platform_data;
        free(wdata);
        free(session);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
        if (nCode >= 0 && wParam == WM_LBUTTONDOWN) {
                // Mouse was clicked
                MSLLHOOKSTRUCT *info = (MSLLHOOKSTRUCT *)lParam;
                POINT pt = info->pt;

                selectedWindow = WindowFromPoint(pt);

                SetEvent(clickEvent);

                UnhookWindowsHookEx(mouseHook);
                mouseHook = NULL;
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static uint64_t windows_select_window(mtlib_session_t *session)
{
        clickEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!clickEvent)
                return 0;

        mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
        if (!mouseHook) {
                CloseHandle(clickEvent);
                return 0;
        }

        WaitForSingleObject(clickEvent, INFINITE);

        CloseHandle(clickEvent);
        clickEvent = NULL;

        return (uint64_t)selectedWindow;
}

static void windows_set_key_down(mtlib_session_t *session, uint64_t w,
                                 char *key)
{
        if (!session || !w || !key)
                return;

        HWND hwnd = (HWND)w;
        WORD vk = mtlib_normalize_key_windows(key);
        if (!vk) {
                return;
        }
        PostMessage(hwnd, WM_KEYDOWN, vk, 0);
}

static void windows_set_key_up(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session || !w || !key)
                return;

        HWND hwnd = (HWND)w;
        WORD vk = mtlib_normalize_key_windows(key);
        if (vk == 0) {
                return;
        }
        PostMessage(hwnd, WM_KEYUP, vk, 0);
}

static void windows_send_key(mtlib_session_t *session, uint64_t w, char *key)
{
        if (!session || !w || !key)
                return;

        HWND hwnd = (HWND)w;
        WORD vk = mtlib_normalize_key_windows(key);
        if (!vk) {
                return;
        }

        PostMessage(hwnd, WM_KEYDOWN, vk, 0);

        // Reminder to myself this is in ms on windows
        Sleep(30);

        PostMessage(hwnd, WM_KEYUP, vk, 0);
}

static mtlib_iset_t windows_iset = {
    .init = windows_init,
    .shutdown = windows_shutdown,
    .select_window = windows_select_window,
    .set_key_down = windows_set_key_down,
    .set_key_up = windows_set_key_up,
    .send_key = windows_send_key,
};

mtlib_iset_t *mtlib_windows_get_iset(void) { return &windows_iset; }

static WORD mtlib_normalize_key_windows(const char *key)
{
        if (strcmp(key, "arrow_up") == 0)
                return VK_UP;
        if (strcmp(key, "arrow_down") == 0)
                return VK_DOWN;
        if (strcmp(key, "arrow_left") == 0)
                return VK_LEFT;
        if (strcmp(key, "arrow_right") == 0)
                return VK_RIGHT;
        if (strcmp(key, "space") == 0)
                return VK_SPACE;
        if (strcmp(key, "control") == 0)
                return VK_CONTROL;

        if (strlen(key) == 1)
                return VkKeyScanA(key[0]) & 0xFF;
        return 0;
}

