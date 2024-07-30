#include <iostream>
#include <format>
#include <cstring>
extern "C"{
#include "sys/fcntl.h"
#include "unistd.h"
#include "sys/signal.h"
#include "sys/wait.h"
#include "linux/ptrace.h"
}
#include "syscalls.h"

extern "C" long int ptrace (int, ...);

void trace_syscall(pid_t pid){
    if(ptrace(PTRACE_SYSCALL, pid, /*addr=ignored*/nullptr,  /*data: 0=no additional signal*/ 0) == -1){
        perror("PTRACE_SYSCALL: ");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv, char** env) {
    if(argc < 2){
        std::cerr << "Please provide program name. USAGE: ./systrace <prog> [prog-args...]";
        exit(EXIT_FAILURE);
    }

    if (pid_t pid = fork()) {
        // wait for SIGSTOP of child
        waitpid(pid, nullptr, 0);

        // dont trace forks
        ptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_EXITKILL | PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC);

        // Start tracing syscalls
        trace_syscall(pid);

        bool syscall_entered = false;
        while(true){
            //Wait until child gets a signal or dies
            int status;
            waitpid(pid, &status, 0);

            // Handle Syscall
            if(WIFSTOPPED(status) && WSTOPSIG(status)==(SIGTRAP|0x80)){
                if(syscall_entered){ //HANDLE SYSCALL_EXIT (return-value)
                    //re-start syscall capture
                    trace_syscall(pid);
                    syscall_entered = false;
                }else{ //HANDLE SYSCALL_ENTRY (arguments)
                    //inspect values + print name
                    ptrace_syscall_info info{};
                    ptrace(PTRACE_GET_SYSCALL_INFO, pid, sizeof(ptrace_syscall_info), &info);
                    const char* name = syscall_name(info.entry.nr);
                    std::cout << "\033[0;34msyscall: " << info.entry.nr  << " (" << (name ? name : "?") << ")\033[0m\n";

                    //capture syscall-exit
                    trace_syscall(pid);
                    syscall_entered = true;
                }
            }
            // Callee Terminated
            else if(WIFEXITED(status)){
                std::cout << "Callee terminated." << std::endl;
                break;
            }
            // Execve
            else if(status>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8))){
                trace_syscall(pid);
            }
            // Unknown.
            else{
                std::cerr << "unexpected signal at tracee\n";
                break;
            }
        }
    } else{
        // Child process
        if(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1){
            perror("PTRACE_TRACEME: ");
        }
        raise(SIGSTOP);

        // Execute program
        char** buf = (char**) alloca(/*for args*/(argc-1)*sizeof(char*) + /*for nullptr*/sizeof(char*));
        std::memcpy(buf, argv+1, (argc-1)*sizeof(char*));
        buf[argc-1] = nullptr;
        execve(argv[1], buf, env);
        perror("execve: ");
    }
}
