#ifndef SCHED_INTERNAL_H
#define SCHED_INTERNAL_H

#include <stddef.h>
#include <ucontext.h>

#include "tcb.h"

/* Full definition of the opaque TCB from tcb.h. Private to the library;
 * only tcb.c (and later timer.c) may include this header. */
struct TCB {
    int id;
    saf_state_t state;
    ucontext_t ctx;
    void (*fn)(void *);
    void *arg;
    char *map;
    size_t map_len;
    unsigned vg_id;
};

#endif
