/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */

#ifndef PORTINGS_F28377D_MRTK_PORT_SYNC_H_
#define PORTINGS_F28377D_MRTK_PORT_SYNC_H_

typedef struct
{
	Uint32* Choosing;
	Uint32* Number;
	Uint32* TaskHoldingLockDeadline;
} LockInfo;

typedef struct
{
	LockInfo LockInfoCpu1;
	LockInfo LockInfoCpu2;
} LockHandler;

#define MAX_NB_LOCKS	2

extern void mrtk_InitLock(LockHandler **lockhandler, Uint16 lockid);

#endif /* PORTINGS_F28377D_MRTK_PORT_SYNC_H_ */
