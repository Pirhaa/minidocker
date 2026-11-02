#define _GNU_SOURCE
#include <sched.h>      /* clone */
#include <stdio.h>
#include <unistd.h>     /* chroot, chdir, execv, pipe, read, write, close */
#include <signal.h>     /* SIGCHLD */
#include <sys/mount.h>  /* mount */
#include <sys/wait.h>   /* waitpid */
#include "container.h"
#include "image.h"
#include "cgroup.h"

/* Child ko ye cheezein chahiye, isliye global rakhi hain */
static const char *g_rootfs;     /* alpine folder */
static char *const *g_cmd;       /* command, jaise /bin/sh */
static int g_pipe[2];            /* Lab 8: parent -> child signal */

static char child_stack[1024 * 1024];   /* child ka stack (1 MB) */

/* ---------- CHILD: container ke andar chalta hai ---------- */
static int child_main(void *arg) {
    char c;
    (void)arg;

    /* Lab 8: pipe se parent ka "go" milne tak ruko */
    close(g_pipe[1]);                /* child sirf padhega */
    read(g_pipe[0], &c, 1);
    close(g_pipe[0]);

    sethostname("minidocker", 10);   /* container ka apna hostname */

    /* mounts host se alag rakho */
    mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL);

    chroot(g_rootfs);                /* alpine folder ko "/" bana do */
    chdir("/");                      /* chroot ke baad zaroori */

    mount("proc", "/proc", "proc", 0, NULL);  
    execv(g_cmd[0], g_cmd);        
    perror("execv");                
    return 1;
}


int run_container(const char *rootfs, char *const argv[]) {
    int status;

    if (image_check(rootfs) != 0)    
        return 1;

    g_rootfs = rootfs;
    g_cmd = argv;
    pipe(g_pipe);                 

    int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD;
    pid_t pid = clone(child_main, child_stack + sizeof(child_stack), flags, NULL);
    if (pid < 0) {
        perror("clone");
        return 1;
    }
    printf("Container started, host PID = %d\n", pid);

    close(g_pipe[0]);         

   
    if (limit_pid(pid, "50M", "20") != 0)
        printf("Warning: limit nahi lagi\n");

    write(g_pipe[1], "g", 1);        
    close(g_pipe[1]);

    waitpid(pid, &status, 0);      
    if (WIFEXITED(status))
        printf("Container exited with code %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Container killed by signal %d\n", WTERMSIG(status));

    show_status();                  
    cleanup_cgroup();              
    return 0;
}
