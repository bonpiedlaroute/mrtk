/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef PORTINGS_F28377D_MRTK_PORT_TIMER_H_
#define PORTINGS_F28377D_MRTK_PORT_TIMER_H_
#include "mrtk_basictypes.h"
#include "F28x_Project.h"

#define mrtk_begincriticalsection() DINT;\
	DRTM

#define mrtk_endcriticalsection() EINT;\
	ERTM

extern void not_implemented();
extern void mrtk_timer_init(Uint16 cpuid, Uint32 period_us, void (*callback)());
extern void mrtk_systick_init();

#endif /* PORTINGS_F28377D_MRTK_PORT_TIMER_H_ */
