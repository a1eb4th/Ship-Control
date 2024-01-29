#include "os.h"

#include "architecture.h"

#include "simulation_parameters.h"

#include <stddef.h>

PREADYQUEUE tasks_queue_X=NULL;
PSCHEDULER scheduler_X=NULL;

PREADYQUEUE tasks_queue_Y=NULL;
PSCHEDULER scheduler_Y=NULL;

PREADYQUEUE tasks_queue_Z=NULL;
PSCHEDULER scheduler_Z=NULL;

// Put all the architecture related initialization code here
int init_architecture()
{
    Readyqueue_create(&tasks_queue_X);
    Scheduler_create(&scheduler_X, SCHEDULING_EARLIEST_DEADLINE_FIRST, tasks_queue_X,1);
    
    Readyqueue_create(&tasks_queue_Y);
    Scheduler_create(&scheduler_Y, SCHEDULING_EARLIEST_DEADLINE_FIRST, tasks_queue_Y,1);
    
    Readyqueue_create(&tasks_queue_Z);
    Scheduler_create(&scheduler_Z, SCHEDULING_EARLIEST_DEADLINE_FIRST, tasks_queue_Z,1);
    return 0;
}
