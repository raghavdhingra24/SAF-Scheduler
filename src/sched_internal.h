#ifndef SCHED_INTERNAL_H
#define SCHED_INTERNAL_H

#include <signal.h>
#include <stddef.h>
#include <ucontext.h>

#include "tcb.h"

/* Feedback-based local scheduling: a thread is classified LIGHT or HEAVY
 * based on how it was last switched out (voluntary yield vs forced timer
 * preemption). Used only to pick which of a worker's own two deques to
 * prefer next; cross-worker stealing is classification-blind. */
typedef enum { SAF_CLASS_LIGHT = 0, SAF_CLASS_HEAVY = 1 } saf_class_t;

/*
Full definition of the opaque TCB from tcb.h. Private to the library.
Only tcb.c (and later timer.c) may include this header.
*/
struct TCB {
    int id;
    saf_state_t state;
    ucontext_t ctx;
    void (*fn)(void *);
    void *arg;
    char *map;
    size_t map_len;
    unsigned vg_id;
    saf_class_t class;     /* current light/heavy classification */
    int class_streak;      /* consecutive samecsignal count (hysteresis) */
};

// Guarded stack representation with a low end guard page
typedef struct {
    char *map;
    size_t map_len;
    void *sp;
    size_t sp_len;
    unsigned vg_id;
} guarded_stack_t;

int alloc_guarded_stack(guarded_stack_t *gs);
void free_mapped_stack(char *map, size_t map_len, unsigned vg_id);

extern _Thread_local TCB *current;
extern _Thread_local ucontext_t sched_ctx;

extern _Thread_local long g_switches;
extern _Thread_local volatile sig_atomic_t g_preemptions;
extern _Thread_local volatile sig_atomic_t g_deferred;

/* forces a preemption of the RUNNING thread, or defers it if preempt_count > 0. */
void sched_on_tick(void);

#ifndef NDEBUG
extern long g_tcb_destroyed;
saf_state_t saf_debug_current_state(void);
#endif

#endif
