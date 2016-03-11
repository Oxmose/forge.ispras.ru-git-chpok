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
 * Created by julien on Mon Jan 19 10:51:40 2009 
 */

#include <config.h>

#ifdef POK_NEEDS_ERROR_HANDLING

#include <types.h>
#include <arch.h>
#include <core/error_arinc.h>
#include <core/partition_arinc.h>
#include <core/debug.h>
#include <core/sched_arinc.h>

#include <assert.h>
#include <libc.h>
#include <uaccess.h>
#include "thread_internal.h"
#include <message.h>

static inline pok_error_kind_t get_error_kind(pok_error_id_t error_id)
{
    return current_partition_arinc->
        thread_error_info->map[error_id].error_kind;
}

static inline const char* get_error_description(pok_error_id_t error_id)
{
    return current_partition_arinc->
        thread_error_info->map[error_id].error_description;
}

const char* error_thread_name = "Error_Handler";

#define ERROR_THREAD_PRIORITY 254

pok_ret_t pok_error_thread_create (uint32_t stack_size, void* __user entry)
{
    pok_thread_t* t;
    pok_partition_arinc_t* part = current_partition_arinc;

    /**
     * We can create a thread only if the partition is in INIT mode
     */
    if (part->mode == POK_PARTITION_MODE_NORMAL) {
        return POK_ERRNO_PARTITION_MODE;
    }
    
    if(part->thread_error)
        return POK_ERRNO_EXISTS;
    
    if (part->nthreads_used == part->nthreads) {
        return POK_ERRNO_TOOMANY;
    }

    // do at least basic check of entry point
    if (!check_access_exec(entry)) {
        return POK_ERRNO_PARAM;
    }


    t = &part->threads[part->nthreads_used];
    
    strncpy(t->name, error_thread_name, MAX_NAME_LENGTH);
    t->entry = entry;
    t->base_priority = ERROR_THREAD_PRIORITY;
    t->period = POK_TIME_INFINITY;
    t->time_capacity = POK_TIME_INFINITY; // TODO: support deadline for error handler
    t->deadline = DEADLINE_SOFT;
    t->user_stack_size = stack_size;
    
    if(!thread_create(t))
       return POK_ERRNO_TOOMANY; // TODO: Change return code for this case.
   
    part->nthreads_used++;
    
    part->thread_error = t;

    return POK_ERRNO_OK;
}

// Should be called with local preemption disabled.
static void take_fixed_action(pok_error_action_t action)
{
    // TODO in case of restart, fill in restart reason (to be inspected later)

    switch (action) {
        case POK_ERROR_ACTION_IGNORE:
            // TODO not all kinds of errors can be ignored just like that
            return;
        case POK_ERROR_ACTION_IDLE:
            pok_partition_set_mode_current(POK_PARTITION_MODE_IDLE);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        case POK_ERROR_ACTION_COLD_START:
            pok_partition_arinc_reset(current_partition_arinc,
                POK_PARTITION_MODE_INIT_COLD);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        case POK_ERROR_ACTION_WARM_START:
            pok_partition_arinc_reset(current_partition_arinc,
                POK_PARTITION_MODE_INIT_WARM);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        default:
            assert(FALSE && "invalid HM action");
    }
}

/* 
 * Attempt to process error at partition level or above.
 * 
 * Return TRUE or doesn't return at all if error has been processed.
 * 
 * Return FALSE if error should be processed at process level.
 * 
 * Should be called with local preemption disabled.
 */
static pok_bool_t process_error_partition(pok_system_state_t system_state,
    pok_error_id_t error_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_error_action_t action;
    
    switch(system_state)
    {
    case POK_SYSTEM_STATE_USER:
#ifdef POK_NEEDS_ERROR_HANDLING
        if(!pok_error_level_select(part->partition_hm_selector,
            system_state, error_id)
            && part->mode == POK_PARTITION_MODE_NORMAL
            && part->thread_error) return FALSE;
        // Fallback
    case POK_SYSTEM_STATE_ERROR_HANDLER: // This state can only be viewed with error handling enabled.
        // Fallback
#endif /* POK_NEEDS_ERROR_HANDLING */
    case POK_SYSTEM_STATE_OS_PART:
        break;
    default:
        pok_fatal("Unexpected partition state while error");
    }
    
    action = part->partition_hm_table->actions[system_state][error_id];
    
    take_fixed_action(action);
    
    if(part->nthreads_unrecoverable)
    {
        part->nthreads_unrecoverable = 0;
        while(1)
            process_error_partition(POK_SYSTEM_STATE_OS_PART,
                POK_ERROR_ID_UNHANDLED_INT);
    }
    
    return TRUE;
}

/*
 * Set state of the thread as unrecoverable. 
 * 
 * Should be called before emitting error.
 * 
 * Should be called with local preemption disabled.
 */
static void thread_set_unrecoverable(pok_thread_t* thread)
{
    if(!thread->is_unrecoverable)
    {
        thread->is_unrecoverable = TRUE;
        current_partition_arinc->nthreads_unrecoverable++;
    }
}

#ifdef POK_NEEDS_ERROR_HANDLING

/*
 * Set error bit in the thread.
 * 
 * Should be executed with local preemption disabled.
 */
static void thread_add_error_bit(pok_thread_t* thread,
    pok_thread_error_bits_t bit)
{
    assert((thread->error_bits & bit) == 0);
    
    thread->error_bits |= bit;
    
    if(list_empty(&thread->error_elem))
    {
        list_add_tail(&thread->error_elem, &current_partition_arinc->error_list);
        pok_sched_local_invalidate();
    }
}

/* 
 * Emit sync error for non-error-handler thread.
 * 
 * Should be called only for process-level errors.
 * 
 * system_state is POK_SYSTEM_STATE_USER.
 * 
 * Additional information about error should already be filled
 * in accordance with description of POK_THREAD_ERROR_BIT_SYNC.
 */
static void thread_emit_sync_error(pok_thread_t* thread,
    pok_error_id_t error_id, void* __user failed_addr)
{
    pok_preemption_local_disable();
    
    current_partition_arinc->sync_error = error_id;
    current_partition_arinc->sync_error_failed_addr = failed_addr;
    
    thread_add_error_bit(thread, POK_THREAD_ERROR_BIT_SYNC);
    
    pok_preemption_local_enable();
}

#endif /* POK_NEEDS_ERROR_HANDLING */

void pok_partition_arinc_process_error(
    pok_system_state_t partition_state,
    pok_error_id_t error_id,
    uint8_t preempt_local_disabled_old,
    void* failed_address)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    
    // TODO: Take into account `preempt_local_disabled_old`
    
    if(process_error_partition(partition_state, error_id)) return;

#ifdef POK_NEEDS_ERROR_HANDLING
    
    assert(partition_state == POK_SYSTEM_STATE_USER);
    assert(part->thread_current != part->thread_error);
    
    pok_preemption_local_disable();
    
    thread_emit_sync_error(part->thread_current, error_id, (__user void*)failed_address);
    
    pok_preemption_local_enable();
#else
    assert(FALSE); // Unreachable
#endif /* POK_NEEDS_ERROR_HANDLING */
}

void pok_thread_emit_deadline_missed(pok_thread_t* thread)
{
    if(process_error_partition(POK_SYSTEM_STATE_USER,
        POK_ERROR_ID_DEADLINE_MISSED)) return;

#ifdef POK_NEEDS_ERROR_HANDLING    
    thread_add_error_bit(thread, POK_THREAD_ERROR_BIT_DEADLINE);
#else
    assert(FALSE); // Unreachable
#endif /* POK_NEEDS_ERROR_HANDLING */
}

void pok_thread_emit_deadline_oor(pok_thread_t* thread)
{
    thread_set_unrecoverable(thread);

    if(process_error_partition(POK_SYSTEM_STATE_USER,
        POK_ERROR_ID_ILLEGAL_REQUEST)) return;

#ifdef POK_NEEDS_ERROR_HANDLING
    thread_add_error_bit(thread, POK_THREAD_ERROR_BIT_DEADLINE_OOR);
#else
    assert(FALSE); // Unreachable
#endif /* POK_NEEDS_ERROR_HANDLING */
}

#ifdef POK_NEEDS_ERROR_HANDLING
pok_ret_t pok_error_raise_application_error (const char* __user msg, size_t msg_size)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread = part->thread_current;

    pok_system_state_t partition_state;
    pok_message_send_t message_send;
    
    if (msg_size > POK_ERROR_MAX_MSG_SIZE || msg_size == 0) {
        return POK_ERRNO_EINVAL;
    }
    
    if (msg_size && !check_access_read(msg, msg_size))
        return POK_ERRNO_EFAULT;
    
    if(thread == part->thread_error)
    {
        partition_state = POK_SYSTEM_STATE_ERROR_HANDLER;
    }
    else
    {
        partition_state = POK_SYSTEM_STATE_USER;
    }
    
    if(process_error_partition(partition_state, POK_ERROR_ID_APPLICATION_ERROR))
        return POK_ERRNO_OK;

    assert(part->thread_error);
    assert(part->thread_current != part->thread_error);
    
    message_send.size = msg_size;
    message_send.data = msg;
    
    thread->wait_private = &message_send;
    
    pok_preemption_local_disable();
    
    thread_emit_sync_error(thread, POK_ERROR_ID_APPLICATION_ERROR, NULL);
    
    pok_preemption_local_enable();

    return POK_ERRNO_OK;
}

void error_check_after_handler(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    if(part->nthreads_unrecoverable)
    {
        /* 
         * Note, that we do not reset unrecoverable counter.
         * 
         * Such a way partition's error processing mechanism treats
         * error as unrecoverable too.
         */
        process_error_partition(POK_SYSTEM_STATE_ERROR_HANDLER,
            POK_ERROR_ID_UNHANDLED_INT);
    }
}


/* 
 * Fill information about error in user-space structure. 
 * 
 * Should be called with local preemption disabled.
 */
static void copy_error_to_user(pok_error_id_t error_id,
    pok_error_status_t* __user status)
{
     const char* msg = get_error_description(error_id);
     size_t msg_size;
     assert(msg); // TODO: Should be another error raised.

     msg_size = strnlen(msg, POK_ERROR_MAX_MSG_SIZE);
     if(msg_size != POK_ERROR_MAX_MSG_SIZE) msg_size++;

     __copy_to_user(msg, status->msg, msg_size);
     __put_user_f(status, msg_size, msg_size);

    __put_user_f(status, error_kind, get_error_kind(error_id));
}

pok_ret_t pok_error_get (pok_error_status_t* __user status)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* thread;
    pok_thread_error_bits_t error_bits;
    
    void* __user failed_addr = NULL;

    if (part->thread_current != part->thread_error) {
        return POK_ERRNO_THREAD;
    }
   
    if(list_empty(&part->error_list)) return POK_ERRNO_UNAVAILABLE;
   
    if(!check_user_write(status)) return POK_ERRNO_EFAULT;
   
    thread = list_first_entry(&part->error_list, pok_thread_t, error_elem);
   
    error_bits = thread->error_bits;
   
    if(error_bits & POK_THREAD_ERROR_BIT_SYNC)
    {
         // Clear bit
         thread->error_bits = error_bits & ~POK_THREAD_ERROR_BIT_SYNC;
       
         if(part->sync_error == POK_ERROR_ID_APPLICATION_ERROR)
         {
             pok_message_send_t* message_send = thread->wait_private;
           
             __copy_user(message_send->data, &status->msg, message_send->size);
             __put_user_f(status, msg_size, message_send->size);
             __put_user_f(status, error_kind, get_error_kind(POK_ERROR_ID_APPLICATION_ERROR));
         }
         else
         {
             copy_error_to_user(part->sync_error, status);
         }
         
         failed_addr = part->sync_error_failed_addr;
    }
    else if(error_bits & POK_THREAD_ERROR_BIT_DEADLINE)
    {
        // Clear bit
        thread->error_bits = error_bits & ~POK_THREAD_ERROR_BIT_DEADLINE;

        copy_error_to_user(POK_ERROR_ID_DEADLINE_MISSED, status);
    }

    else if(error_bits & POK_THREAD_ERROR_BIT_DEADLINE_OOR)
    {
        // Clear bit
        thread->error_bits = error_bits & ~POK_THREAD_ERROR_BIT_DEADLINE_OOR;

        copy_error_to_user(POK_ERROR_ID_ILLEGAL_REQUEST, status);
    }
    else
    {
        assert(FALSE);
    }
    
    __put_user_f(status, failed_addr, failed_addr);
   
    if(thread->error_bits == 0)
    {
        list_del_init(&thread->error_elem);
    }

    return POK_ERRNO_OK;
}

#endif /* POK_NEEDS_ERROR_HANDLING */

#endif

