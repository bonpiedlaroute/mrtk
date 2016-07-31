/****************************************************************************/
/*  OMAP4460.cmd                                                            */
/*  Copyright (c) 2012  Texas Instruments Incorporated                      */
/*  Author: Rafael de Souza                                                 */
/*                                                                          */
/*    Description: This file is a sample linker command file that can be    */
/*                 used for linking programs built with the C compiler and  */
/*                 running the resulting .out file on an OMAP4460.          */
/*                 Use it as a guideline.  You will want to                 */
/*                 change the memory layout to match your specific          */
/*                 target system.  You may want to change the allocation    */
/*                 scheme according to the size of your program.            */
/*                                                                          */
/****************************************************************************/

MEMORY
{
#ifdef M3_CORE     /* A9 memory map */

    //L3_OCM_RAM      o = 0x40300000  l = 0x0000E000  /* 56kB L3 Internal memory */
    //EMIF_CS0_SDRAM  o = 0x80000000  l = 0x10000000  /* 1GB external SDRAM memory on CS0 */
    //EMIF_CS1_SDRAM  o = 0xC0000000  l = 0x10000000  /* 1GB external SDRAM memory on CS1 */

	//L3_OCM_RAM      o = 0x40300000  l = 0x0000E000  /* 56kB L3 Internal memory */
	VECTS_CORE0		o = 0x00000800	l = 0x00000500
	VECTS_CORE1		o = 0x00000D00	l = 0x00000500
	M3RAM           o = 0x55020000  l = 0x00010000  /* 64kB local RAM */
    EMIF_CS1_SDRAM  o = 0x80000000  l = 0x00800000  /* 500 MB external SDRAM memory on CS0 */
    EMIF_CS0_SDRAM  o = 0x80800000  l = 0x00800000  /* 500 MB external SDRAM memory on CS1 */
    EMIF_CS2_SDRAM  o = 0xC0000000  l = 0x10000000  /* 1GB external SDRAM memory on CS2 */

#else               /* M3 memory map */

    M3ROM           o = 0x55000000  l = 0x00004000  /* 16kB local ROM */
    M3RAM           o = 0x55020000  l = 0x00010000  /* 64kB local RAM */

#endif
}

SECTIONS
{
#ifdef M3_CORE    /* A9 memory map */

	.vectors {mrtk_port_exception.obj (.data:Vector_Core0)}	   >  VECTS_CORE1
    .text          >  EMIF_CS1_SDRAM
    .stack         >  EMIF_CS1_SDRAM
    .bss           >  EMIF_CS1_SDRAM
    .cio           >  EMIF_CS1_SDRAM
    .const         >  EMIF_CS1_SDRAM
    .data          >  EMIF_CS1_SDRAM
    .switch        >  EMIF_CS1_SDRAM
    .sysmem        >  EMIF_CS1_SDRAM
    .far           >  EMIF_CS1_SDRAM
    .args          >  EMIF_CS1_SDRAM
    .ppinfo        >  EMIF_CS1_SDRAM
    .ppdata        >  EMIF_CS1_SDRAM
    /* TI-ABI sections */
    .pinit         >  EMIF_CS1_SDRAM
    .cinit         >  EMIF_CS1_SDRAM
  
    /* EABI sections */
    .binit         >  EMIF_CS1_SDRAM
    .init_array    >  EMIF_CS1_SDRAM
    .neardata      >  EMIF_CS1_SDRAM
    .fardata       >  EMIF_CS1_SDRAM
    .rodata        >  EMIF_CS1_SDRAM
    .c6xabi.exidx  >  EMIF_CS1_SDRAM
    .c6xabi.extab  >  EMIF_CS1_SDRAM


#else              /* M3 memory map */

    .text          >  M3RAM
    .stack         >  M3RAM
    .bss           >  M3RAM
    .cio           >  M3RAM
    .const         >  M3RAM
    .data          >  M3RAM
    .switch        >  M3RAM
    .sysmem        >  M3RAM
    .far           >  M3RAM
    .args          >  M3RAM
    .ppinfo        >  M3RAM
    .ppdata        >  M3RAM
  
    /* TI-ABI or COFF sections */
    .pinit         >  M3RAM
    .cinit         >  M3RAM
  
    /* EABI sections */
    .binit         >  M3RAM
    .init_array    >  M3RAM
    .neardata      >  M3RAM
    .fardata       >  M3RAM
    .rodata        >  M3RAM
    .c6xabi.exidx  >  M3RAM
    .c6xabi.extab  >  M3RAM

#endif
}
