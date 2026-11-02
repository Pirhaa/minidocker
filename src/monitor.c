#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include "monitor.h"


static int is_number(const char *s) {
    if (s[0] == '\0')
        return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isdigit(s[i]))
            return 0;
    }
    return 1;
}


void list_processes(void) {
    DIR *dir;
    struct dirent *entry;
    char path[300];
    char name[256];
    FILE *f;
    int count = 0;

    dir = opendir("/proc");             
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    printf("PID      NAME\n");
    printf("------------------------\n");

    while ((entry = readdir(dir)) != NULL) {   

        if (!is_number(entry->d_name))         
            continue;

        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

        f = fopen(path, "r");                   
        if (f == NULL)
            continue;

        if (fgets(name, sizeof(name), f) != NULL) {

        
            for (int i = 0; name[i] != '\0'; i++) {
                if (name[i] == '\n') {
                    name[i] = '\0';
                    break;
                }
            }

            printf("%-8s %s\n", entry->d_name, name);
            count++;
        }
        fclose(f);
    }

    closedir(dir);
    printf("------------------------\n");
    printf("Total processes: %d\n", count);
}


void print_status(int pid) {
    char path[64], line[256];

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror(path);
        return;
    }

    while (fgets(line, sizeof(line), f) != NULL) {
      
        int is_state = (line[0] == 'S' && line[1] == 't' && line[2] == 'a' &&
                        line[3] == 't' && line[4] == 'e');

    
        int is_rss = (line[0] == 'V' && line[1] == 'm' && line[2] == 'R' &&
                      line[3] == 'S' && line[4] == 'S');

        if (is_state || is_rss)
            printf("%s", line);
    }
    fclose(f);
}
