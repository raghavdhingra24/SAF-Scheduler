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

/* current is non-NULL exactly while some thread is RUNNING (I10).
 * sched_ctx is the hub every thread switches through (I1); it exists
 * only for the duration of one scheduler_run() call. */
extern TCB *current;
extern ucontext_t sched_ctx;

extern long g_switches;

#ifndef NDEBUG
/* Debug-only instrumentation used by tests/test_coop.c. */
extern long g_tcb_destroyed;
saf_state_t saf_debug_current_state(void);
#endif

#endif
