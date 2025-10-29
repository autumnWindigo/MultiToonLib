#ifndef MULTITOONLIB_H
#define MULTITOONLIB_H

#include <stdint.h>
#include <xdo.h>
typedef uint64_t mt_window_t;

typedef struct mtlib_session mtlib_session_t;
typedef struct mtlib_iset mtlib_iset_t;

struct mtlib_iset {
  mtlib_session_t *(*init)(void);
  void (*shutdown)(mtlib_session_t *);
  uint64_t (*select_window)(mtlib_session_t *);
};

struct mtlib_session {
  void *platform_data;
  mtlib_iset_t *iset;
};

mtlib_session_t *mtlib_init(void);
void mtlib_shutdown(mtlib_session_t *session);
uint64_t mtlib_select_window(mtlib_session_t *session);

#endif
