/*
 *                               POK header
 *
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team
 *
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
 *  Copyright (C) 2013-2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by julien on Thu Jan 15 23:34:13 2009
 */

/**
 * \file    core/thread.c
 * \author  Julien Delange
 * \date    2008-2009
 * \brief   Thread management in kernel
 */

#include <config.h>

#include <types.h>

#include <arch.h>
#include <core/debug.h>
#include <core/error.h>
#include <core/thread.h>
#include <core/sched_arinc.h>
#include <core/partition.h>
#include <core/time.h>
#include <libc.h>

#include <core/instrumentation.h>

#include "thread_internal.h"
#include <uaccess.h>


/*
 * Find thread by name.
 * 
 * Note: Doesn't require disable local preemption.
 * 
 * Note: Name should be located in kernel space.
 */
static pok_thread_t* find_thread(const char name[MAX_NAME_LENGTH])
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    pok_thread_t* t = part->threads;
    pok_thread_t* t_end = t + part->nthreads_used;
    
    
    for(t = t + POK_PARTITION_ARINC_MAIN_THREAD_ID;
        t != t_end;
        t++)
    {
#ifdef POK_NEEDS_ERROR_HANDLING
		if(part->thread_error == t) continue; /* error thread is not searchable. */
#endif
        if(pok_compare_names(t->name, name)) return t;
    }
    
    return NULL;
}

/*
 * Return thread by id.
 * 
 * Return NULL if no such thread created.
 * 
* Note: Doesn't require disable local preemption.
 */
static pok_thread_t* get_thread_by_id(pok_thread_id_t id)
{
	pok_partition_arinc_t* part = current_partition_arinc;
	
	if(id == 0 /* main thread have no id*/
		|| id >= part->nthreads_used /* thread is not created yet */
#ifdef POK_NEEDS_ERROR_HANDLING
		|| part->thread_error == &part->threads[id] /* error thread has no id */
#endif		
		)
		return NULL;
	
	return &part->threads[id];
}


pok_ret_t pok_thread_create (pok_thread_id_t* __user thread_id,
							   const pok_thread_attr_t  * __user attr)
{
    pok_thread_t* t;
    pok_partition_arinc_t* part = current_partition_arinc;

    /**
     * We can create a thread only if the partition is in INIT mode
     */
    if (part->mode == POK_PARTITION_MODE_NORMAL) {
        return POK_ERRNO_PARTITION_MODE;
    }
    
    if(!check_user_read(attr)) return POK_ERRNO_EFAULT;
    if(!check_user_write(thread_id)) return POK_ERRNO_EFAULT;
    
    if (part->nthreads_used == part->nthreads) {
        return POK_ERRNO_TOOMANY;
    }

    t = &part->threads[part->nthreads_used];
    
    __copy_from_user(&attr->process_name, t->name, MAX_NAME_LENGTH);
    t->entry = __get_user_f(attr, entry);
    t->base_priority = __get_user_f(attr, priority);
    t->period = __get_user_f(attr, period);
    t->time_capacity = __get_user_f(attr, time_capacity);
    t->deadline = __get_user_f(attr, deadline);

    if (t->period == 0) {
        return POK_ERRNO_PARAM;
    }
    if (t->time_capacity == 0) {
        return POK_ERRNO_PARAM;
    }
   
    if(!pok_time_is_infinity(t->period))
    {
        if(pok_time_is_infinity(t->time_capacity)) {
            // periodic process must have definite time capacity
            return POK_ERRNO_PARAM;
        }
       
        if(!pok_time_is_infinity(t->time_capacity)
            && t->time_capacity > t->period) {
            // for periodic process, time capacity <= period
            return POK_ERRNO_PARAM;
        }
   }

    // do at least basic check of entry point
    if (!check_access_exec(t->entry)) {
        return POK_ERRNO_PARAM;
    }

    t->user_stack_size = __get_user_f(attr, stack_size);
    
    if(!thread_create(t)) {
       return POK_ERRNO_TOOMANY; // TODO: Change return code for this case.
    }
   
    __put_user(thread_id, part->nthreads_used);
    
    part->nthreads_used++;

    return POK_ERRNO_OK;
}

#ifdef POK_NEEDS_THREAD_SLEEP

/* 
 * Turn current thread into the sleep for a while.
 * 
 * NOTE: It is possible to sleep forever(ARINC prohibits that).
 */
pok_ret_t pok_thread_sleep(pok_time_t time)
{
    if(!thread_is_waiting_allowed()) return POK_ERRNO_MODE;

	pok_preemption_local_disable();

	if(time == 0)
		thread_yield(current_thread);
    else
        thread_wait_common(current_thread, time);

	pok_preemption_local_enable();
	
	return POK_ERRNO_OK;
}
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
pok_ret_t pok_thread_sleep_until(pok_time_t time)
{
	if(!is_waiting_allowed()) return POK_ERRNO_MODE;
    
	pok_preemption_local_disable();

	ret = thread_wait_timed(current_thread, time);
out:
	pok_preemption_local_enable();
	
	return POK_ERRNO_OK;
}
#endif

pok_ret_t pok_thread_yield (void)
{
	pok_preemption_local_disable();
	thread_yield(current_thread);
	pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

// Called with local preemption disabled
static pok_ret_t thread_delayed_start_internal (pok_thread_t* thread,
												pok_time_t ms)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    pok_time_t thread_start_time;
    
    if (pok_time_is_infinity(ms)) {
        return POK_ERRNO_EINVAL;
    }
    
    if (thread->state != POK_STATE_STOPPED) {
        return POK_ERRNO_UNAVAILABLE;
    }

    if (!pok_time_is_infinity(thread->period) && ms >= thread->period) {
        return POK_ERRNO_EINVAL;
    }
    
    thread->priority = thread->base_priority;
	thread->sp = 0;
  
	if(part->mode != POK_PARTITION_MODE_NORMAL)
	{
		/* Delay thread's starting until normal mode. */
		thread->delayed_time = ms;
		thread_wait(thread);
		
		return POK_ERRNO_OK;
	}

    // Normal mode.
    if (pok_time_is_infinity(thread->period)) {
        // aperiodic process
        thread_start_time = POK_GETTICK() + ms;
    }
    else {
		// periodic process
		thread_start_time = get_next_periodic_processing_start() + ms;
		thread->next_activation = thread_start_time + thread->period;
	}
	
	if(!pok_time_is_infinity(thread->time_capacity))
		thread_set_deadline(thread, thread_start_time + thread->time_capacity);

	/* Only non-delayed aperiodic process starts immediately */
	if(ms == 0 && pok_time_is_infinity(thread->period))
        thread_start(thread);
	else 
		thread_wait_timed(thread, thread_start_time);

    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_delayed_start (pok_thread_id_t id, pok_time_t ms)
{
	pok_ret_t ret;
    
    pok_thread_t *thread = get_thread_by_id(id);
    if(!thread) return POK_ERRNO_PARAM;
    
	pok_preemption_local_disable();
	ret = thread_delayed_start_internal(thread, ms);
	pok_preemption_local_enable();
	
	return ret;
}

pok_ret_t pok_thread_start (pok_thread_id_t id)
{
	pok_ret_t ret;

    pok_thread_t *thread = get_thread_by_id(id);
    if(!thread) return POK_ERRNO_PARAM;
	
	pok_preemption_local_disable();
	ret = thread_delayed_start_internal(thread, 0);
	pok_preemption_local_enable();
	
	return ret;
}


pok_ret_t pok_thread_get_status (pok_thread_id_t id, pok_thread_status_t* __user status)
{
    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return POK_ERRNO_PARAM;
    
    if(!check_user_write(status)) return POK_ERRNO_EFAULT;

	__put_user_f(status, attributes.priority, t->base_priority);
	__put_user_f(status, attributes.entry, t->entry);
	__put_user_f(status, attributes.period, t->period);
	__put_user_f(status, attributes.deadline, t->deadline);
	__put_user_f(status, attributes.time_capacity, t->time_capacity);
	__put_user_f(status, attributes.stack_size, t->user_stack_size);


    pok_preemption_local_disable();

	__put_user_f(status, suspended, t->suspended);
	__put_user_f(status, current_priority, t->priority);

	if(t->state == POK_STATE_RUNNABLE && t->suspended)
		__put_user_f(status, state, POK_STATE_WAITING);
	else
		__put_user_f(status, state, t->state);
	
	if(pok_time_is_infinity(t->time_capacity))
		__put_user_f(status, deadline_time, POK_TIME_INFINITY);
	else
		__put_user_f(status, deadline_time, t->thread_deadline_event.timepoint);

	pok_preemption_local_enable();
	
	return POK_ERRNO_OK;
}

pok_ret_t pok_thread_set_priority(pok_thread_id_t id, uint32_t priority)
{
    pok_ret_t ret;
    
    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return POK_ERRNO_PARAM;
    
    pok_preemption_local_disable();

    ret = POK_ERRNO_UNAVAILABLE;
    if (t->state == POK_STATE_STOPPED) goto out;

    t->priority = priority;
    
    thread_yield(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_resume(pok_thread_id_t id)
{
    pok_ret_t ret;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return POK_ERRNO_PARAM;

    // can't resume self, lol
    if (t == current_thread) return POK_ERRNO_THREADATTR;

	// although periodic process can never be suspended anyway,
	// ARINC-653 requires different error code
    if (pok_thread_is_periodic(t)) return POK_ERRNO_MODE;

    pok_preemption_local_disable();

    ret = POK_ERRNO_MODE;
    if (t->state == POK_STATE_STOPPED) goto out;
    
	ret = POK_ERRNO_UNAVAILABLE;
    if (!t->suspended) goto out;

    thread_resume(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_suspend(pok_thread_id_t id)
{
    pok_ret_t ret;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return POK_ERRNO_PARAM;

	// can't suspend current process
	// _using this function_
	// (use pok_thread_suspend instead)
    if (t == current_thread) return POK_ERRNO_THREADATTR;

	// although periodic process can never be suspended anyway,
	// ARINC-653 requires different error code
    if (pok_thread_is_periodic(t)) return POK_ERRNO_MODE;

    pok_preemption_local_disable();

    ret = POK_ERRNO_MODE;
    // can't suspend stopped (dormant) process
    if (t->state == POK_STATE_STOPPED) goto out;

    // Cannot suspend process holded preemption lock.
    if (current_partition_arinc->lock_level > 0
		&& current_partition_arinc->thread_locked == t) goto out;

	ret = POK_ERRNO_UNAVAILABLE;
    if (t->suspended) goto out;

    thread_suspend(t);
    
	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_suspend_self(int64_t ms) // Time should be converted? How to pass 64bit through system call?
{
    pok_thread_t *t = current_thread;
    
	// although periodic process can never be suspended anyway,
	// ARINC-653 requires different error code
    if (pok_thread_is_periodic(t)) return POK_ERRNO_MODE;

    if (!thread_is_waiting_allowed()) return POK_ERRNO_MODE;
    
    pok_preemption_local_disable();

	if(ms == 0) goto out; // Nothing to do with 0 timeout.

    thread_suspend(t);
    
	if(!pok_time_is_infinity(ms))
		thread_delay_event(t, POK_GETTICK() + ms, &thread_resume);

out:
	pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

pok_ret_t pok_thread_stop(pok_thread_id_t id)
{
    pok_ret_t ret = POK_ERRNO_PARAM;

    pok_thread_t *t = get_thread_by_id(id);
    if(!t) return POK_ERRNO_PARAM;

	// can's stop self
	// use pok_thread_stop to do that
    if (t == current_thread) return POK_ERRNO_THREADATTR;
    
    pok_preemption_local_disable();
    
    ret = POK_ERRNO_UNAVAILABLE;
    if (t->state == POK_STATE_STOPPED) goto out;

	thread_stop(t);

	ret = POK_ERRNO_OK;
out:
	pok_preemption_local_enable();

    return ret;
}

pok_ret_t pok_thread_stop_self(void)
{
    pok_preemption_local_disable();
	thread_stop(current_thread);
    
#ifdef POK_NEEDS_ERROR_HANDLING
    if(current_thread == current_partition_arinc->thread_error)
    {
        error_check_after_handler();
    }
#endif
	pok_preemption_local_enable();
	
	return POK_ERRNO_OK;
}

pok_ret_t pok_thread_find(const char* __user name, pok_thread_id_t* __user id)
{
    char kernel_name[MAX_NAME_LENGTH];
    pok_thread_t* t;
    
    if(!check_access_read(name, MAX_NAME_LENGTH)) return POK_ERRNO_EFAULT;
    if(!check_user_write(id)) return POK_ERRNO_EFAULT;
    
    __copy_from_user(name, kernel_name, MAX_NAME_LENGTH);
    
    t = find_thread(kernel_name);
    
    if(!t) return POK_ERRNO_EINVAL; // TODO: INVALID_CONFIG for ARINC
    
    __put_user(id, t - current_partition_arinc->threads);
    
	return POK_ERRNO_OK;
}

// called by periodic process when it's done its work
// ARINC-653 PERIODIC_WAIT
pok_ret_t pok_sched_end_period(void)
{
    pok_thread_t* t = current_thread;
    
    if(!pok_thread_is_periodic(t)) return POK_ERRNO_MODE;

	if(thread_is_waiting_allowed()) return POK_ERRNO_MODE;

    pok_preemption_local_disable();

	thread_wait_timed(t, t->next_activation);
	thread_set_deadline(t, t->next_activation + t->time_capacity);
	t->next_activation += t->period;

	pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

pok_ret_t pok_sched_replenish(pok_time_t budget)
{
    pok_ret_t ret;

    pok_thread_t* t = current_thread;
    pok_time_t calculated_deadline;

#ifdef POK_NEEDS_ERROR_HANDLING
    if(t == current_partition_arinc->thread_error) return POK_ERRNO_UNAVAILABLE;
#endif
    
    if(current_partition_arinc->mode != POK_PARTITION_MODE_NORMAL)
		return POK_ERRNO_UNAVAILABLE;
    
    if (budget > INT32_MAX) return POK_ERRNO_ERANGE;
    
    if(pok_time_is_infinity(t->time_capacity)) return POK_ERRNO_OK; //nothing to do
    
    pok_preemption_local_disable();
    
    calculated_deadline = POK_GETTICK() + budget;
    
    if(!pok_time_is_infinity(t->period)
		&& calculated_deadline >= t->next_activation)
    {
        ret = POK_ERRNO_MODE;
        goto out;
    }
    
    thread_set_deadline(t, calculated_deadline);
    ret = POK_ERRNO_OK;

out:    
    pok_preemption_local_enable();
    
    return ret;
}

pok_ret_t pok_sched_get_current(pok_thread_id_t* __user thread_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    if(part->mode != POK_PARTITION_MODE_NORMAL)
		return POK_ERRNO_THREAD;

#ifdef POK_NEEDS_ERROR_HANDLING
	if(part->thread_current == part->thread_error)
		return POK_ERRNO_THREAD;
#endif

    if(!check_user_write(thread_id)) return POK_ERRNO_EFAULT;

	__put_user(thread_id, part->thread_current - part->threads);

    return POK_ERRNO_OK;
}
