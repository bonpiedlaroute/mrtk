/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_SCHED
#define MRTK_SCHED


typedef struct
{
	Uint16 processorid;
	Float32 total_capacity;
} ProcessorCapacity;

extern void mrtk_sched();

extern Uint16 Task2ProcessorTable[NB_TASK_MAX];

extern Uint32 Processor2TaskPrioTable[MAX_NB_PROCESSOR];

extern ProcessorCapacity ProcessorCapacityList[MAX_NB_PROCESSOR];

extern Boolean mrtk_partitioned();

extern void mrtk_InitMaxCpuUsagePerCore(Float32 maxcpuusage);

#endif
