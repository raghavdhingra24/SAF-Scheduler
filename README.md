# SAF-Scheduler

A small user-space **M:N thread scheduler** prototype, built up in stages:

- **Stage 1** — cooperative scheduling: TCBs, `ucontext` switching, a single FIFO run queue.
- **Stage 2** — timer-driven preemption: `ITIMER_VIRTUAL`/`SIGVTALRM` ticks force a switch even if a thread never yields, with a preemption guard around `saf_printf`/`saf_malloc`/etc.
- **Stage 3** — M:N scheduling: M pthread workers, each pinned to a core, pulling from per-worker Chase-Lev work-stealing deques so idle cores steal work from busy ones.
- **Multi-level feedback queue** — each worker actually owns a *pair* of deques, LIGHT and HEAVY. A thread's classification is driven by how it was last switched out (voluntary `thread_yield()` vs. forced timer preemption), with a 3-consecutive-signal streak threshold to avoid thrashing between classes. `sched_next_thread` prefers a worker's own LIGHT deque, then its own HEAVY deque, then steals from peers (LIGHT before HEAVY); cross-worker stealing stays classification-blind.
- **GUI** — `viz.py`, a PyQt6 timeline visualizer that runs `./scheduler -t` (trace mode), parses the `TRACE,<timestamp_ns>,<worker_id>,<thread_id>,<START|YIELD|EXIT>` lines it emits, and animates each worker's thread timeline.

## Build and test

The documented target is Linux x86-64 with GCC and glibc.

```sh
make
make test
make test-long
make test-mem
make tsan
make clean && make DEBUG=0 test
```

`make test-mem` requires Valgrind. `make tsan` builds and runs the test suite under ThreadSanitizer to catch data races in the work-stealing deques. See [STAGE2.md](STAGE2.md) for the Stage 2 architecture, safety rules, and invariants (I1–I10), and [STAGE3.md](STAGE3.md) for the current M:N architecture, module breakdown, and test suite.

## Running the demo

```sh
./scheduler --workers 4 --threads 6   # M:N run, 4 workers / 6 user threads
./scheduler --spin 3                  # fairness check: 3 CPU-bound spinning threads
SAF_PREEMPT=0 ./scheduler             # preemption disabled, strict round-robin order
```

See `main.c` for the full flag list (`-w/--workers`, `-n/--threads`, `-i/--infinite`, `-t/--trace`, `--spin N`).

## GUI visualizer

```sh
make            # build ./scheduler first
python3 viz.py
```

Pick a worker count and thread count in the window and run it; `viz.py` shells out to `./scheduler -t` under the hood and animates the resulting trace. Requires `PyQt6`.
