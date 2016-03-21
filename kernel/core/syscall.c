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
 * Created by julien on Wed Oct 21 13:12:27 2009
 */

#include <config.h>

#include <bsp.h>
#include <types.h>
#include <libc.h>
#include <ioports.h>

#include <errno.h>
#include <core/debug.h>
#include <core/syscall.h>
#include <core/partition.h>
#include <core/thread.h>
#include <core/lockobj.h>
#include <core/time.h>
#include <core/error.h>

#include <middleware/port.h>

#define SYSCALL_ENTRY(id) case id: return pok_syscall_wrapper_ ## id(args, infos); break;

/**
 * \file kernel/core/syscalls.c
 * \brief This file implement generic system calls
 * \author Julien Delange
 */

pok_ret_t pok_core_syscall (const pok_syscall_id_t       syscall_id,
                            const pok_syscall_args_t*    args,
                            const pok_syscall_info_t*    infos)
{
   switch (syscall_id)
   {
#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG)
      case POK_SYSCALL_CONSWRITE:
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
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg1 + infos->base_addr)
         return pok_gettick_by_pointer ((uint64_t*) (args->arg1 + infos->base_addr));
         break;
#endif

      case POK_SYSCALL_THREAD_CREATE:
         return pok_partition_thread_create  ((pok_thread_id_t*)  (args->arg1 + infos->base_addr),
                                             (pok_thread_attr_t*) (args->arg2 + infos->base_addr),
                                             (pok_partition_id_t) infos->partition);
         break;

      
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

#ifdef POK_NEEDS_PARTITIONS
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_SET_MODE)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_ID)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_PERIOD)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_DURATION)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_LOCK_LEVEL)
	SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_OPERATING_MODE)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_GET_START_CONDITION)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_INC_LOCK_LEVEL)
   SYSCALL_ENTRY(POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL)
#endif

#ifdef POK_NEEDS_ERROR_HANDLING
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_HANDLER_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_GET)
   SYSCALL_ENTRY(POK_SYSCALL_ERROR_IS_HANDLER)
#endif

         /* Middleware syscalls */
#ifdef POK_NEEDS_PORTS_SAMPLING
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_CREATE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_WRITE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_READ)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_ID)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_SAMPLING_STATUS)
#endif /* POK_NEEDS_PORTS_SAMPLING */


#ifdef POK_NEEDS_PORTS_QUEUEING
      case POK_SYSCALL_MIDDLEWARE_QUEUEING_CREATE: 
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg1 + infos->base_addr)
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg2 + infos->base_addr)
        
         const pok_port_queueing_create_arg_t *arg1 = (const pok_port_queueing_create_arg_t*) (args->arg1 + infos->base_addr);

         return pok_port_queueing_create(
            (const char *) ((uintptr_t)arg1->name + infos->base_addr),
            arg1->message_size,
            arg1->max_nb_message,
            arg1->direction,
            arg1->discipline,
            (pok_port_id_t*)                        (args->arg2 + infos->base_addr)
         );
         break;

   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_SEND)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_RECEIVE)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_ID)
   SYSCALL_ENTRY(POK_SYSCALL_MIDDLEWARE_QUEUEING_STATUS)
#endif /* POK_NEEDS_PORTS_QUEUEING */

#ifdef POK_NEEDS_LOCKOBJECTS
      case POK_SYSCALL_LOCKOBJ_CREATE:
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg2+infos->base_addr)
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg1+infos->base_addr)
         return pok_lockobj_partition_create    ((pok_lockobj_id_t*)    (args->arg1 + infos->base_addr),
                                                (pok_lockobj_attr_t*)   (args->arg2 + infos->base_addr));
         break;

      case POK_SYSCALL_LOCKOBJ_OPERATION:
         if (args->arg2 == 0)
         {
            return pok_lockobj_partition_wrapper ((pok_lockobj_id_t) args->arg1, NULL);
         }
         else
         {
            POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg2 + infos->base_addr)
            return pok_lockobj_partition_wrapper   ((pok_lockobj_id_t) args->arg1,
                                                   (pok_lockobj_lockattr_t*) (args->arg2 + infos->base_addr));
         }
         break;
      case POK_SYSCALL_LOCKOBJ_STATUS:
         POK_CHECK_PTR_OR_RETURN(infos->partition, args->arg2 + infos->base_addr)
         return pok_lockobj_partition_status((pok_lockobj_id_t) args->arg1, 
                                             (pok_lockobj_status_t*) (args->arg2 + infos->base_addr));
         break;
#endif /* POK_NEEDS_LOCKOBJECTS */

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

      default:
       /*
        * Unrecognized system call ID.
        */
#ifdef POK_NEEDS_ERROR_HANDLING
         POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_ILLEGAL_REQUEST);
         pok_sched();
#else
         #ifdef POK_NEEDS_DEBUG
            printf ("Tried to use syscall %d\n", syscall_id);
         #endif
         POK_FATAL ("Unknown syscall");
#endif
         break;
   }

   return POK_ERRNO_EINVAL;
}
