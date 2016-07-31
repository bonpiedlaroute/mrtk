/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_basictypes.h"
#include "mrtk_port.h"
#include "mrtk_sync.h"
#include "mrtk.h"
#include <math.h>
#include "mrtk_heap.h"
#include "mrtk_task.h"
#include "mrtk_port_timer.h"
#include "mrtk_sched.h"

extern volatile Uint32* Irq_Start_Stack;

/* the maximum cpu utilization allowed in each core */
static Float32 MaxCpuUtilization = 1.0;

void mrtk_SortProcessors();

/* task to processor mapping table */
Uint16 Task2ProcessorTable[NB_TASK_MAX];

/* processor to task mapping table */
Uint32 Processor2TaskPrioTable[MAX_NB_PROCESSOR];

/* Task per processor allocation during partitioning step */
static Uint16 TaskPerProcAllocation[MAX_NB_PROCESSOR][NB_TASK_MAX] = {0};
/* Number of tasks per processor during the partitioning step */
static Uint16 NumberOfTasksPerProc[MAX_NB_PROCESSOR];

/* processor organization by remaining capacity */
ProcessorCapacity ProcessorCapacityList[MAX_NB_PROCESSOR] = {0};

extern volatile Uint32* TaskToExecuteSP;
extern volatile Uint32* TaskToSleepSP;

/*
 * @brief	This function verify if the task(identify with its id) passes in parameter can be scheduled
 * 			with tasks already allocated to cpuid, such that all tasks meet their deadlines.
 * 			If the resulting tasks set is schedulable, the function returns TRUE, FALSE otherwise.
 *
 * @param
 * 			Uint16*	tasksalreadyallocated 	The task set already allocated
 *			Uint16  size					The size of the task set
 * @return
 * 			TRUE		if the new task set is schedulable
 * 			FALSE		if the new task set is not schedulable
 */
Boolean mrtk_IsEdfSchedulingFeasible(Uint16* tasksalreadyallocated,
		Uint16 size)
{
	Uint32 i = 0, max_margin = 0, margin = 0;
	Float32 limit = 0.0, t = 1.0;
	Boolean demand_overflow = FALSE, utilization_overflow = FALSE;
	Float32 sum = 0.0, utilization = 0.0;

	for( i = 0; i < size; i++ )
	{
	    utilization += (Float32)TaskTcbTable[tasksalreadyallocated[i]].ExecutionTime /
	            (Float32)TaskTcbTable[tasksalreadyallocated[i]].Period;
	}

	if( utilization > MaxCpuUtilization )
	{
	    utilization_overflow = TRUE;
	}
	else
	{
        /* we compute the maximum margin, necessary for computation of time limit
          in which we will verify the schdedulability */
        for( i = 0; i < size; i++ )
        {
            margin = TaskTcbTable[tasksalreadyallocated[i]].Period -
                    TaskTcbTable[tasksalreadyallocated[i]].Deadline;

            if (margin > max_margin )
                max_margin = margin;
        }
        /* we compute the time limit, until when we will verify the schedulability */
        limit = (Float32) ((MaxCpuUtilization / (1.0 - MaxCpuUtilization ) ) * (Float32) max_margin );

        while( t < limit && demand_overflow == FALSE)
        {
            for( i = 0; i < size; i++ )
            {
                sum += (((t - (Float32)TaskTcbTable[tasksalreadyallocated[i]].Deadline) /
                        (Float32)TaskTcbTable[tasksalreadyallocated[i]].Period) + 1.0) *
                                (Float32)TaskTcbTable[tasksalreadyallocated[i]].ExecutionTime;
            }

            if ( sum >= t )
                demand_overflow = TRUE;

            sum = 0.0;
            t = t + 1.0;
        }
	}

    /* was there a utilization overflow or a demand overflow? */
    if( utilization_overflow == TRUE || demand_overflow == TRUE )
        return FALSE;/* yes, return unschedulable */
    else
        return TRUE;/*no, return schedulable */
}

/*
 * @brief
 * 			This function allocates each task in the task set to the different processor, in such a
 * 			manner so that, all tasks complete by their deadlines. The tasklit must be sorted
 * 			according to their relative deadline
 *
 * @param
 * 			none
 * @return
* 			TRUE				if the task set is correctly distributed, and then feasible
* 			FALSE				if the task set cannot be distributed in the differrent processor
* 								so that each task meet their deadlines
 *
 */
Boolean mrtk_partitioned()
{
	Uint16 i = 0, j = 0;
	Boolean found = FALSE;

	for(j = 0; j < MAX_NB_PROCESSOR; j++)
		NumberOfTasksPerProc[j] = 0;

	/* sort tasks to partitioned according to the decreasing capacity */
	mrtk_SortTasksToPartitioned();

	/* we go through all tasks */
	for( i = 0; i < NbTaskToPartitioned; i++ )
	{
		found = FALSE;
		/* we go through all processors*/
		for( j = 0; ( j < NbProcessors) && ( found == FALSE); j++)
		{
			Uint16 cpuid = ProcessorCapacityList[j].processorid;

			TaskPerProcAllocation[cpuid][NumberOfTasksPerProc[cpuid]] = TasksToPartitioned[i].taskid;
			/* can the task be added in this processor ? */
			if( TRUE == mrtk_IsEdfSchedulingFeasible(TaskPerProcAllocation[cpuid], NumberOfTasksPerProc[cpuid] + 1))
			{/* yes */
				NumberOfTasksPerProc[cpuid]++;
				mrtk_InsertInQueue(cpuid, TasksToPartitioned[i].taskid,
						TaskTcbTable[TasksToPartitioned[i].taskid].Deadline, READY_TASKS_QUEUE );
				found = TRUE;

				ProcessorCapacityList[j].total_capacity += (Float32)TaskTcbTable[TasksToPartitioned[i].taskid].ExecutionTime /
						(Float32)TaskTcbTable[TasksToPartitioned[i].taskid].Period;
			}
		}

		if( FALSE == found )
			break;
		else
			mrtk_SortProcessors();/* rearrange the processor according to the total capacity */
	}

	return found;
}
/*
*                mrtk_sched()
*
* @brief
*			This function allows to schedule the task with the nearest deadline.
* @param
*			none
* @return
*			none
*/

void mrtk_sched()
{
    Uint16 cpuid = 0, hghrdytask = 0, runtask = 0;

    /* we disable all interrupts */
    mrtk_begincriticalsection();

    cpuid  = mrtk_GetCpuId();

    hghrdytask = mrtk_GetTaskWithNearestDeadline(cpuid, READY_TASKS_QUEUE);

    runtask = RunningTasks[cpuid];

    if( hghrdytask != runtask )
    {
    	/* we remove the highest priority task in ready queue */
    	mrtk_RemoveFirstElementInQueue(cpuid, READY_TASKS_QUEUE);

		/* we get the stack pointer of each tack*/
		TaskToExecuteSP = (Uint32*)&(TaskTcbTable[hghrdytask].StartStkptr);
		TaskToSleepSP = (Uint32*)&(TaskTcbTable[runtask].StartStkptr);


		/* we re-insert the task in ready queue, if it is an unsollicited preemption */
		if (TaskTcbTable[runtask].TaskId <= NB_USERS_TASK_MAX && TaskTcbTable[runtask].State == EXECUTING )
		{
			TaskTcbTable[runtask].State = READY;
			mrtk_InsertInQueue(cpuid, TaskTcbTable[runtask].TaskId, TaskTcbTable[runtask].AbsoluteDeadline
					, READY_TASKS_QUEUE);
		}

		/* we set the highest ready task state to executing, before performing the context switch */
		if (TaskTcbTable[hghrdytask].TaskId <= NB_USERS_TASK_MAX && TaskTcbTable[hghrdytask].State == READY )
		{
			TaskTcbTable[hghrdytask].State = EXECUTING;
		}
		/* we set the new running task in this processor */
		RunningTasks[cpuid] = hghrdytask;

		ContextSwitch();
    }
    else
    {/* no scheduling is needed!*/
    }
    mrtk_endcriticalsection();
}

/*
*                mrtk_sched_from_isr()
*
* @brief
*			This function allows to schedule the task with the nearest deadline when we are in a interrupt.
* @param
*			none
* @return
*			none
*/

void mrtk_sched_from_isr()
{
    Uint16 cpuid = 0, hghrdytask = 0, runtask = 0;

    /* we disable all interrupts */
    mrtk_begincriticalsection();

    cpuid  = mrtk_GetCpuId();

    hghrdytask = mrtk_GetTaskWithNearestDeadline(cpuid, READY_TASKS_QUEUE);

    runtask = RunningTasks[cpuid];

    if( hghrdytask != runtask )
    {
    	/* we remove the highest priority task in ready queue */
    	mrtk_RemoveFirstElementInQueue(cpuid, READY_TASKS_QUEUE);

		/* we get the stack pointer of each tack*/
		TaskToExecuteSP = (Uint32*)TaskTcbTable[hghrdytask].StartStkptr;
		TaskToSleepSP = (Uint32*)TaskTcbTable[runtask].StartStkptr;
		/* we set the task which is going to sleep, to ready for next run */
		TaskTcbTable[runtask].State = READY;

		/* we re-insert the task in ready queue */
		if (TaskTcbTable[runtask].TaskId > NB_USERS_TASK_MAX )/* is it idle task ?*/
		{/* no need to add idle task */

		}
		else
		{
			mrtk_InsertInQueue(cpuid, TaskTcbTable[runtask].TaskId, TaskTcbTable[runtask].AbsoluteDeadline
					, READY_TASKS_QUEUE);
		}
		/* we set the new running task in this processor */
		RunningTasks[cpuid] = hghrdytask;

		/* context switch from an isr  */
//		SwitchFromIsr();
    }
    else
    {/* no scheduling is needed!*/
    }
    mrtk_endcriticalsection();
}

static void SwapProcessorItems(Uint16 ip_index1, Uint16 ip_index2)
{
    Uint16 Tmp_processorid = 0;
    Float32 Tmp_total_capacity = 0;

    Tmp_processorid = ProcessorCapacityList[ip_index1].processorid;

    Tmp_total_capacity = ProcessorCapacityList[ip_index1].total_capacity;

    ProcessorCapacityList[ip_index1].processorid = ProcessorCapacityList[ip_index2].processorid;

    ProcessorCapacityList[ip_index1].total_capacity = ProcessorCapacityList[ip_index2].total_capacity;

    ProcessorCapacityList[ip_index2].processorid = Tmp_processorid;

    ProcessorCapacityList[ip_index2].total_capacity = Tmp_total_capacity;

    return;

}

/*
 * @brief	Sort processor according to increasing capacity, that is use at partition step, to always choice the
 * 			processor with the lower capacity
 * @param
 * 			none
 * @return
 * 			none
 */
void mrtk_SortProcessors()
{
	Uint16 i = 0, j = 0;

	for(i = 1; i < NbProcessors; i++ )
	{
		j = i;
		while( j > 0 && ProcessorCapacityList[j].total_capacity < ProcessorCapacityList[j-1].total_capacity )
		{
			SwapProcessorItems(j, j-1);
			j--;
		}
	}
}

/*
 * @brief	initialize the maximum cpu usage per core
 *
 * @param	Float32		maximum cpu usage
 *
 * @return	None
 */
void mrtk_InitMaxCpuUsagePerCore(Float32 maxcpuusage)
{
	MaxCpuUtilization = maxcpuusage;
}
