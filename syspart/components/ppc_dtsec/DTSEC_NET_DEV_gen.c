/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/ppc_dtsec/config.yaml).
 */
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
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
 */

#include <lib/common.h>
#include "DTSEC_NET_DEV_gen.h"



    static ret_t __wrapper_dtsec_send_frame(self_t *arg0, char * arg1, size_t arg2, size_t arg3, size_t arg4)
    {
        return dtsec_send_frame((DTSEC_NET_DEV*) arg0, arg1, arg2, arg3, arg4);
    }

    static ret_t __wrapper_dtsec_flush_send(self_t *arg0)
    {
        return dtsec_flush_send((DTSEC_NET_DEV*) arg0);
    }



      ret_t _DTSEC_NET_DEV_call_portB_handle_impl(DTSEC_NET_DEV *self, const uint8_t * arg1, size_t arg2)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component DTSEC_NET_DEV was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->handle(self->out.portB.owner, arg1, arg2);
      }
      ret_t DTSEC_NET_DEV_call_portB_handle(DTSEC_NET_DEV *self, const uint8_t * arg1, size_t arg2)
      __attribute__ ((weak, alias ("_DTSEC_NET_DEV_call_portB_handle_impl")));

 pok_ret_t DTSEC_NET_DEV_get_memory_block_status(
         DTSEC_NET_DEV *self,
         const char *name,
         jet_memory_block_status_t *mb_status)
 {
     char full_name[30]; //use MAX_NAME_LENGTH instead??
     snprintf(full_name, 30, "%s_%s", self->instance_name, name);
     return jet_memory_block_get_status(full_name, mb_status);
 }

void __DTSEC_NET_DEV_init__(DTSEC_NET_DEV *self)
{
            self->in.portA.ops.send = __wrapper_dtsec_send_frame;
            self->in.portA.ops.flush = __wrapper_dtsec_flush_send;

        dtsec_component_init(self);
}

void __DTSEC_NET_DEV_activity__(DTSEC_NET_DEV *self)
{
        dtsec_receive_activity(self);
}
