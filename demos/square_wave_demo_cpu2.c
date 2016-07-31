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

int main(int argc, char* argv[])
{
	mrtk_Init(NB_PROCESSOR);

	mrtk_createtimer_us(1, 50, NULL);

	mrtk_start(); /* never return !! */

	return 0;
}

