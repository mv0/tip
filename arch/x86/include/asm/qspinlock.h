#ifndef _ASM_X86_QSPINLOCK_H
#define _ASM_X86_QSPINLOCK_H

#include <asm-generic/qspinlock_types.h>

#if !defined(CONFIG_X86_OOSTORE) && !defined(CONFIG_X86_PPRO_FENCE)

/*
 * Queue spinlock union structure to be used within this header file only
 */
union qspinlock_x86 {
	struct qspinlock slock;
	u8		 lock;	/* Lock bit	*/
};

#define queue_spin_trylock_unfair queue_spin_trylock_unfair
/**
 * queue_spin_trylock_unfair - try to acquire the lock ignoring the qcode
 * @lock: Pointer to queue spinlock structure
 * Return: 1 if lock acquired, 0 if failed
 */
static __always_inline int queue_spin_trylock_unfair(struct qspinlock *lock)
{
	union qspinlock_x86 *qlock = (union qspinlock_x86 *)lock;

	if (!ACCESS_ONCE(qlock->lock) &&
	   (cmpxchg(&qlock->lock, 0, _QSPINLOCK_LOCKED) == 0))
		return 1;
	return 0;
}

#define	queue_spin_unlock queue_spin_unlock
/**
 * queue_spin_unlock - release a queue spinlock
 * @lock : Pointer to queue spinlock structure
 *
 * No special memory barrier other than a compiler one is needed for the
 * x86 architecture. A compiler barrier is added at the end to make sure
 * that the clearing the lock bit is done ASAP without artificial delay
 * due to compiler optimization.
 */
static inline void queue_spin_unlock(struct qspinlock *lock)
{
	union qspinlock_x86 *qlock = (union qspinlock_x86 *)lock;

	barrier();
	ACCESS_ONCE(qlock->lock) = 0;
	barrier();
}

#endif /* !CONFIG_X86_OOSTORE && !CONFIG_X86_PPRO_FENCE */

#include <asm-generic/qspinlock.h>

#endif /* _ASM_X86_QSPINLOCK_H */
