/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk.h"
#include "mrtk_port_timer.h"
#include "mrtk_sched.h"
#include "mrtk_port_exception.h"
#include "mrtk_eventhandler.h"

void do_not_used();
void reset_handler();
void nmi_handler();
void hardfault_handler();

void (*irq_table_function[MAX_NUMBER_OF_IRQ])(void) = {not_implemented};


STACK MainProgramStack[MAIN_PROGRAM_STACK_SIZE];

asm(" .sect .intvecs");
void (*Vector_Core0[])(void) =
{
	(void (*)( void ))(MainProgramStack + MAIN_PROGRAM_STACK_SIZE), /* Initial stack pointer */
	reset_handler,			/* Reset */
	nmi_handler,			/* Nmi */
	hardfault_handler,			/* Hard fault */
	not_implemented,		/* MPU fault */
	not_implemented,		/* Bus fault */
	not_implemented,		/* Usage fault */
	0,
	0,
	0,
	0,
	not_implemented,		/* SVCall handler */
	not_implemented,		/* Debug */
	0,
	not_implemented,		/* PendSV */
	systickhandler,	/* SysTick */

	/* Vendor Handlers */
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,	/* Semaphore interrupt */
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
	not_implemented,
};

void do_ipi();



void do_undefined_instruction()
{

}

void do_ipi()
{
    unsigned int mrtk_interrupt_source = 0;

    mrtk_interrupt_source = *(volatile unsigned int *)(GICC_BASE + GICC_IAR ) & ~0xFFFFFFC0;

    switch(mrtk_interrupt_source)
    {
        case IPI_FOR_START_UP_CPU:
        {
            mrtk_start();
            break;
        }
        case IPI_FOR_RESCHEDULING:
        {
            mrtk_sched();
            break;
        }
        default:
        {
            break;
        }
    }
}

void do_prefetch_abort()
{

}

void do_data_abort()
{

}

void do_not_used()
{

}

void do_fiq()
{

}

void reset_handler()
{

}

void nmi_handler()
{
	while(1);
}

void hardfault_handler()
{
	while(1);
}
/*
*   @brief  allow the process irq
*   
*   @param  None
*
*   @return None
*/
void do_irq()
{
	unsigned int interrrupt_id = 0;

	/* read the interrupt id. */
	interrrupt_id = GIC_READ( GICC_BASE + GICC_IAR );

	if ( interrrupt_id > 0 && interrrupt_id < MAX_NUMBER_OF_IRQ )
	{
		/* acknowledge the interrupt. */
		GIC_WRITE( GICC_BASE + GICC_EOIR, interrrupt_id );

		/* Call the associated handler. */
		irq_table_function[interrrupt_id]();
	}
	else
	{
		/* This is a spurious interrupt, do nothing. */
	}
}

