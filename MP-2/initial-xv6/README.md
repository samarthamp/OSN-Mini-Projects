# XV6 Enhanced Kernel

## Overview

This project involves modifying the XV6 kernel to implement advanced operating system concepts including system call tracing, user-level interrupt handling, and alternative process scheduling algorithms. The modifications touch both kernel-space (process management, trap handling, system calls) and user-space (new utilities and test programs).

## Features and Implementation

### 1. System Call Tracing

**Description:**
A feature to track and count the usage of specific system calls by a process and its children.

**Implementation:**

* **System Call:** Added `getSysCount(mask)`. This accepts a bitmask where the set bit corresponds to the system call number to be traced.
* **Kernel Modifications:**
* Modified `struct proc` to store the tracing mask and a running counter.
* Updated `syscall()` in `kernel/syscall.c` to intercept execution and increment the counter if the mask matches.
* Updated `exit()` to aggregate counts from child processes to their parents and print the final tally upon the parent's exit.


* **User Program:** Added `syscount.c`, a utility that sets the mask and executes a command (e.g., `syscount 32768 grep hello README`).

### 2. User-Level Alarm

**Description:**
A mechanism allowing user processes to request a callback function to be executed periodically after a specified amount of CPU time (ticks).

**Implementation:**

* **System Calls:**
* `sigalarm(interval, handler)`: Registers a handler function to be called every `interval` ticks.
* `sigreturn()`: Returns control from the handler to the interrupted instruction.


* **Kernel Modifications:**
* Modified `struct proc` to track alarm intervals, elapsed ticks, and handler addresses.
* Added a backup `trapframe` in `struct proc` to save the CPU state (registers) before jumping to the handler.
* Modified `usertrap()` in `kernel/trap.c` to increment the tick counter and redirect execution flow to the handler when the interval is reached.



### 3. Process Scheduling

The default Round-Robin scheduler was extended to support compile-time selection of two additional algorithms.

#### A. Lottery Based Scheduling (LBS)

**Description:**
A preemptive scheduler where processes are assigned "tickets." The CPU is awarded probabilistically; a process with more tickets has a higher chance of being selected.

* **Tie-Breaking:** If two processes have the same number of tickets, the one with the earlier arrival time is selected.
* **System Call:** Added `settickets(int number)` to allow processes to modify their ticket count (default is 1).

#### B. Multi-Level Feedback Queue (MLFQ)

**Description:**
A preemptive scheduler using four priority queues (0-3), where 0 is the highest priority.

* **Priority Management:**
* Processes start at priority 0.
* If a process exhausts its time slice, it is demoted to the next lower priority.
* Time slices: Queue 0 (1 tick), Queue 1 (4 ticks), Queue 2 (8 ticks), Queue 3 (16 ticks).


* **Priority Boosting:** Every 48 ticks, all processes are promoted to the highest priority queue (0) to prevent starvation.
* **Round Robin:** Used for scheduling processes within the same queue (specifically the lowest queue).

## Compilation Instructions

The scheduling policy is selected at compile time using the `SCHEDULER` flag.

1. **Clean the build environment:**
```bash
make clean

```


2. **Compile and Run:**
* **Default (Round Robin):**
```bash
make qemu

```


* **Lottery Based Scheduling (LBS):**
```bash
make qemu SCHEDULER=LBS

```


* **Multi-Level Feedback Queue (MLFQ):**
```bash
make qemu SCHEDULER=MLFQ

```





## Testing Instructions

Once the OS is booted (using `make qemu`), run the following commands to verify functionality.

### 1. Testing System Call Tracing

Run the `syscount` utility with a mask and a command.

* **Example (Count `open` syscalls):**
```bash
$ syscount 32768 grep hello README

```


*Output:* Should print the command output followed by `PID <id> called open <n> times.`

### 2. Testing Alarm

Run the provided test suite `alarmtest`.

```bash
$ alarmtest

```

*Output:* Must pass `test0`, `test1`, `test2`, and `test3`.

### 3. Testing Scheduler (LBS)

Ensure the kernel was compiled with `SCHEDULER=LBS`. Run the custom test program `lbstest`.

```bash
$ lbstest

```

*Output:* Creates two child processes (e.g., 10 tickets vs 50 tickets). The process with more tickets should complete significantly more work (loop iterations) than the one with fewer tickets.

### 4. Testing Scheduler (MLFQ)

Ensure the kernel was compiled with `SCHEDULER=MLFQ`. Run the custom test program `mlfqtest`.

```bash
$ mlfqtest

```

*Verification:* While the test runs, press `Ctrl+P` (or the configured process dump key) repeatedly. You should observe the process migrating from priority 0 down to priority 3, and periodically resetting to priority 0 (boosting).