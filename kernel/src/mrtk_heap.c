/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_heap.h"
#include "limits.h"

/* the node is empty */
#define MRTK_QUEUE_TAG_EMPTY    0xEEE

/* the node is available */
#define MRTK_QUEUE_TAG_AVAILABLE    0xAAA

/* queue of tasks ready to run */
static HEAP    RdyQueue[MAX_NB_PROCESSOR][MRTK_QUEUE_SIZE];

/* queue of taks waiting for an event */
static HEAP    WaitingQueue[MAX_NB_PROCESSOR][MRTK_QUEUE_SIZE];

/* the position of the last element on ready the queue (for each queue) */
static Uint16 last_element_rdy[MAX_NB_PROCESSOR] = {0};
/* the full level on ready queue (for each queue) */
static Uint16 full_level_rdy[MAX_NB_PROCESSOR] = {0};

/* the position of the last element on waiting the queue (for each queue) */
static Uint16 last_element_wq[MAX_NB_PROCESSOR] = {0};
/* the full level on waiting queue (for each queue) */
static Uint16 full_level_wq[MAX_NB_PROCESSOR] = {0};

typedef HEAP HeapTab[MAX_NB_PROCESSOR][MRTK_QUEUE_SIZE];


/*
            mrtk_PriorityQueueInit

@brief: This function initialize the priority queue

Argument:


@return:

*/

void mrtk_QueuesInit()
{
   Uint16 i = 0, j = 0;

    for (i = 0; i < MAX_NB_PROCESSOR ; i++)
    {
    	last_element_rdy[i] = 0;
    	full_level_rdy[i] = 0;

    	last_element_wq[i] = 0;
    	full_level_wq[i] = 0;
    	for( j = 0; j < MRTK_QUEUE_SIZE; j++)
    	{
    		RdyQueue[i][j].TaskId = 0;
			RdyQueue[i][j].Deadline = LLONG_MAX;
			WaitingQueue[i][j].TaskId = 0;
			WaitingQueue[i][j].Deadline = LLONG_MAX;
    	}
    }
}
/*
            SwapQueueItems

@brief: This function performs the swap between item on the queue with 
             index1 and item on the queue with index2. The two index must be
             comprise between 1 ... 255. The two items must be locked before 
             the call of this function.

@param:
        ip_index1   the first item index
        ip_index2  the second item index

@return:

*/
static void SwapQueueItems(Uint16 ip_cpuid, Uint16 ip_index1, Uint16 ip_index2, QueueType qtype)
{
    Uint16 Tmp_TaskId = 0;
    Uint32 Tmp_Deadline = 0;
    HeapTab* pqueuetab;

    if( qtype == READY_TASKS_QUEUE )
    {
    	pqueuetab = &RdyQueue;
    }
    else
    {
    	pqueuetab = &WaitingQueue;
    }

    Tmp_TaskId = (*pqueuetab)[ip_cpuid][ip_index1].TaskId;

    Tmp_Deadline = (*pqueuetab)[ip_cpuid][ip_index1].Deadline;

    (*pqueuetab)[ip_cpuid][ip_index1].TaskId = (*pqueuetab)[ip_cpuid][ip_index2].TaskId;

    (*pqueuetab)[ip_cpuid][ip_index1].Deadline = (*pqueuetab)[ip_cpuid][ip_index2].Deadline;

    (*pqueuetab)[ip_cpuid][ip_index2].TaskId = Tmp_TaskId;

    (*pqueuetab)[ip_cpuid][ip_index2].Deadline = Tmp_Deadline;

    return;

}

/*
*            mrtk_InsertInQueue
*
* @brief    This function insert the new taskid in the priorityqueue of a processor.
*
*
* @param  [in] Uint16 ip_cpuid 	the cpu concerned by this insertion
*         [in] Uint16 ip_taskid   the task's identifier
*         [in] Long64 ip_deadline the absolute deadline of the task to insert
*		  [in] QueueType qtype	  Queue Type
* @return
*		 none
*/

void mrtk_InsertInQueue(Uint16 ip_cpuid, Uint16 ip_taskid, Long64 ip_deadline, QueueType qtype)
{
    Uint16 i = 0, parent = 0;
    Uint16* last_element, *full_level;
    HeapTab* pqueuetab;

    if( qtype == READY_TASKS_QUEUE )
	{
		pqueuetab = &RdyQueue;
		last_element = last_element_rdy;
		full_level = full_level_rdy;
	}
	else
	{
		pqueuetab = &WaitingQueue;
		last_element = last_element_wq;
		full_level = full_level_wq;
	}

    last_element[ip_cpuid] = last_element[ip_cpuid]  + 1;

    if( last_element[ip_cpuid]  >= (full_level[ip_cpuid]  << 1))
    {
    	full_level[ip_cpuid]  = last_element[ip_cpuid] ;
    }
    i = last_element[ip_cpuid] ;

    (*pqueuetab)[ip_cpuid][i].TaskId = ip_taskid;
    (*pqueuetab)[ip_cpuid][i].Deadline = ip_deadline;

    parent = i >> 1;

    while( ( i != 1) && ((*pqueuetab)[ip_cpuid][i].Deadline < (*pqueuetab)[ip_cpuid][parent].Deadline) )
    {
    	SwapQueueItems(ip_cpuid, i, parent, qtype);

    	i = parent;

    	parent = i >> 1;
    }
}

/*
*                mrtk_RemoveFirstElementInQueue
*
* @brief
* 		 This function deletes the top item on the queue. The delete
*            operation also adjust the heap starting at the bottom.
* @param
 * 			Uint16 ip_cpuid		The cpu id
* @return
*        	None
*/

void mrtk_RemoveFirstElementInQueue(Uint16 ip_cpuid, QueueType qtype)
{
    Uint16 i = 0, j = 0, left_child = 0, right_child = 0, min_child = 0;
    Uint16* last_element, *full_level;
    HeapTab* pqueuetab;

	if( qtype == READY_TASKS_QUEUE )
	{
		pqueuetab = &RdyQueue;
		last_element = last_element_rdy;
		full_level = full_level_rdy;
	}
	else
	{
		pqueuetab = &WaitingQueue;
		last_element = last_element_wq;
		full_level = full_level_wq;
	}


    if( 0 == last_element[ip_cpuid] )
    {/* we return the idle task of the processor */
    	return ;
    }
    else
    {
    	i = 1;
    	j = last_element[ip_cpuid];
    	last_element[ip_cpuid] = last_element[ip_cpuid] - 1;

    	if( last_element[ip_cpuid] < full_level[ip_cpuid] )
    	{
    		full_level[ip_cpuid] = full_level[ip_cpuid] >> 1;
    	}
    	if( 1 == j )
    	{
    		(*pqueuetab)[ip_cpuid][1].TaskId = 0xFFFF;
    		(*pqueuetab)[ip_cpuid][1].Deadline = LLONG_MAX;

    		return ;
    	}
    	(*pqueuetab)[ip_cpuid][1].TaskId = (*pqueuetab)[ip_cpuid][j].TaskId;
    	(*pqueuetab)[ip_cpuid][1].Deadline = (*pqueuetab)[ip_cpuid][j].Deadline;

    	(*pqueuetab)[ip_cpuid][j].TaskId = 0xFFFF;
    	(*pqueuetab)[ip_cpuid][j].Deadline = LLONG_MAX;

    	left_child = i << 1;
    	right_child = i << 1 + 1;

    	min_child = (*pqueuetab)[ip_cpuid][left_child].Deadline < (*pqueuetab)[ip_cpuid][right_child].Deadline ?
    			left_child : right_child;

    	while( (*pqueuetab)[ip_cpuid][i].Deadline > (*pqueuetab)[ip_cpuid][min_child].Deadline )
    	{
    		SwapQueueItems(ip_cpuid, i, min_child, qtype);

			i = min_child;
			left_child = i << 1;
		    right_child = i << 1 + 1;

		    if( left_child < MRTK_QUEUE_SIZE && right_child < MRTK_QUEUE_SIZE )
		    {
		    	min_child = (*pqueuetab)[ip_cpuid][left_child].Deadline < (*pqueuetab)[ip_cpuid][right_child].Deadline ?
					left_child : right_child;
		    }
		    else
		    {
		    	break;
		    }
    	}
    }
}
/*
 * @brief
 * 			This function provides the task id in the cpu, with the nearest deadline
 *
 * @param
 * 			Uint16 ip_cpuid		The cpu id
 *
 * @return
 * 			Uint16				the task id with the nearest deadline
 *
 */
Uint16 mrtk_GetTaskWithNearestDeadline(Uint16 ip_cpuid, QueueType qtype)
{
	Uint16* last_element;
	HeapTab* pqueuetab;

	if( qtype == READY_TASKS_QUEUE )
	{
		pqueuetab = &RdyQueue;
		last_element = last_element_rdy;
	}
	else
	{
		pqueuetab = &WaitingQueue;
		last_element = last_element_wq;
	}

	/* is there a task in the queue ?*/
	if( 0 == last_element[ip_cpuid])
	{/* yes */
		/* we return the idle task of the cpu */
		return (NB_USERS_TASK_MAX + ip_cpuid);
	}
	else
	{/* no */
		/* we return the taskid with the nearest deadline */
		return (*pqueuetab)[ip_cpuid][1].TaskId;
	}
}


/*
*                mrtk_RemoveElementInQueue
*
* @brief
* 		 This function deletes one item on the queue. The delete
*            operation also adjust the heap if necessary.
* @param
 * 			Uint16 ip_cpuid		The cpu id
 * 			Uint16 ip_task_id	task's identifier
 * 			QueueType qtype		queue type
* @return
*        	None
*/

void mrtk_RemoveElementInQueue(Uint16 ip_cpuid, Uint16 ip_task_id, QueueType qtype)
{
    Uint16 i = 0, j = 0, left_child = 0, right_child = 0, min_child = 0, k = 0;
    Uint16* last_element, *full_level;
    HeapTab* pqueuetab;
	Boolean found = FALSE;
	Uint16 index = 0, level = 0;

	if( qtype == READY_TASKS_QUEUE )
	{
		pqueuetab = &RdyQueue;
		last_element = last_element_rdy;
		full_level = full_level_rdy;
	}
	else
	{
		pqueuetab = &WaitingQueue;
		last_element = last_element_wq;
		full_level = full_level_wq;
	}

    if( 0 == last_element[ip_cpuid] )
    {/* we return the idle task of the processor */
    	return ;
    }
    else
    {
    	for(k = 1; k <= last_element[ip_cpuid]; k++ )
    	{
    		if( (*pqueuetab)[ip_cpuid][k].TaskId == ip_task_id )
    		{
    			found = TRUE;
    			index = k;
    			break;
    		}
    	}

    	if( found == FALSE )
    		return;

    	i = index;
    	j = last_element[ip_cpuid];
    	level = full_level[ip_cpuid];
    	last_element[ip_cpuid] = last_element[ip_cpuid] - 1;

    	if( last_element[ip_cpuid] < full_level[ip_cpuid] )
    	{
    		full_level[ip_cpuid] = full_level[ip_cpuid] >> 1;
    	}
    	if( 1 == j )/* it was remaining just one element? */
    	{
    		(*pqueuetab)[ip_cpuid][1].TaskId = 0xFFFF;
    		(*pqueuetab)[ip_cpuid][1].Deadline = LLONG_MAX;

    		return ;
    	}
    	if(index >= level ) /* the element was at last level? */
    	{
    		(*pqueuetab)[ip_cpuid][index].TaskId = 0xFFFF;
			(*pqueuetab)[ip_cpuid][index].Deadline = LLONG_MAX;
			return ;
    	}
    	(*pqueuetab)[ip_cpuid][index].TaskId = (*pqueuetab)[ip_cpuid][j].TaskId;
    	(*pqueuetab)[ip_cpuid][index].Deadline = (*pqueuetab)[ip_cpuid][j].Deadline;

    	(*pqueuetab)[ip_cpuid][j].TaskId = 0xFFFF;
    	(*pqueuetab)[ip_cpuid][j].Deadline = LLONG_MAX;

    	left_child = i << 1;
    	right_child = i << 1 + 1;

    	min_child = (*pqueuetab)[ip_cpuid][left_child].Deadline < (*pqueuetab)[ip_cpuid][right_child].Deadline ?
    			left_child : right_child;

    	while( (*pqueuetab)[ip_cpuid][i].Deadline > (*pqueuetab)[ip_cpuid][min_child].Deadline )
    	{
    		SwapQueueItems(ip_cpuid, i, min_child, qtype);

			i = min_child;
			left_child = i << 1;
		    right_child = i << 1 + 1;

		    if( left_child < MRTK_QUEUE_SIZE && right_child < MRTK_QUEUE_SIZE )
		    {
		    	min_child = (*pqueuetab)[ip_cpuid][left_child].Deadline < (*pqueuetab)[ip_cpuid][right_child].Deadline ?
					left_child : right_child;
		    }
		    else
		    {
		    	break;
		    }
    	}
    }
}
