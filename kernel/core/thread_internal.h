#ifndef __POK_THREAD_INTERNAL_H__
#define __POK_THREAD_INTERNAL_H__

#include <types.h>

#include <core/thread.h>
#include <core/partition_arinc.h>
#include <core/time.h>

/* 
 * Functions for ARINC threads, used only in implementation of the
 * kernel itself.
 */

/*
 * Create thread.
 * 
 * After this operation thread can be accessed from any thread
 * of partition.
 * 
 * Should be executed with local preemption disabled.
 * 
 * Before given function, next fields should be initialized manually:
 * 
 * - name  [NAME]
 * - entry [ENTRY_POINT]
 * - base_priority [BASE_PRIORITY]
 * - period [PERIOD]
 * - time_capacity [TIME_CAPACITY]
 * - deadline [DEADLINE]
 * - user_stack_size [STACK_SIZE]
 * 
 */
pok_bool_t thread_create(pok_thread_t* t);


/*
 * Postpone event for given thread.
 * 
 * Called with local preemption disabled.
 */
static inline void thread_delay_event(pok_thread_t* t, pok_time_t delay_time,
	void (*thread_delayed_func)(pok_thread_t* t))
{
	t->thread_delayed_func = thread_delayed_func;

	delayed_event_add(&t->thread_delayed_event, delay_time,
		&current_partition_arinc->queue_delayed);
}


/*
 * Cancel postponed event for given thread, if it was.
 * 
 * Called with local preemption disabled.
 */
static inline void thread_delay_event_cancel(pok_thread_t* t)
{
	delayed_event_remove(&t->thread_delayed_event);
}

/*
 * Set deadline for given thread.
 * 
 * Called with local preemption disabled.
 */
void thread_set_deadline(pok_thread_t* t, pok_time_t deadline_time);

/* 
 * Mark thread as waiting on any condition except suspension.
 * 
 * Called with local preemption disabled.
 */
void thread_wait(pok_thread_t* t);


/*
 * Turn current thread to wait until specified time.
 * 
 * Should be called with local preemption disabled.
 */
void thread_wait_timed(pok_thread_t *thread, pok_time_t time);


/* 
 * Turn thread to wait for given duration, or, if duration is
 * infinite, forever.
 * 
 * Convinience function, used when implements ports and intra-partition
 * communication mechanisms.
 * 
 * Should be called with local preemption disabled.
 * 
 * NOTE: duration shouldn't be 0.
 */
static inline void thread_wait_common(pok_thread_t* t, pok_time_t duration)
{
    assert(duration != 0);
    if(pok_time_is_infinity(duration))
        thread_wait(t);
    else
        thread_wait_timed(t, POK_GETTICK() + duration);
}

/*
 * Awoke given thread.
 * 
 * This function can be used when:
 *   1. Thread sleeps (on `thread_delayed_event`),
 *      and sleeping time is over.
 *   2. Thread waits (on `wait_elem`), and waited object become
 *      accesible.
 *   3. Thread waits (on `wait_elem`), and timeout
 *      (on `thread_delayed_event`) is fired.
 * 
 *   4. Thread waits in INIT_* mode, and mode has been set to NORMAL.
 *      (All previouse cases related to NORMAL mode).
 * 
 * NOE: It is assumed that thread is alredy delete itself from queue
 * which is fired.
 * 
 * Called with local preemption disabled.
 * 
 * May be used as callback.
 */
void thread_wake_up(pok_thread_t* t);

/* 
 * Suspend given thread.
 * 
 * Thread shouldn't be suspended already.
 */
void thread_suspend(pok_thread_t* t);


/* 
 * Resume given thread.
 * 
 * Thread should be suspended.
 */
void thread_resume(pok_thread_t* t);


/*
 * Stop given thread in NORMAL mode.
 * 
 * Called with local preemption disabled.
 * 
 * Note: For stop error handler additional actions should be taken.
 */
void thread_stop(pok_thread_t* t);

/* 
 * Whether waiting is allowed in the current context.
 * 
 * Note: Doesn't require disabled local preemption.
 */
static inline pok_bool_t thread_is_waiting_allowed(void)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	if(part->lock_level // In the INIT_* mode lock level is positive, no need to check it explicitely.
#ifdef POK_NEEDS_ERROR_HANDLING
		|| part->thread_error == current_thread /* error thread cannot wait */
#endif		
      )
      return FALSE;

   return TRUE;
}

/* 
 * Mark given thread as runnable in NORMAL mode.
 * 
 * Thread shouldn't wait on any time/resource.
 * 
 * Called with local preemption disabled.
 */
void thread_start(pok_thread_t* t);

/*
 * If thread is eligible, reorder it in the eligible queue after
 * other threads with same priority.
 * 
 * May be called after changing thread's priority.
 * 
 * Called with local preemption disabled.
 */
void thread_yield(pok_thread_t *t);
#endif /* __POK_THREAD_INTERNAL_H__ */
