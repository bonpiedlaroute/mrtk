/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */

#ifndef PORTINGS_F28377D_MRTK_PORT_H_
#define PORTINGS_F28377D_MRTK_PORT_H_
#include "mrtk_basictypes.h"
#include "mrtk_port_sync.h"

typedef Uint32 STACK;

typedef Uint32 REG; /* type of a cpu register */



extern STACK* mrtk_taskcontextinit(void (*taskfct)(void* ), STACK * start_stackpt, STACK * end_stackpt,void* opt);
extern Boolean mrtk_HardwareAtomicTest(LockHandler *ipp_lock);
extern void mrtk_HardwareAtomicSet(LockHandler *ipp_lock);
extern void InitContextSwitch();
extern void RTOSINT_Handler();
extern void SwitchFromIsr();
extern void Starthghrdyprio();
extern void mrtk_BoardInit();
extern Uint32 CpuId;
extern volatile Uint32* TaskToExecuteSP;
extern volatile Uint32* TaskToSleepSP;

#define ContextSwitch() asm(" TRAP #16")

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
#define mrtk_GetCpuId()	(Uint16)(*(volatile Uint16 *)0x7026D & 0x00FF) - 1

#endif /* PORTINGS_F28377D_MRTK_PORT_H_ */
