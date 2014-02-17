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
#include <linux/smp.h>
#include <linux/bug.h>
#include <linux/cpumask.h>
#include <linux/percpu.h>
#include <linux/hardirq.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

/*
 * The basic principle of a queue-based spinlock can best be understood
 * by studying a classic queue-based spinlock implementation called the
 * MCS lock. The paper below provides a good description for this kind
 * of lock.
 *
 * http://www.cise.ufl.edu/tr/DOC/REP-1992-71.pdf
 *
 * This queue spinlock implementation is based on the MCS lock with twists
 * to make it fit the following constraints:
 * 1. A max spinlock size of 4 bytes
 * 2. Good fastpath performance
 * 3. No change in the locking APIs
 *
 * The queue spinlock fastpath is as simple as it can get, all the heavy
 * lifting is done in the lock slowpath. The main idea behind this queue
 * spinlock implementation is to keep the spinlock size at 4 bytes while
 * at the same time implement a queue structure to queue up the waiting
 * lock spinners.
 *
 * Since preemption is disabled before getting the lock, a given CPU will
 * only need to use one queue node structure in a non-interrupt context.
 * A percpu queue node structure will be allocated for this purpose and the
 * cpu number will be put into the queue spinlock structure to indicate the
 * tail of the queue.
 *
 * To handle spinlock acquisition at interrupt context (softirq or hardirq),
 * the queue node structure is actually an array for supporting nested spin
 * locking operations in interrupt handlers. If all the entries in the
 * array are used up, a warning message will be printed (as that shouldn't
 * happen in normal circumstances) and the lock spinner will fall back to
 * busy spinning instead of waiting in a queue.
 */

/*
 * The queue node structure
 *
 * This structure is essentially the same as the mcs_spinlock structure
 * in mcs_spinlock.h file. This structure is retained for future extension
 * where new fields may be added.
 */
struct qnode {
	u32		 wait;		/* Waiting flag		*/
	struct qnode	*next;		/* Next queue node addr */
};

/*
 * The 24-bit queue node code is divided into the following 2 fields:
 * Bits 0-1 : queue node index (4 nodes)
 * Bits 2-23: CPU number + 1   (4M - 1 CPUs)
 *
 * The 16-bit queue node code is divided into the following 2 fields:
 * Bits 0-1 : queue node index (4 nodes)
 * Bits 2-15: CPU number + 1   (16K - 1 CPUs)
 *
 * A queue node code of 0 indicates that no one is waiting for the lock.
 * As the value 0 cannot be used as a valid CPU number. We need to add
 * 1 to it before putting it into the queue code.
 */
#ifndef _QCODE_VAL_OFFSET
#define _QCODE_VAL_OFFSET	_QCODE_OFFSET
#endif
#define MAX_QNODES		4
#define GET_QN_IDX(code)	(((code) >> _QCODE_VAL_OFFSET) & 3)
#define GET_CPU_NR(code)	(((code) >> (_QCODE_VAL_OFFSET + 2)) - 1)
#ifndef _SET_QCODE
#define _SET_QCODE(cpu, idx)	((((cpu) + 1) << (_QCODE_VAL_OFFSET + 2)) |\
				((idx) << _QCODE_VAL_OFFSET) |\
				 _QSPINLOCK_LOCKED)
#endif

struct qnode_set {
	int		node_idx;	/* Current node to use */
	struct qnode	nodes[MAX_QNODES];
};

/*
 * Per-CPU queue node structures
 */
static DEFINE_PER_CPU(struct qnode_set, qnset) ____cacheline_aligned
	= { 0 };

/**
 * xlate_qcode - translate the queue code into the queue node address
 * @qcode: Queue code to be translated
 * Return: The corresponding queue node address
 */
static inline struct qnode *xlate_qcode(u32 qcode)
{
	u32 cpu_nr = GET_CPU_NR(qcode);
	u8  qn_idx = GET_QN_IDX(qcode);

	return per_cpu_ptr(&qnset.nodes[qn_idx], cpu_nr);
}

#ifndef queue_spin_trylock_unfair
/**
 * queue_spin_trylock_unfair - try to acquire the lock ignoring the qcode
 * @lock: Pointer to queue spinlock structure
 * Return: 1 if lock acquired, 0 if failed
 */
static __always_inline int queue_spin_trylock_unfair(struct qspinlock *lock)
{
	int qlcode = atomic_read(lock->qlcode);

	if (!(qlcode & _QSPINLOCK_LOCKED) && (atomic_cmpxchg(&lock->qlcode,
		qlcode, qlcode|_QSPINLOCK_LOCKED) == qlcode))
			return 1;
	return 0;
}
#endif /* queue_spin_trylock_unfair */

#ifndef queue_get_lock_qcode
/**
 * queue_get_lock_qcode - get the lock & qcode values
 * @lock  : Pointer to queue spinlock structure
 * @qcode : Pointer to the returned qcode value
 * @mycode: My qcode value (not used)
 * Return : > 0 if lock is not available, = 0 if lock is free
 */
static inline int
queue_get_lock_qcode(struct qspinlock *lock, u32 *qcode, u32 mycode)
{
	int qlcode = atomic_read(&lock->qlcode);

	*qcode = qlcode;
	return qlcode & _QSPINLOCK_LOCKED;
}
#endif /* queue_get_lock_qcode */

#ifndef queue_spin_trylock_and_clr_qcode
/**
 * queue_spin_trylock_and_clr_qcode - Try to lock & clear qcode simultaneously
 * @lock : Pointer to queue spinlock structure
 * @qcode: The supposedly current qcode value
 * Return: true if successful, false otherwise
 */
static inline int
queue_spin_trylock_and_clr_qcode(struct qspinlock *lock, u32 qcode)
{
	return atomic_cmpxchg(&lock->qlcode, qcode, _QSPINLOCK_LOCKED) == qcode;
}
#endif /* queue_spin_trylock_and_clr_qcode */

/**
 * get_qnode - Get a queue node address
 * @qn_idx: Pointer to queue node index [out]
 * Return : queue node address & queue node index in qn_idx, or NULL if
 *	    no free queue node available.
 */
static struct qnode *get_qnode(unsigned int *qn_idx)
{
	struct qnode_set *qset = this_cpu_ptr(&qnset);
	int i;

	if (unlikely(qset->node_idx >= MAX_QNODES))
		return NULL;
	i = qset->node_idx++;
	*qn_idx = i;
	return &qset->nodes[i];
}

/**
 * put_qnode - Return a queue node to the pool
 */
static void put_qnode(void)
{
	struct qnode_set *qset = this_cpu_ptr(&qnset);

	qset->node_idx--;
}

/**
 * queue_spin_lock_slowpath - acquire the queue spinlock
 * @lock : Pointer to queue spinlock structure
 * @qsval: Current value of the queue spinlock 32-bit word
 */
void queue_spin_lock_slowpath(struct qspinlock *lock, int qsval)
{
	unsigned int cpu_nr, qn_idx;
	struct qnode *node, *next;
	u32 prev_qcode, my_qcode;

#ifdef queue_spin_trylock_quick
	/*
	 * Try the quick spinning code path
	 */
	if (queue_spin_trylock_quick(lock, qsval))
		return;
#endif
	/*
	 * Get the queue node
	 */
	cpu_nr = smp_processor_id();
	node   = get_qnode(&qn_idx);

	if (unlikely(!node)) {
		/*
		 * This shouldn't happen, print a warning message
		 * & busy spinning on the lock.
		 */
		printk_sched(
		  "qspinlock: queue node table exhausted at cpu %d!\n",
		  cpu_nr);
		while (!queue_spin_trylock_unfair(lock))
			arch_mutex_cpu_relax();
		return;
	}

	/*
	 * Set up the new cpu code to be exchanged
	 */
	my_qcode = _SET_QCODE(cpu_nr, qn_idx);

	/*
	 * Initialize the queue node
	 */
	node->wait = true;
	node->next = NULL;

	/*
	 * The lock may be available at this point, try again if no task was
	 * waiting in the queue.
	 */
	if (!(qsval >> _QCODE_OFFSET) && queue_spin_trylock(lock)) {
		put_qnode();
		return;
	}

#ifdef queue_code_xchg
	prev_qcode = queue_code_xchg(lock, my_qcode);
#else
	/*
	 * Exchange current copy of the queue node code
	 */
	prev_qcode = atomic_xchg(&lock->qlcode, my_qcode);
	/*
	 * It is possible that we may accidentally steal the lock. If this is
	 * the case, we need to either release it if not the head of the queue
	 * or get the lock and be done with it.
	 */
	if (unlikely(!(prev_qcode & _QSPINLOCK_LOCKED))) {
		if (prev_qcode == 0) {
			/*
			 * Got the lock since it is at the head of the queue
			 * Now try to atomically clear the queue code.
			 */
			if (atomic_cmpxchg(&lock->qlcode, my_qcode,
					  _QSPINLOCK_LOCKED) == my_qcode)
				goto release_node;
			/*
			 * The cmpxchg fails only if one or more tasks
			 * are added to the queue. In this case, we need to
			 * notify the next one to be the head of the queue.
			 */
			goto notify_next;
		}
		/*
		 * Accidentally steal the lock, release the lock and
		 * let the queue head get it.
		 */
		queue_spin_unlock(lock);
	} else
		prev_qcode &= ~_QSPINLOCK_LOCKED;	/* Clear the lock bit */
	my_qcode &= ~_QSPINLOCK_LOCKED;
#endif /* queue_code_xchg */

	if (prev_qcode) {
		/*
		 * Not at the queue head, get the address of the previous node
		 * and set up the "next" fields of the that node.
		 */
		struct qnode *prev = xlate_qcode(prev_qcode);

		ACCESS_ONCE(prev->next) = node;
		/*
		 * Wait until the waiting flag is off
		 */
		while (smp_load_acquire(&node->wait))
			arch_mutex_cpu_relax();
	}

	/*
	 * At the head of the wait queue now
	 */
	while (true) {
		u32 qcode;
		int retval;

		retval = queue_get_lock_qcode(lock, &qcode, my_qcode);
		if (retval > 0)
			;	/* Lock not available yet */
		else if (retval < 0)
			/* Lock taken, can release the node & return */
			goto release_node;
		else if (qcode != my_qcode) {
			/*
			 * Just get the lock with other spinners waiting
			 * in the queue.
			 */
			if (queue_spin_trylock_unfair(lock))
				goto notify_next;
		} else {
			/*
			 * Get the lock & clear the queue code simultaneously
			 */
			if (queue_spin_trylock_and_clr_qcode(lock, qcode))
				/* No need to notify the next one */
				goto release_node;
		}
		arch_mutex_cpu_relax();
	}

notify_next:
	/*
	 * Wait, if needed, until the next one in queue set up the next field
	 */
	while (!(next = ACCESS_ONCE(node->next)))
		arch_mutex_cpu_relax();
	/*
	 * The next one in queue is now at the head
	 */
	smp_store_release(&next->wait, false);

release_node:
	put_qnode();
}
EXPORT_SYMBOL(queue_spin_lock_slowpath);
