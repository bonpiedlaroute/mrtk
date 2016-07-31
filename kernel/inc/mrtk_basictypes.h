/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_TYPES
#define MRTK_TYPES

#include <stdio.h>

/* unsigned data on 8 bits */
typedef unsigned char Uint8;

/* signed data on 8 bits */
typedef signed char Int8;

/* unsigned data on 16 bits */
//typedef unsigned short Uint16;

/* signed data on 16 bits */
//typedef signed short Int16;

/* unsigned data on 32 bits */
//typedef unsigned int Uint32;

/* signed data on 32 bits */
//typedef signed int Int32;

#ifndef DSP28_DATA_TYPES
#define DSP28_DATA_TYPES
typedef int             	int16;
typedef long            	int32;
typedef long long			int64;
typedef unsigned int    	Uint16;
typedef unsigned long   	Uint32;
typedef unsigned long long	Uint64;
typedef float           	float32;
typedef long double     	float64;



typedef int             	Int16;
typedef long            	Int32;
#endif

/* single precision floating point */
typedef float Float32;

/* double precision floating point */
typedef double Double;

typedef long long Long64;

#define MAX_UINT32_VALUE	0xFFFFFFFF

/* boolean type */
typedef enum
{
    TRUE = 0x33,
    FALSE = 0x77
} Boolean;

#endif
