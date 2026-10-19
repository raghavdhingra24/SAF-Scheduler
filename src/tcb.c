#include "tcb.h"
#include "sched_internal.h"
#include "runqueue.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#if defined(__has_include)
#  if __has_include(<valgrind/valgrind.h>)
#    include <valgrind/valgrind.h>
#    define HAS_VALGRIND 1
#  endif
#endif

#define STACK_SIZE ((size_t)64 * 1024)
#define SAF_MAX_THREADS 1024

static TCB *current_thread = NULL;
static ucontext_t main_context;
static int next_id = 1;
static int rq_initialized = 0;
static TCB *all_threads[SAF_MAX_THREADS];
static size_t all_count = 0;

static void tcb_destroy(TCB *t) {
#ifdef HAS_VALGRIND
    if (t->vg_id != 0) {
        VALGRIND_STACK_DEREGISTER(t->vg_id);
    }
#endif
    munmap(t->map, t->map_len);
    free(t);
}

static void tramp(void) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);

    current_thread->fn(current_thread->arg);
    thread_exit();
}

/* Stage 1 structure kept deliberately: threads still swap directly to
 * each other via the scheduler's own stack frame, not through a
 * dedicated scheduler context. The hub-and-spoke rewrite (I1, I2, I5,
 * D1, D2, D5, D11) is task 04. This also means the known single-ready-
 * thread self-swap (D1) can still happen here; it is harmless and is
 * fixed properly in task 04. */
static void schedule(void) {
    TCB *prev = current_thread;

    if (prev != NULL && prev->state == SAF_READY) {
        (void)rq_enqueue(prev);
    }

    TCB *next = rq_dequeue();
    if (next == NULL) {
        current_thread = NULL;
        setcontext(&main_context);
        return;
    }

    current_thread = next;
    next->state = SAF_RUNNING;

    if (prev != NULL) {
        swapcontext(&prev->ctx, &next->ctx);
    } else {
        swapcontext(&main_context, &next->ctx);
    }
}

int thread_create(void (*fn)(void *), void *arg) {
    if (fn == NULL) {
        return -EINVAL;
    }

    if (!rq_initialized) {
        int rc = rq_init(SAF_MAX_THREADS);
        if (rc != 0) {
            return rc;
        }
        rq_initialized = 1;
    }

    TCB *t = calloc(1, sizeof(TCB));
    if (t == NULL) {
        return -ENOMEM;
    }

    long page = sysconf(_SC_PAGESIZE);
    size_t map_len = STACK_SIZE + (size_t)page;
    char *map = mmap(NULL, map_len, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (map == MAP_FAILED) {
        int err = errno;
        free(t);
        return -err;
    }

    if (mprotect(map, (size_t)page, PROT_NONE) == -1) {
        int err = errno;
        munmap(map, map_len);
        free(t);
        return -err;
    }

    unsigned vg_id = 0;
#ifdef HAS_VALGRIND
    vg_id = VALGRIND_STACK_REGISTER(map + page, map + map_len);
#endif

    if (getcontext(&t->ctx) == -1) {
        int err = errno;
#ifdef HAS_VALGRIND
        VALGRIND_STACK_DEREGISTER(vg_id);
#endif
        munmap(map, map_len);
        free(t);
        return -err;
    }

    t->ctx.uc_stack.ss_sp = map + page;
    t->ctx.uc_stack.ss_size = STACK_SIZE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &main_context;
    sigaddset(&t->ctx.uc_sigmask, SIGVTALRM);

    t->id = next_id++;
    t->state = SAF_READY;
    t->fn = fn;
    t->arg = arg;
    t->map = map;
    t->map_len = map_len;
    t->vg_id = vg_id;

    makecontext(&t->ctx, tramp, 0);

    int rc = rq_enqueue(t);
    if (rc != 0) {
#ifdef HAS_VALGRIND
        VALGRIND_STACK_DEREGISTER(vg_id);
#endif
        munmap(map, map_len);
        free(t);
        next_id--;
        return rc;
    }

    all_threads[all_count++] = t;

    return t->id;
}

void thread_yield(void) {
    if (current_thread == NULL) {
        return;
    }
    current_thread->state = SAF_READY;
    schedule();
}

void thread_exit(void) {
    if (current_thread == NULL) {
        return;
    }
    current_thread->state = SAF_FINISHED;
    schedule();
}

int scheduler_run(void) {
    if (current_thread != NULL) {
        return -EBUSY;
    }

    if (rq_size() == 0) {
        return 0;
    }

    schedule();

    /* D5: all stacks are still freed only here, at shutdown, not as
     * each thread finishes. Per-thread freeing lands in task 04. */
    for (size_t i = 0; i < all_count; i++) {
        tcb_destroy(all_threads[i]);
    }
    all_count = 0;
    next_id = 1;
    rq_destroy();
    rq_initialized = 0;

    return 0;
}

/* Stub: real counters are wired up in task 07. */
void saf_get_stats(saf_stats_t *out) {
    if (out == NULL) {
        return;
    }
    out->switches = 0;
    out->preemptions = 0;
    out->deferred = 0;
    out->ticks = 0;
    out->max_queue_len = 0;
}
