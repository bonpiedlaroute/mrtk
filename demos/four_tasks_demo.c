/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "stdio.h"
#include "mrtk.h"
#include "mrtk_task.h"
#include "mrtk_eventhandler.h"
#include "mrtk_port_timer.h"
#include "mrtk_msgqueue.h"

#define NB_PROCESSOR	2

#define NB_TASKS		4

#define QUEUE_SIZE		16


#define IEN             ((unsigned short)1 << 8)
#define PTU             ((unsigned short)3 << 3)
#define OFF_PD			((unsigned short)1 << 12)
#define OFF_EN			((unsigned short)1 << 9)
#define OFF_IN			((unsigned short)1 << 10)
#define M3              3



STACK TaskContext[NB_TASKS][TASK_CONTEXT_SIZE];

Uint32 CpuIdleCounterBeforeStart[NB_PROCESSOR] = {0};

EVENT* msg_queue_event;
void * Task4_Queue[QUEUE_SIZE];

static unsigned int task1_counter = 0;
static int task2_counter = 0;
static int task3_counter = 0;
static int task4_counter = 0;

void task1(void *args)
{

	while(1)
	{
		int i = 0;
		mrtk_wait(100);

		if( task1_counter == 0xFFFFFFFF )
			task1_counter = 0;

		for(i = 0; i < 10; i++)
			task1_counter++;

	}
}

void task2(void *args)
{
	while(1)
	{
		int i = 0;
		mrtk_wait(800);


		if( task2_counter == 0xFFFFFFFF )
			task2_counter = 0;

		for(i = 0; i < 10; i++)
		task2_counter++;
	}
}

void task3(void *args)
{

	while(1)
	{
		mrtk_wait(800);
		task3_counter++;
		EVENT_ERROR error;

		mrtk_Qpost(msg_queue_event, (void *)&task3_counter, &error);
	}
}

void task4(void *args)
{
	while(1)
	{
		void* data;
		EVENT_ERROR error;
		data = mrtk_Qpend(msg_queue_event, 0, &error);

		if( data != NULL )
		{
			task4_counter = *((Uint32 *) data);
			task4_counter++;
		}
	}
}

int main(int argc, char * argv[])
{
	TaskProperties task_prop;

	mrtk_Init(NB_PROCESSOR);

	msg_queue_event = mrtk_Qcreate((void**)&Task4_Queue, QUEUE_SIZE);

	task_prop.taskid = 1;
	task_prop.executiontime = 30;
	task_prop.deadline = 100;
	task_prop.period = 100;
	mrtk_CreateTask(&task_prop, task1,
			NULL, TaskContext[0], TaskContext[0] + TASK_CONTEXT_SIZE);



	task_prop.taskid = 2;
	task_prop.executiontime = 100;
	task_prop.deadline = 800;
	task_prop.period = 800;
	mrtk_CreateTask(&task_prop, task2,
			NULL, TaskContext[1], TaskContext[1] + TASK_CONTEXT_SIZE);

	task_prop.taskid = 3;
	task_prop.executiontime = 100;
	task_prop.deadline = 700;
	task_prop.period = 700;
	mrtk_CreateTask(&task_prop, task3,
			NULL, TaskContext[2], TaskContext[2] + TASK_CONTEXT_SIZE);

	task_prop.taskid = 4;
	task_prop.executiontime = 100;
	task_prop.deadline = 1000;
	task_prop.period = 1000;
	mrtk_CreateTask(&task_prop, task4,
			NULL, TaskContext[3], TaskContext[3] + TASK_CONTEXT_SIZE);

	mrtk_start(); /* never return ! */


	return 0; /* we should not reach here */
}
