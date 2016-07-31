/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include <stdio.h>
#include "mrtk.h"
#include "mrtk_sched.h"
#include "mrtk_sync.h"
#include "mrtk_task.h"
#include "mrtk_heap.h"
#include "mrtk_port_timer.h"
#include "mrtk_eventhandler.h"

/* the table of Task's TCB */
TCB TaskTcbTable[NB_TASK_MAX];

/* the lock of the TCB */
Boolean TaskTcbTablelock = FALSE;

/* the table of event*/
EVENT EventTable[NB_EVENT_MAX] = {0};
Uint32 EventTblFreeEntry = 0;

/* Number of processors */
Uint16 NbProcessors = 0;

/* the lock of the mrtktime */
Boolean MrtkTimelock = FALSE;

/* mrk time in ms */
Long64 MrtkTime_ms[MAX_NB_PROCESSOR]={0};

/* mrk idle counter */
Long64 MrtkIdle_counter[MAX_NB_PROCESSOR]={0};

/* idle task function */
STACK IdleTaskContext[MAX_NB_PROCESSOR][TASK_CONTEXT_SIZE];

/* kernel running state */
Boolean MrtkRunning = FALSE;

/* list of tasks running in each processor */
Uint16 RunningTasks[MAX_NB_PROCESSOR];

volatile Uint32* TaskToExecuteSP = NULL;
volatile Uint32* TaskToSleepSP = NULL;



static void mrtk_idletask_cpu(void * args)
{
	Uint16 cpuid = 0;

	for(;;)
	{
		/* we disable all interrupts in this core */
		mrtk_begincriticalsection();

		cpuid = mrtk_GetCpuId();

		/* we enable all interrupts on this core ²*/
		mrtk_endcriticalsection();

		MrtkIdle_counter[cpuid]++;


	}
}

/*
                mrtk_TcbInit
@brief:    this function initialize the overall TCB Table

@param:  
        none

@return :
        none
*/

static void mrtk_TcbInit()
{
    Uint16 i = 0;
    /* we initialize all elements of the TCB table */
    for(i = 0; i < NB_TASK_MAX; i++)
    {
        TaskTcbTable[i].TaskId = 0;
        TaskTcbTable[i].ExecutionTime = 0;
        TaskTcbTable[i].Deadline = 0;
        TaskTcbTable[i].AbsoluteDeadline = 0;
        TaskTcbTable[i].Period = 0;
        TaskTcbTable[i].TaskName[0] = '?';
        TaskTcbTable[i].TaskName[1] = '\0';
        TaskTcbTable[i].State = SLEEPING;
        TaskTcbTable[i].EventList = NULL;
        TaskTcbTable[i].StartStkptr = NULL;
        TaskTcbTable[i].EndStkptr = NULL;
        TaskTcbTable[i].Delay = 0;
    }

}

/*
                mrtk_EventHandlerInit
@brief: this function initialize Event Handler module

@param:
        none
@return:
        none
*/

static void mrtk_EventHandlerInit()
{
    Uint16 i = 0;
    /* We initialize all elements of the events table */
    for(i = 0; i < NB_EVENT_MAX; i++)
    {
        EventTable[i].EventType = NO_EVENT;
        EventTable[i].Data = NULL;
        EventTable[i].EventCounter = 0;
        EventTable[i].TaskId = 0;
        EventTable[i].IsTimeoutEnable = FALSE;
    }
    /* we initialize the nesting interrupts table */
    for(i = 0; i < MAX_NB_PROCESSOR; i++)
    {
    	MrtkInterruptNesting[i] = 0;
    }
}

/*
 * 				mrtk_InitMisc
 * 	@brief Initialization of miscellaneous mrtk variables
 *
 * 	@param		None
 * 	@return		None
 *
 */
static void mrtk_InitMisc()
{
	Uint32 i = 0;

	for(i = 0; i < MAX_NB_PROCESSOR; i++)
	{
		MrtkTime_ms[i] = 0;
		MrtkIdle_counter[i] = 0;
		RunningTasks[i] = 0;
	}

	for(i=0; i < NbProcessors; i++)
	{
		ProcessorCapacityList[i].processorid = i;
		ProcessorCapacityList[i].total_capacity = 0;
	}
}
/*
*                mrtk_Init
* @brief    This function initialize mrtk. This means the initialisation
*                of the number of processor, the TCB, the event table, the
*                global slack factor.
*
* @param [in] Uint16 ip_nbprocessor  the number of processor (2..254)
*
* @return
*        MRTK_KERNEL_INITIALIZED the kernel has been initialized
*        MRTK_KERNEL_ERROR_PARAMETER the number of processor is not in bounds
*
*/

Int16 mrtk_Init(Uint16 ip_nbprocessor, Float32 maxcpuusage)
{
    Int16 r = MRTK_KERNEL_ERROR_PARAMETER;
    Uint32 i = 0;
    TaskProperties attr;
    /* the number of processor is not in bounds ?*/
    if ( ip_nbprocessor < MIN_NB_PROCESSOR || ip_nbprocessor > MAX_NB_PROCESSOR )
    {/* yes */
        /* we return an error */
        r = MRTK_KERNEL_ERROR_PARAMETER;
    }
    else
    {
    	/* we initialize some hardware specific stuff */
    	mrtk_BoardInit();

         /* we set the number of processor */
        NbProcessors = ip_nbprocessor;

        /* Initialize the maximum cpu utilization on each core */
        mrtk_InitMaxCpuUsagePerCore(maxcpuusage);

        /* we initialize the TCB */
        mrtk_TcbInit();

        /* we initialize the event handler module */
        mrtk_EventHandlerInit();

        /* we initialize the priority queue */
        mrtk_QueuesInit();

        /* Initialize miscellaneous OS variables */
        mrtk_InitMisc();

        attr.executiontime = 0x0BADC0DE;
        attr.period = 0x0BADC0DE;
        attr.deadline = 0xFFFFFFFF;
        /* Creation of idle tasks */
        for(i = 0; i < ip_nbprocessor; i++)
        {
            attr.taskid = NB_USERS_TASK_MAX + i;
            mrtk_CreateTask ( &attr, mrtk_idletask_cpu,
                            NULL, IdleTaskContext[i], IdleTaskContext[i] + TASK_CONTEXT_SIZE);
        }

        r = MRTK_KERNEL_INITIALIZED;
    }

    return r;
}

/*
 *                mrtk_start
 *
 *    @brief    This function initialize mrtk. This means the initialization of the number of processor
 *              the TCB, the event table, partitioning the tasks into the different cpu cores.
 *
 *    @param    None
 *    @return   None
 */
void mrtk_start()
{
	Uint16 cpuid = 0, hghrdytask = 0;

	/* we divide tasks between the different cores */
	mrtk_partitioned();

	/* we get the cpu id of the current processor */
	cpuid  = mrtk_GetCpuId();

	hghrdytask = mrtk_GetTaskWithNearestDeadline(cpuid, READY_TASKS_QUEUE);

	/* we remove the task in ready queue */
	mrtk_RemoveFirstElementInQueue(cpuid, READY_TASKS_QUEUE);

	/* we get the stack pointer of the task to execute */
	TaskToExecuteSP = (Uint32*)&(TaskTcbTable[hghrdytask].StartStkptr);

	MrtkRunning = TRUE;

	/* we set the new running task in this processor */
	RunningTasks[cpuid] = hghrdytask;
	TaskTcbTable[hghrdytask].State = EXECUTING;

	/* we initialize system tick */
	mrtk_systick_init();

	/* we start the high priority task */
	Starthghrdyprio();
}
