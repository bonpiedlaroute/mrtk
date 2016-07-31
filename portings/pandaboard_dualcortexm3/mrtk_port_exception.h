/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_PORT_EXCEPTION_H_
#define MRTK_PORT_EXCEPTION_H_

#define MAIN_PROGRAM_STACK_SIZE	1024

extern void (*irq_table_function[])(void);
extern void (*Vector_Core0[])(void);

extern void do_undefined_instruction();
extern void do_prefetch_abort();
extern void do_data_abort();
extern void do_not_used();
extern void do_fiq();
extern void do_irq();

#endif /* MRTK_PORT_EXCEPTION_H_ */
