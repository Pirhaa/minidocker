#include <stdio.h>
 #include <string.h>
#include <stdlib.h>
#include "monitor.h"
#include "container.h"

int main(int argc, char **argv) {

if (argc >= 2 && strcmp(argv[1], "list") == 0) {
list_processes();
return 0;
}

if (argc >= 3 && strcmp(argv[1], "run") == 0) {
char *default_cmd[] = {"/bin/sh", NULL};
if (argc == 3)
return run_container(argv[2], default_cmd);
return run_container(argv[2], &argv[3]);
}

printf("Usage: list | run [cmd]\n");
return 1;
}
