_This project has been created as part of the 42 curriculum by srosu._

# Codexion

## Description

Codexion is a concurrency simulation in C, built on POSIX threads. Coders sit in a
circular co-working hub and share a set of USB dongles — there are as many dongles
as coders, one between each pair of neighbours. A coder must hold **two** dongles
at once to compile, so any two adjacent coders are in direct competition and no
more than half of them can compile at the same time.

Each coder is a thread running the same cycle: acquire both dongles, compile,
release them, debug, then refactor, and immediately try to compile again. Coders
never communicate with each other and have no knowledge of anyone else's state.

The constraint is a deadline. If a coder does not *start* a new compile within
`time_to_burnout` milliseconds of starting its previous one, it burns out and the
whole simulation stops. That window covers debugging, refactoring, and any time
spent waiting for a dongle, so the arbitration policy directly decides who
survives.

Two arbitration policies are implemented, selected at launch: **FIFO**, which
grants a contended dongle to whichever request arrived first, and **EDF**
(Earliest Deadline First), which grants it to whichever coder is closest to
burning out. Both are backed by a hand-written binary heap. A dedicated monitor
thread watches every coder's deadline and stops the simulation the moment one is
missed.

The goal is a program that is correct under every schedule the OS may choose:
no deadlock, no starvation, no data race, no interleaved output, and no CPU burned
on waiting.

---

## Instructions

### Compilation

```bash
make          # build ./codexion
make clean    # remove object files
make fclean   # remove object files and the binary
make re       # fclean + all
```

Compiled with `-Wall -Wextra -Werror -pthread`. No external dependencies, no libft.

### Usage

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
           <dongle_cooldown> <scheduler>
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coder threads, and also the number of dongles. Must be ≥ 1. |
| `time_to_burnout` | Max delay in ms between two compile starts before a coder burns out. |
| `time_to_compile` | Duration in ms of the compile phase, during which two dongles are held. |
| `time_to_debug` | Duration in ms of the debug phase. |
| `time_to_refactor` | Duration in ms of the refactor phase. |
| `number_of_compiles_required` | Simulation stops once every coder reaches this count. |
| `dongle_cooldown` | Delay in ms before a released dongle can be taken again. |
| `scheduler` | `fifo` or `edf`, lowercase, exact match. |

All eight arguments are mandatory. Every argument except `scheduler` must be a
non-negative integer within `INT_MAX`; negative numbers, signs, non-digits and
overflowing values are rejected. `number_of_coders` must additionally be ≥ 1.

### Output

One line per state change on `stdout`. Timestamps are milliseconds elapsed since
the start of the simulation. Coders are numbered from `1`.

```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
400 1 is refactoring
803 1 burned out
```

The program exits when a coder burns out, or when every coder has compiled
`number_of_compiles_required` times. Argument errors are written to `stderr` and
the program exits with status `1`.

### Usage examples

**Nominal run, both policies**

```bash
./codexion 4 2000 200 200 200 3 10 fifo
./codexion 4 2000 200 200 200 3 10 edf
```

Same timings, different arbitration. Both terminate with no burnout once every
coder has compiled three times. Useful for comparing log ordering side by side.

**No deadlock at scale**

```bash
./codexion 100 10000 66 24 87 10 10 fifo
```

100 coders with tight per-phase timings. Terminates cleanly with exactly 10
compiles per coder and no burnout, which is what the parity-based acquisition
order guarantees.

**No busy-waiting under contention**

```bash
./codexion 10 100000 300 50 50 200 300 fifo > /dev/null &
sleep 2; ps -o pcpu=,nlwp= -p $!
```

Long compiles and a high cooldown keep most coders blocked on a dongle for most of
the run. CPU usage stays around 2%, which is the monitor's polling loop plus
condition variable wake-ups — waiting coders consume nothing.

**Single coder**

```bash
./codexion 1 800 200 200 200 5 10 fifo
```

With one coder there is one dongle on the table. Compiling requires two, so the
coder takes the only dongle available and burns out once its window elapses:

```
0 1 has taken a dongle
801 1 burned out
```

**Burnout on infeasible parameters**

```bash
./codexion 4 400 300 300 300 100 10 fifo
```

The fixed phases alone (900 ms) exceed the burnout window (400 ms), so a burnout at
roughly 400 ms is the correct outcome rather than a synchronisation failure. Good
for checking detection precision against the printed timestamp.

**Cooldown as the limiting factor**

```bash
./codexion 2 1000 100 100 100 5 2000 fifo
```

The cooldown (2000 ms) is double the burnout window (1000 ms), so a released dongle
stays unavailable longer than either coder can afford to wait. Demonstrates that
the cooldown is genuinely enforced.

---

## Technical choices

### Project layout

```
include/codexion.h        all types and prototypes
src/main.c                entry point, top-level error paths
src/parsing.c             argument dispatch
src/parse_utils.c         per-argument validation, overflow, scheduler
src/free.c                mutex/cond destruction and teardown
src/heap/heap_create.c    heap allocation, priority comparison, sift up/down
src/heap/heap_utils.c     push/pop/remove, paired operations on two dongles
src/thread/init.c         data init, dongle init, thread creation, start barrier
src/thread/run.c          coder routine, monitor routine
src/thread/run_utils.c    dongle acquisition and release
src/thread/operations.c   compile, debug, refactor
src/thread/time.c         clock, timestamp, interruptible sleep, dongle wait
src/thread/utils.c        shared-flag accessors, deadline check
src/thread/log.c          serialized output
```

### Data structures

| Type | Role |
|---|---|
| `t_data` | Simulation-wide configuration and shared flags. |
| `t_coder` | One per thread: id, compile count, last compile start, its two dongles. |
| `t_dongle` | One per coder: `taken` flag, `last_release`, own mutex, cond and request queue. |
| `t_heap` | Binary heap of pending requests for one dongle, ordered by the active policy. |
| `t_request` | A coder's claim on a dongle: coder id, arrival time, burnout deadline. |

Coders form a ring: the coder at index `i` holds `dongles[i]` as its right dongle
and `dongles[(i + 1) % n]` as its left one, so each dongle sits between exactly two
neighbours. There are no global variables; every thread receives its state through
its `pthread_create` argument.

### Priority queue

Arbitration is backed by a hand-written binary min-heap, one per dongle. A single
comparison function, `priority()`, implements both policies against the same
structure:

- **FIFO** orders by `arrival`, the timestamp at which the request was pushed.
- **EDF** orders by `deadline` (`last_compile_start + time_to_burnout`), then by
  arrival time, then by coder id. The two tie-breakers make the ordering total, so
  EDF stays deterministic even when two deadlines land on the same millisecond.

The heap exposes insertion, extraction of the minimum, and removal by coder id.
That last operation is what allows a coder to withdraw cleanly from a queue it is
no longer waiting on, which in turn is what keeps the queue an accurate picture of
who is genuinely waiting. Because a dongle is structurally contended by only its
two neighbours, the heap never holds more than two entries at a time — the
structure is sized for the general case rather than that bound.

### Startup barrier

All coder threads and the monitor thread are created first, then block on
`gate_cond` until `coders_ready` is set. The simulation clock and every coder's
initial `last_compile` are stamped at that same instant, so no coder starts its
burnout window before the others and no thread races ahead of the clock.

---

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

A deadlock requires all four of Coffman's conditions to hold at once: mutual
exclusion, hold-and-wait, no preemption, and circular wait. The first three are
inherent to the problem as stated and cannot be removed — a dongle must be
exclusively held while in use, a coder legitimately holds one while waiting for the
other, and no thread can strip a dongle from another. Codexion breaks the fourth,
which is sufficient on its own.

`assign_dongle` alternates the acquisition order by coder parity: odd-numbered
coders reach for their left dongle first, even-numbered coders for their right.
The resulting lock ordering is acyclic for any coder count, so the classic
dining-philosophers cycle — every coder holding one dongle and waiting on the one
held by its neighbour — can never form.

This ordering is preserved across waits. A coder that has to block releases
everything and restarts its acquisition from the top of the loop rather than
re-testing on wake-up, which is what prevents it from ever acquiring the two
mutexes in the reverse order and reintroducing the cycle it just avoided.

Verified with 100 coders under tight timings: the simulation terminates cleanly
every time.

### Starvation prevention

Both policies arbitrate against a queue that only ever contains genuine waiters: a
request is pushed when a coder starts waiting and removed when it stops, so a
neighbour idle in its debug or refactor phase never holds a place in the queue.

Under FIFO, a neighbour that finishes and immediately re-requests is placed behind
the coder already queued, so no coder can be indefinitely passed over. Under EDF,
losing arbitration brings a coder's own deadline closer, which raises its priority
on the next round — the coder under the most time pressure is always favoured,
which is what gives the liveness guarantee the subject requires.

### Cooldown handling

`last_release` is stamped on both dongles when they are released, and
`dongle_is_ready` refuses a dongle whose elapsed time since release is below
`dongle_cooldown`.

A cooldown expires through the passage of time, not through any thread's action, so
no broadcast will ever announce it. `wait_for_dongle` handles this case with
`pthread_cond_timedwait` against an absolute deadline computed from
`last_release + dongle_cooldown`, rather than a fixed sleep, so the wait ends at
the right moment regardless of when the check happened to run. Every other reason
for blocking is an event another thread causes and can signal, and uses a plain
`pthread_cond_wait`.

### Precise burnout detection

A dedicated monitor thread scans every coder and compares
`last_compile + time_to_burnout` against the current time. A coder that has already
met its compile quota is exempt, so a simulation that ends normally is never
reported as a burnout.

On a miss, the monitor sets the burnout flag, wakes every sleeping thread, prints
the burnout line, and exits. Measured detection latency is 1–2 ms, well inside the
10 ms the subject allows.

### Log serialization

Every line goes through `log.c` under `write_mtx`, held only around the write
itself, so two coders' messages can never interleave into a corrupted line. Each
log function checks the end-of-simulation flag before printing, so no state change
is announced after the simulation has already stopped.

---

## Thread synchronization mechanisms

### Mutexes

Every piece of shared state has exactly one mutex responsible for it.

| Mutex | Protects |
|---|---|
| `data->write_mtx` | `stdout`. |
| `data->gate_mtx` | `coders_ready` and `finished`. Paired with `gate_cond` for the start barrier. |
| `data->burnout_mtx` | The `burnout` flag. |
| `coder->mutex` | `last_compile`. Paired with `coder->cond` for interruptible sleep. |
| `dongle->dongle_mtx` | `taken`, `last_release`, and that dongle's request queue. Paired with `dongle_cond`. |

Nesting is bounded and ordered: the two dongle mutexes are always taken in the
order `assign_dongle` returns, and `write_mtx` is only ever taken innermost and
released immediately.

### Condition variables

Each dongle owns a condition variable. When a coder cannot acquire, it identifies
which of its two dongles is actually blocking it and waits on *that* dongle's
condition variable, in `wait_for_dongle`.

The distinction matters. A condition variable is only signalled by the thread whose
action unblocks the predicate, so waiting on the free dongle rather than the
contended one would mean waiting for a signal that never arrives. The non-blocking
dongle is released before the wait so nothing is held idle, and the blocking
dongle's mutex is never released between the failed check and entry into the wait,
which is what rules out a lost wake-up.

Because a waiting coder holds only one condition variable, every state change that
can satisfy someone else's predicate broadcasts explicitly. There are three such
points: `release_dongles`, which clears `taken` and stamps `last_release`;
`push_dongles` and `remove_dongles`, which change which coder sits at the head of a
queue; and the burnout path, which wakes everyone. Removal matters as much as
insertion — a coder that abandons its request or reaches its quota would otherwise
leave its successor asleep with no remaining source of wake-up.

### Custom event: end-of-simulation propagation

`is_end()` is the single source of truth for whether the simulation is over, and is
checked at every loop boundary. It returns true when every coder has met its quota
or when the burnout flag is set.

Sleeps go through `ft_usleep`, built on `pthread_cond_timedwait` rather than
`usleep`, precisely because `usleep` cannot be interrupted once entered. `ft_usleep`
returns when either its duration elapses or the simulation ends, whichever comes
first, so no coder keeps running out a long refactor phase after everything else has
stopped. On burnout the monitor broadcasts on every coder condition variable **and**
every dongle condition variable, so threads blocked in either kind of wait observe
the end instead of sleeping through it.

### Preventing race conditions

Shared flags are never read directly. `is_end`, `is_burnout` and `deadline_missed`
copy the value out under the lock and return the copy, so no caller can act on a
field it read outside a critical section. This is what avoids a time-of-check to
time-of-use race, where a value read under one operation is used under another and
changes in between.

The same reasoning applies to acquisition. Both dongle mutexes are held together
while the full condition is checked and while the dongles are claimed, so no other
thread can invalidate one half of the condition between the check and the claim.
Testing the two dongles under separate locks would allow exactly that: the first
dongle could be taken by a neighbour while the second is still being tested, and the
coder would proceed on a condition that was never true at any single instant.

### Coder–monitor communication

Coders and the monitor never exchange messages; they communicate only through
shared state under lock. A coder stamps `last_compile` under its own mutex at the
start of each compile. The monitor reads that same field under that same mutex and
compares it to the current time. Neither side needs to know what the other is doing
— which matches the rule that coders do not communicate and cannot know that
another is about to burn out.

The reverse direction is the burnout flag, protected by `burnout_mtx`. The monitor
sets it, then broadcasts so that any thread parked in a wait re-evaluates
`is_end()` and unwinds. `pthread_join` on every coder thread then returns, and
teardown destroys every mutex and condition variable before freeing the heaps, the
dongle array and the coder array.

---

## Resources

- ["The Dining Philosophers in C: Threads, Race Conditions and Deadlocks"](https://youtu.be/zOpzGHwJ3MU?si=puNrMJNSsaLrQPNF) — Oceano (YouTube)
- ["Codexion"](https://dev.to/yel-bakk/codexion-4fk8) — Yassir El bakkari (dev.to)
- ["Thread Synchronization"](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-8-thread-synchronization/topic/the-need-for-thread-synchronization/) — Nordic Developer Academy
- ["Multithreading in OS - Different Models"](https://www.geeksforgeeks.org/operating-systems/multithreading-in-operating-system/) — GeeksforGeeks
- ["General Concepts"](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html) — The Open Group Base Specifications, Issue 7 (POSIX.1-2017)
- ["Chapter 4: Threads & Concurrency"](https://www.andrew.cmu.edu/course/14-712-s20/applications/ln/14712-l5.pdf) — Carnegie Mellon University, 14-712 course notes

### AI usage

Claude was used as a discussion partner on the concurrency design and to review the code for
races and blocking cases. Some of the resulting code in `run_utils.c`, `time.c`
and `log.c` was written with its help. It also drafted this README.

Every line of code was designed and written by hand, and all debugging was done
independently.