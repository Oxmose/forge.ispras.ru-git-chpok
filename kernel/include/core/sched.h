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
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#ifndef __POK_SCHED_H__
#define __POK_SCHED_H__

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/schedvalues.h>
#include <core/partition.h>

typedef struct
{
    uint64_t duration; // Set in deployment.c
    uint64_t offset; // Set in deployment.c

    pok_partition_t* partition; // Set in deployment.c

    pok_bool_t periodic_processing_start; // Set in deployment.c

    uint32_t id; // Set in deployment.c
} pok_sched_slot_t;

/*
 * Array of schedule slots.
 * 
 * Set in deployment.c.
 */
extern const pok_sched_slot_t pok_module_sched[];

/*
 * Number of schedule slots.
 * 
 * Set in deployment.c.
 */
extern const uint8_t pok_module_sched_n;

/*
 * Major time frame.
 * 
 * Set in deployment.c.
 */
extern const pok_time_t pok_config_scheduling_major_frame;

void pok_sched_init(void); /* Initialize scheduling stuff */


/**
 *  Starts critical section, when scheduler will not run.
 * 
 * Shouldn't be called from interrupt handler
 * (see pok_preemption_disable_from_interrupt below).
 * 
 * NOTE: Critical sections are NOT reentrant.
 */
void pok_preemption_disable(void);

/**
 *  Ends critical section, and runs scheduler.
 * 
 * Should be paired with pok_preemption_disable or
 * pok_preemption_disable_from_interrupt.
 */
void pok_preemption_enable(void);

/** 
 * Mark scheduler state as needed to be rechecked.
 * 
 * If called outside of critical section, action will be taken at next
 * scheduler run.
 */
void pok_sched_invalidate(void);

/**
 * Disables preemption in interrupt handler.
 * 
 * If preemption is not disabled by normal context, disable it
 * and return TRUE.
 * 
 * Otherwise return FALSE.
 * 
 * DEV: If preemption will be implemented via disabling interrupts,
 * this will return TRUE always.
 */
pok_bool_t pok_preemption_disable_from_interrupt(void);

/**
 * Set `invalidate` flag for scheduler.
 * 
 * Can be called outside of critical section. So scheduler will found
 * the flag set on the next call.
 */
void pok_sched_invalidate(void);

/**
 * Disable preemption before scheduler (re)start.
 * 
 * DEV: When kernel is initialized, this function should be called
 * before enabling interrupts. So, triggering timer interrupt will
 * not call scheduler until it will be ready.
 */
void pok_preemption_off(void);

/**
 * Start scheduler.
 * 
 * Never returns.
 * 
 * Should be called from context, different from ones used by partitions.
 * 
 * Assume partitions to be initialized at this step.
 */
void pok_sched_start(void);

/**
 * Retart scheduler.
 * 
 * Never returns.
 * 
 * Should be called from context, different from ones used by partitions.
 * 
 * Note, that partitions are not cleared here. You need to clear them before.
 */
void pok_sched_restart(void);

/**
 * Should be called from interrupt handler after time is changed.
 * 
 * Called with disabled interrupts.
 */
void pok_sched_on_time_changed(void);

/**
 * Return next release point for periodic process in current partition.
 */
pok_time_t get_next_periodic_processing_start(void);

#endif /* !__POK_SCHED_H__ */
