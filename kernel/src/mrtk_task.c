/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include <stdio.h>
#include "mrtk.h"
#include "mrtk_sync.h"
#include "mrtk_sched.h"
#include "mrtk_eventhandler.h"
#include "mrtk_task.h"
#include "mrtk_port_timer.h"
/* the task is created */
#define MRTK_TASK_CREATED   1

/* an error in arguments of mrtk_CreatedTask */
#define MRTK_TASK_ERROR_PARAMETER   0

/* the task is already created */
#define MRTK_TASK_ALREADY_CREATED   -1

/* the task is not schedulable with the tasks already created */
#define MRTK_TASK_UNSCHEDULABLE     -2

/* the task has been deleted */
#define MRTK_TASK_DELETED   1

/* the task has already been deleted */
#define MRTK_TASK_ALREADY_DELETED   -1

/* the task is in an interrupt and cannot be deleted */
#define MRTK_TASK_IN_INTERRUPT		-3

/* the task is in another processor and cannot be deleted */
#define MRTK_TASK_IN_ANOTHER_PROCESSOR	-4

/* the default return value */
#define MRTK_TASK_DEFAULT_RETURN_VALUE  10

/* the list of task to schedule */
TaskCapacity TasksToPartitioned[MAX_TASK_ID] = {0};
/* the number of task to schedule */
Uint16 NbTaskToPartitioned = 0;



/*
                mrtk_DeleteTask
@brief:    This function allows to delete a task in mrtk. The task cannot be 
                elected to be schedule on one processor.

@param:
          ip_taskid   the task's id

@return :
        MRTK_TASK_DELETED   The task has been deleted
        MRTK_TASK_ERROR_PARAMETER The priority is not in the bounds (1..255)
        MRTK_TASK_ALREADY_DELETED The task has already been deleted
        MRTK_TASK_IN_INTERRUPT	  The task is in an interrupt, we cannot delete it!

*/

Int16 mrtk_DeleteTask(Uint16 ip_taskid)
{
    Int16 Result = MRTK_TASK_DEFAULT_RETURN_VALUE;

    Uint32 cpuid = 0;
    
    /* we disable all interrupts */
    mrtk_begincriticalsection();
    
    cpuid = mrtk_GetCpuId();

    /* we take the lock for kernel critical section code */


    /* the priority is not in bounds ? */
    if ( ip_taskid >= MAX_TASK_ID )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ERROR_PARAMETER;
    }

    /* the task is already deleted ?*/
    if ( TaskTcbTable[ip_taskid].State == SLEEPING )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ALREADY_DELETED;
    }

    /* are we in an interrupt ? */
    if( MrtkInterruptNesting [cpuid] > 0)
    {
    	/* we report an error */
    	Result = MRTK_TASK_IN_INTERRUPT;
    }

    /* @todo verify that we are not deleting the idle task running in this processor */
    /* is there any error ? */
    if ( Result == MRTK_TASK_DEFAULT_RETURN_VALUE)
    {/* no */
        switch(TaskTcbTable[ip_taskid].State)
        {
        	case READY:
        	{
        		/* we remove the task in ready queue */


				Result = MRTK_TASK_DELETED;

        		break;
        	}
        	case EXECUTING:
        	{
        		if( ip_taskid == Processor2TaskPrioTable[cpuid])
        		{
        			/* we remove the task in running queue */


					Result = MRTK_TASK_DELETED;
        		}
        		else
        			Result = MRTK_TASK_IN_ANOTHER_PROCESSOR;

        		break;
        	}
        	default:
        	{
        		break;
        	}
        }

        if( MRTK_TASK_DELETED == Result )
        {
			TaskTcbTable[ip_taskid].TaskId = 0;
			TaskTcbTable[ip_taskid].TaskName[0] = '?';
			TaskTcbTable[ip_taskid].TaskName[1] = '\0';
			TaskTcbTable[ip_taskid].ExecutionTime = 0;
			TaskTcbTable[ip_taskid].Deadline = 0;
			TaskTcbTable[ip_taskid].Period = 0;
			/*TaskTcbTable[ip_taskid].TaskType[0] = '?';
			TaskTcbTable[ip_taskid].TaskType[1] = '\0';*/
			TaskTcbTable[ip_taskid].State = SLEEPING;
			TaskTcbTable[ip_taskid].EventList = NULL;
			TaskTcbTable[ip_taskid].StartStkptr = NULL;
			TaskTcbTable[ip_taskid].EndStkptr = NULL;

			/* we free the lock for kernel critical section code */

			/*is the kernel running ? */
			if( TRUE == MrtkRunning )
				mrtk_sched();
        }
        else
        {
        	/* we free the lock for kernel critical section code */

        }
    }
    else
    {
        /* we free the lock for kernel critical section code */

    }

    /* we disable all interrupts */
    mrtk_endcriticalsection();

    return Result;
}

/*
                mrtk_CreateTask

@brief:    This function allows to create a new task in mrtk. The TCB
                of the task is initialized. Each task must have an unique 
                priority and the deadline of the task should be less than his 
                period. When the task is created mrtk decides on which 
                processor the task is executed. mrtk must be initialized
@param :
    TaskProperties* att   The task id between 1..255
    void (*fct)(void *arg)    pointer to the task code
    void *arg    pointer to data passed to the task when it is first invokes
    STACK *ip_startstack  start address of the task's stack
    STACK *ip_endstack    end address of the task's stack

@return :
    MRTK_TASK_CREATED if the task is created correctly
    MRTK_TASK_ERROR_PARAMETER if there is an error in input parameter
    MRTK_TASK_ALREADY_CREATED if the task was already created, in this case 
                                no other task is created!
    MRTK_TASK_UNSCHEDULABLE if the task is not schedulable with the tasks already created,
                            the new task is not created in this case!
*/

Int16 mrtk_CreateTask (TaskProperties* task_prop, void (*fct)(void *arg),
		void *arg, STACK *ip_startstack, STACK *ip_endstack)
{
    Int16 Result = MRTK_TASK_DEFAULT_RETURN_VALUE;
    STACK *TopOfStack;

    /* we disable all interrupts */
    mrtk_begincriticalsection();

    /* the priority is not in the bounds ? */
    if ( task_prop->taskid >= MAX_TASK_ID )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ERROR_PARAMETER;
    }

    /* the task's code pointer, the start stack pointer and end stack pointer  
     are not valid ? */
    if ( fct == NULL || ip_startstack == NULL )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ERROR_PARAMETER;
    }

    /* is the task already created ? */
    if ( Result == MRTK_TASK_DEFAULT_RETURN_VALUE && TaskTcbTable[task_prop->taskid].State != SLEEPING )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ALREADY_CREATED;
    }

    /* we initialize the context of the task */
    TopOfStack = mrtk_taskcontextinit(fct,ip_startstack, ip_endstack, NULL);

    /* is there an error ?*/
    if ( Result == MRTK_TASK_DEFAULT_RETURN_VALUE )
    {/* no */
        TaskTcbTable[task_prop->taskid].TaskId = task_prop->taskid;
        TaskTcbTable[task_prop->taskid].ExecutionTime = task_prop->executiontime;
        TaskTcbTable[task_prop->taskid].Deadline = task_prop->deadline;
        TaskTcbTable[task_prop->taskid].AbsoluteDeadline = task_prop->deadline;
        TaskTcbTable[task_prop->taskid].Period = task_prop->period;
        TaskTcbTable[task_prop->taskid].State = READY;
        TaskTcbTable[task_prop->taskid].EventList = EventTable;
        TaskTcbTable[task_prop->taskid].StartStkptr = TopOfStack;
        TaskTcbTable[task_prop->taskid].EndStkptr = ip_endstack;

        if( task_prop->taskid < (NB_TASK_MAX - MAX_NB_PROCESSOR) )
        {
			TasksToPartitioned[NbTaskToPartitioned].taskid = task_prop->taskid;
			TasksToPartitioned[NbTaskToPartitioned].capacity = (Float32) TaskTcbTable[task_prop->taskid].ExecutionTime /
					(Float32) TaskTcbTable[task_prop->taskid].Period;
			NbTaskToPartitioned++;
        }
        Result = MRTK_TASK_CREATED;
    }
    else
    {

    }

    /* we disable all interrupts */
    mrtk_endcriticalsection();


    /* we return the result of the operation */
    return Result;
}


/*
 * @todo this function allows to create tasks when the kernel is already running
 *
 */
Int16 mrtk_CreateTaskGeneralized (TaskProperties* task_prop, void (*fct)(void *arg),
		void *arg, STACK *ip_startstack, STACK *ip_endstack)
{
    Int16 Result = MRTK_TASK_DEFAULT_RETURN_VALUE;


    /* we disable all interrupts */
    mrtk_begincriticalsection();
    /* we take the lock for kernel critical section code */


    /* the priority is not in the bounds ? */
    if ( task_prop->taskid >= MAX_TASK_ID )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ERROR_PARAMETER;
    }

    /* the task's code pointer, the start stack pointer and end stack pointer
     are not valid ? */
    if ( fct == NULL || ip_startstack == NULL )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ERROR_PARAMETER;
    }

    /* is the task already created ? */
    if ( Result == MRTK_TASK_DEFAULT_RETURN_VALUE && TaskTcbTable[task_prop->taskid].State != SLEEPING )
    {/* yes */
        /* we report an error */
        Result = MRTK_TASK_ALREADY_CREATED;
    }

    /* we initialize the context of the task */
    mrtk_taskcontextinit(fct,ip_startstack, ip_endstack, NULL);;

    /* is there an error ?*/
    if ( Result == MRTK_TASK_DEFAULT_RETURN_VALUE )
    {/* no */
        TaskTcbTable[task_prop->taskid].TaskId = task_prop->taskid;
        TaskTcbTable[task_prop->taskid].ExecutionTime = task_prop->executiontime;
        TaskTcbTable[task_prop->taskid].Deadline = task_prop->deadline;
        TaskTcbTable[task_prop->taskid].Period = task_prop->period;
        TaskTcbTable[task_prop->taskid].State = READY;
        TaskTcbTable[task_prop->taskid].EventList = EventTable;
        TaskTcbTable[task_prop->taskid].StartStkptr = ip_startstack;
        TaskTcbTable[task_prop->taskid].EndStkptr = ip_endstack;

        /* we free the lock for kernel critical section code */


        /* 1. select the processor where the tasks will execute and respect its deadline */
        /* 2. insert the task in the ready queue of the selected processor */
        /*is the kernel running ? */
        if( TRUE == MrtkRunning )
            mrtk_sched();

        Result = MRTK_TASK_CREATED;
    }
    else
    {
        /* we free the lock for kernel critical section code */

    }

    /* we disable all interrupts */
    mrtk_endcriticalsection();


    /* we return the result of the operation */
    return Result;
}

/*
            SwapQueueItems

@brief: This function performs the swap between item on partition table with
             index1 and item with index2.
Argument:
        ip_index1   the first item index
        ip_index2  the second item index

@return:

*/
static void SwapPartitionItems(Uint16 ip_index1, Uint16 ip_index2)
{
    Uint16 Tmp_taskid = 0;
    Float32 Tmp_capacity = 0;

    Tmp_taskid = TasksToPartitioned[ip_index1].taskid;

    Tmp_capacity = TasksToPartitioned[ip_index1].capacity;

    TasksToPartitioned[ip_index1].taskid = TasksToPartitioned[ip_index2].taskid;

    TasksToPartitioned[ip_index1].capacity = TasksToPartitioned[ip_index2].capacity;

    TasksToPartitioned[ip_index2].taskid = Tmp_taskid;

    TasksToPartitioned[ip_index2].capacity = Tmp_capacity;

    return;

}

/*
*            mrtk_SortTaskToPartitioned
*
* @brief    allows to sort tasks to partitioned according to decreasing capacity
*
*
* @param  	None
*
* @return
*		 	None
*/

void mrtk_SortTasksToPartitioned()
{
	Uint16 i = 0, j = 0;

	for(i = 1; i < NbTaskToPartitioned; i++ )
	{
		j = i;
		while( j > 0 && TasksToPartitioned[j].capacity > TasksToPartitioned[j-1].capacity )
		{
			SwapPartitionItems(j, j-1);
			j--;
		}
	}
}
