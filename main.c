#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tcb.h"

static int g_infinite = 0;
static int g_max_iterations = 4;

static void worker(void *arg) {
    int id = *(int *)arg;
    int count = 0;
    while (g_infinite || count < g_max_iterations) {
        count++;
        printf("Thread %d (ID: %d) - iteration %d\n", id, id, count);
        thread_yield();
    }
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-i|--infinite] [iterations]\n", prog);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--infinite") == 0) {
            g_infinite = 1;
            continue;
        }

        char *end = NULL;
        long val = strtol(argv[i], &end, 10);
        if (end == argv[i] || *end != '\0' || val <= 0) {
            usage(argv[0]);
            return 1;
        }
        g_max_iterations = (int)val;
    }

    printf("==================================================\n");
    printf("   SAF-Scheduler: Stage 1 Cooperative Test        \n");
    printf("==================================================\n");
    if (g_infinite) {
        printf("[Config] Running indefinitely (Ctrl+C to stop)\n");
    } else {
        printf("[Config] Running %d iterations per thread\n", g_max_iterations);
    }

    int ids[3] = {1, 2, 3};

    printf("[Init] Registering 3 cooperative user threads...\n");
    for (int i = 0; i < 3; i++) {
        int rc = thread_create(worker, &ids[i]);
        if (rc < 0) {
            fprintf(stderr, "thread_create failed: %d\n", rc);
            return 1;
        }
    }

    printf("[Run] Starting scheduler...\n");
    int rc = scheduler_run();
    if (rc < 0) {
        fprintf(stderr, "scheduler_run failed: %d\n", rc);
        return 1;
    }

    printf("[Done] All user threads completed successfully.\n");
    printf("[Cleanup] Memory cleanup complete. Stage 1 milestone passed!\n");

    return 0;
}
