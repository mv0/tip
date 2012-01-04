#ifndef BUILTIN_H
#define BUILTIN_H

#include "util/util.h"
#include "util/strbuf.h"

extern const char bug_usage_string[];
extern const char bug_more_info_string[];

extern void list_common_cmds_help(void);
extern const char *help_unknown_cmd(const char *cmd);
extern void prune_packed_objects(int);
extern int read_line_with_nul(char *buf, int size, FILE *file);
extern int check_pager_config(const char *cmd);

extern int cmd_help(int argc, const char **argv, const char *prefix);
extern int cmd_version(int argc, const char **argv, const char *prefix);
extern int cmd_test(int argc, const char **argv, const char *prefix);

#endif
