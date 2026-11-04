# SonarQube Cloud Findings

Project: minidocker 

## Severity Summary
| Severity | Count |
|----------|-------|
| Blocker  | 0 |
| High     | 3 |
| Medium   | 0 |
| Low      | 0 |
| Info     | 0 |

All 3 findings are of type **Security** (total  effort: 15 min).

## Triaged Issues
| # | Rule | File:Line | Severity | Risk |
|---|------|-----------|----------|------|
| 1 | The return value of "chroot" should be checked to make sure that the rest of the program really executes in the jailed environment | cgroup.c:34 | High | If chroot() fails and nobody checks it, the program keeps running outside the jail while we think the container is isolated, which breaks the whole security idea. |
| 2 | Check the return value of "chdir", if this operation fails, your application might be insecure | src/cgroup.c:35 | High | If chdir() fails silently, the process stays in the old directory and may read or write files outside the intended jail. |
| 3 | Check the return value of "chdir", if this operation fails, your application might be insecure | src/container.c:35 | High | Same problem: an unchecked chdir() can leave the container process in the wrong directory and give it access to host files. |

## Note
Sonar reported only 3 issues in this project, so only 3 are listed here
