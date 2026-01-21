# Custom C-Shell

A fully functional **Unix-like shell** implemented in **C**, designed to closely mimic the behavior of standard shells such as **Bash** and **Zsh**. This project emphasizes **modularity**, **process control**, **signal handling**, and **persistent state management**, and is built incrementally across **17 well-defined specifications**.

---

## How to Run

### 1️⃣ Compile
Navigate to the project directory and run:
```bash
make
```

### 2️⃣ Execute

```bash
./myshell
```

### 3️⃣ Clean Up

```bash
make clean
```

---

## Specifications & Implementation Details

### **1. Display Requirement (Prompt)**



Displays a dynamic shell prompt on every input line:

```
<Username@SystemName:CurrentDirectory>
```

**Implementation**

* Uses `getcwd()` to fetch the working directory
* Uses `getpwuid()` and `gethostname()` for user/system info
* Displays `~` for paths inside the shell’s home directory

---

### **2. Input Requirements**



Supports multiple commands separated by:

* `;` → sequential execution
* `&` → background execution

**Implementation**

* Manual parsing of delimiters (`;`, `&`)
* `strtok()` used only for argument tokenization
* Background processes are not waited upon

---

### **3. `hop` — Change Directory**



Built-in alternative to `cd`. Supports:

```
.  ..  ~  -
```

**Implementation**

* Uses `chdir()` (must run in parent process)
* Maintains `prev_dir` for `-`
* Supports multiple arguments executed sequentially
* Prints absolute path after each directory change

---

### **4. `reveal` — File Listing**



Lists directory contents (similar to `ls`).

**Supported Flags**

* `-a` → show hidden files
* `-l` → detailed listing

**Implementation**

* Uses `scandir()` + `alphasort` for lexicographic order
* Uses `stat()` for permissions, size, ownership, and timestamps
* Color coding:

  * **Blue** → directories
  * **Green** → executables
  * **White** → regular files

---

### **5. `log` — Command History**



Persistent command history (max **15** entries).

**Rules**

* Stored across sessions in `.myshell_history`
* No immediate duplicates
* Commands containing `log` are not stored
* Commands separated by `;` or `&` are stored as a single entry

**Commands**

* `log` → display history (oldest → newest)
* `log purge` → clear history
* `log execute <index>` → execute command (most recent = index 1)

---

### **6. External System Commands**



Executes standard Unix commands (`sleep`, `vim`, `echo`, etc.).

**Implementation**

* Uses `fork()` + `execvp()`
* Foreground processes block using `waitpid()`
* Background processes print PID and return immediately

---

### **7. `proclore` — Process Information**



Displays detailed information about a process.

**Implementation**

* Reads `/proc/[pid]/stat` for status and memory
* Uses `/proc/[pid]/exe` to find executable path
* Determines FG/BG using process group IDs

---

### **8. `seek` — File Search**



Recursively searches files/directories by name or prefix.

**Flags**

* `-d` → directories only
* `-f` → files only
* `-e` → execute / enter if exactly one match

**Implementation**

* Recursive traversal using `opendir()` and `readdir()`
* Maintains relative paths during recursion

---

### **9. `.myshrc` — Shell Configuration**



Loads aliases and custom functions at startup.

**Implementation**

* Parses `.myshrc` line-by-line
* Performs alias expansion before execution
* Supports positional arguments (`$1`, `$2`, ...)

---

### **10. Input / Output Redirection**



Supports:

```
<   >   >>
```

**Implementation**

* Scans command string before argument parsing
* Uses `open()` and `dup2()` for FD redirection
* Removes redirection tokens before execution

---

### **11. Pipes (`|`)**



Allows chaining commands using pipes.

**Implementation**

* Splits input on `|`
* Uses `pipe()` and `dup2()` for FD chaining
* Forks a process per pipeline stage

---

### **12. Pipes + Redirection**



Ensures redirection works correctly inside pipelines.

**Implementation**

* Pipe setup happens first
* Redirection applied afterward in child processes
* Explicit file redirection overrides pipe endpoints

---

### **13. `activities` — Background Jobs**



Lists all background processes started by the shell.

**Implementation**

* Maintains a linked list of background jobs
* Reads `/proc/[pid]/stat` for live status
* Outputs sorted lexicographically

---

### **14. Signal Handling**

**Handled Signals**

* `Ctrl+C` → interrupt foreground process
* `Ctrl+Z` → stop foreground process, move to background
* `Ctrl+D` → exit shell cleanly
* `ping <pid> <signal>` → send arbitrary signals

**Implementation**

* Uses `kill()`, `SIGINT`, `SIGTSTP`, `SIGCONT`
* Tracks foreground PID globally

---

### **15. `fg` and `bg`**



Move processes between foreground and background.

**Implementation**

* `fg` → `SIGCONT` + `waitpid()`
* `bg` → `SIGCONT`
* Updates job tracking structures

---

### **16. `neonate`**



Prints the most recently created PID every *N* seconds until `x` is pressed.

**Implementation**

* Reads `/proc/loadavg`
* Uses `termios` for raw keyboard input
* Uses `select()` for non-blocking timing

---

### **17. `iMan` — Internet Man Pages**



Fetches man pages directly from the internet.

**Implementation**

* Uses sockets (`AF_INET`, TCP)
* Sends HTTP GET requests to `man.he.net`
* Strips HTML tags for readable output

---

## 🗂️ File Structure

```
main.c              → Shell loop & initialization
header.h            → Global includes & macros
prompt-1.c          → Prompt rendering
command-2.c         → Parsing & dispatch
hop-3.c             → Directory navigation
reveal-4.c          → File listing
log-5.c             → Command history
proclore-7.c        → Process inspection
seek-8.c            → Recursive search
myshrc-9.c          → Config loader
ioredir-10.c        → I/O redirection
pipes-11.c          → Pipelines
activities-13.c     → Background jobs
signals-14.c        → Signal handling
fgbg-15.c           → fg / bg commands
neonate-16.c        → PID monitor
iman-17.c           → Internet man pages
```

---

## Conclusion

This project demonstrates a complete understanding of **Unix process control**, **file systems**, **signals**, **pipes**, and **shell internals**.

---
