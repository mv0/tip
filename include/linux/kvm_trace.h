/*
 * kvm_trace_guest.h
 *
 * Copyright (C) 2013, Passau University
 *
 * Authors:
 *   Marius Vlad <mv@sec.uni-passau.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */
#ifndef __KVM_TRACE_H
#define __KVM_TRACE_H

#include <linux/perf_event.h>
#include <linux/syscalls.h>
#include <linux/errno.h>


/* ioctls sent by vmm */
struct trace_syscall {
        unsigned int long idt_index;
        char reg[4];
};

struct trace_userspace {
	struct perf_event_attr *event_attr;
	pid_t	pid;
	int	cpu;
	int 	group_fd;
	unsigned long flags;
};

#define KVM_START_USERSPACE_TRACE       _IOW(KVMIO, 0xf0, struct trace_userspace)
#define KVM_STOP_USERSPACE_TRACE        _IOR(KVMIO, 0xf1, int)
#define KVM_USERSPACE_TRACE_GET_FDS	_IOR(KVMIO, 0xf2, int *)

#define KVM_START_SYSCALL_TRACE         _IOW(KVMIO, 0xf3, struct trace_syscall)
#define KVM_STOP_SYSCALL_TRACE          _IO(KVMIO, 0xf4)


#define KVM_START_SYSCALL_TRACE_SS                      _IO(KVMIO, 0xf5)
#define KVM_STOP_SYSCALL_TRACE_SS                       _IO(KVMIO, 0xf6)

#if 0
int errno;
#define ___syscall_return(type, res) \
do { \
    if ((unsigned long)(res) >= (unsigned long)(-(128 + 1))) { \
        errno = -(res); \
        res = -1; \
    } \
    return (type) (res); \
} while (0)

#define __syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
long __res; \
__asm__ volatile ("push %%ebx ; movl %2,%%ebx ; int $0x80 ; pop %%ebx" \
    : "=a" (__res) \
    : "0" (__NR_##name),"ri" ((long)(arg1)),"c" ((long)(arg2)), \
          "d" ((long)(arg3)) : "memory"); \
___syscall_return(type,__res); \
}


#define __syscall5(type, name, type1, arg1, type2, arg2,\
		   type3, arg3, type4,arg4, type5, arg5) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) \
{ \
long __res; \
__asm__ volatile ("push %%rbx ; movq %2,%%rbx ; movq %1,%%rax ; " \
                  "int $0x80 ; pop %%rbx" \
    : "=a" (__res) \
    : "i" (__NR_##name),"ri" ((long)(arg1)),"c" ((long)(arg2)), \
      "d" ((long)(arg3)),"S" ((long)(arg4)),"D" ((long)(arg5)) \
    : "memory"); \
___syscall_return(type,__res); \
}

#define __NR_nperf_event_open	__NR_perf_event_open
static inline __syscall5(int, nperf_event_open, struct perf_event_attr *, attr, 
                          pid_t, pid, int, cpu, int, group_fd, unsigned long, flags);
#endif

#endif	/* __KVM_TRACE_H */
