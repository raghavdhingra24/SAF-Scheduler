
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <ucontext.h>

using namespace std;

// Thread states
enum State {
    READY,
    RUNNING,
    FINISHED
};

// Thread Control Block
struct TCB {
    int id;
    ucontext_t context;
    void* stack;
    State state;
    void (*func)();
};

// Constants and global variables
const size_t STACK_SIZE = 64 * 1024;

TCB* current_thread = nullptr;
ucontext_t main_context;

vector<TCB*> threads;
int current_index = -1;

// Function declarations
void scheduler();
void context_switch(TCB* next_thread);
void thread_yield();
void thread_entry();

// Initialize a thread
void init_thread(TCB* t, int id, void (*func)()) {
    t->id = id;
    t->func = func;
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

    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_size = STACK_SIZE;
    t->context.uc_stack.ss_flags = 0;

    // The wrapper handles thread termination.
    t->context.uc_link = nullptr;

    // Begin execution in thread_entry().
    makecontext(&t->context, thread_entry, 0);
}

// Entry point for every user-level thread
void thread_entry() {
    // current_thread points to the thread selected by the scheduler.
    TCB* finished_thread = current_thread;

    // Run the actual thread function.
    finished_thread->func();

    // The function returned, so the thread has finished.
    finished_thread->state = FINISHED;

    // Find another READY thread or return to main.
    scheduler();

    // A finished thread must never continue executing.
    abort();
}

// Switch execution to another thread
void context_switch(TCB* next_thread) {
    TCB* previous_thread = current_thread;

    // Avoid switching to the same context.
    if (previous_thread == next_thread) {
        next_thread->state = RUNNING;
        return;
    }

    // Mark the selected thread as running.
    current_thread = next_thread;
    next_thread->state = RUNNING;

    if (previous_thread != nullptr) {
        // Save the previous thread and restore the next one.
        if (swapcontext(
                &previous_thread->context,
                &next_thread->context
            ) == -1) {
            perror("swapcontext failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // First switch: save main's context.
        if (swapcontext(
                &main_context,
                &next_thread->context
            ) == -1) {
            perror("swapcontext failed");
            exit(EXIT_FAILURE);
        }
    }
}

// Voluntarily give up the CPU
void thread_yield() {
    if (current_thread == nullptr) {
        return;
    }

    // The thread is pausing, not finishing.
    current_thread->state = READY;

    scheduler();
}

// Round-robin scheduler
void scheduler() {
    int n = static_cast<int>(threads.size());

    // No threads were created.
    if (n == 0) {
        return;
    }

    // Search circularly for the next READY thread.
    for (int i = 1; i <= n; i++) {
        int next_index = (current_index + i) % n;
        TCB* next_thread = threads[next_index];

        if (next_thread->state == READY) {
            current_index = next_index;

            context_switch(next_thread);
            return;
        }
    }

    // No READY threads remain.
    if (current_thread != nullptr) {
        // Save this information before clearing the pointer.
        current_thread = nullptr;

        // main_context was saved during the first context switch.
        if (setcontext(&main_context) == -1) {
            perror("setcontext failed");
            exit(EXIT_FAILURE);
        }

        // setcontext() does not return on success.
        abort();
    }

    // No thread is running and nothing is ready.
    // This also handles an empty initial scheduling attempt.
}

// Example thread functions
void task1() {
    for (int i = 1; i <= 3; i++) {
        cout << "Thread 1: " << i << endl;
        thread_yield();
    }

    cout << "Thread 1 finished" << endl;
}

void task2() {
    for (int i = 1; i <= 3; i++) {
        cout << "Thread 2: " << i << endl;
        thread_yield();
    }

    cout << "Thread 2 finished" << endl;
}

void task3() {
    for (int i = 1; i <= 2; i++) {
        cout << "Thread 3: " << i << endl;
        thread_yield();
    }

    cout << "Thread 3 finished" << endl;
}

int main() {
    // Create the threads.
    for (int i = 1; i <= 3; i++) {
        TCB* t = new TCB{};

        if (i == 1) {
            init_thread(t, i, task1);
        } else if (i == 2) {
            init_thread(t, i, task2);
        } else {
            init_thread(t, i, task3);
        }

        threads.push_back(t);
    }

    cout << "Starting scheduler..." << endl;

    // Start the first thread.
    scheduler();

    cout << "All threads finished." << endl;

    // Free allocated stacks and TCBs.
    for (TCB* t : threads) {
        free(t->stack);
        delete t;
    }

    threads.clear();

    return 0;
}