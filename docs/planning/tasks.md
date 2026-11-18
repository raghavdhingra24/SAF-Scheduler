## Stage 1: Foundation & Architecture (Weeks 1-2)

*Goal: Establish cooperative multitasking before introducing hardware interrupts.*

* **Core OS:** Design the `TCB` (Thread Control Block). Allocate 64KB stacks using `malloc` or `mmap` aligned to page boundaries. Wire up `makecontext` and `swapcontext` for cooperative context switching.
* **Algorithms:** Build a standard Ring Buffer for the local Run Queue. Do not attempt work-stealing yet; just prove tasks can queue and dequeue safely on a single thread.
* **Concurrency:** Research memory models and C11 atomics (`<stdatomic.h>`).
* **QA & Integration:** Initialize the Git repository, write the master `Makefile`, and integrate `valgrind` to catch stack memory leaks early.
* **Milestone:** Two user threads successfully taking turns executing via manual `yield()` calls.

## Stage 2: Preemption & Signal Masking (Week 3)

*Goal: Force involuntary context switches using hardware timers.*

* **Core OS:** Implement `setitimer` with `ITIMER_VIRTUAL` to fire every ~10ms. Bind `SIGVTALRM` to your scheduler via `sigaction`.
* **Algorithms:** Wire the signal handler to the scheduler loop: save the current thread, push it to the back of the Ring Buffer, and pop the next one.
* **Concurrency:** Implement `sigprocmask`. You must mask the timer signal *before* starting a context switch or modifying the queue. If a timer interrupt fires while you are halfway through swapping registers, the CPU state will corrupt.
* **QA & Integration:** Write aggressive infinite-loop user threads to verify the scheduler forces them to share CPU time.
* **Milestone:** A stable 1:N preemptive Round-Robin scheduler.

## Stage 3: The M:N Engine & Work-Stealing (Weeks 4-5)

*Goal: Distribute user threads across multiple physical CPU cores.*

* **Core OS:** Spawn $N$ underlying POSIX `pthreads` (Worker Threads) at boot. Use `sched_setaffinity` to pin each Worker to a specific physical core to maximize L1/L2 cache hits.
* **Algorithms:** Upgrade the Ring Buffer to a Double-Ended Queue (Deque). Workers push/pop tasks exclusively from the bottom of their own deque.
* **Concurrency:** Implement the lock-free `steal()` function. Use C11 Compare-And-Swap (CAS) so an idle Worker can safely pop a task from the top of another Worker's Deque without locking the entire system.
* **QA & Integration:** Run workloads compiled with GCC's ThreadSanitizer (`-fsanitize=thread`) to hunt down data races in the work-stealing logic.
* **Milestone:** Full M:N scheduling where overloaded cores are actively relieved by idle cores.

## Stage 4: Telemetry Pipeline (Week 6)

*Goal: Generate empirical data to prove the scheduler's efficiency.*

* **Core OS & Algorithms:** Inject `clock_gettime(CLOCK_MONOTONIC)` hooks right before and after `swapcontext` to measure the exact nanosecond overhead of your context switch.
* **Concurrency:** Implement Thread-Local Storage (TLS) for logging. If all Workers write to the same global log buffer simultaneously, lock contention will destroy your performance.
* **QA & Integration:** Write Bash wrappers to compile the code, execute parallel workloads (e.g., recursive matrix multiplication), and dump the TLS logs into `.csv` files.
* **Milestone:** Automated generation of raw performance datasets.

## Stage 5: Analytics & Hardening (Week 7)

*Goal: Polish the codebase and finalize the academic deliverables.*

* **All Roles:** Memory hardening. Ensure every TCB and stack is properly freed when a user thread terminates. Fix any residual segmentation faults.
* **QA & Integration:** Write Python or R scripts to parse the `.csv` files. Generate density plots showing task distribution across the Workers, and calculate the standard deviation of queue lengths to prove the work-stealing algorithm actually balances the load.
* **Milestone:** The final C library, the statistical plots, and a fully formatted LaTeX architecture report.
