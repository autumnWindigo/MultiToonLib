#include "mtlib.h"
#include <Carbon/Carbon.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFCGTypes.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFMachPort.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreGraphics/CGEvent.h>
#include <CoreGraphics/CGEventSource.h>
#include <CoreGraphics/CGEventTypes.h>
#include <CoreGraphics/CGRemoteOperation.h>
#include <CoreGraphics/CGWindow.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_pid_t.h>
#include <unistd.h>

#define MAX_SHORT 65535

typedef struct {
        CGEventSourceRef source;
} macos_data_t;

typedef struct {
        const char *panda_key;
        CGKeyCode mac_key;
} PandaToMacKey;

static mtlib_iset_t macos_iset;
static CGKeyCode panda_to_macos_key(const char *key);
static volatile bool mouseClicked = false;
CGPoint click_location = {0, 0};

static pid_t get_foremost_window_pid(void);

static mtlib_session_t *macos_init(void)
{
        mtlib_session_t *session = malloc(sizeof(mtlib_session_t));
        macos_data_t *mdata = malloc(sizeof(macos_data_t));
        mdata->source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
        session->platform_data = mdata;
        session->iset = &macos_iset;
        return session;
}

static void macos_shutdown(mtlib_session_t *session)
{
        if (!session)
                return;
        macos_data_t *mdata = (macos_data_t *)session->platform_data;
        if (mdata)
                free(mdata);
        free(session);
}

static void macos_send_key(mtlib_session_t *session, uint64_t pid, char *key)
{
        macos_data_t *mdata = (macos_data_t *)session->platform_data;
        CGKeyCode vK = panda_to_macos_key(key);
        if (vK == MAX_SHORT)
                return;
        CGEventRef key_down =
            CGEventCreateKeyboardEvent(mdata->source, vK, true);
        CGEventRef key_up =
            CGEventCreateKeyboardEvent(mdata->source, vK, false);
        CGEventPostToPid((pid_t)pid, key_down);
        usleep(15000);
        CGEventPostToPid((pid_t)pid, key_up);
}

static void macos_set_key_down(mtlib_session_t *session, uint64_t pid,
                               char *key)
{
        macos_data_t *mdata = (macos_data_t *)session->platform_data;
        CGKeyCode vK = panda_to_macos_key(key);
        if (vK == MAX_SHORT)
                return;
        CGEventRef key_down =
            CGEventCreateKeyboardEvent(mdata->source, vK, true);
        CGEventPostToPid((pid_t)pid, key_down);
}

static void macos_set_key_up(mtlib_session_t *session, uint64_t pid, char *key)
{
        macos_data_t *mdata = (macos_data_t *)session->platform_data;
        CGKeyCode vK = panda_to_macos_key(key);
        if (vK == MAX_SHORT)
                return;
        CGEventRef key_up =
            CGEventCreateKeyboardEvent(mdata->source, vK, false);
        CGEventPostToPid((pid_t)pid, key_up);
        }

CGEventRef left_click_callback(CGEventTapProxy proxy, CGEventType type,
                               CGEventRef event, void *refcon)
{
        if (type != kCGEventLeftMouseDown) {
                return event;
        }
        click_location = CGEventGetLocation(event);
        mouseClicked = true;
        return event;
}

static uint64_t macos_select_window(mtlib_session_t *session)
{
        macos_data_t *mdata = (macos_data_t *)session->platform_data;

        CFMachPortRef event_tap = CGEventTapCreate(
            kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
            (1 << kCGEventLeftMouseDown), left_click_callback, NULL);

        if (!event_tap)
                return 0;

        CFRunLoopSourceRef loop_src =
            CFMachPortCreateRunLoopSource(kCFAllocatorDefault, event_tap, 0);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), loop_src,
                           kCFRunLoopCommonModes);

        CGEventTapEnable(event_tap, true);

        while (!mouseClicked) {
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
        }

        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), loop_src,
                              kCFRunLoopCommonModes);
        CFRelease(loop_src);
        CFRelease(event_tap);

        pid_t pid = get_foremost_window_pid();

        return (uint64_t)pid;
}

static pid_t get_pid_from_CGPoint(CGPoint *p) {
        CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        if (!window_list)
                return 0;
        pid_t pid = 0;

        for (CFIndex i = 0; i < CFArrayGetCount(window_list); i++) {

        }

        CFRelease(window_list);
        return pid;
}

static pid_t get_foremost_window_pid(void)
{
        CFArrayRef window_list = CGWindowListCopyWindowInfo(
            kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        if (!window_list)
                return 0;
        pid_t pid = 0;

        for (CFIndex i = 0; i < CFArrayGetCount(window_list); i++) {
                CFDictionaryRef window_info =
                    CFArrayGetValueAtIndex(window_list, i);
                CFNumberRef is_window_on_top =
                    CFDictionaryGetValue(window_info, kCGWindowLayer);

                if (is_window_on_top) {
                        CFNumberRef pid_temp = CFDictionaryGetValue(
                            window_info, kCGWindowOwnerPID);
                        if (pid_temp) {
                                CFNumberGetValue(pid_temp, kCFNumberSInt32Type,
                                                 &pid);
                                break;
                        }
                }
        }

        CFRelease(window_list);
        return pid;
}

static mtlib_iset_t macos_iset = {
    .init = macos_init,
    .shutdown = macos_shutdown,
    .select_window = macos_select_window,
    .set_key_down = macos_set_key_down,
    .set_key_up = macos_set_key_up,
    .send_key = macos_send_key,
};

mtlib_iset_t *mtlib_macos_get_iset(void) { return &macos_iset; }

static PandaToMacKey panda_to_mac_key_map[] = {
    {"arrow_up", kVK_UpArrow},
    {"arrow_down", kVK_DownArrow},
    {"arrow_left", kVK_LeftArrow},
    {"arrow_right", kVK_RightArrow},

    {"a", kVK_ANSI_A},
    {"b", kVK_ANSI_B},
    {"c", kVK_ANSI_C},
    {"d", kVK_ANSI_D},
    {"e", kVK_ANSI_E},
    {"f", kVK_ANSI_F},
    {"g", kVK_ANSI_G},
    {"h", kVK_ANSI_H},
    {"i", kVK_ANSI_I},
    {"j", kVK_ANSI_J},
    {"k", kVK_ANSI_K},
    {"l", kVK_ANSI_L},
    {"m", kVK_ANSI_M},
    {"n", kVK_ANSI_N},
    {"o", kVK_ANSI_O},
    {"p", kVK_ANSI_P},
    {"q", kVK_ANSI_Q},
    {"r", kVK_ANSI_R},
    {"s", kVK_ANSI_S},
    {"t", kVK_ANSI_T},
    {"u", kVK_ANSI_U},
    {"v", kVK_ANSI_V},
    {"w", kVK_ANSI_W},
    {"x", kVK_ANSI_X},
    {"y", kVK_ANSI_Y},
    {"z", kVK_ANSI_Z},

    {"1", kVK_ANSI_1},
    {"2", kVK_ANSI_2},
    {"3", kVK_ANSI_3},
    {"4", kVK_ANSI_4},
    {"5", kVK_ANSI_5},
    {"6", kVK_ANSI_6},
    {"7", kVK_ANSI_7},
    {"8", kVK_ANSI_8},
    {"9", kVK_ANSI_9},
    {"0", kVK_ANSI_0},

    {"space", kVK_Space},
    {"enter", kVK_Return},
    {"escape", kVK_Escape},
    {"tab", kVK_Tab},
    {"shift", kVK_Shift},
    {"control", kVK_Control},
    {"option", kVK_Option},
    {"command", kVK_Command},
    {"home", kVK_Home},
    {"end", kVK_End},
    {"page_up", kVK_PageUp},
    {"page_down", kVK_PageDown},
    {"backspace", kVK_Delete},
    {"delete", kVK_ForwardDelete},
    {"capslock", kVK_CapsLock},

    {"f1", kVK_F1},
    {"f2", kVK_F2},
    {"f3", kVK_F3},
    {"f4", kVK_F4},
    {"f5", kVK_F5},
    {"f6", kVK_F6},
    {"f7", kVK_F7},
    {"f8", kVK_F8},
    {"f9", kVK_F9},
    {"f10", kVK_F10},
    {"f11", kVK_F11},
    {"f12", kVK_F12},
};

static CGKeyCode panda_to_macos_key(const char *key)
{
        for (int i = 0;
             i < (sizeof(panda_to_mac_key_map) / sizeof(PandaToMacKey)); i++) {
                if (strcmp(panda_to_mac_key_map[i].panda_key, key) == 0)
                        return panda_to_mac_key_map[i].mac_key;
        }
        return MAX_SHORT;
};

