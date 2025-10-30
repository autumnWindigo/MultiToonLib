#ifndef MULTITOONLIB_H
#define MULTITOONLIB_H

#include <X11/X.h>
#include <stdint.h>
#include <xdo.h>
typedef uint64_t mt_window_t;

typedef struct mtlib_session mtlib_session_t;
typedef struct mtlib_iset mtlib_iset_t;

struct mtlib_iset {
        mtlib_session_t *(*init)(void);
        void (*shutdown)(mtlib_session_t *);
        uint64_t (*select_window)(mtlib_session_t *);
        void (*set_key_down)(mtlib_session_t *, Window w, char *key);
        void (*set_key_up)(mtlib_session_t *, Window w, char *key);
        void (*send_key)(mtlib_session_t *, Window w, char *key);
};

struct mtlib_session {
        void *platform_data;
        mtlib_iset_t *iset;
};

mtlib_session_t *mtlib_init(void);
void mtlib_shutdown(mtlib_session_t *session);
uint64_t mtlib_select_window(mtlib_session_t *session);
void mtlib_set_key_down(mtlib_session_t *session, Window w, char *key);
void mtlib_set_key_up(mtlib_session_t *session, Window w, char *key);
void mtlib_send_key(mtlib_session_t *session, Window w, char *key);

const char *mtlib_normalize_key(const char *key);
#endif
