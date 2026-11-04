#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include "overlay.h"

int setup_overlay(const char *base, const char *id, char *merged_out) {
    char lower[256], upper[256], work[256], opts[1024];

    // Paths set karo (snprintf safe hota hai)
    snprintf(lower, sizeof(lower), "images/%s", base);
    snprintf(upper, sizeof(upper), "images/%s_upper", id);
    snprintf(work, sizeof(work), "images/%s_work", id);
    snprintf(merged_out, 256, "images/%s_merged", id);

    // Folders banao
    mkdir(upper, 0777);
    mkdir(work, 0777);
    mkdir(merged_out, 0777);

    // Mount ke options (lower, upper, work)
    snprintf(opts, sizeof(opts), "lowerdir=%s,upperdir=%s,workdir=%s", lower, upper, work);

    printf("[Overlay] Setup chal raha hai...\n");

    // Asli mount system call
    if (mount("overlay", merged_out, "overlay", 0, opts) != 0) {
        perror("Overlay mount failed");
        return 1; // Fail
    }

    printf("[Overlay] Ready: %s\n", merged_out);
    return 0; // Success
}
