#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

QUEUING_PORT_ID_TYPE QP1, QP2;
SAMPLING_PORT_ID_TYPE SP1;

static void check_ret(RETURN_CODE_TYPE ret)
{
    if (ret != NO_ERROR) {
        printf("error: %d\n", (int) ret);
    }
}

static void first_process(void)
{
    RETURN_CODE_TYPE ret;

    struct {
        unsigned x;
        char message[32];
        unsigned y;
    } __attribute__((packed)) msg;

    msg.x = 0;
    strcpy(msg.message, "test sp message");
    msg.y = -1;

    while (1) {
        WRITE_SAMPLING_MESSAGE(SP1, (MESSAGE_ADDR_TYPE) &msg, sizeof(msg), &ret);
        check_ret(ret);

        msg.x++;
        msg.y--;
        
        TIMED_WAIT(1LL * 1000 * 1000 * 1000 / 20, &ret);
    }
}

static void second_process(void)
{
    struct {
        unsigned x;
        char message[32];
        unsigned y;
    } __attribute__((packed)) msg;

    unsigned last_x = 0;

    while (1) {
        MESSAGE_SIZE_TYPE len;
        RETURN_CODE_TYPE ret;

        RECEIVE_QUEUING_MESSAGE(QP1, INFINITE_TIME_VALUE, &msg, &len, &ret);

        if (ret == NO_ERROR) {
            printf("Received queueing %u %s %u\n", msg.x, msg.message, msg.y);
        } else {
            printf("error %d\n", (int) ret);
        }

        if (msg.x < last_x) {
            printf("warning! received QP message out of order\n");
        } else if (msg.x > last_x + 1) {
            printf("warning: possible QP packet loss\n");
        } else if (msg.x == last_x && last_x != 0) {
            printf("warning: possible QP duplicate message\n");
        }   
        last_x = msg.x;
    }
}

int main(void)
{
    RETURN_CODE_TYPE ret;
    BUFFER_ID_TYPE id;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        return 1;
    }
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return 1;
    }

    // create process 2
    process_attrs.ENTRY_POINT = second_process;
    strncpy(process_attrs.NAME, "process 2", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 2: %d\n", (int) ret);
        return 1;
    }

    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 2: %d\n", (int) ret);
        return 1;
    }

    // create ports
    CREATE_QUEUING_PORT("QP1", 64, 10, DESTINATION, FIFO, &QP1, &ret);
    CREATE_SAMPLING_PORT("SP1", 64, SOURCE, 1000000000LL, &SP1, &ret);

    printf("going to NORMAL mode...\n");

    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    } 

    STOP_SELF();
    return 0;
}
