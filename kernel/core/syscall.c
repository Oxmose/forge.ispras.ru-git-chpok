/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2013-2014, 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include <config.h>

#include <bsp_common.h>
#include <types.h>
#include <libc.h>
#include <gcov.h>
#include <ioports.h>

#include <errno.h>
#include <core/debug.h>
#include <core/syscall.h>
#include <core/partition_arinc.h>
#include <core/thread.h>
#include <core/time.h>
#include <core/error.h>
#include <memory.h>

#include <core/port.h>
#include <uaccess.h>

/* Call given function without protection(with enabled interrupts). */
static pok_ret_t unprotected_syscall(
   pok_ret_t (*f)(const pok_syscall_args_t* args),
   const pok_syscall_args_t*    args)
{
   pok_ret_t ret;
   pok_arch_preempt_enable();
   ret = f(args);
   pok_partition_return_user();
   return ret;
}

#define SYSCALL_ENTRY(id) case id: return unprotected_syscall(pok_syscall_wrapper_ ## id, args); break;

/**
 * \file kernel/core/syscalls.c
 * \brief This file implement generic system calls
 * \author Julien Delange
 */

static inline pok_ret_t pok_core_syscall_internal (const pok_syscall_id_t       syscall_id,
                            const pok_syscall_args_t*    args,
                            const pok_syscall_info_t*    infos)
{
   switch (syscall_id)
   {
#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG)
      case POK_SYSCALL_CONSWRITE:
         // TODO: It should be a call protected with global preemption disabled.
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg1 + infos->base_addr)
         if (pok_cons_write ((const char*)args->arg1 + infos->base_addr, args->arg2))
         {
            return POK_ERRNO_OK;
         }
         else
         {
            return POK_ERRNO_EINVAL;
         }
         break;
#endif

#ifdef POK_NEEDS_PORTS_VIRTUAL
      case POK_SYSCALL_MIDDLEWARE_VIRTUAL_CREATE:
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg1 + infos->base_addr)
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg2 + infos->base_addr)
         return pok_port_virtual_id ( (char*) (args->arg1 + infos->base_addr), (pok_port_id_t*) (args->arg2 + infos->base_addr));
         break;

      case POK_SYSCALL_MIDDLEWARE_VIRTUAL_NB_DESTINATIONS:
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg2 + infos->base_addr)
         return pok_port_virtual_nb_destinations ( (pok_port_id_t) (args->arg1), (uint32_t*) (args->arg2 + infos->base_addr));
         break;

      case POK_SYSCALL_MIDDLEWARE_VIRTUAL_DESTINATION:
         POK_CHECK_PTR_OR_RETURN(infos->partition, ((void*) args->arg3)+infos->base_addr)
         return pok_port_virtual_destination ( (pok_port_id_t) (args->arg1), (uint32_t) (args->arg2), (uint32_t*) (args->arg3 + infos->base_addr));
         break;

      case POK_SYSCALL_MIDDLEWARE_VIRTUAL_GET_GLOBAL:
         POK_CHECK_PTR_OR_RETURN(infos->partition, (void*) (args->arg2 + infos->base_addr))
         return pok_port_virtual_get_global ((pok_port_id_t) (args->arg1), (pok_port_id_t*) (args->arg2 + infos->base_addr));
         break;

#endif

#if defined POK_NEEDS_GETTICK
      case POK_SYSCALL_GETTICK:
         return pok_gettick_by_pointer ((pok_time_t* __user) args->arg1);
         break;
#endif

      SYSCALL_ENTRY(POK_SYSCALL_THREAD_CREATE)
      
#ifdef POK_NEEDS_THREAD_SLEEP
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_SLEEP)
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_SLEEP_UNTIL)
#endif

      SYSCALL_ENTRY(POK_SYSCALL_THREAD_PERIOD)

#if defined (POK_NEEDS_THREAD_SUSPEND) || defined (POK_NEEDS_ERROR_HANDLING)
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_SUSPEND)
#endif

#ifdef POK_NEEDS_THREAD_ID
      SYSCALL_ENTRY(POK_SYSCALL_THREAD_ID)
#endif
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_DELAYED_START)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_SET_PRIORITY)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_RESUME)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_SUSPEND_TARGET)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_YIELD)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_REPLENISH)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_STOP)
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_STOPSELF)
   
   SYSCALL_ENTRY(POK_SYSCALL_THREAD_FIND)

#ifdef POK_NEEDS_PARTITIONS
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_SET_MODE)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_INC_LOCK_LEVEL)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL)
#endif

#ifdef POK_NEEDS_BUFFERS
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BUFFER_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BUFFER_SEND)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BUFFER_RECEIVE)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BUFFER_ID)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BUFFER_STATUS)
#endif
#ifdef POK_NEEDS_BLACKBOARDS
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BLACKBOARD_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BLACKBOARD_READ)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BLACKBOARD_DISPLAY)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BLACKBOARD_CLEAR)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BLACKBOARD_ID)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_BLACKBOARD_STATUS)
#endif
#ifdef POK_NEEDS_SEMAPHORES
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_SEMAPHORE_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_SEMAPHORE_WAIT)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_SEMAPHORE_SIGNAL)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_SEMAPHORE_ID)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_SEMAPHORE_STATUS)
#endif
#ifdef POK_NEEDS_EVENTS
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_EVENT_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_EVENT_SET)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_EVENT_RESET)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_EVENT_WAIT)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_EVENT_ID)
   SYSCALL_ENTRY(POK_SYSCALL_INTRA_EVENT_STATUS)
#endif


#ifdef POK_NEEDS_ERROR_HANDLING
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_HANDLER_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_GET)
//   SYSCALL_ENTRY(POK_SYSCALL_ERROR_IS_HANDLER)
#endif

   /* Middleware syscalls */
#ifdef POK_NEEDS_PORTS_SAMPLING
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_WRITE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_READ)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_ID)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_STATUS)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_CHECK)
#endif /* POK_NEEDS_PORTS_SAMPLING */


#ifdef POK_NEEDS_PORTS_QUEUEING
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_SEND)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_RECEIVE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_ID)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_STATUS)
#endif /* POK_NEEDS_PORTS_QUEUEING */

#ifdef POK_NEEDS_IO
      case POK_SYSCALL_INB:
         if ((args->arg1 < pok_partitions[infos->partition].io_min) ||
             (args->arg1 > pok_partitions[infos->partition].io_max))
         {
            return -POK_ERRNO_EPERM;
         }
         else
         {
            return inb((unsigned short) args->arg1);
         }
         break;

      case POK_SYSCALL_OUTB:
         if ((args->arg1 < pok_partitions[infos->partition].io_min) ||
             (args->arg1 > pok_partitions[infos->partition].io_max))
         {
            return -POK_ERRNO_EPERM;
         }
         else
         {
            outb((unsigned short) args->arg1, (unsigned char) args->arg2);
            return POK_ERRNO_OK;
         }
       break;
#endif /* POK_NEEDS_IO */

      //TODO rewrite this! This two syscall needs to return pok_ret_t!
      case POK_SYSCALL_MEM_VIRT_TO_PHYS:
          if (POK_CHECK_PTR_IN_PARTITION(infos->partition, args->arg1 + infos->base_addr))
              return pok_virt_to_phys(args->arg1);
          else 
              return 0;

      case POK_SYSCALL_MEM_PHYS_TO_VIRT:
      {
          uintptr_t virt = pok_phys_to_virt(args->arg1);
          //This is very strange. But current memory structure forces doing this.
          if (POK_CHECK_PTR_IN_PARTITION(infos->partition, virt + infos->base_addr))
              return virt;
          else
              return 0;
      }

      case POK_SYSCALL_GET_BSP_INFO:
      {
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg1 + infos->base_addr)
         //TODO add check that current partition is system
         pok_bsp_get_info((void *)(args->arg1 + infos->base_addr));
         return 0;
      }

#ifdef POK_NEEDS_GCOV
      case POK_SYSCALL_GCOV_INIT:
          gcov_init_libpok((struct gcov_info**) (args->arg1 + infos->base_addr), (size_t) args->arg2, infos);
          return POK_ERRNO_OK;
      break;
#endif /* POK_NEEDS_GCOV */

      default:
       /*
        * Unrecognized system call ID.
        */
#ifdef POK_NEEDS_ERROR_HANDLING
         //TODO
         //POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_ILLEGAL_REQUEST);
         //pok_sched(); 
#else
         #ifdef POK_NEEDS_DEBUG
            printf ("Tried to use syscall %d\n", syscall_id);
         #endif
         POK_FATAL ("Unknown syscall");
#endif
         break;
   }

   return POK_ERRNO_EINVAL; // TODO: Unreachable?
}

pok_ret_t pok_core_syscall (const pok_syscall_id_t       syscall_id,
                            const pok_syscall_args_t*    args,
                            const pok_syscall_info_t*    infos)
{
    pok_ret_t ret;

    current_partition->entry_sp_user = global_thread_stack;

    pok_in_user_space = FALSE;

    ret = pok_core_syscall_internal(syscall_id, args, infos);

    return ret;

    pok_in_user_space = TRUE;
}
