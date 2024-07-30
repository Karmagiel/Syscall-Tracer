# Syscall tracer

Executes a process and traces its system-calls  (similar to strace).

The main goal was for me to learn about linux ptrace, with the goal of writing a compatibility layer (modifying syscalls) for another OS (on same architecture or using binary-translation) in the future.

# Build
1. `mkdir build`
2. `cmake -S . -B build -G Ninja`
3. `cmake --build build`

# Run
1. `./build/systrace <program-to-run> [program-args...]`