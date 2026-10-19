#include "tcb.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

TCB* current_thread = nullptr;
ucontext_t main_context;

std::vector<TCB*> threads;
int current_index = -1;

static void thread_wrapper(void) {
    if (current_thread && current_thread->entry_func) {
        current_thread->entry_func();
    }
    if (current_thread) {
        current_thread->state = SAF_FINISHED;
    }
    // Yield to the scheduler to switch to next ready thread or return to main
    scheduler();
}

void init_thread(TCB* t, int id, void (*func)(void)) {
    memset(t, 0, sizeof(*t));
    t->id = id;
    t->state = SAF_READY;
    t->entry_func = func;

    // Allocate stack memory aligned to 16-bytes for x86-64 ABI requirements
    if (posix_memalign(&t->stack, 16, STACK_SIZE) != 0 || t->stack == nullptr) {
        perror("Stack allocation failed");
        exit(EXIT_FAILURE);
    }
    memset(t->stack, 0, STACK_SIZE);

#ifdef HAS_VALGRIND
    t->valgrind_stack_id = VALGRIND_STACK_REGISTER(t->stack, (char*)t->stack + STACK_SIZE);
#endif

    // Initialize context
    if (getcontext(&t->context) == -1) {
        perror("getcontext failed");
        exit(EXIT_FAILURE);
    }

    // Configure the thread's stack
    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_size = STACK_SIZE;
    t->context.uc_stack.ss_flags = 0;

    // Fallback if makecontext returns directly
    t->context.uc_link = &main_context;

    // Set the wrapper function where execution begins
    makecontext(&t->context, thread_wrapper, 0);

    // Register with run queue
    threads.push_back(t);
}

void context_switch(TCB* next_thread) {
    TCB* previous_thread = current_thread;

    current_thread = next_thread;
    current_thread->state = SAF_RUNNING;

    if (previous_thread != nullptr) {
        if (previous_thread->state != SAF_FINISHED) {
            previous_thread->state = SAF_READY;
        }

        if (swapcontext(
                &previous_thread->context,
                &next_thread->context
            ) == -1) {
            perror("swapcontext failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // First switch: save main's context
        if (swapcontext(
                &main_context,
                &next_thread->context
            ) == -1) {
            perror("swapcontext failed");
            exit(EXIT_FAILURE);
        }
    }
}

void scheduler(void) {
    int n = static_cast<int>(threads.size());
    if (n == 0) {
        return;
    }

    // Search for the next READY thread using round-robin.
    for (int i = 1; i <= n; i++) {
        int next_index = (current_index + i) % n;

        if (threads[next_index]->state == SAF_READY) {
            current_index = next_index;

            // Switch execution to the selected thread.
            context_switch(threads[next_index]);
            return;
        }
    }

    // No READY threads remain. Return to main context.
    current_thread = nullptr;

    if (setcontext(&main_context) == -1) {
        perror("setcontext failed");
        exit(EXIT_FAILURE);
    }
}

void thread_yield(void) {
    if (current_thread == nullptr) {
        return;
    }

    current_thread->state = SAF_READY;

    scheduler();
}

void cleanup_threads(void) {
    for (size_t i = 0; i < threads.size(); i++) {
#ifdef HAS_VALGRIND
        if (threads[i]->valgrind_stack_id != 0) {
            VALGRIND_STACK_DEREGISTER(threads[i]->valgrind_stack_id);
            threads[i]->valgrind_stack_id = 0;
        }
#endif
        if (threads[i]->stack != nullptr) {
            free(threads[i]->stack);
            threads[i]->stack = nullptr;
        }
    }
    threads.clear();
    current_thread = nullptr;
    current_index = -1;
}
