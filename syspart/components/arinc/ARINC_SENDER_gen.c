/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
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
#include "ARINC_SENDER_gen.h"





      ret_t ARINC_SENDER_call_portA_send(ARINC_SENDER *self, char * arg1, size_t arg2, size_t arg3)
      {
         if (self->out.portA.ops == NULL) {
             printf("WRONG CONFIG: out port portA of component ARINC_SENDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portA.ops->send(self->out.portA.owner, arg1, arg2, arg3);
      }
      ret_t ARINC_SENDER_call_portA_flush(ARINC_SENDER *self)
      {
         if (self->out.portA.ops == NULL) {
             printf("WRONG CONFIG: out port portA of component ARINC_SENDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portA.ops->flush(self->out.portA.owner);
      }


void __ARINC_SENDER_init__(ARINC_SENDER *self)
{

        arinc_sender_init(self);
}

void __ARINC_SENDER_activity__(ARINC_SENDER *self)
{
        arinc_sender_activity(self);
}
