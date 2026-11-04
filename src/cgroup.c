#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cgroup.h"

static char cgroup_path[256];

static void write_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, value, strlen(value));
        close(fd);
    }
}

int limit_pid(int pid, char *mem, char *pids) {
    char path[512];

    snprintf(cgroup_path, sizeof(cgroup_path), "/sys/fs/cgroup/minidocker/p%d", pid);

    mkdir("/sys/fs/cgroup/minidocker", 0755);
    mkdir(cgroup_path, 0755);

    write_file("/sys/fs/cgroup/minidocker/cgroup.subtree_control", "+memory +pids");

    snprintf(path, sizeof(path), "%s/memory.max", cgroup_path);
    write_file(path, mem);

    snprintf(path, sizeof(path), "%s/memory.swap.max", cgroup_path);
    write_file(path, "0");

    snprintf(path, sizeof(path), "%s/pids.max", cgroup_path);
    write_file(path, pids);

    snprintf(path, sizeof(path), "%s/cgroup.procs", cgroup_path);
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    write_file(path, pid_str);

    return 0;
}

// UPDATED: Ab show_status PID leta hai
void show_status(int pid) {
    char path[512], buf[128];
    int fd, n;

    // PID ke hisaab se path set karo
    snprintf(cgroup_path, sizeof(cgroup_path), "/sys/fs/cgroup/minidocker/p%d", pid);

    printf("\n--- Cgroup Status for PID %d ---\n", pid);

    snprintf(path, sizeof(path), "%s/memory.current", cgroup_path);
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("Memory Used: %s", buf);
        }
        close(fd);
    }

    snprintf(path, sizeof(path), "%s/memory.events", cgroup_path);
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            if (strstr(buf, "oom_kill 1")) {
                printf("Result: KILLED (memory limit exceeded)\n");
            } else {
                printf("Result: RUNNING / EXITED normally\n");
            }
        }
        close(fd);
    }
}

void cleanup_cgroup() {
    rmdir(cgroup_path);
}
