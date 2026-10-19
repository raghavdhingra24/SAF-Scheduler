#include "preempt.h"
#include "preempt_internal.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t preempt_count = 0;
volatile sig_atomic_t preempt_pending = 0;

static sigset_t vtalrm_set;
static int vtalrm_set_ready = 0;

static void ensure_vtalrm_set(void) {
    if (!vtalrm_set_ready) {
        sigemptyset(&vtalrm_set);
        sigaddset(&vtalrm_set, SIGVTALRM);
        vtalrm_set_ready = 1;
    }
}

void sig_block_timer(void) {
    ensure_vtalrm_set();
    sigprocmask(SIG_BLOCK, &vtalrm_set, NULL);
}

void sig_unblock_timer(void) {
    ensure_vtalrm_set();
    sigprocmask(SIG_UNBLOCK, &vtalrm_set, NULL);
}

/* Stage 2 task 05 replaces the guard below with a real
 * preempt_count/preempt_pending implementation (I7). */
void preempt_disable(void) {
}

void preempt_enable(void) {
}

int saf_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf(fmt, ap);
    va_end(ap);
    return ret;
}

void *saf_malloc(size_t n) {
    return malloc(n);
}

void *saf_calloc(size_t n, size_t sz) {
    return calloc(n, sz);
}

void *saf_realloc(void *p, size_t n) {
    return realloc(p, n);
}

void saf_free(void *p) {
    free(p);
}
