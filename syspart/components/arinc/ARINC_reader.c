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
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>
#include <stdio.h>

#include <port_info.h>

#include <smalloc.h>

#include "ARINC_PORT_READER_gen.h"
#define C_NAME "ARINC_PORT_READER: "

#define SECOND 1000000000LL
#define NO_WAIT_TIME 0

#define SAMPLING_REFRESH_PERIOD SECOND


static int receive_msg_queuing(ARINC_PORT_READER *self)
{
    RETURN_CODE_TYPE ret;
    RECEIVE_QUEUING_MESSAGE(
            self->state.port_id,
            NO_WAIT_TIME,
            (MESSAGE_ADDR_TYPE ) (self->state.port_buffer + self->state.prepend_overhead),
            &self->state.message_size,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NOT_AVAILABLE)
            printf(C_NAME"%s port error: %u\n", self->state.port_name, ret);
        return -1;
    }
    return 0;
}

static int receive_msg_samping(ARINC_PORT_READER *self)
{
    RETURN_CODE_TYPE ret;
    VALIDITY_TYPE   validity;

    //~ printf("SYS_CHECK: %d\n", SYS_SAMPLING_PORT_CHECK_IS_NEW_DATA(self->state.port_id));
    if (SYS_SAMPLING_PORT_CHECK_IS_NEW_DATA(self->state.port_id))
        return -1;

    READ_SAMPLING_MESSAGE(
            self->state.port_id,
            (MESSAGE_ADDR_TYPE ) (self->state.port_buffer + self->state.prepend_overhead),
            &self->state.message_size,
            &validity,
            &ret
            );

    if (ret != NO_ERROR) {
        if (ret != NO_ACTION)
            printf(C_NAME"%s port error: %u\n", self->state.port_name, ret);
        return -1;
    }

    return 0;
}

void arinc_port_reader_activity(ARINC_PORT_READER *self)
{
    int receive_error;
    if (self->state.is_queuing_port)
        receive_error = receive_msg_queuing(self);
    else
        receive_error = receive_msg_samping(self);

    if (receive_error != 0)
        return;

    ret_t res = ARINC_PORT_READER_call_portA_send(self,
            self->state.port_buffer + self->state.prepend_overhead,
            self->state.message_size,
            self->state.prepend_overhead,
            self->state.append_overhead
            );

    if (res != EOK)
        printf(C_NAME"Error in send_udp\n");

    ARINC_PORT_READER_call_portA_flush(self);
}

void arinc_port_reader_init(ARINC_PORT_READER *self)
{
    RETURN_CODE_TYPE ret;
    if (self->state.is_queuing_port) {
        CREATE_QUEUING_PORT(
                self->state.port_name,
                self->state.port_max_message_size,
                self->state.q_port_max_nb_messages,
                self->state.port_direction,
                FIFO,
                &self->state.port_id,
                &ret);
    } else {
        CREATE_SAMPLING_PORT(
                self->state.port_name,
                self->state.port_max_message_size,
                self->state.port_direction,
                SAMPLING_REFRESH_PERIOD, //in future should be any positive number
                &self->state.port_id,
                &ret);
    }

    if (ret != NO_ERROR) {
        printf(C_NAME"error %d during creation %s port\n", ret, self->state.port_name);
        return;
    }

    printf(C_NAME"successfuly create %s port\n", self->state.port_name);

    self->state.port_buffer = smalloc(
            self->state.prepend_overhead +
            self->state.port_max_message_size +
            self->state.append_overhead);
}