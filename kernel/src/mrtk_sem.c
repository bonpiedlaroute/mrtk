/*
 *      Copyright (c) 2016 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk.h"
#include "mrtk_sem.h"
#include "mrtk_heap.h"
#include "mrtk_sched.h"
#include "mrtk_port_timer.h"
#include "mrtk_eventhandler.h"


EVENT* mrtk_SemCreate(Uint8 counter)
{
	EVENT* pevent = NULL;
	if( counter == 0 )
	{
		return pevent;
	}

	mrtk_begincriticalsection();
	/* if no free entry in event table, return null pointer */
	if( EventTblFreeEntry == NB_EVENT_MAX )
	{
		mrtk_endcriticalsection();
		return pevent;
	}

	/* pick one free entry on event table */
	pevent = &(EventTable[EventTblFreeEntry]);
	EventTblFreeEntry++;

	mrtk_endcriticalsection();

	pevent->EventType = SEMAPHORE_EVENT;
	pevent->EventCounter = (Uint16)counter;

	return pevent;
}

void mrtk_SemPend(EVENT* pevent, Uint32 timeout_ms, EVENT_ERROR* error)
{
	Uint16 cpuid = 0, currenttask = 0;

	mrtk_begincriticalsection();
	/* reject if event pointer is null */
	if( pevent == NULL )
	{
		*error = ERROR_NULL_EVENT;
		mrtk_endcriticalsection();
		return;
	}
	/* reject if it's not msg queue event */
	if( pevent->EventType != SEMAPHORE_EVENT )
	{
		*error = ERROR_BAD_EVENT_TYPE;
		mrtk_endcriticalsection();
		return;
	}

	/* get the current task'id */
	cpuid  = mrtk_GetCpuId();
	/* do not pend if we are in an interrupt */
	if( MrtkInterruptNesting[cpuid] > 0 )
	{
		*error = ERROR_PENDING_IN_INTERRUPT;
		mrtk_endcriticalsection();
		return;
	}
	/* take semaphore if available */
	if( pevent->EventCounter > 0 )
	{
		pevent->EventCounter--;
		*error = NO_ERROR;
		mrtk_endcriticalsection();
		return;
	}

	currenttask = RunningTasks[cpuid];

	if( timeout_ms == 0 )
	{/* set task's state to waiting if no timeout */
		TaskTcbTable[currenttask].State = WAITING;
		pevent->IsTimeoutEnable = FALSE;
	}
	else
	{/* set task's state to suspended */
		TaskTcbTable[currenttask].State = SUSPENDED;
		TaskTcbTable[currenttask].Delay = timeout_ms;
		pevent->IsTimeoutEnable = TRUE;
	}
	TaskTcbTable[currenttask].AbsoluteDeadline = MrtkTime_ms[cpuid] + (Long64)TaskTcbTable[currenttask].Deadline;
	mrtk_InsertInQueue(cpuid, currenttask, TaskTcbTable[currenttask].AbsoluteDeadline
					, WAITING_TASKS_QUEUE);
	mrtk_endcriticalsection();
	mrtk_sched();
	mrtk_begincriticalsection();
	/* if we are here due to timeout, return null pointer (no msg!) */
	if( pevent->IsTimeoutEnable == TRUE && TaskTcbTable[currenttask].Delay == 0 )
	{
		mrtk_RemoveElementInQueue(cpuid, currenttask, WAITING_TASKS_QUEUE);
		pevent->IsTimeoutEnable = FALSE;
		mrtk_endcriticalsection();
		*error = EVENT_TIMEOUT;
		return;
	}

	mrtk_endcriticalsection();
	*error = NO_ERROR;
}


void mrtk_SemPost(EVENT* pevent, EVENT_ERROR* error)
{
	Uint16 cpuid = 0, taskid = 0;
	/* reject if null event pointer */
	if( pevent == NULL )
	{
		*error = ERROR_NULL_EVENT;
		return;
	}
	/* reject if it's not msg queue event */
	if( pevent->EventType != SEMAPHORE_EVENT )
	{
		*error = ERROR_BAD_EVENT_TYPE;
		return;
	}

	*error = NO_ERROR;

	cpuid = mrtk_GetCpuId();

	mrtk_begincriticalsection();
	/* get the task with the nearest deadline on waiting queue */
	taskid = mrtk_GetTaskWithNearestDeadline(cpuid, WAITING_TASKS_QUEUE);

	/* if there is a task waiting to the semaphore */
	if( taskid < NB_USERS_TASK_MAX )
	{/* find the task, insert it in ready task queue*/
		mrtk_RemoveFirstElementInQueue(cpuid, WAITING_TASKS_QUEUE);
		TaskTcbTable[taskid].AbsoluteDeadline = MrtkTime_ms[cpuid] + (Long64)TaskTcbTable[taskid].Deadline;
		mrtk_InsertInQueue(cpuid, taskid, TaskTcbTable[taskid].AbsoluteDeadline
							, READY_TASKS_QUEUE);
		mrtk_endcriticalsection();
		/* try to schedule */
		mrtk_sched();
		return;
	}
	else
	{
		if( pevent->EventCounter < 0xFFFF )
		{
			pevent->EventCounter++;
			mrtk_endcriticalsection();
			return;
		}
		mrtk_endcriticalsection();
		*error = ERROR_SEM_OVERFLOW;
		return;
	}
}
