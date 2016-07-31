/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_port_timer.h"
#include "mrtk_eventhandler.h"
#include "mrtk_task.h"
#include "mrtk.h"
#include "mrtk_timer.h"

#define NB_PROCESSOR	2
#define NB_TASKS		1


STACK TaskContext[NB_TASKS][TASK_CONTEXT_SIZE];

void task1(void * args)
{
	Uint16 i = 0;
	GpioDataRegs.GPADAT.bit.GPIO0    =0;
	while(1)
	{
		GpioDataRegs.GPADAT.bit.GPIO0  ^=1;

		mrtk_wait(20);
		i = 0;
		while(i < 2000 )
			i++;

	}
}


int main(int argc, char* argv[])
{

	mrtk_Init(NB_PROCESSOR, 0.95);


	TaskProperties task_prop;
	task_prop.taskid = 1;
	task_prop.executiontime = 1;
	task_prop.deadline = 20;
	task_prop.period = 20;

	mrtk_CreateTask(&task_prop, task1,
			NULL, TaskContext[0], TaskContext[0] + TASK_CONTEXT_SIZE);

	mrtk_createtimer_us(1, 50, NULL);

	mrtk_start(); /* never return !! */

	return 0;
}
