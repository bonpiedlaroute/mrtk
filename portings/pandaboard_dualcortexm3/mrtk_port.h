/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_PORT
#define MRTK_PORT
#include "mrtk_basictypes.h"

typedef Uint32 STACK;

typedef Uint32 REG; /* type of a cpu register */

#define NB_MAX_ALIAS_ADDRESS	8

typedef struct
{
	volatile unsigned long * AliasAddress;
	volatile unsigned char * BitBandAddress;
	unsigned int nbit; /* n° bit in BitBand Address memory */
} LockHandler;

typedef struct
{
    /* user's registers*/
    REG r4, r5, r6, r7, r8, r9, r10, r11, r13_sp, r14_lr, r15_pc, cpsr;
    /* Fast interrupt registers */
//    REG r8_fiq, r9_fiq, r10_fiq, r11_fiq, r12_fiq, r13_fiq, r14_fiq, spsr_fiq;
    /* supervisor registers */
//    REG r13_svc, r14_svc, spsr_fiq;
    /* abort registers */
//    REG r13_abt, r14_abt, spsr_abt;
    /* irq registers */
//    REG r13_irq, r14_irq, spsr_irq;
    /* undef registers */
//    REG r13_undef, r14_undef, spsr_undef;
} *CTXSWITCH_SAVED_REGISTER;

typedef struct
{
    /* user's registers*/
    REG  r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13_sp, r14_lr, cpsr, pc;
} *REGISTER_CPU;


enum
{
	IPI_FOR_START_UP_CPU = 1, /* inter-processor interrupt to start a cpu */
    IPI_FOR_RESCHEDULING, /* inter-processor interrupt to reschedule a start */
    MAX_SOFT_GENERATED_INT = 16
};

/* register that contains the state of one lock */
#define SPINLOCK_LOCK_REG_i     0x4A0F6800

#define MAX_VALUE_REG_i         32

#define SPINLOCK_SYSCONFIG		0x4A0F6010

#define CM_L3_1_CLKSTCTRL		0x4A008700

#define CM_L3_2_CLKSTCTRL		0x4A008800

#define CM_L4CFG_CLKSTCTRL		0x4A008D00

#define SPINLOCK_SYSSTATUS		0x4A0F6014

#define IRQ_STACK_SIZE	1024

#define SVC_STACK_SIZE	1024

#define BIT_BAND_BASE_REG  0x20000000
#define BIT_BAND_ALIAS_REG 0x22000000

/* mrtk inter-processor interrupt Interrrupt ID */
#define IPI_INTERRUPT_ID          ( 1 )

extern Uint16 mrtk_GetCpuId();
extern void mrtk_sendschedipi(Uint32 cpuid, Uint32 interruptid);
extern void mrtk_handleschedipi();
extern void mrtk_taskcontextinit(void (*taskfct)(void* ), STACK * stackpt,void* opt);
extern Boolean mrtk_HardwareAtomicTest(LockHandler *ipp_lock);
extern void mrtk_HardwareAtomicSet(LockHandler *ipp_lock);
extern void InitContextSwitch();
extern void Inithghrdyprio();
extern void ContextSwitch();
extern void SwitchFromIsr();
extern void Starthghrdyprio();
extern void mrtk_BoardInit();
extern Uint32 CpuId;
extern Uint16 CpuIdFromISR;
extern volatile Uint32* TaskToExecuteSP;
extern volatile Uint32* TaskToSleepSP;

extern void CpuInit();
extern void HoldCore();
extern void mrtk_IpiInit();

#endif
