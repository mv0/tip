/*
 * Queue spinlock
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * (C) Copyright 2013-2014 Hewlett-Packard Development Company, L.P.
 *
 * Authors: Waiman Long <waiman.long@hp.com>
 */
#ifndef __ASM_GENERIC_QSPINLOCK_TYPES_H
#define __ASM_GENERIC_QSPINLOCK_TYPES_H

#include <linux/types.h>
#include <linux/atomic.h>
#include <asm/barrier.h>
#include <asm/processor.h>

/*
 * The queue spinlock data structure - a 32-bit word
 *
 * The bits assignment are:
 *   Bit  0   : Set if locked
 *   Bits 1-7 : Not used
 *   Bits 8-31: Queue code
 */
typedef struct qspinlock {
	atomic_t	qlcode;	/* Lock + queue code */
} arch_spinlock_t;

#define _QCODE_OFFSET		8
#define _QSPINLOCK_LOCKED	1U
#define	_QCODE(lock)	(atomic_read(&(lock)->qlcode) >> _QCODE_OFFSET)
#define	_QLOCK(lock)	(atomic_read(&(lock)->qlcode) & _QSPINLOCK_LOCKED)

#endif /* __ASM_GENERIC_QSPINLOCK_TYPES_H */
