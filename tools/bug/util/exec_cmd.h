#ifndef __PERF_EXEC_CMD_H
#define __PERF_EXEC_CMD_H

extern void bug_set_argv_exec_path(const char *exec_path);
extern const char *bug_extract_argv0_path(const char *path);
extern const char *bug_exec_path(void);
extern void setup_path(void);
extern int execv_bug_cmd(const char **argv); /* NULL terminated */
extern int execl_bug_cmd(const char *cmd, ...);
extern const char *system_path(const char *path);

#endif /* __PERF_EXEC_CMD_H */
