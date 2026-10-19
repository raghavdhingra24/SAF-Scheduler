#include "tcb.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int g_sequence[9];
static int g_seq_len = 0;

static void worker(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 3; i++) {
        g_sequence[g_seq_len++] = id;
        thread_yield();
    }
}

int main(void) {
    setenv("SAF_PREEMPT", "0", 1);

    int ids[3] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        int rc = thread_create(worker, &ids[i]);
        assert(rc > 0);
    }

    int rc = scheduler_run();
    assert(rc == 0);

    int expected[9] = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    assert(g_seq_len == 9);
    for (int i = 0; i < 9; i++) {
        assert(g_sequence[i] == expected[i]);
    }

    printf("PASS test_coop\n");
    return 0;
}
