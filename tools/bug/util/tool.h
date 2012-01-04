#ifndef __PERF_TOOL_H
#define __PERF_TOOL_H

#include <stdbool.h>

struct bug_session;
union bug_event;
struct bug_evlist;
struct bug_evsel;
struct bug_sample;
struct bug_tool;
struct machine;

typedef int (*event_sample)(struct bug_tool *tool, union bug_event *event,
			    struct bug_sample *sample,
			    struct bug_evsel *evsel, struct machine *machine);

typedef int (*event_op)(struct bug_tool *tool, union bug_event *event,
			struct bug_sample *sample, struct machine *machine);

typedef int (*event_attr_op)(union bug_event *event,
			     struct bug_evlist **pevlist);
typedef int (*event_simple_op)(struct bug_tool *tool, union bug_event *event);

typedef int (*event_synth_op)(union bug_event *event,
			      struct bug_session *session);

typedef int (*event_op2)(struct bug_tool *tool, union bug_event *event,
			 struct bug_session *session);

struct bug_tool {
	event_sample	sample,
			read;
	event_op	mmap,
			comm,
			fork,
			exit,
			lost,
			throttle,
			unthrottle;
	event_attr_op	attr;
	event_synth_op	tracing_data;
	event_simple_op	event_type;
	event_op2	finished_round,
			build_id;
	bool		ordered_samples;
	bool		ordering_requires_timestamps;
};

#endif /* __PERF_TOOL_H */
