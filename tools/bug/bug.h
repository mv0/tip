#ifndef _PERF_PERF_H
#define _PERF_PERF_H

struct winsize;

void get_term_dimensions(struct winsize *ws);

#if defined(__i386__)
#include "../../arch/x86/include/asm/unistd.h"
#define rmb()		asm volatile("lock; addl $0,0(%%esp)" ::: "memory")
#define cpu_relax()	asm volatile("rep; nop" ::: "memory");
#define CPUINFO_PROC	"model name"
#endif

#if defined(__x86_64__)
#include "../../arch/x86/include/asm/unistd.h"
#define rmb()		asm volatile("lfence" ::: "memory")
#define cpu_relax()	asm volatile("rep; nop" ::: "memory");
#define CPUINFO_PROC	"model name"
#endif

#ifdef __powerpc__
#include "../../arch/powerpc/include/asm/unistd.h"
#define rmb()		asm volatile ("sync" ::: "memory")
#define cpu_relax()	asm volatile ("" ::: "memory");
#define CPUINFO_PROC	"cpu"
#endif

#ifdef __s390__
#include "../../arch/s390/include/asm/unistd.h"
#define rmb()		asm volatile("bcr 15,0" ::: "memory")
#define cpu_relax()	asm volatile("" ::: "memory");
#endif

#ifdef __sh__
#include "../../arch/sh/include/asm/unistd.h"
#if defined(__SH4A__) || defined(__SH5__)
# define rmb()		asm volatile("synco" ::: "memory")
#else
# define rmb()		asm volatile("" ::: "memory")
#endif
#define cpu_relax()	asm volatile("" ::: "memory")
#define CPUINFO_PROC	"cpu type"
#endif

#ifdef __hppa__
#include "../../arch/parisc/include/asm/unistd.h"
#define rmb()		asm volatile("" ::: "memory")
#define cpu_relax()	asm volatile("" ::: "memory");
#define CPUINFO_PROC	"cpu"
#endif

#ifdef __sparc__
#include "../../arch/sparc/include/asm/unistd.h"
#define rmb()		asm volatile("":::"memory")
#define cpu_relax()	asm volatile("":::"memory")
#define CPUINFO_PROC	"cpu"
#endif

#ifdef __alpha__
#include "../../arch/alpha/include/asm/unistd.h"
#define rmb()		asm volatile("mb" ::: "memory")
#define cpu_relax()	asm volatile("" ::: "memory")
#define CPUINFO_PROC	"cpu model"
#endif

#ifdef __ia64__
#include "../../arch/ia64/include/asm/unistd.h"
#define rmb()		asm volatile ("mf" ::: "memory")
#define cpu_relax()	asm volatile ("hint @pause" ::: "memory")
#define CPUINFO_PROC	"model name"
#endif

#ifdef __arm__
#include "../../arch/arm/include/asm/unistd.h"
/*
 * Use the __kuser_memory_barrier helper in the CPU helper page. See
 * arch/arm/kernel/entry-armv.S in the kernel source for details.
 */
#define rmb()		((void(*)(void))0xffff0fa0)()
#define cpu_relax()	asm volatile("":::"memory")
#define CPUINFO_PROC	"Processor"
#endif

#ifdef __mips__
#include "../../arch/mips/include/asm/unistd.h"
#define rmb()		asm volatile(					\
				".set	mips2\n\t"			\
				"sync\n\t"				\
				".set	mips0"				\
				: /* no output */			\
				: /* no input */			\
				: "memory")
#define cpu_relax()	asm volatile("" ::: "memory")
#define CPUINFO_PROC	"cpu model"
#endif

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "../../include/linux/perf_event.h"
#include "util/types.h"
#include <stdbool.h>

static inline unsigned long long rdclock(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/*
 * Pick up some kernel type conventions:
 */
#define __user
#define asmlinkage

#define unlikely(x)	__builtin_expect(!!(x), 0)
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

void pthread__unblock_sigwinch(void);

extern const char bug_version_string[];

#endif
