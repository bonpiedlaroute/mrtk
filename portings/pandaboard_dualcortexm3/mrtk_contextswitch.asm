; 		(c) copyright 2015 timecriticalsystem.
; 			All rights reserved.
	.global _CpuId
	.word  0x0
	.ref TaskToExecuteSP
	.ref TaskToSleepSP
	.ref Irq_Start_Stack

	.global InitContextSwitch
InitContextSwitch: .asmfunc
	STMDB sp!, {r2} ; push r2 into the stack , so we can use it
	STMDB sp!, {r1}  ; push the r1 in the stack so we can use it
	LDR r1, TaskToSleepSP_Addr
	LDR r1, [r1]  ; get the address of the task context
	.global Inithghrdyprio
Inithghrdyprio:
	STMDB sp!, {r0}  ; push the r0 in the stack so we can use it
	LDR r0, TaskToExecuteSP_Addr
	LDR r0, [r0]
	MOV pc, lr
	.endasmfunc

	.global ContextSwitch
ContextSwitch: .asmfunc
	ldr r2, [sp] ; r2 = r0
	stmia r1!, {r2} ; store r0
	ADD SP, SP, #4
	ldr r2, [sp] ; r2 = r1
	stmia r1!, {r2}  ; store r1
	ADD SP, SP, #4
	ldr r2, [sp] ; r2 = r2
	ADD SP, SP, #4 ; put the stack in its initial point
	stmia r1!, {r2}  ; store r2
	stmia r1!, {r3-r12}
	stmia r1!, {r13}
	stmia r1!, {r14}
	MRS r2, apsr ; r2 = cpsr
	stmia r1!, {r2}
	stmia r1!, {r14}
	.global Starthghrdyprio
Starthghrdyprio:
	ADD r0, r0, #8
	ldmia r0!, {r2-r12}
	ldr r1, [r0, #8] ; r1 = value of cpsr store
	MSR apsr_nzcvq, r1 ;cpsr = r1
	ldmia r0!, {r13}
	ldr lr, [r0]
	ldr r1, [r0, #8]
	STMDB sp!, {r1} ; push the pc on the stack
	SUB r0, r0, #56 ; we go back to r0
	ldr r1, [r0, #4] ; load the real value of r1
	ldr r0, [r0]  ;load the real value of r0
	LDMIA sp!, {pc}
	.endasmfunc

	.global SwitchFromIsr
SwitchFromIsr: .asmfunc
	ldr sp, Irq_Stack_Addr
    ldr sp,[sp]
    sub sp, sp, #64
    ldmia sp!, {r0 - r12}
    add sp, sp, #4
    ldmia sp!, {lr}
    sub lr, lr, #4	; the real value of the pc
    ldr sp, TaskToSleepSP_Addr
    ldr sp, [sp]
    stmia sp!, {r0 - r12} ; store tasks registers in its real stack
    add sp, sp, #12
    stmia sp!, {lr} ; store task pc
    mov lr, pc
    adds pc, lr, #0 ; return to exception, and stay here
    mov r0, sp  ; store in r0 the task's stack
    MRS r1, apsr ; r1 = cpsr ; store in r1 the task cspr
    mov r2, lr ; r2 = lr, store in r2 the value of task lr
    ldr sp, TaskToSleepSP_Addr
    ldr sp, [sp]
    add sp, sp, #52
    stmia sp!, {r0} ; save the task's stack
    stmia sp!, {r2} ; save the task link register
    stmia sp!, {r1} ; save the task's cpsr
    ldr r0, TaskToExecuteSP_Addr ; we load the register of the new task to execute
    ldr r0, [r0]
    bl Starthghrdyprio
	.endasmfunc

_CpuId: .usect ".bss", 0, 4
	.global CpuId_Addr
CpuId_Addr: .word _CpuId


	.global _ReadCpuId
_ReadCpuId: .asmfunc
	STMDB sp!, {r0}  ; push the r0 into the stack so we can use it
	STMDB sp!, {r1}  ; push the r1 into the stack so we can use it
	MRC p15, #0, r0, c0, c0, #5 ; Read multiprocessor affinity register
	LDR r1, CpuId_Addr
	STR r0, [r1]
	LDMIA sp!, {r1}
	LDMIA sp!, {r0}
	MOV pc, lr
	.endasmfunc

Irq_Stack_Addr: .word Irq_Start_Stack
TaskToExecuteSP_Addr: .word TaskToExecuteSP
TaskToSleepSP_Addr: .word TaskToSleepSP
