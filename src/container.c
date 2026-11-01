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

#define STACK_SIZE (1024 * 1024)

struct child_args {
    const char *rootfs;     
    char *const *cmd;      
    int pipefd[2];         
};

static int child_main(void *arg) {
    struct child_args *a = (struct child_args *)arg;
    char c;

    close(a->pipefd[1]);
    read(a->pipefd[0], &c, 1);
    close(a->pipefd[0]);

 
    sethostname("minidocker", 10);

  
    mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL);

    if (chroot(a->rootfs) != 0) { perror("chroot"); return 1; }
    chdir("/");


    mount("proc", "/proc", "proc", 0, NULL);

   
    execv(a->cmd[0], a->cmd);
    perror("execv");
    return 1;
}


int run_container(const char *rootfs, char *const argv[]) {
    struct child_args args;

    if (image_check(rootfs) != 0) return 1;

    args.rootfs = rootfs;
    args.cmd = argv;

    if (pipe(args.pipefd) < 0) { perror("pipe"); return 1; }

    char *stack = malloc(STACK_SIZE);
    if (!stack) { perror("malloc"); return 1; }
    char *stack_top = stack + STACK_SIZE;

    int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD;

    pid_t pid = clone(child_main, stack_top, flags, &args);
    if (pid < 0) { perror("clone"); free(stack); return 1; }

    printf("Container started, host PID = %d\n", pid);

    close(args.pipefd[0]);


    write(args.pipefd[1], "g", 1);
    close(args.pipefd[1]);

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        printf("Container exited with code %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Container killed by signal %d\n", WTERMSIG(status));

    free(stack);
    return 0;
}
