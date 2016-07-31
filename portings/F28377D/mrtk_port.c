/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_basictypes.h"
#include "mrtk.h"
#include "mrtk_port.h"
#include "F28x_Project.h"

Uint16     GetCPUST0(void);
Uint16     GetCPUST1(void);

__interrupt void cpu_timer0_isr(void);

#ifdef CPU2
__interrupt void cpu_timer1_isr(void);
#endif

/*
                mrtk_BoardInit

@brief:    This function initializes hardware specific stuff, such as spinlock reset

@param:
            none
@return:
            none
*/
void mrtk_BoardInit()
{

	// 1. Initialize System Control:
	// PLL, WatchDog, enable Peripheral Clocks

	InitSysCtrl();
#ifdef CPU1
	// 2. Initialize GPIO:
	//InitGpio();
	//GPIO_SetupPinMux(65, GPIO_MUX_CPU1, 0);
	//GPIO_SetupPinOptions(65, GPIO_OUTPUT, GPIO_PUSHPULL);

	/* Give CPU2 control of GPIO34 */
	//GPIO_SetupPinMux(12,GPIO_MUX_CPU2,0);
	//GPIO_SetupPinOptions(12, GPIO_OUTPUT,0);

	/* Give CPU1 control of GPIO31 */
	//GPIO_SetupPinMux(15,GPIO_MUX_CPU1,0);
	//GPIO_SetupPinOptions(15, GPIO_OUTPUT,0);
	EALLOW;
	GpioCtrlRegs.GPAMUX1.all = 0x00000000;  // All GPIO
	GpioCtrlRegs.GPAMUX2.all = 0x00000000;  // All GPIO
	GpioCtrlRegs.GPBMUX1.all = 0x00000000;  // All GPIO

	GpioCtrlRegs.GPADIR.all = 0xFFFFFFFF;   // All outputs
	GpioCtrlRegs.GPBDIR.all = 0x00001FFF;   // All outputs
	GPIO_SetupPinMux(1,GPIO_MUX_CPU2,0);
	GPIO_SetupPinOptions(1, GPIO_OUTPUT,0);
	EDIS;
#endif

	//3. Clear all interrupts and initialize PIE vector table:
	//Disable CPU interrupts
	DINT;

	// Initialize the PIE control registers to their default state.
	InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	InitPieVectTable();

	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	EALLOW;  // This is needed to write to EALLOW protected registers
	PieVectTable.TIMER0_INT = &cpu_timer0_isr;
	PieVectTable.RTOS_INT = &RTOSINT_Handler;
#ifdef CPU2
	PieVectTable.TIMER1_INT = &cpu_timer1_isr;
	GpioDataRegs.GPADAT.bit.GPIO1    =0;
#endif
	EDIS;    // This is needed to disable write to EALLOW protected registers

	// Step 4. Initialize the Device Peripheral. This function can be
	//         found in F2806x_CpuTimers.c
	InitCpuTimers();   // For this example, only initialize the Cpu Timers

}

/*
                mrtk_IniLockHandler
@brief:    This function returns a new lock, that can be use by the application. there is
                32 locks, for the omap4460

@param:
            none
@return:
            LockHandler*        The lock handler
*/
void mrtk_IniLockHandler(LockHandler * handler, Uint32 numlock)
{

}
/*
                mrtk_HardwareAtomicTest

@brief:    This function test a lock register atomically

@param:
            ipp_lock     pointer to the lock variable.
                        It must be a pointer to LockHandler

@return:
            TRUE        If the variable has been set
            FALSE       Otherwise
*/
Boolean mrtk_HardwareAtomicTest(LockHandler *ipp_lock)
{
	return TRUE;
}


/*
                mrtk_HardwareAtomicSet
@brief:    This function set a lock register atomically

@param:
            ipp_lock     pointer to the lock variable.
                        It must be a pointer to LockHandler

@return:
            TRUE        If the variable has been set
            FALSE       Otherwise
*/
void mrtk_HardwareAtomicSet(LockHandler *ipp_lock)
{

}

/*
		mrtk_taskstackinit
@brief: This function initializes the context of a new task

@param:

	void (*)(void* )	pointer to the code of the task
	STACK *				start stack pointer
	STACK *				end stack pointer
	void * 				specific options
*/
STACK* mrtk_taskcontextinit(void (*taskfct)(void* ), STACK * start_stackpt, STACK * end_stackpt,void* opt)
{
 Uint16 *stack = (Uint16 *)start_stackpt;
 stack++;
 Uint32 *taskstack = (Uint32 *)stack;
 taskstack++;

 *taskstack++ = (0x11110000) | GetCPUST0(); /* T:ST0 */
 *taskstack++ = 0x00000000;					/* AH:AL */
 *taskstack++ = 0x00000000;                 /* PH:PL */
 *taskstack++ = 0x00000000;                 /* AR1:AR0 */
 *taskstack++ = (0x00000000) | GetCPUST1(); /* DP:ST1 */
 *taskstack++ = 0x00000000;                  /* DBGSTAT:IER */
 *taskstack++ = (Uint32) taskfct;

 *taskstack++ = 0x00000000;		/* AR1H:AR0H */
 *taskstack++ = 0x00000000;      /* XAR2 */
 *taskstack++ = 0x00000000;		/* XAR3 */
 *taskstack++ = 0;		/* XAR4 */
 *taskstack++ = 0x00000000;		/* XAR5 */
 *taskstack++ = 0x00000000;		/* XAR6 */
 *taskstack++ = 0x00000000;		/* XAR7 */
 *taskstack++ = 0x00000000;		/* XT */
 *taskstack++ = 0;		/* RPC */

#if __TMS320C28XX_FPU32__ == 1                /* Save FPU registers, if enabled. */
 *taskstack++ = 0x00000000;                 /*   R0H  */
 *taskstack++ = 0x00000000;                 /*   R1H  */
 *taskstack++ = 0x00000000;                 /*   R2H  */
 *taskstack++ = 0x00000000;                 /*   R3H  */
 *taskstack++ = 0x00000000;                 /*   R4H  */
 *taskstack++ = 0x00000000;                /*   R5H  */
 *taskstack++ = 0x00000000;                /*   R6H  */
 *taskstack++ = 0x00000000;                /*   R7H  */
 *taskstack++ = 0x00000000;                /*   STF  */
 *taskstack++ = 0x00000000;                /*   RB  */
#endif

 return taskstack;
}

