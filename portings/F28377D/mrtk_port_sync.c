/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_basictypes.h"
#include "mrtk.h"
#include "mrtk_port_sync.h"
#include "mrtk_port_timer.h"

#define CPU1_ID	0
#define CPU2_ID	1

static LockHandler TabLockHandler[MAX_NB_LOCKS] =
{
  {
    {(Uint32*)0x0003fc00, (Uint32*)0x0003fc02, (Uint32*)0x0003fc04}, {(Uint32*)0x0003f800, (Uint32*)0x0003f802, (Uint32*)0x0003f804}
  },
  {
    {(Uint32*)0x0003fc06, (Uint32*)0x0003fc08, (Uint32*)0x0003fc0a}, {(Uint32*)0x0003f806, (Uint32*)0x0003f808, (Uint32*)0x0003f80a}
  }
};

/*
 * @brief	Initialize a lock according to an identifier, task which call this function must not have the same
 * 			deadline as the task in other cpu, which will take the lock
 *
 * @param	LockHandler* lockhandler	Hanlder to be initialize, so that it can
 * 										be use later
 * @param	Uint16 		 lockid			lock identifier to initialize
 *
 * @return	None
 */
void mrtk_InitLock(LockHandler **lockhandler, Uint16 lockid)
{
	if( lockid >= MAX_NB_LOCKS )
	{
		return;
	}
	Uint16 cpuid = mrtk_GetCpuId();

	mrtk_begincriticalsection();
	Uint16 taskid = RunningTasks[cpuid];

	if( cpuid == CPU1_ID )
	{
		*(TabLockHandler[lockid].LockInfoCpu1.TaskHoldingLockDeadline) = TaskTcbTable[taskid].Deadline;
	}
	else
	{
		*(TabLockHandler[lockid].LockInfoCpu2.TaskHoldingLockDeadline) = TaskTcbTable[taskid].Deadline;
	}
	mrtk_endcriticalsection();

	*lockhandler = &(TabLockHandler[lockid]);
}

/*
 * @brief	set the choosing value for the correspondent processor
 *
 * @param	LockHandler* lockhandler	lock's handler
 * @param	Uint16 		 cpuid			the processor id
 * @param	Uint16		 value			value to set
 *
 */
void mrtk_SetChoosing(LockHandler *lockhandler,Uint16 cpuid, Uint16 value)
{
	switch(cpuid)
	{
		case CPU1_ID:
		{
			*(lockhandler->LockInfoCpu1.Choosing) = (Uint32)value;
			break;
		}
		case CPU2_ID:
		{
			*(lockhandler->LockInfoCpu2.Choosing) = (Uint32)value;
			break;
		}
		default:
		{
			break;
		}
	}
}

/*
 * @brief	retrieve the choosing state of a processor
 *
 * @param	LockHandler *lockhandler	lock's handler
 * @param	Uint16		 j				processor's id
 * @return	Uint16						choosing state of the processor
 */
Uint16 mrtk_GetChoosing(LockHandler *lockhandler, Uint16 j)
{
	Uint16 result = 0;
	switch(j)
	{
		case CPU1_ID:
		{
			result = (Uint16)(*(lockhandler->LockInfoCpu1.Choosing));
			break;
		}
		case CPU2_ID:
		{
			result = (Uint16)(*(lockhandler->LockInfoCpu2.Choosing));
			break;
		}
		default:
		{
			break;
		}
	}
	return result;
}


/*
 * @brief	retrieve the maximum number value
 *
 * @param	LockHandler *lockhandler	lock's handler
 *
 * @return	Uint16						the maximum number
 */
Uint16 mrtk_GetMaximumNumber(LockHandler *lockhandler)
{
	Uint16 result = 0;
	if( *(lockhandler->LockInfoCpu1.Number) > *(lockhandler->LockInfoCpu2.Number) )
	{
		result = (Uint16)(*(lockhandler->LockInfoCpu1.Number));
	}
	else
	{
		result = (Uint16)(*(lockhandler->LockInfoCpu2.Number));
	}
	return result;
}

/*
 * @brief	set the number attributed to thre processor
 *
 * @param	LockHandler *lockhandler	lock's handler
 * @param	Uint16		 cpuid			processor's id
 * @param	Uint16		 my_number		number to assign
 *
 * @return	None
 */
void mrtk_SetNumber(LockHandler *lockhandler, Uint16 cpuid, Uint16 my_number)
{
	switch(cpuid)
	{
		case CPU1_ID:
		{
			*(lockhandler->LockInfoCpu1.Number) = (Uint32)my_number;
			break;
		}
		case CPU2_ID:
		{
			*(lockhandler->LockInfoCpu2.Number) = (Uint32)my_number;
			break;
		}
		default:
		{
			break;
		}
	}
}

/*
 * @brief	retrieve the number of a processor
 *
 * @param	LockHandler *lockhandler	lock's handler
 * @param	Uint16		 j				processor's id
 * @return	Uint16						number of the processor
 */
Uint16 mrtk_GetNumber(LockHandler *lockhandler, Uint16 j)
{
	Uint16 result = 0;
	switch(j)
	{
		case CPU1_ID:
		{
			result = (Uint16)(*(lockhandler->LockInfoCpu1.Number));
			break;
		}
		case CPU2_ID:
		{
			result = (Uint16)(*(lockhandler->LockInfoCpu2.Number));
			break;
		}
		default:
		{
			break;
		}
	}
	return result;
}

/*
 * @brief allows to know if a processor turn is reach
 *
 * @param Uint16 j			other processor's id
 * @param Uint16 number_j	other processor's number
 * @param Uint16 cpuid		id of the processor who make the request
 * @param Uint16 my_number	number of the processor who make the request
 *
 */
Boolean mrtk_IsMyTurn(LockHandler* lockhandler, Uint16 j, Uint16 number_j, Uint16 cpuid, Uint16 my_number)
{
	if( number_j != 0 )
	{
		if( number_j < my_number )
			return FALSE;
		else
		{	Uint16 taskdeadline_j = 0;
			Uint16 taskdeadline_cpuid = 0;
			if( j == CPU1_ID )
			{
				taskdeadline_j = (Uint16)(*(lockhandler->LockInfoCpu1.TaskHoldingLockDeadline));
				taskdeadline_cpuid = (Uint16)(*(lockhandler->LockInfoCpu2.TaskHoldingLockDeadline));
			}
			else
			{
				taskdeadline_j = (Uint16)(*(lockhandler->LockInfoCpu2.TaskHoldingLockDeadline));
				taskdeadline_cpuid = (Uint16)(*(lockhandler->LockInfoCpu1.TaskHoldingLockDeadline));
			}
			if( (number_j == my_number) &&
				(taskdeadline_j < taskdeadline_cpuid)	)
			{
				return FALSE;
			}
		}
	}

	return TRUE;

}
