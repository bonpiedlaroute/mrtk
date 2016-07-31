/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk.h"
#include "mrtk_sync.h"
#include "mrtk_sched.h"
#include "mrtk_heap.h"
#include "mrtk_eventhandler.h"
#include "mrtk_port_timer.h"

/* counting nesting interrupt in each processor */
Uint16 MrtkInterruptNesting[MAX_NB_PROCESSOR];

/*
            mrtk_wait

@brief: This function delays the execution of the current task during the 
            delay "ip_time_ms". if the delay equals to zero, the task will 
            simply continues its execution.

@param
        ip_time_ms      The time delay

@return
*/
void mrtk_wait(Uint32 ip_time_ms)
{
    Uint32 cpuid = 0, currenttaskid = 0;

    if( ip_time_ms != 0 )
    {
    	/* we disable all interrupts */
    	mrtk_begincriticalsection();

        /* we get the priority of the task running in the current processor */
        cpuid = mrtk_GetCpuId();

        currenttaskid = RunningTasks[cpuid];

        /* we set the state of the task to "suspended" */
        TaskTcbTable[currenttaskid].State = SUSPENDED;
        /* we set the new delay of the task*/
        TaskTcbTable[currenttaskid].Delay = ip_time_ms;

        /* we enable all interrupts */
        mrtk_endcriticalsection();

        mrtk_sched(); /* we find the next task to run ! */
    }

}

/*
 @brief This function allows us to ticks the kernel.

 @param

 @return
*/
void systickhandler()
{
	Uint32 cpuid = 0;

	/* we disable all interrupts */
	mrtk_begincriticalsection();

	cpuid = mrtk_GetCpuId(); /* the processor id */

	/* we enable all interrupts */
	mrtk_endcriticalsection();

    /* we tell to the kernel that we are in an interrupt */
    mrtk_enterinterrupt(cpuid);
    /* we process the tick */
    mrtk_tick(cpuid);

    /* we tell to the kernel that we are no longer in an interrupt */
    mrtk_exitinterrupt(cpuid);
}

/*
 @brief: This function process  a tick of 1ms. this corresponds to the 
              kernel tick. This function should be call on each processor tick

@param:

@return
*/
void mrtk_tick(Uint32 ip_cpuid)
{
    Uint16 count = 0;

    /* we disable all interrupts */
    mrtk_begincriticalsection();

    MrtkTime_ms[ip_cpuid]++;

    /* we go throught all tasks */
    while( count < NB_USERS_TASK_MAX )
    {
        /* is the task suspended for a delay ? */
        if ( TaskTcbTable[count].State == SUSPENDED && TaskTcbTable[count].Delay != 0 )
        {
            TaskTcbTable[count].Delay--;
            /* is the delay ended ? */
            if( TaskTcbTable[count].Delay == 0 )
            {
                TaskTcbTable[count].State = READY;
                TaskTcbTable[count].AbsoluteDeadline = MrtkTime_ms[ip_cpuid] + (Long64)TaskTcbTable[count].Deadline;
                mrtk_InsertInQueue(ip_cpuid, TaskTcbTable[count].TaskId, TaskTcbTable[count].AbsoluteDeadline
                		, READY_TASKS_QUEUE);
            }
        }
        count++;
    }

    /* we enable all interrupts */
    mrtk_endcriticalsection();
}


/*

@brief: This function manages nested interrupts on the current cpu.It must
            be call at the begining of an interrupt request. 

@param:

@return

*/
void mrtk_enterinterrupt(Uint32 ip_cpuid)
{
    if( MrtkInterruptNesting[ip_cpuid] < (Uint16)0xFFFF )
    {
        MrtkInterruptNesting[ip_cpuid]++;
    }
}

/*
@brief: This function manages nested interrupts, it calls the scheduling 
            function when there is no nested interrupt in the current cpu. It
            must be call after the process of an interrupt

@param:

@return

*/
void mrtk_exitinterrupt(Uint32 ip_cpuid)
{
    if( MrtkInterruptNesting[ip_cpuid] !=  0 )
    {
        MrtkInterruptNesting[ip_cpuid]--;
    }

    if( MrtkInterruptNesting[ip_cpuid] == 0 )
    {
    	mrtk_sched();
    }
}
