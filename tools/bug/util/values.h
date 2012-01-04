#ifndef __PERF_VALUES_H
#define __PERF_VALUES_H

#include "types.h"

struct bug_read_values {
	int threads;
	int threads_max;
	u32 *pid, *tid;
	int counters;
	int counters_max;
	u64 *counterrawid;
	char **countername;
	u64 **value;
};

void bug_read_values_init(struct bug_read_values *values);
void bug_read_values_destroy(struct bug_read_values *values);

void bug_read_values_add_value(struct bug_read_values *values,
				u32 pid, u32 tid,
				u64 rawid, const char *name, u64 value);

void bug_read_values_display(FILE *fp, struct bug_read_values *values,
			      int raw);

#endif /* __PERF_VALUES_H */
