/*
 * tests/test_light_heavy.c — Verify feedback-based classification makes a
 * worker prefer its own light (cooperative) threads over a heavy
 * (non-yielding, forcibly-preempted) thread.
 *
 * Single worker, no stealing involved: this isolates the local
 * light-vs-heavy deque preference from cross-worker work-stealing.
 */

#include "tcb.h"
#include "timer.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
        exit(1); \
    } \
} while (0)

#define NUM_LIGHT 20
#define HEAVY_TARGET_TICKS 150

static _Atomic int g_light_done = 0;
static _Atomic int g_heavy_done = 0;
static _Atomic int g_violations = 0; /* a light thread finished after heavy did */

/* Never yields: relies entirely on forced timer preemption to share the
 * CPU, same technique as main.c's spin_worker. Runs long enough to be
 * preempted many times, giving classify_on_signal() ample opportunity to
 * flip it to SAF_CLASS_HEAVY well before it actually finishes. */
static void heavy_worker(void *arg) {
    (void)arg;
    while (timer_ticks() < HEAVY_TARGET_TICKS) {
        /* busy spin */
    }
    atomic_fetch_add(&g_heavy_done, 1);
}

static void light_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 5; i++) {
        thread_yield();
    }
    if (atomic_load(&g_heavy_done) != 0) {
        atomic_fetch_add(&g_violations, 1);
    }
    atomic_fetch_add(&g_light_done, 1);
}

int main(void) {
    setenv("SAF_WORKERS", "1", 1);
    setenv("SAF_PREEMPT", "1", 1);
    setenv("SAF_TICK_US", "10000", 1); /* default-ish interval: a much shorter
                                           tick (e.g. 2000us) is unreliable
                                           under Valgrind's heavy instrumentation,
                                           same class of issue as test_spin.c
                                           relying on the library's own default. */

    int rc = thread_create(heavy_worker, NULL);
    CHECK(rc > 0);

    for (int i = 0; i < NUM_LIGHT; i++) {
        rc = thread_create(light_worker, NULL);
        CHECK(rc > 0);
    }

    rc = scheduler_run();
    CHECK(rc == 0);

    CHECK(atomic_load(&g_heavy_done) == 1);
    CHECK(atomic_load(&g_light_done) == NUM_LIGHT);
    CHECK(atomic_load(&g_violations) == 0);

    puts("PASS test_light_heavy");
    return 0;
}
