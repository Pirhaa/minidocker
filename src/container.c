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
#include "cgroup.h"  // Cgroup functions call karne ke liye
#include "overlay.h" // Overlay setup ke liye

#define STACK_SIZE (1024 * 1024)

struct child_args {
    const char *rootfs;     
    char *const *cmd;      
    int pipefd[2];         
    char merged_dir[512];  // Overlay ke liye
};

static int child_main(void *arg) {
    struct child_args *a = (struct child_args *)arg;
    char c;

    // 1. Pipe synchronization (Lab 8)
    close(a->pipefd[1]);
    read(a->pipefd[0], &c, 1);
    close(a->pipefd[0]);

    // 2. Hostname set karo (Lab 6/7)
    sethostname("minidocker", 10);

    // 3. Mount private (Lab 7)
    mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL);

    // 4. Overlay Setup Call
    if (setup_overlay(a->rootfs, "test", a->merged_dir) != 0) {
        perror("setup_overlay");
        return 1;
    }

    // 5. Chroot ab direct rootfs nahi, balke merged_dir par
    if (chroot(a->merged_dir) != 0) { 
        perror("chroot"); 
        return 1;
    }

    if (chdir("/") != 0) {
        perror("chdir");
        return 1;
    }

    // 6. /proc mount karo
    mount("proc", "/proc", "proc", 0, NULL);

    // 7. Shell chalao
    execv(a->cmd[0], a->cmd);
    
    perror("execv");
    return 1;
}

int run_container(const char *rootfs, char *const argv[]) {
    struct child_args args;

    if (image_check(rootfs) != 0) return 1;

    args.rootfs = rootfs;
    args.cmd = argv;

    if (pipe(args.pipefd) < 0) { 
        perror("pipe"); 
        return 1; 
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack) { 
        perror("malloc"); 
        return 1; 
    }

    int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD;
    pid_t pid = clone(child_main, stack + STACK_SIZE, flags, &args);
    
    if (pid < 0) { 
        perror("clone"); 
        free(stack); 
        return 1; 
    }

    printf("Container started, host PID = %d\n", pid);

    // Pipe sync
    close(args.pipefd[0]);
    write(args.pipefd[1], "g", 1);
    close(args.pipefd[1]);

    // Cgroup limit apply karo (Ye function cgroup.c mein hai)
    if (limit_pid(pid, "50M", "20") != 0) {
        printf("Warning: cgroup limits failed\n");
    }

    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        printf("Container exited with code %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Container killed by signal %d\n", WTERMSIG(status));
    }

    // Status dikhao aur cleanup karo (Ye bhi cgroup.c se aayega)
    show_status();
    cleanup_cgroup();

    free(stack);
    return 0;
}
