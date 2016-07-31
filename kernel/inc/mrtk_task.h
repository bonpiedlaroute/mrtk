/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_TASK
#define MRTK_TASK
#include "mrtk_basictypes.h"

typedef struct
{
	Uint16 taskid; /* task identifier */
	Uint32 executiontime; /* The execution time of the task */
	Uint32 deadline; /* the deadline of the task */
	Uint32 period; /*The period of the task (we should have deadline <=
                period) */
} TaskProperties;

typedef	struct
{
	Uint16 taskid;
	Float32	capacity;
} TaskCapacity;


extern Int16 mrtk_CreateTask (TaskProperties* task_prop, void (*fct)(void *arg),
		void *arg, STACK *ip_startstack, STACK *ip_endstack);

extern Int16 mrtk_DeleteTask(Uint16 ip_taskid);

extern void mrtk_SortTasksToPartitioned();

extern TaskCapacity TasksToPartitioned[MAX_TASK_ID];
extern Uint16 NbTaskToPartitioned;
#endif
