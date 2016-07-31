/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_port_timer.h"
#include "mrtk_eventhandler.h"
#include "F28x_Project.h"


void not_implemented()
{

}

void mrtk_timer_init(Uint16 cpuid, Uint32 period_us, void (*callback)())
{
	ConfigCpuTimer(&CpuTimer1, 200, period_us);
	CpuTimer1Regs.TCR.all = 0x4000;
	IER |= M_INT13;
}

void mrtk_systick_init()
{


	// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
	// 200MHz CPU Freq, 1 second Period (in uSeconds)
	ConfigCpuTimer(&CpuTimer0, 200, 100);
//	ConfigCpuTimer(&CpuTimer1, 200, 50);
	//ConfigCpuTimer(&CpuTimer2, 200, 1000000);

	// To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
	// of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the
	// below settings must also be updated.
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
//	CpuTimer1Regs.TCR.all = 0x4000;
	//CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	// Step 5. User specific code, enable interrupts:

	// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
	// which is connected to CPU-Timer 1, and CPU int 14, which is connected
	// to CPU-Timer 2:
	IER |= M_INT1;
	//IER |= M_INT13;
	//IER |= M_INT14;

	// Enable TINT0 in the PIE: Group 1 interrupt 7
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

	// Enable global Interrupts and higher priority real-time debug events:
	EINT;  // Enable Global interrupt INTM
	ERTM;  // Enable Global realtime interrupt DBGM
}

__interrupt void cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;

   systickhandler();
   // Acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
#ifdef CPU2
__interrupt void cpu_timer1_isr(void)
{

   GpioDataRegs.GPADAT.bit.GPIO1    ^=1;

   CpuTimer1.InterruptCount++;
}
#endif

