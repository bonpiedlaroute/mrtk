/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk.h"

#ifndef MRTK_HEAP
#define MRTK_HEAP

/* The different data in each heap item */
typedef struct heap
{
    Uint16  TaskId; /* the task's id */
    Long64  Deadline; /* the deadline of the task */
} HEAP;

typedef enum
{
	READY_TASKS_QUEUE,
	WAITING_TASKS_QUEUE
}QueueType;

#define MRTK_QUEUE_SIZE    (NB_TASK_MAX+1)

extern void mrtk_InsertInQueue(Uint16 ip_cpuid, Uint16 ip_taskid, Long64 ip_deadline,
		QueueType qtype);

extern void mrtk_RemoveFirstElementInQueue(Uint16 ip_cpuid, QueueType qtype);

extern Uint16 mrtk_GetTaskWithNearestDeadline(Uint16 ip_cpuid, QueueType qtype);

extern void mrtk_QueuesInit();

extern void mrtk_RemoveElementInQueue(Uint16 ip_cpuid, Uint16 ip_task_id, QueueType qtype);

#endif
