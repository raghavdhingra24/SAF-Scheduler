#ifndef TCB_H
#define TCB_H

#include <ucontext.h>
#include <stddef.h>

#if defined(__has_include)
#  if __has_include(<valgrind/valgrind.h>)
#    include <valgrind/valgrind.h>
#    define HAS_VALGRIND 1
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SAF_READY,
    SAF_RUNNING,
    SAF_FINISHED
} State;

typedef struct TCB {
    int id;
    ucontext_t context;
    void* stack;
    State state;
    void (*entry_func)(void);
#ifdef HAS_VALGRIND
    unsigned int valgrind_stack_id;
#endif
} TCB;

#define STACK_SIZE (64 * 1024)

// Initialize a thread and register it with the scheduler
void init_thread(TCB* t, int id, void (*func)(void));

// Context switch from current thread to next thread
void context_switch(TCB* next_thread);

// Run the round-robin cooperative scheduler
void scheduler(void);

// Yield CPU to the next ready thread
void thread_yield(void);

// Free allocated stacks and reset scheduler state
void cleanup_threads(void);

/* Stage 2 API — implemented in task 03/04
typedef struct TCB TCB;
typedef enum { SAF_READY, SAF_RUNNING, SAF_FINISHED } saf_state_t;
int  thread_create(void (*fn)(void *), void *arg);
void thread_yield(void);
void thread_exit(void);
int  scheduler_run(void);
typedef struct { long switches, preemptions, deferred, ticks, max_queue_len; } saf_stats_t;
void saf_get_stats(saf_stats_t *out);
*/

#ifdef __cplusplus
}
#endif

#endif
