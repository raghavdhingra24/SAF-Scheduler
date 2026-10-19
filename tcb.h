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
    READY,
    RUNNING,
    FINISHED
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

#ifdef __cplusplus
}
#endif

#endif
