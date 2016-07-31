/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_msgqueue.h"
#include "mrtk_port_timer.h"
#include "mrtk_sched.h"
#include "mrtk_heap.h"
#include "mrtk_eventhandler.h"

/* Msg Queue Control Block, use to manipulate storage provided, as circular buffer */
MQueueControlBlock MrtkMsgQueues[MRTK_MAX_MSG_QUEUE];
Uint32 MsgQueueFreeEntry = 0;

/*
 * @brief	create a msg queue event
 *
 * @param [in]	void **qbegin	an array of pointer to void*, each element of the array is a pointer
 * 								to a message
 * @param [in]  Uint16 size		size of the array
 *
 * @return 		EVENT*			pointer to the newly create event, NULL if an error occurs
 */
EVENT* mrtk_Qcreate(void ** qbegin, Uint16 size)
{
	EVENT* pevent = NULL;
	MQueueControlBlock* pqueue;

	mrtk_begincriticalsection();
	/* if no more free entry n msg queue or event table, return NULL */
	if( MsgQueueFreeEntry == MRTK_MAX_MSG_QUEUE || EventTblFreeEntry == NB_EVENT_MAX )
	{
		mrtk_endcriticalsection();
		return (EVENT *) pevent;
	}

	/* pick one free entry on event table */
	pevent = &(EventTable[EventTblFreeEntry]);
	EventTblFreeEntry++;
	/* pick one free entry on msg queue table */
	pqueue = &(MrtkMsgQueues[MsgQueueFreeEntry]);
	MsgQueueFreeEntry++;

	/* Initialize msg queue data structure with storage provided */
	pqueue->QStart = qbegin;
	pqueue->QEnd = &qbegin[size];
	pqueue->QIn = qbegin;
	pqueue->QOut = qbegin;
	pqueue->QSize = size;
	pqueue->QEntries = 0;
	/* Initialize  event data structure */
	pevent->EventType = MSG_QUEUE_EVENT;
	pevent->EventCounter = 0;
	pevent->TaskId = 0;
	pevent->Data = (void*) pqueue;
	pevent->IsTimeoutEnable = FALSE;

	mrtk_endcriticalsection();
	return pevent;
}

/*
 * @brief	wait for a msg, to be posted on msg queue
 *
 * @param [in]	EVENT* pevent		event that the task is waiting for
 * @param [in]  Uint32 timeout_ms	the task is released if no event until this timeout
 * 									if its value is zero, the task will wait until the event
 * 									occurs
 *
 * @param [out]	EVENT_ERROR* error	error status
 *
 */
void* mrtk_Qpend(EVENT* pevent, Uint32 timeout_ms, EVENT_ERROR* error)
{
	MQueueControlBlock* pqueue;
	Uint16 cpuid = 0, currenttask = 0;

	void* msg;
	/* reject if event pointer is null */
	if( pevent == NULL )
	{
		*error = ERROR_NULL_EVENT;
		return NULL;
	}
	/* reject if it's not msg queue event */
	if( pevent->EventType != MSG_QUEUE_EVENT )
	{
		*error = ERROR_BAD_EVENT_TYPE;
		return NULL;
	}
	mrtk_begincriticalsection();
	/* get the current task'id */
	cpuid  = mrtk_GetCpuId();
	/* do not pend if we are in an interrupt */
	if( MrtkInterruptNesting[cpuid] > 0 )
	{
		*error = ERROR_PENDING_IN_INTERRUPT;
		mrtk_endcriticalsection();
		return NULL;
	}

	/* get the pointer msg data structure */
	pqueue = (MQueueControlBlock*)pevent->Data;

	/* if there is already msg pick one */
	if( pqueue->QEntries > 0 )
	{
		msg = *pqueue->QOut++;
		pqueue->QEntries--;
		if( pqueue->QOut == pqueue->QEnd )
		{
			pqueue->QOut = pqueue->QStart;
		}
		mrtk_endcriticalsection();
		*error = NO_ERROR;
		return msg;
	}

	currenttask = RunningTasks[cpuid];
	pevent->TaskId = currenttask;
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
	mrtk_endcriticalsection();
	mrtk_sched();
	mrtk_begincriticalsection();
	/* if we are here due to timeout, return null pointer (no msg!) */
	if( pevent->IsTimeoutEnable == TRUE && TaskTcbTable[currenttask].Delay == 0 )
	{
		pevent->IsTimeoutEnable = FALSE;
		mrtk_endcriticalsection();
		*error = EVENT_TIMEOUT;
		return (void *) NULL;
	}
	/* there is msg, we pick one */
	msg = *pqueue->QOut++;
	pqueue->QEntries--;
	/*if we reach the end of the storage, we go back to the starting point */
	if( pqueue->QOut == pqueue->QEnd )
	{
		pqueue->QOut = pqueue->QStart;
	}
	mrtk_endcriticalsection();
	*error = NO_ERROR;
	return msg;
}

/*
 * @brief	Post a msg to msg queue of a task
 *
 * @param[in]	EVENT* pevent		pointer to msg queue event
 * @param[in]	void * msg			pointer to the msg to post
 * @param[out]	EVENT_ERROR* error	error status
 *
 */
void mrtk_Qpost(EVENT* pevent, void * msg, EVENT_ERROR* error)
{
	MQueueControlBlock* pqueue;
	Uint16 cpuid = 0;
	/* reject if null event pointer */
	if( pevent == NULL )
	{
		*error = ERROR_NULL_EVENT;
		return;
	}
	/* reject if it's not msg queue event */
	if( pevent->EventType != MSG_QUEUE_EVENT )
	{
		*error = ERROR_BAD_EVENT_TYPE;
		return;
	}

	/* retreive msg queue data structure */
	mrtk_begincriticalsection();
	pqueue = (MQueueControlBlock*) pevent->Data;
	/* check if the msg queue is not full */
	if( pqueue->QEntries >= pqueue->QSize )
	{
		mrtk_endcriticalsection();
		*error = ERROR_QUEUE_FULL;
		return;
	}
	/* push the msg  in the queue */
	*pqueue->QIn++ = msg;
	pqueue->QEntries++;
	/* if we have reach the end, go back to the starting point */
	if( pqueue->QIn == pqueue->QEnd )
	{
		pqueue->QIn = pqueue->QStart;
	}
	if( pevent->TaskId > 0 && pevent->TaskId < NB_TASK_MAX)
	{/* set the task as ready, and add it in priority queue */
		cpuid = mrtk_GetCpuId();
		TaskTcbTable[pevent->TaskId].State = READY;
		TaskTcbTable[pevent->TaskId].AbsoluteDeadline = MrtkTime_ms[cpuid] + (Long64)TaskTcbTable[pevent->TaskId].Deadline;
		mrtk_InsertInQueue(cpuid, pevent->TaskId, TaskTcbTable[pevent->TaskId].AbsoluteDeadline
				, READY_TASKS_QUEUE);
	}
	mrtk_endcriticalsection();
	mrtk_sched();
	*error = NO_ERROR;
	return;
}


