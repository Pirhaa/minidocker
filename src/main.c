#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "monitor.h"
#include "container.h"
#include "cgroup.h"

int main(int argc, char **argv) {
    // 1. List command
    if (argc >= 2 && strcmp(argv[1], "list") == 0) {
        list_processes();
        return 0;
    }

    // 2. Run command
    if (argc >= 3 && strcmp(argv[1], "run") == 0) {
        char *default_cmd[] = {"/bin/sh", NULL};
        if (argc == 3)
            return run_container(argv[2], default_cmd);
        return run_container(argv[2], &argv[3]);
    }

    // 3. Limit command (Naya)
    if (argc == 5 && strcmp(argv[1], "limit") == 0) {
        int pid = atoi(argv[2]);
        printf("Applying limit to PID %d...\n", pid);
        limit_pid(pid, argv[3], argv[4]);
        return 0;
    }

    // 4. Status command (Naya)
    if (argc == 3 && strcmp(argv[1], "status") == 0) {
        int pid = atoi(argv[2]);
        show_status(pid);
        return 0;
    }

    printf("Usage:\n  ./minidocker list\n  sudo ./minidocker run <image>\n  sudo ./minidocker limit <pid> <mem> <pids>\n  sudo ./minidocker status <pid>\n");
    return 1;
}
