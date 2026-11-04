#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include "container.h"
#include "image.h"
#include "overlay.h" // Overlay ke liye

#define STACK_SIZE (1024 * 1024)

struct child_args {
    const char *rootfs;     
    char *const *cmd;      
    int pipefd[2];         
    char merged_dir[256];  // Overlay ka naya folder save karne ke liye
};

static int child_main(void *arg) {
    struct child_args *a = (struct child_args *)arg;
    char c;

    // Lab 8: Pipe synchronization (taake parent aur child sync rahein)
    close(a->pipefd[1]);
    read(a->pipefd[0], &c, 1);
    close(a->pipefd[0]);

    // 1. Hostname set karo (CLONE_NEWUTS)
    sethostname("minidocker", 10);

    // Lab 7: Mount private karo taake host par asar na ho
    mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL);

    // 2. OverlayFS Setup (Lab 5/7 mount system call)
    if (setup_overlay(a->rootfs, "test_container", a->merged_dir) != 0) {
        perror("setup_overlay");
        return 1;
    }

    // 3. Chroot ab direct rootfs nahi, balke overlay wale merged_dir par
    if (chroot(a->merged_dir) != 0) { 
        perror("chroot"); 
        return 1;
    }

    if (chdir("/") != 0) {
        perror("chdir");
        return 1;
    }

    // Lab 6/7: /proc mount karo (CLONE_NEWPID ke liye)
    mount("proc", "/proc", "proc", 0, NULL);

    // Lab 6: execv se shell chalao
    execv(a->cmd[0], a->cmd);
    
    perror("execv");
    return 1;
}

int run_container(const char *rootfs, char *const argv[]) {
    struct child_args args;

    if (image_check(rootfs) != 0) return 1;

    args.rootfs = rootfs;
    args.cmd = argv;

    // Lab 8: Pipe banao
    if (pipe(args.pipefd) < 0) { 
        perror("pipe"); 
        return 1; 
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack) { 
        perror("malloc"); 
        return 1; 
    }

    // Lab 6/7: clone se naya process (namespace ke sath) banao
    int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD;
    pid_t pid = clone(child_main, stack + STACK_SIZE, flags, &args);
    
    if (pid < 0) { 
        perror("clone"); 
        free(stack); 
        return 1; 
    }

    printf("Container started, host PID = %d\n", pid);

    // Lab 8: Parent pipe close karke signal bhejta hai
    close(args.pipefd[0]);
    write(args.pipefd[1], "g", 1);
    close(args.pipefd[1]);

    // Lab 6: waitpid se child ka wait karo
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        printf("Container exited with code %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Container killed by signal %d\n", WTERMSIG(status));
    }

    free(stack);
    return 0;
}
