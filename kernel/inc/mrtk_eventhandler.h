/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_EVENT_HANDLER
#define MRTK_EVENT_HANDLER
#include "mrtk_basictypes.h"
#include "mrtk.h"

extern void mrtk_tick(Uint32 ip_cpuid);
extern void mrtk_enterinterrupt(Uint32 ip_cpuid);
extern void mrtk_exitinterrupt(Uint32 ip_cpuid);
extern Uint16 MrtkInterruptNesting[MAX_NB_PROCESSOR];
extern void mrtk_wait(Uint32 ip_time_ms);
extern void systickhandler();

#endif
