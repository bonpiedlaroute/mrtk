/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_basictypes.h"
#include "mrtk.h"
#include "mrtk_port.h"
#include "mrtk_port_timer.h"
#include "mrtk_sync.h"
#include "mrtk_eventhandler.h"
#include "mrtk_port_exception.h"

#define MAX_CPU_INTERFACES  8 /* maximum cpu interfaces in cortex a9 */

extern void InitReadCpuId();
extern void ReadCpuId();
extern void ReadCpuIdFromISR();

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
	/* set vector table offset for this core */
		*(volatile unsigned long **)(VTOR) =
			(unsigned long *)((unsigned long)Vector_Core0 /*& 0x1fffffffUL*/);

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
	Uint32 lock_counter = numlock / 4;
	Uint32 nb_bit = numlock & 3; /* ==> numlock%4 { numlock & (2^2 - 1) }*/

	handler->BitBandAddress = (volatile unsigned char *)(BIT_BAND_BASE_REG + lock_counter);
	handler->AliasAddress = (volatile unsigned long *)(BIT_BAND_ALIAS_REG + lock_counter * 32 + nb_bit * 4);
	*(handler->BitBandAddress) = 0;

	handler->nbit = nb_bit;
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
	volatile unsigned long * aliasaddress = ipp_lock->AliasAddress;
	volatile unsigned char * bitbandaddress = ipp_lock->BitBandAddress;
	unsigned char value;
	Boolean result = FALSE;

	unsigned char expected_value = ((unsigned char)(1) << ipp_lock->nbit) ;

	*aliasaddress = 1; /* atomic write to one bit on bitband region */
	value = *bitbandaddress;

	if( value == expected_value )
		result = TRUE;

	return result;
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
	*(ipp_lock->AliasAddress) = 0;
}
/*
*            mrtk_GetCpuId
*
*   @brief  This function return the identifier of the cpu where the current
*            code execute
*
*   @param  None
*   
*   @return Uint16      The Cpu Identifier
*
*/
Uint16 mrtk_GetCpuId()
{
    return (Uint16)(*(volatile unsigned long *)0xe00fffe0);
}


void mrtk_sendschedipi(Uint32 ip_cpuid, Uint32 ip_interruptid)
{
    /* Forward the interrupt to the CPU interfaces specified in the CPUTargetList field  */
    *(volatile unsigned int *)(GICD_BASE + GICD_SGIR) &= ~(3 << 24);

    if( ip_cpuid < MAX_CPU_INTERFACES)
    {
       *(volatile unsigned int *)(GICD_BASE + GICD_SGIR) |= ((0x00000001 << ip_cpuid) << 16 );
    }

    *(volatile unsigned int *)(GICD_BASE + GICD_SGIR) |= ip_interruptid;
}


/*
		mrtk_taskstackinit
@brief: This function initializes the context of a new task

@param:

	void (*)(void* )	pointer to the code of the task
	STACK *				stack pointer
	void * 				specific options
*/
void mrtk_taskcontextinit(void (*taskfct)(void* ), STACK * stackpt,void* opt)
{
	STACK* taskcontext = stackpt - 17;
	REGISTER_CPU taskstack = (REGISTER_CPU)taskcontext;

	taskstack->r0 = 0;
	taskstack->r1 = 0;
	taskstack->r2 = 0;
	taskstack->r3 = 0;
	taskstack->r4 = 0;
	taskstack->r5 = 0;
	taskstack->r6 = 0;
	taskstack->r7 = 0;
	taskstack->r8 = 0;
	taskstack->r9 = 0;
	taskstack->r10 = 0;
	taskstack->r11 = 0;
	taskstack->r12 = 0;
	taskstack->r13_sp = (REG) (taskcontext - 1);
	taskstack->r14_lr = (REG) taskfct;
	taskstack->cpsr = 0x13;
	taskstack->pc = (REG) taskfct;
}

