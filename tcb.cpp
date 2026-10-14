#include <iostream>
#include <cstdlib>
#include <vector>
#include <ucontext.h>

// Thread states
enum State {
    READY,
    RUNNING,
    FINISHED
};
struct TCB {
    int id;
    ucontext_t context;
    void* stack;
    State state;
};

TCB* current_thread = nullptr;
ucontext_t main_context;

std::vector<TCB*> threads;
int current_index = -1;

//Stck size
const size_t STACK_SIZE = 64 * 1024;


// Thread Control Block

void init_thread(TCB* t, int id, void (*func)()) {
    t->id = id;
    t->state = READY;

    // Allocate stack memory
    t->stack = malloc(STACK_SIZE);

    if (t->stack == nullptr) {
        perror("Stack allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize context
    if (getcontext(&t->context) == -1) {
        perror("getcontext failed");
        exit(EXIT_FAILURE);
    }

    // Configure the thread's stack
    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_size = STACK_SIZE;
    t->context.uc_stack.ss_flags = 0;

    // Return to main if the function returns
    t->context.uc_link = &main_context;

    // Set the function where execution begins
    makecontext(&t->context, func, 0);
}

void context_switch(TCB* next_thread) {
    TCB* previous_thread = current_thread;

    current_thread = next_thread;
    current_thread->state = RUNNING;

    if (previous_thread != nullptr) {
        if (previous_thread->state != FINISHED) {
            previous_thread->state = READY;
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

void scheduler() {
    int n = threads.size();

    // Search for the next READY thread using round-robin.
    for (int i = 1; i <= n; i++) {
        int next_index = (current_index + i) % n;

        if (threads[next_index]->state == READY) {
            current_index = next_index;

            // Switch execution to the selected thread.
            context_switch(threads[next_index]);
            return;
        }
    }

    // No READY threads remain. Return to main.
    current_thread = nullptr;

    if (setcontext(&main_context) == -1) {
        perror("setcontext failed");
        exit(EXIT_FAILURE);
    }
}

void thread_yield() {
    if (current_thread == nullptr) {
        return;
    }

    current_thread->state = READY;

    // Give control back to the scheduler
    scheduler();
}
