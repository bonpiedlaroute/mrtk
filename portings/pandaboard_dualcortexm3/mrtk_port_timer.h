/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_PORT_TIMER
#define MRTK_PORT_TIMER

/*****************************************************/
/*              GIC Distributor Interface            */
/*                                                   */
/*****************************************************/
/* Generic interrupt controller Distributor Base Address*/
#define GICD_BASE   (0x48241000)

/* GIC Distributor Control Register offset */
#define GICD_CTLR   (0x00000000)

/* GIC Distributor Interrupt Set-Enable Registers Offset */
#define GICD_ISER   (0x00000100)

/* GIC Distributor Interrupt Clear-Pending Registers Offset */
#define GICD_ICPR   (0x00000280)

/* GIC Distributor Interrupt Priority Register Offset */
#define GICD_IPR    (0x00000400)

/* GIC Distributor Interrupt Processor Targets Register Offset */
#define GICD_IPTR   (0x00000800)

/* GIC Distributor Interrupt Configuration Register Offset */
#define GICD_ICFR   (0x00000C00)

/* GIC Distributor Software generated interrupt offset */
#define GICD_SGIR   (0x00000F00)

/* GIC Distributor Interrupt set-pending register offset */
#define GICD_ISPENDR   (0x00000200)


/*****************************************************/
/*              GIC Cpu Interface                    */
/*                                                   */
/*****************************************************/

/* GIC Cpu interface Base Address*/
#define GICC_BASE   (0x48240100)

/* GIC Cpu interface Control Register offset */
#define GICC_ICR    (0x00000000)

/* Interrupt Priority Mask */
#define GICC_PMR    (0x00000004)

/* Binary Point Register */
#define GICC_BPR    (0x00000008)

/* GIC interrupt acknowledge register */
#define GICC_IAR    (0x0000000C)

/* GIC End of interrupt reagister */
#define GICC_EOIR   (0x00000010)

/* Timer Interrrupt ID */
#define TIMER_INTERRUPT_ID          ( 29 )

#define SGI_INTERRUPT_ID    (1)

/* Arm cortex A9 Private Timers. */
#define TIMER_BASE                  ( 0x48240600)
/* Timer Load register */
#define TIMER_LOAD_REGISTER         ( ( volatile unsigned int * ) ( TIMER_BASE + 0x00 ) )
/* Timer control register */
#define TIMER_CONTROL_REGISTER      ( ( volatile unsigned int * ) ( TIMER_BASE + 0x08 ) )

/* Timer Control Register Flag */
#define ENABLE_IRQ                  ( 0x00000004 )
#define AUTO_RELOAD                 ( 0x00000002 )
#define ENABLE_TIMER                ( 0x00000001 )


#define CPU_CLOCK_HZ                ( ( unsigned int ) 700000000 )
#define CPU_PERIPH_HZ               ( CPU_CLOCK_HZ / 2 )

#define TIMER_PRESCALE              ( 99 )
#define TIMER_RATE_HZ               ( ( unsigned short ) 1000 )

/* functions to operate on GIC registers */
#define GIC_WRITE(address,value)    ( *( ( volatile unsigned int * )( address ) ) = ( value ) )
#define GIC_SET(address,value)      ( *( ( volatile unsigned int * )( address ) ) |= ( value ) )
#define GIC_READ(address)           ( *( ( volatile unsigned int * )( address ) ) )
#define GIC_CLEAR(address,value)    ( *( ( volatile unsigned int * )( address ) ) &= ~( value ) )

/* Interrupt Priority Levels. */
#define TIMER_INTERRUPT_PRIORITY    ( 0x08 )
#define SGI_INTERRUPT_PRIORITY    ( 0x0C )
#define LOWEST_INTERRUPT_PRIORITY   ( 0xFF )




/* Total Number of interrrupts */
#define MAX_NUMBER_OF_IRQ   128

/* Vector Table Offset Register Cortex-m3 */
#define VTOR	0xE000ED08



/*************************************/
#ifndef configKERNEL_INTERRUPT_PRIORITY
	#define configKERNEL_INTERRUPT_PRIORITY 255
#endif

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL		( ( volatile unsigned long *) 0xe000e010 )
#define portNVIC_SYSTICK_LOAD		( ( volatile unsigned long *) 0xe000e014 )
#define portNVIC_SYSPRI2			( ( volatile unsigned long *) 0xe000ed20 )
#define portNVIC_IRQPRI0			( ( volatile unsigned long *) 0xe000e400 )
#define portNVIC_IRQENABLE0			( ( volatile unsigned long *) 0xe000e100 )
#define portNVIC_SYSTICK_CLK		0x00000004
#define portNVIC_SYSTICK_INT		0x00000002
#define portNVIC_SYSTICK_ENABLE		0x00000001
#define portNVIC_IRQCORE_ENABLE		0x00000008
#define portNVIC_PENDSV_PRI			( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 16 )
#define portNVIC_SYSTICK_PRI		( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 24 )
#define portNVIC_IRQCORE_PRI		( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 24 )


#define configCPU_CLOCK_HZ			( ( unsigned long ) 200000000 )
#define configTICK_RATE_HZ			( ( unsigned long ) 1000 )

//	GIC_WRITE( GICC_BASE + GICC_PMR, 0x01 )
/* Functions for Critical Section Management */
#define mrtk_begincriticalsection()\
	*(portNVIC_SYSTICK_CTRL) |= portNVIC_SYSTICK_INT
//	GIC_WRITE( GICC_BASE + GICC_PMR, (unsigned int)0xFF )
#define mrtk_endcriticalsection()\
	*(portNVIC_SYSTICK_CTRL) &= ~portNVIC_SYSTICK_INT

extern void not_implemented();
extern void mrtk_timer_init(Uint32 period_us, void (*callback)());
extern void mrtk_systick_init();
extern void mrtk_generateSGI();
#endif
