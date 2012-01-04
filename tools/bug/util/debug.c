/* For general debugging purposes */

#include "../bug.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <linux/kernel.h>
#include "cache.h"
#include "color.h"
#include "debug.h"
#include "util.h"

int verbose;
bool dump_trace = false, quiet = false;

int eprintf(int level, const char *fmt, ...)
{
	va_list args;
	int ret = 0;

	if (verbose >= level) {
		va_start(args, fmt);
		ret = vfprintf(stderr, fmt, args);
		va_end(args);
	}

	return ret;
}

int dump_printf(const char *fmt, ...)
{
	va_list args;
	int ret = 0;

	if (dump_trace) {
		va_start(args, fmt);
		ret = vprintf(fmt, args);
		va_end(args);
	}

	return ret;
}

#ifdef NO_NEWT_SUPPORT
int ui__warning(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	return 0;
}
#endif
