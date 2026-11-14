#ifndef TCB_H
#define TCB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TCB TCB;
typedef enum { SAF_READY, SAF_RUNNING, SAF_FINISHED } saf_state_t;

int  thread_create(void (*fn)(void *), void *arg);  /* >0 thread id, <0 error */
void thread_yield(void);
void thread_exit(void);                              /* does not return */
int  scheduler_run(void);                            /* 0 when all threads finished, <0 error */

/* Scheduler counters are cumulative; max_queue_len is the lifetime high-water mark. */
/* light_count/heavy_count are a best-effort snapshot of how many READY
 * threads currently sit in light vs heavy deques across all workers; only
 * meaningful while scheduler_run() is active (0/0 otherwise), and do not
 * include whichever single thread is RUNNING on each worker. */
typedef struct { long switches, preemptions, deferred, ticks, max_queue_len;
                 long light_count, heavy_count; } saf_stats_t;
void saf_get_stats(saf_stats_t *out);

#ifdef __cplusplus
}
#endif

#endif
