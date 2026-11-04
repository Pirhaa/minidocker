#ifndef CGROUP_H
#define CGROUP_H

int limit_pid(int pid, char *mem, char *pids);
void show_status(int pid);
void cleanup_cgroup();

#endif
