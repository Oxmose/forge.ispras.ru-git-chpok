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


#ifdef POK_NEEDS_ARINC653_PROCESS

#include <core/dependencies.h>

#include <core/thread.h>
#include <arinc653/arincutils.h>
#include <arinc653/types.h>
#include <arinc653/process.h>
#include <libc/string.h>

#include <utils.h>
#include <core/partition.h>

#define MAP_ERROR(from, to) case (from): *return_code = (to); break
#define MAP_ERROR_DEFAULT(to) default: *return_code = (to); break

#define CHECK_PROCESS_ID() \
    if (process_id == 0) { \
        *return_code = INVALID_PARAM; \
        return; \
    }

void GET_PROCESS_ID(
    PROCESS_NAME_TYPE process_name,
    PROCESS_ID_TYPE   *process_id,
    RETURN_CODE_TYPE  *return_code)
{
    pok_thread_id_t id;
    if (get_process_id(process_name, &id)) {
        *process_id = id + 1;
        *return_code = NO_ERROR;
    } else {
        *return_code = INVALID_CONFIG;  
    }
}

void GET_MY_ID (PROCESS_ID_TYPE   *process_id,
		RETURN_CODE_TYPE  *return_code )
{
    pok_ret_t core_ret;
    pok_thread_id_t thread_id;

    core_ret = pok_thread_id(&thread_id);
    if (core_ret != POK_ERRNO_OK) {
        *return_code = INVALID_MODE;
    } else {
        *process_id = thread_id + 1;
	*return_code = NO_ERROR;
    }
}

void GET_PROCESS_STATUS (
    PROCESS_ID_TYPE     process_id,
    PROCESS_STATUS_TYPE *process_status,
    RETURN_CODE_TYPE    *return_code)
{
    pok_thread_status_t status;
    pok_ret_t           core_ret;

    CHECK_PROCESS_ID();
	
    core_ret = pok_thread_status(process_id - 1, &status);

    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR_DEFAULT(INVALID_PARAM);

    }

    if (core_ret != POK_ERRNO_OK) {
        return;
    }

    process_status->DEADLINE_TIME = ms_to_arinc_time(status.deadline_time);
#define MAP_STATUS(from, to) case (from): process_status->PROCESS_STATE = (to); break
    switch (status.state) {
        MAP_STATUS(POK_STATE_STOPPED, DORMANT);
        MAP_STATUS(POK_STATE_RUNNABLE, READY);
        MAP_STATUS(POK_STATE_WAITING, WAITING);
        MAP_STATUS(POK_STATE_LOCK, WAITING);
        MAP_STATUS(POK_STATE_WAIT_NEXT_ACTIVATION, WAITING);
        MAP_STATUS(POK_STATE_DELAYED_START, WAITING);
    }
    if (status.suspended) {
        process_status->PROCESS_STATE = WAITING;
    }
#undef MAP_STATUS
    strcpy(process_status->ATTRIBUTES.NAME, arinc_process_attribute[process_id - 1].NAME);
    process_status->ATTRIBUTES.BASE_PRIORITY = status.attributes.priority;
    if (status.attributes.deadline == DEADLINE_SOFT) {
        process_status->ATTRIBUTES.DEADLINE = SOFT;
    } else {
        process_status->ATTRIBUTES.DEADLINE = HARD;
    }
    process_status->CURRENT_PRIORITY = status.current_priority;
    process_status->ATTRIBUTES.PERIOD = ms_to_arinc_time(status.attributes.period);
    process_status->ATTRIBUTES.TIME_CAPACITY = ms_to_arinc_time(status.attributes.time_capacity);
    process_status->ATTRIBUTES.ENTRY_POINT = status.attributes.entry;
    process_status->ATTRIBUTES.STACK_SIZE = status.attributes.stack_size;
}

void CREATE_PROCESS (
    PROCESS_ATTRIBUTE_TYPE  *attributes,
    PROCESS_ID_TYPE         *process_id,
    RETURN_CODE_TYPE        *return_code)
{
    pok_thread_attr_t core_attr;
    pok_ret_t         core_ret;
    pok_thread_id_t   core_process_id;

    if (attributes->NAME[0] == '\0') {
        // empty names are not allowed
        *return_code = INVALID_PARAM;
        return;
    }

    if (get_process_id(attributes->NAME, NULL)) {
        // already created
        *return_code = NO_ACTION;
        return;
    }
    if (attributes->BASE_PRIORITY > MAX_PRIORITY_VALUE || 
        attributes->BASE_PRIORITY < MIN_PRIORITY_VALUE)
    {
        *return_code = INVALID_PARAM;
        return;
    }
    core_attr.priority        = (uint8_t) attributes->BASE_PRIORITY;
    core_attr.entry           = attributes->ENTRY_POINT;
    core_attr.period          = arinc_time_to_ms(attributes->PERIOD);
    if (attributes->DEADLINE == SOFT) {
        core_attr.deadline = DEADLINE_SOFT;
    } else if (attributes->DEADLINE == HARD) {
        core_attr.deadline = DEADLINE_HARD;
    } else {
        *return_code = INVALID_PARAM;
        return;
    }
    core_attr.time_capacity   = arinc_time_to_ms(attributes->TIME_CAPACITY);
    core_attr.stack_size      = attributes->STACK_SIZE;

    core_ret = pok_thread_create (&core_process_id, &core_attr);
    if (core_ret == POK_ERRNO_OK) {
        arinc_process_attribute[core_process_id].BASE_PRIORITY = attributes->BASE_PRIORITY;
        strcpy(arinc_process_attribute[core_process_id].NAME, attributes->NAME);
        
        *process_id = core_process_id + 1;
        // ARINC specifies that threads shall be created in the DORMANT state
        pok_thread_stop(core_process_id);
    }
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_PARAM, INVALID_CONFIG);
        MAP_ERROR(POK_ERRNO_TOOMANY, INVALID_CONFIG);
        MAP_ERROR(POK_ERRNO_PARTITION_MODE, INVALID_MODE);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

void STOP_SELF ()
{
    pok_thread_stop_self ();
}


void SET_PRIORITY (
    PROCESS_ID_TYPE  process_id,
    PRIORITY_TYPE    priority,
    RETURN_CODE_TYPE *return_code)
{
    CHECK_PROCESS_ID();

    if (priority < MIN_PRIORITY_VALUE || priority > MAX_PRIORITY_VALUE) {
        *return_code = INVALID_PARAM;
        return;
    }

    pok_ret_t core_ret = pok_thread_set_priority(process_id - 1, priority);
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_MODE);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

void SUSPEND_SELF (
    SYSTEM_TIME_TYPE time_out,
    RETURN_CODE_TYPE *return_code)
{
    pok_ret_t core_ret = pok_thread_suspend(arinc_time_to_ms(time_out));

    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_TIMEOUT, TIMED_OUT);
        MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }

}

void SUSPEND (
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *return_code)
{
    CHECK_PROCESS_ID();

    pok_ret_t core_ret = pok_thread_suspend_target(process_id - 1);
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
        MAP_ERROR(POK_ERRNO_UNAVAILABLE, NO_ACTION);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

void RESUME (
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *return_code)
{
    CHECK_PROCESS_ID();

    pok_ret_t core_ret = pok_thread_resume(process_id - 1);
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_UNAVAILABLE, NO_ACTION);
        MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

void START (
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *return_code)
{
    DELAYED_START(process_id, 0, return_code);
}

void STOP(
    PROCESS_ID_TYPE     process_id,
    RETURN_CODE_TYPE    *return_code)
{
    CHECK_PROCESS_ID();

    pok_ret_t core_ret = pok_thread_stop(process_id - 1);
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_UNAVAILABLE, NO_ACTION);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

void DELAYED_START(
    PROCESS_ID_TYPE   process_id,
    SYSTEM_TIME_TYPE  delay_time,
    RETURN_CODE_TYPE *return_code)
{
    CHECK_PROCESS_ID();

    pok_ret_t core_ret = pok_thread_delayed_start(process_id - 1, arinc_time_to_ms(delay_time));
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_UNAVAILABLE, NO_ACTION);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

void LOCK_PREEMPTION (LOCK_LEVEL_TYPE *LOCK_LEVEL, RETURN_CODE_TYPE *return_code)
{
    pok_ret_t ret = pok_partition_inc_lock_level(LOCK_LEVEL);
    switch (ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_MODE, NO_ACTION);
        MAP_ERROR(POK_ERRNO_EINVAL, INVALID_CONFIG); // yes, it's an error here...
        MAP_ERROR_DEFAULT(INVALID_PARAM); // shouldn't happen
    }
}

void UNLOCK_PREEMPTION (LOCK_LEVEL_TYPE *LOCK_LEVEL, RETURN_CODE_TYPE *return_code)
{
    pok_ret_t ret = pok_partition_dec_lock_level(LOCK_LEVEL);
    switch (ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_MODE, NO_ACTION);
        MAP_ERROR(POK_ERRNO_EINVAL, NO_ACTION); // ...but here it's just NO_ACTION
        MAP_ERROR_DEFAULT(INVALID_PARAM); // shouldn't happen
    }
}

#endif
