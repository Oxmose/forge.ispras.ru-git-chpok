#include "wrappers.h"


void entry_point_method(void)
{
	// add some output (print log)
    
    
    // actually this never prints 'cause thread can only be created 
    // if partition state is INIT mode
    printf("the created process is working");  
    
	return;	
}



/*
 * For all wrappers
 * 
 * pos - position of tested param
 * 
 * other _n params are autocompleted
 * 
 * 
 **/


/////////////////////////// THREADS ///////////////////////////
 
/*
pok_ret_t pok_thread_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    char* name = "process1";
    void* entry = entry_point_method;
			
	pok_thread_attr_t attr = {
		.priority = 0,
		//.entry = entry_point_method,
		.period = INFINITE_TIME_VALUE,
		.deadline = SOFT,
		.time_capacity = INFINITE_TIME_VALUE,
		.stack_size = 8096,
	};
		
	pok_thread_id_t* id;
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_thread_create(&param, &entry, &attr, &id); 
			break;
		
		case 1:
            ret = pok_thread_create(&name, &param, &attr, &id);
			break;
        case 2:
            ret = pok_thread_create(&name, &entry, &param, &id);
			break;
	}
	
	return ret;
}


pok_ret_t pok_thread_sleep_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_sleep(&param); 
	
	return ret;
}

pok_ret_t pok_thread_sleep_until_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_sleep_until(&param);
    
    return ret;
}

pok_ret_t pok_thread_suspend_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_suspend(&param); 
	
	return ret;
}

pok_ret_t pok_sched_get_current_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_thread_suspend(&param); 
	
	return ret;
}

// there is something wring, check it
pok_ret_t pok_thread_get_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_thread_id_t thread_id = 0; // TODO created thread needed!!!
    
    
    char* name = "user1";
    void* entry = entry_point_method;
			
	pok_thread_attr_t attr = {
		.priority = 0,
		//.entry = entry_point_method,
		.period = INFINITE_TIME_VALUE,
		.deadline = SOFT,
		.time_capacity = INFINITE_TIME_VALUE,
		.stack_size = 8096,
	};
		
	pok_thread_id_t id;
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_thread_get_status(&param, &entry, &attr, &id); 
			break;
		
		case 1:
            ret = pok_thread_get_status(&name, &param, &attr, &id);
			break;
        case 2:
            ret = pok_thread_get_status(&name, &entry, &param, &id);
			break;
	}
	
	return ret;
}


pok_ret_t pok_thread_delayed_start_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_thread_id_t thread_id = 0; // TODO: create thread (in advance)
    
    ret = pok_thread_delayed_start(thread_id, &param);
    
    return ret;
}

pok_ret_t pok_sched_replenish_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_sched_replenish(&param); 
	
	return ret;
}

pok_ret_t pok_thread_find_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    char* name = "process1"; // TODO: there better should be process with that name
		
	pok_thread_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_thread_find(&param, &id); 
			break;
		
		case 1:
            ret = pok_thread_find(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_current_partition_get_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_current_partition_get_status(&param); 
	
	return ret;
}

pok_ret_t pok_current_partition_inc_lock_level_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_current_partition_inc_lock_level(&param); 
	
	return ret;
}

pok_ret_t pok_current_partition_dec_lock_level_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    ret = pok_current_partition_dec_lock_level(&param); 
	
	return ret;
}
*/

/////////////////////////// BUFFERS ///////////////////////////

pok_ret_t pok_buffer_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    char* name = "buffer1";
    
    pok_message_size_t max_message_size; // TODO: check right values
    pok_message_range_t max_nb_message;
    pok_queuing_discipline_t discipline;
    
    pok_buffer_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_buffer_create(&param, &max_message_size, &max_nb_message, &discipline, &id); 
			break;
		
		case 4:
            ret = pok_buffer_create(&name, &max_message_size, &max_nb_message, &discipline, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_buffer_send_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_buffer_id_t id; // pre-created buffer needed
    
    const void* data;           // TODO: check the right values
    pok_message_size_t length;
    const pok_time_t* timeout;
    
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_buffer_send(&id, &param, &length, &timeout); 
			break;
		
		case 3:
            ret = pok_buffer_send(&id, &data, &length, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_buffer_receive_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_buffer_id_t id; // pre-created buffer needed
    
    const pok_time_t* timeout;
    const void* data;           // TODO: check the right values
    pok_message_size_t* length;
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_buffer_receive(&id, &param, &data, &length); 
			break;
		
		case 2:
			;
			ret = pok_buffer_receive(&id, &timeout, &param, &length);     
			break;
            
        case 3:
			;
			ret = pok_buffer_receive(&id, &timeout, &data, &param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_buffer_get_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    char* name; // pre-created buffer needed (check also with no buffer)
    
    pok_buffer_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_buffer_get_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_buffer_get_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_buffer_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_buffer_id_t id; // pre-created buffer needed (check also with no buffer)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_buffer_status(&id, &param); 
			break;
	}
	
	return ret;                     
}

/*
/////////////////////////// BLACKBOARDS ///////////////////////////

pok_ret_t pok_buffer_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    char* name = "blackboard1";
    
    pok_message_size_t max_message_size; // TODO: check right value
    
    pok_blackboard_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_blackboard_create(&param, &max_message_size, &id); 
			break;
		
		case 2:
            ret = pok_blackboard_create(&name, &max_message_size, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_read_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_blackboard_id_t id; // pre-created blackboard needed
    
    const pok_time_t* timeout; // TODO: check the right values
    void* data;           
    pok_message_size_t* len;
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_blackboard_read(&id, &param, &data, &len); 
			break;
        
        case 2:
			;
			ret = pok_blackboard_read(&id, &timeout, &param, &len); 
			break;
		
		case 3:
            ret = pok_blackboard_read(&id, &timeout, &data, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_display_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_blackboard_id_t id; // pre-created blackboard needed
    
    pok_message_size_t len; // TODO: check the right values
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_blackboard_display(&id, &param, &len); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_clear_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_blackboard_clear(&param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created blackboard needed (check also with no blackboard)
    
    pok_blackboard_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_blackboard_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_blackboard_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_blackboard_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_blackboard_id_t id; // pre-created blackboard needed (check also with no blackboard)
    pok_blackboard_status_t* status; // out param (?)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_blackboard_status(&id, &param); 
			break;
	}
	
	return ret;                     
}

/////////////////////////// SEMAPHORES ///////////////////////////

pok_ret_t pok_semaphore_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name = "semaphore1";
    
    pok_sem_value_t value;       // TODO: check right values
    pok_sem_value_t max_value;
    pok_queuing_discipline_t discipline;
    
    pok_sem_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_semaphore_create(&param, &value, &max_value, &discipline, &id); 
			break;
		
		case 4:
            ret = pok_semaphore_create(&name, &value, &max_value, &discipline, &param);
			break;
	}
	
	return ret;                     
}


pok_ret_t pok_semaphore_wait_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_sem_id_t id; // pre-created sem needed
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_semaphore_wait(&id, &param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_semaphore_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created sem needed (check also with no sem)
    
    pok_sem_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_semaphore_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_semaphore_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_semaphore_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_sem_id_t id; // pre-created sem needed (check also with no sem)
    pok_semaphore_status_t* status; // out param (?)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_semaphore_status(&id, &param); 
			break;
	}
	
	return ret;                     
}

/////////////////////////// EVENTS ///////////////////////////

pok_ret_t pok_event_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name = "event1";
    
    pok_event_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_event_create(&param, &id); 
			break;
		
		case 1:
            ret = pok_event_create(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_event_wait_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_sem_id_t id; // pre-created event needed
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_event_wait(&id, &param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_event_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created event needed (check also with no event)
    
    pok_event_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_event_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_event_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_event_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_event_id_t id; // pre-created event needed (check also with no event)
    pok_event_status_t* status; // out param (?)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_event_status(&id, &param); 
			break;
	}
	
	return ret;                     
}

/////////////////////////// ERROR HANDLING ///////////////////////////

pok_ret_t pok_error_thread_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    uint32_t stack_size; // TODO: check right value
    
	switch (pos)
	{
		case 1:
            ret = pok_error_thread_create(&stack_size, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_error_raise_application_error_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    size_t msg_size; // TODO: check right value
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_error_raise_application_error(&param, &msg_size); 
			break;
	}
	
	return ret;                     
}

// check what this syscall does
pok_ret_t pok_error_get_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_error_status_t* status; // TODO: check right values
    void* msg;
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_error_get(&param, &msg); 
			break;
		
		case 1:
            ret = pok_error_get(&status, &param);
			break;
	}
	
	return ret;                     
}

//////////---////////--- MIDDLEWARE SYSCALLS ---//////////---////////////

/////////////////////////// PORT SAMPLING ///////////////////////////

pok_ret_t pok_port_sampling_create_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name = "port1";
    
    pok_port_size_t size;           // TODO: check right value
    pok_port_direction_t direction;
    const pok_time_t* refresh;
    
    pok_port_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_sampling_create(&param, &size, &direction, &refresh, &id); 
			break;
            
        case 3:
            ret = pok_port_sampling_create(&name, &size, &direction, &param, &id);
			break;
        
		case 4:
            ret = pok_port_sampling_create(&name, &size, &direction, &refresh, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_write_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    pok_port_size_t len; // TODO: check the right value
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_sampling_write(&id, &param, &len); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_read_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    void* data;      // TODO: check the right values
    pok_port_size_t* len; // out??
    pok_bool_t* valid;
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_sampling_read(&id, &param, &len, &valid); 
			break;
        
        case 2:
			;
			ret = pok_port_sampling_read(&id, &data, &param, &valid); 
			break;
		
		case 3:
            ret = pok_blackboard_read(&id, &data, &len, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created port needed (check also with no port)
    
    pok_port_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_sampling_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_port_sampling_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_sampling_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed (check also with no port)
    pok_port_sampling_status_t* status; // out param (?)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_sampling_status(&id, &param); 
			break;
	}
	
	return ret;                     
}


/////////////////////////// PORT QUEUEING ///////////////////////////


pok_ret_t pok_port_queuing_create_packed_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name = "port1";
    
    const pok_port_queuing_create_arg_t* arg;  // TODO: check right value
    
    pok_port_id_t* id; // TODO: check if it is out-param
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_queuing_create_packed(&param, &arg, &id); 
			break;
            
        case 1:
            ret = pok_port_queuing_create_packed(&name, &param, &id);
			break;
        
		case 2:
            ret = pok_port_queuing_create_packed(&name, &arg, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_send_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    const void* data;   // TODO: check the right value
    pok_port_size_t len; 
    const pok_time_t* timeout;
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_queuing_send(&id, &param, &len, &timeout); 
			break;
        
        case 3:
			;
			ret = pok_port_queuing_send(&id, &data, &len, &param); 
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_receive_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed
    
    const pok_time_t* timeout; // TODO: check the right value
    void* data;   
    pok_port_size_t* len; 
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_queuing_receive(&id, &param, &data, &len); 
			break;
        
        case 2:
			;
			ret = pok_port_queuing_receive(&id, &timeout, &param, &len); 
			break;
		
		case 3:
            ret = pok_port_queuing_receive(&id, &timeout, &data, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_id_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    const char* name; // pre-created port needed (check also with no port)
    
    pok_port_id_t* id; // out param (?)
    
	switch (pos)
	{
		case 0:
			;
			ret = pok_port_queuing_id(&param, &id); 
			break;
		
		case 1:
            ret = pok_port_queuing_id(&name, &param);
			break;
	}
	
	return ret;                     
}

pok_ret_t pok_port_queuing_status_wrapper(void* param, int pos)
{
	pok_ret_t ret = 0;
    
    pok_port_id_t id; // pre-created port needed (check also with no port)
    
	switch (pos)
	{
		case 1:
			;
			ret = pok_port_queuing_status(&id, &param); 
			break;
	}
	
	return ret;                     
}

*/
