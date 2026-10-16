#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tcb.h"

static int g_infinite = 0;
static int g_max_iterations = 4;

void thread1(void) {
    int count = 0;
    while (g_infinite || count < g_max_iterations) {
        count++;
        printf("Thread 1 (ID: 1) - iteration %d\n", count);
        thread_yield();
    }
}

void thread2(void) {
    int count = 0;
    while (g_infinite || count < g_max_iterations) {
        count++;
        printf("Thread 2 (ID: 2) - iteration %d\n", count);
        thread_yield();
    }
}

void thread3(void) {
    int count = 0;
    while (g_infinite || count < g_max_iterations) {
        count++;
        printf("Thread 3 (ID: 3) - iteration %d\n", count);
        thread_yield();
    }
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--infinite") == 0) {
            g_infinite = 1;
        } else {
            int iters = atoi(argv[i]);
            if (iters > 0) {
                g_max_iterations = iters;
            }
        }
    }

    printf("==================================================\n");
    printf("   SAF-Scheduler: Stage 1 Cooperative Test        \n");
    printf("==================================================\n");
    if (g_infinite) {
        printf("[Config] Running indefinitely (Ctrl+C to stop)\n");
    } else {
        printf("[Config] Running %d iterations per thread\n", g_max_iterations);
    }

    TCB t1, t2, t3;

    printf("[Init] Registering 3 cooperative user threads...\n");
    init_thread(&t1, 1, thread1);
    init_thread(&t2, 2, thread2);
    init_thread(&t3, 3, thread3);

    printf("[Run] Starting scheduler...\n");
    scheduler();

    printf("[Done] All user threads completed successfully.\n");
    printf("[Cleanup] Freeing thread stack allocations...\n");
    cleanup_threads();
    printf("[Cleanup] Memory cleanup complete. Stage 1 milestone passed!\n");

    return 0;
}
