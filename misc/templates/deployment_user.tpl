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

#include <arinc_config.h>

#ifdef POK_NEEDS_ARINC653_BUFFER
// Maximum number of buffers.
size_t arinc_config_nbuffers = {{part.num_arinc653_buffers}};
#endif /* POK_NEEDS_ARINC653_BUFFER */

#ifdef POK_NEEDS_ARINC653_BLACKBOARD
// Maximum number of blackboards. Set in deployment.c
extern size_t arinc_config_nblackboards;
#endif /* POK_NEEDS_ARINC653_BLACKBOARD */

#ifdef POK_NEEDS_ARINC653_SEMAPHORE
// Maximum number of semaphores. Set in deployment.c
extern size_t arinc_config_nsemaphores;
#endif /* POK_NEEDS_ARINC653_SEMAPHORE */

#ifdef POK_NEEDS_ARINC653_EVENT
// Maximum number of events. Set in deployment.c
extern size_t arinc_config_nevents;
#endif /* POK_NEEDS_ARINC653_EVENT */

#if defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD)
// Memory for messages, used by buffers and blackboards.
size_t arinc_config_messages_memory_size = {{part.buffer_data_size + part.blackboard_data_size}};
#endif /* defined(POK_NEEDS_ARINC653_BUFFER) || defined(POK_NEEDS_ARINC653_BLACKBOARD) */

{% include 'arch/' + part.arch + '/deployment_user' %}

{%if part.is_system%}
{% include 'deployment_user_system'%}
{%endif%}
