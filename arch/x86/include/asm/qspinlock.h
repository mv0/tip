#ifndef _ASM_X86_QSPINLOCK_H
#define _ASM_X86_QSPINLOCK_H

#include <asm-generic/qspinlock_types.h>

#if !defined(CONFIG_X86_OOSTORE) && !defined(CONFIG_X86_PPRO_FENCE)

/*
 * Queue spinlock union structure to be used within this header file only
 * Besides the slock and lock fields, the other fields are only valid with
 * less than 16K CPUs.
 */
union qspinlock_x86 {
	struct qspinlock slock;
	struct {
		u8  lock;	/* Lock bit	*/
		u8  wait;	/* Waiting bit	*/
		u16 qcode;	/* Queue code	*/
	};
	u16 lock_wait;		/* Lock and wait bits */
	int qlcode;
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

#ifndef _Q_MANY_CPUS
/*
 * With less than 16K CPUs, the following optimizations are possible with
 * the x86 architecture:
 *  1) The 2nd byte of the 32-bit lock word can be used as a pending bit
 *     for waiting lock acquirer so that it won't need to go through the
 *     MCS style locking queuing which has a higher overhead.
 *  2) The 16-bit queue code can be accessed or modified directly as a
 *     16-bit short value without disturbing the first 2 bytes.
 */
#define	_QSPINLOCK_WAITING	0x100U	/* Waiting bit in 2nd byte   */
#define	_QSPINLOCK_LWMASK	0xffff	/* Mask for lock & wait bits */

/*
 * As the qcode will be accessed as a 16-bit word, no offset is needed
 */
#define _QCODE_VAL_OFFSET	0
#define _SET_QCODE(cpu, idx)	(((cpu) + 1) << 2 | (idx))

#define queue_spin_trylock_quick queue_spin_trylock_quick
/**
 * queue_spin_trylock_quick - fast spinning on the queue spinlock
 * @lock : Pointer to queue spinlock structure
 * @qsval: Old queue spinlock value
 * Return: 1 if lock acquired, 0 if failed
 *
 * This is an optimized contention path for 2 contending tasks. It
 * should only be entered if no task is waiting in the queue. This
 * optimized path is not as fair as the ticket spinlock, but it offers
 * slightly better performance. The regular MCS locking path for 3 or
 * more contending tasks, however, is fair.
 *
 * Depending on the exact timing, there are several different paths where
 * a contending task can take. The actual contention performance depends
 * on which path is taken. So it can be faster or slower than the
 * corresponding ticket spinlock path. On average, it is probably on par
 * with ticket spinlock.
 */
static inline int queue_spin_trylock_quick(struct qspinlock *lock, int qsval)
{
	union qspinlock_x86 *qlock = (union qspinlock_x86 *)lock;
	u16		     old;

	/*
	 * Fall into the quick spinning code path only if no one is waiting
	 * or the lock is available.
	 */
	if (unlikely((qsval != _QSPINLOCK_LOCKED) &&
		     (qsval != _QSPINLOCK_WAITING)))
		return 0;

	old = xchg(&qlock->lock_wait, _QSPINLOCK_WAITING|_QSPINLOCK_LOCKED);

	if (old == 0) {
		/*
		 * Got the lock, can clear the waiting bit now
		 */
		ACCESS_ONCE(qlock->wait) = 0;
		return 1;
	} else if (old == _QSPINLOCK_LOCKED) {
try_again:
		/*
		 * Wait until the lock byte is cleared to get the lock
		 */
		do {
			cpu_relax();
		} while (ACCESS_ONCE(qlock->lock));
		/*
		 * Set the lock bit & clear the waiting bit
		 */
		if (cmpxchg(&qlock->lock_wait, _QSPINLOCK_WAITING,
			   _QSPINLOCK_LOCKED) == _QSPINLOCK_WAITING)
			return 1;
		/*
		 * Someone has steal the lock, so wait again
		 */
		goto try_again;
	} else if (old == _QSPINLOCK_WAITING) {
		/*
		 * Another task is already waiting while it steals the lock.
		 * A bit of unfairness here won't change the big picture.
		 * So just take the lock and return.
		 */
		return 1;
	}
	/*
	 * Nothing need to be done if the old value is
	 * (_QSPINLOCK_WAITING | _QSPINLOCK_LOCKED).
	 */
	return 0;
}

#define queue_code_xchg	queue_code_xchg
/**
 * queue_code_xchg - exchange a queue code value
 * @lock : Pointer to queue spinlock structure
 * @qcode: New queue code to be exchanged
 * Return: The original qcode value in the queue spinlock
 */
static inline u32 queue_code_xchg(struct qspinlock *lock, u32 qcode)
{
	union qspinlock_x86 *qlock = (union qspinlock_x86 *)lock;

	return (u32)xchg(&qlock->qcode, (u16)qcode);
}

#define queue_spin_trylock_and_clr_qcode queue_spin_trylock_and_clr_qcode
/**
 * queue_spin_trylock_and_clr_qcode - Try to lock & clear qcode simultaneously
 * @lock : Pointer to queue spinlock structure
 * @qcode: The supposedly current qcode value
 * Return: true if successful, false otherwise
 */
static inline int
queue_spin_trylock_and_clr_qcode(struct qspinlock *lock, u32 qcode)
{
	qcode <<= _QCODE_OFFSET;
	return atomic_cmpxchg(&lock->qlcode, qcode, _QSPINLOCK_LOCKED) == qcode;
}

#define queue_get_lock_qcode queue_get_lock_qcode
/**
 * queue_get_lock_qcode - get the lock & qcode values
 * @lock  : Pointer to queue spinlock structure
 * @qcode : Pointer to the returned qcode value
 * @mycode: My qcode value
 * Return : > 0 if lock is not available
 *	   = 0 if lock is free
 *	   < 0 if lock is taken & can return after cleanup
 *
 * It is considered locked when either the lock bit or the wait bit is set.
 */
static inline int
queue_get_lock_qcode(struct qspinlock *lock, u32 *qcode, u32 mycode)
{
	u32 qlcode;

	qlcode = (u32)atomic_read(&lock->qlcode);
	/*
	 * With the special case that qlcode contains only _QSPINLOCK_LOCKED
	 * and mycode. It will try to transition back to the quick spinning
	 * code by clearing the qcode and setting the _QSPINLOCK_WAITING
	 * bit.
	 */
	if (qlcode == (_QSPINLOCK_LOCKED | (mycode << _QCODE_OFFSET))) {
		u32 old = qlcode;

		qlcode = atomic_cmpxchg(&lock->qlcode, old,
				_QSPINLOCK_LOCKED|_QSPINLOCK_WAITING);
		if (qlcode == old) {
			union qspinlock_x86 *slock =
				(union qspinlock_x86 *)lock;
try_again:
			/*
			 * Wait until the lock byte is cleared
			 */
			do {
				cpu_relax();
			} while (ACCESS_ONCE(slock->lock));
			/*
			 * Set the lock bit & clear the waiting bit
			 */
			if (cmpxchg(&slock->lock_wait, _QSPINLOCK_WAITING,
				    _QSPINLOCK_LOCKED) == _QSPINLOCK_WAITING)
				return -1;	/* Got the lock */
			goto try_again;
		}
	}
	*qcode = qlcode >> _QCODE_OFFSET;
	return qlcode & _QSPINLOCK_LWMASK;
}

#endif /* _Q_MANY_CPUS */
#endif /* !CONFIG_X86_OOSTORE && !CONFIG_X86_PPRO_FENCE */

#include <asm-generic/qspinlock.h>

#endif /* _ASM_X86_QSPINLOCK_H */
