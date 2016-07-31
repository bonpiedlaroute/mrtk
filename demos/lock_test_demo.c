/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_port.h"
#include "mrtk.h"
#include "mrtk_task.h"
#include "mrtk_eventhandler.h"
#include "mrtk_port_sync.h"
#include "mrtk_sync.h"

#define NB_PROCESSOR	2
#define NB_TASKS		2

STACK TaskContext[NB_TASKS][TASK_CONTEXT_SIZE];

LockHandler* my_lock;

void task1(void * args)
{
	Uint32 i = 0;
	mrtk_InitLock(&my_lock, 0);
	while(1)
	{
		i = 0;
		mrtk_wait(20);

		mrtk_lock(my_lock);
		while(i < 2000)
			i++;
		mrtk_unlock(my_lock);
	}
}

void task2(void * args)
{
	Uint32 i = 0;
	mrtk_InitLock(&my_lock, 0);
	while(1)
	{
		i = 0;
		mrtk_wait(40);

		mrtk_lock(my_lock);
		while(i < 2000)
			i++;
		mrtk_unlock(my_lock);
	}
}

int main(int argc, char* argv[])
{
	mrtk_Init(NB_PROCESSOR, 0.99);

	TaskProperties task_prop;
	task_prop.taskid = 1;
	task_prop.executiontime = 1;
	task_prop.deadline = 20;
	task_prop.period = 20;

	mrtk_CreateTask(&task_prop, task1,
			NULL, TaskContext[0], TaskContext[0] + TASK_CONTEXT_SIZE);

	task_prop.taskid = 2;
	task_prop.executiontime = 1;
	task_prop.deadline = 40;
	task_prop.period = 40;

	mrtk_CreateTask(&task_prop, task2,
				NULL, TaskContext[1], TaskContext[1] + TASK_CONTEXT_SIZE);

	mrtk_start(); /* never return !! */
	return 0;
}


