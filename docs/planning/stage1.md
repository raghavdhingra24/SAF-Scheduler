Here is the exact task breakdown for each team member.

**Core OS Engineer: Contexts and Memory**
Your job is to build the actual thread switching mechanism. If the stack is corrupted here, the whole scheduler fails.

* **Define the TCB:** Create the `thread_control_block` struct. It needs an ID, a state flag (READY, RUNNING, EXITED), a `ucontext_t` object to hold CPU registers, and a pointer to its dynamically allocated stack.
* **Stack allocation:** Write the `thread_create` function. Use `malloc` or `posix_memalign` to allocate memory for the stack (use `SIGSTKSZ`, typically 8KB, or allocate 64KB for safety).
* **x86-64 ABI alignment:** This is a classic trap. The x86-64 architecture requires the stack pointer (`uc_stack.ss_sp`) to be 16-byte aligned before making a function call. Ensure your allocated memory respects this, or functions like `printf` will mysteriously segfault inside your user threads.
* **Wire up ucontext:** Inside `thread_create`, call `getcontext()` to initialize the context, configure `uc_stack` and `uc_link` (where the thread goes when it finishes), and use `makecontext()` to bind the user's C function to this context.
* **The yield function:** Write `thread_yield()`. This function will call `swapcontext(&current_tcb->context, &next_tcb->context)` to pause the current thread and load the next one.

**Algorithm Specialist: The Run Queue**
Do not worry about work-stealing or multiple queues yet. Keep it dead simple so the OS Engineer can test their context switch.

* **Build the Ring Buffer:** Implement a standard FIFO circular array of TCB pointers.
* **Queue operations:** Write `enqueue(TCB *thread)` and `dequeue()`. Handle the basic edge cases (queue full, queue empty).
* **The scheduler loop:** Write the main `schedule()` function. It should dequeue the next READY thread and pass it to the OS Engineer's `swapcontext` logic. When a thread yields, push it back to the end of the queue.

**Concurrency Lead: Kernel Workers and TLS**
Your job in Stage 1 is setting up the environment that will eventually host the advanced scheduling.

* **Spawn the kernel pool:** Write the initialization function that calls standard `pthread_create` to spawn the $N$ worker threads.
* **Thread-Local Storage (TLS):** This is critical. Because multiple kernel threads will eventually be running your scheduler loop simultaneously, global variables like `current_user_thread` will cause race conditions. Use the C `_Thread_local` keyword (or `__thread` in GCC) to declare TLS variables. This ensures every kernel worker has its own private pointer to the user thread it is currently executing.
* **CPU Pinning:** Research and implement `pthread_setaffinity_np` (requires `#define _GNU_SOURCE`) to pin each of your worker `pthreads` to a specific physical CPU core.

**QA & Integration Lead: Infrastructure and Harness**
You are responsible for making sure the team's code compiles cleanly and doesn't leak memory.

* **The Makefile:** Write a rigid Makefile. Use flags `-Wall -Wextra -Werror` to ban sloppy code, `-g` for GDB debugging symbols, and `-pthread` for the underlying kernel workers.
* **Valgrind integration:** Write a bash script (`make test-mem`) that compiles the code and runs it through `valgrind --leak-check=full`. The OS Engineer is dynamically allocating stacks; if they forget to `free()` them when a thread exits, Valgrind must catch it immediately.
* **The milestone test:** Write `main.c`. Spawn 3 user threads that just print their Thread ID and call `thread_yield()` inside a `while` loop. If you see the console output alternating perfectly (1, 2, 3, 1, 2, 3...), Stage 1 is complete.

1. **Initialize the POSIX worker pool:** Concurrency Lead.
Spawn the underlying pthreads and pin them to CPU cores using thread affinity.


2. **Allocate TCB and aligned stack memory:** Core OS Engineer.
Use posix_memalign to create the stack, ensuring 16-byte alignment to prevent x86-64 segmentation faults.


3. **Initialize the execution context:** Core OS Engineer.
Call getcontext(), configure uc_stack, and bind the target C function using makecontext().


4. **Queue the thread:** Algorithm Specialist.
Push the newly created TCB pointer into the lock-free Ring Buffer.


5. **Execute and yield:** All Roles.
The scheduler loop pops the thread, calls swapcontext(), and the user thread manually yields back to the scheduler when done.
