/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_port_timer.h"
#include "mrtk_eventhandler.h"
#include "mrtk_port_exception.h"
#include "mrtk_eventhandler.h"

STACK Irq_Stack[MAX_NB_PROCESSOR][IRQ_STACK_SIZE];
STACK Svc_Stack[MAX_NB_PROCESSOR][SVC_STACK_SIZE];

volatile Uint32* Irq_Start_Stack = (volatile Uint32* )(Irq_Stack[1] + TASK_CONTEXT_SIZE);
volatile Uint32* Svc_Start_Stack = (volatile Uint32* )(Svc_Stack[1] + TASK_CONTEXT_SIZE);

/*
 @brief     This function initializes a timer of 1ms for kernel ticks

 @param     U
 @return    None
*/
void mrtk_timer_init(Uint32 period_us, void (*callback)())
{

}

void SGIHandler()
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

    /* clear SGI interrupt */
    GIC_WRITE( GICD_BASE + GICD_ICPR, SGI_INTERRUPT_ID );

    /* we tell to the kernel that we are no longer in an interrupt */
    mrtk_exitinterrupt(cpuid);
}

void mrtk_sgi_init(Uint16 cpuid)
{
    Uint32 regoffset32 = 0, fieldtoset32 = 0, regoffset16 = 0, fieldtoset16 = 0, regoffset4 = 0,
            fieldtoset4 = 0;

    /* Start the SGI that will interrupt the second core.  Interrupts are disabled
    here already. */

    /* Record the Handler. */
    irq_table_function[ SGI_INTERRUPT_ID ] = SGIHandler;

    /* Compute offsets for the specific register of the interrupt ID */
    regoffset32 = 4 * ( SGI_INTERRUPT_ID / 32 );
    fieldtoset32 = SGI_INTERRUPT_ID % 32;
    regoffset16 = 4 * ( SGI_INTERRUPT_ID / 16 );
    fieldtoset16 = SGI_INTERRUPT_ID % 16;
    regoffset4 = 4 * ( SGI_INTERRUPT_ID / 4 );
    fieldtoset4 = SGI_INTERRUPT_ID % 4;

    GIC_SET( (GICD_BASE + regoffset16 + GICD_ICFR), ( GIC_READ(GICD_BASE + regoffset16) | ( 0x02 << ( fieldtoset16 * 2 ) ) ) );

    /* Set the Priority. */
    GIC_WRITE( (GICD_BASE + GICD_IPR) + regoffset4, ( ( (unsigned int)SGI_INTERRUPT_PRIORITY ) << ( fieldtoset4 * 8 ) ) );

    /* Set the targeted Processors. */
    GIC_WRITE( (GICD_BASE + regoffset4 + GICD_IPTR), ( ( (unsigned int)(1<<cpuid) ) << ( fieldtoset4 * 8 ) ) );

    /* Enable the Interrupt. */
    GIC_SET( (GICD_BASE + regoffset32 + GICD_ICPR), ( GIC_READ( (GICD_BASE + regoffset32 + GICD_ICPR) ) | ( 1 << fieldtoset32 ) ) );
    GIC_SET( (GICD_BASE + regoffset32 + GICD_ISER), ( GIC_READ( (GICD_BASE + regoffset32 + GICD_ISER) ) | ( 1 << fieldtoset32 ) ) );
}

/*
 * @brief   allow to initialize systick for all cores processor
 *
 * @param   None
 * @return  None
 *
 */
void mrtk_systick_init()
{
	/* Make PendSV, CallSV and SysTick the same priroity as the kernel. */
		*(portNVIC_SYSPRI2) |= portNVIC_PENDSV_PRI;
		*(portNVIC_SYSPRI2) |= portNVIC_SYSTICK_PRI;
		/* Make core interrupt the same priority as the kernel. */
		*(portNVIC_IRQPRI0) |= portNVIC_IRQCORE_PRI;
		/* Enable core interrupt. */
		*(portNVIC_IRQENABLE0) |= portNVIC_IRQCORE_ENABLE;

		/* Configure SysTick to interrupt at the requested rate. */
		*(portNVIC_SYSTICK_LOAD) = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
		*(portNVIC_SYSTICK_CTRL) = portNVIC_SYSTICK_CLK | portNVIC_SYSTICK_INT | portNVIC_SYSTICK_ENABLE;
}

void mrtk_generateSGI()
{

    if ( (GIC_READ(GICD_BASE + GICD_ISPENDR) &  (Uint32)SGI_INTERRUPT_ID) == (Uint32)0 )
    {
        GIC_WRITE(GICD_BASE + GICD_SGIR, ( ( 0 << 24 ) | ( ( 1 << 16 ) << ( 1 ) ) | SGI_INTERRUPT_ID ) );
        /* this is to allow the SGI to propagate */
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
        asm(" nop");
    }


}
void not_implemented()
{

}
