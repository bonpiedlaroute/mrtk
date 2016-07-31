/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_port.h"
#include "mrtk_port_timer.h"

void mrtk_createtimer_us(Uint16 cpuid, Uint32 period_us, void (*callback)())
{
	if( cpuid == mrtk_GetCpuId() )
	{
		mrtk_timer_init(cpuid, period_us, callback);
	}
}
