/*
 * builtin-test.c
 *
 * Builtin regression testing command: ever growing number of sanity tests
 */
#include "builtin.h"

#include "util/cache.h"
#include "util/debug.h"
#include "util/debugfs.h"
#include "util/parse-options.h"
#include "../../include/linux/hw_breakpoint.h"

#include <sys/mman.h>

static int test__getpid(void)
{
	int pid = getpid();

	/* Oh, a very broken kernel: */
	if (!pid)
		return -1;

	return 0;
}

static struct test {
	const char *desc;
	int (*func)(void);
} tests[] = {
	{
		.desc = "getpid",
		.func = test__getpid,
	},
	{
		.func = NULL,
	},
};

static bool bug_test__matches(int curr, int argc, const char *argv[])
{
	int i;

	if (argc == 0)
		return true;

	for (i = 0; i < argc; ++i) {
		char *end;
		long nr = strtoul(argv[i], &end, 10);

		if (*end == '\0') {
			if (nr == curr + 1)
				return true;
			continue;
		}

		if (strstr(tests[curr].desc, argv[i]))
			return true;
	}

	return false;
}

static int __cmd_test(int argc, const char *argv[])
{
	int i = 0;

	while (tests[i].func) {
		int curr = i++, err;

		if (!bug_test__matches(curr, argc, argv))
			continue;

		pr_info("%2d: %s:", i, tests[curr].desc);
		pr_debug("\n--- start ---\n");
		err = tests[curr].func();
		pr_debug("---- end ----\n%s:", tests[curr].desc);
		pr_info(" %s\n", err ? "FAILED!\n" : "Ok");
	}

	return 0;
}

static int bug_test__list(int argc, const char **argv)
{
	int i = 0;

	while (tests[i].func) {
		int curr = i++;

		if (argc > 1 && !strstr(tests[curr].desc, argv[1]))
			continue;

		pr_info("%2d: %s\n", i, tests[curr].desc);
	}

	return 0;
}

int cmd_test(int argc, const char **argv, const char *prefix __used)
{
	const char * const test_usage[] = {
	"bug test [<options>] [{list <test-name-fragment>|[<test-name-fragments>|<test-numbers>]}]",
	NULL,
	};
	const struct option test_options[] = {
	OPT_INTEGER('v', "verbose", &verbose,
		    "be more verbose (show symbol address, etc)"),
	OPT_END()
	};

	argc = parse_options(argc, argv, test_options, test_usage, 0);
	if (argc >= 1 && !strcmp(argv[0], "list"))
		return bug_test__list(argc, argv);

	return __cmd_test(argc, argv);
}
