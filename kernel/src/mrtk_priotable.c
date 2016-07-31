#include "mrtk.h"
#include "mrtk_priotable.h"


/* table of ready queue */
Uint32 RdyPrioTable[NB_GROUP];

/* the group of ready task */
Uint8 RdyGrp = 0;

/* table of running queue */
Uint32 RunPrioTable[NB_GROUP];

/* the group of running queue */
Uint8 RunGrp = 0;


/*
                mrtk_findhghbitsetonright32

@brief:
            This function finds the first bit set on the right
            in the 32 bits value passed in parameter. If the value passed
            in parameter is zero, the return value will be 31.

Argument:
        ip_value the number on which we find the first bit set

@return:
        Uint32  the index of the first bit set (between 0 and 31)
*/
Uint32 mrtk_findhghbitsetonright32(Uint32 ip_value)
{
    Uint32 hghbitset = 0;

    /* is the first bit equal to 1? */
    if( ip_value & (Uint32)0x1 )
    {
        /* we are done! */
        hghbitset = 0;
    }
    else
    {
        /* There is at least one zero */
        hghbitset = 1;

        /* are the 16 first bits equal to zero? */
        if( (ip_value & (Uint32)0xFFFF) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value >>= 16;
            hghbitset += 16;
        }

        /* are the 8 next bits equal to zero? */
        if( (ip_value & (Uint32)0xFF) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value >>=8;
            hghbitset += 8;
        }

        /* are the 4 next bits equal to zero? */
        if( (ip_value & (Uint32)0xF) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value >>= 4;
            hghbitset += 4;
        }

        /* are the two next bits equal to zero? */
        if( (ip_value & (Uint32)0x3) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value >>= 2;
            hghbitset += 2;
        }

        hghbitset -= (ip_value & (Uint32)0x1);
    }

    return hghbitset;
}

/*
                mrtk_findhghbitsetonright8

@brief:
            This function finds the first bit set on the right
            in the 8 bits value passed in parameter. If the value passed
            in parameter is zero, the return value will be 7.

Argument:
        ip_value the number on which we find the first bit set

@return:
        Uint32  the index of the first bit set (between 0 and 7)
*/
Uint8 mrtk_findhghbitsetonright8(Uint8 ip_value)
{
    Uint8 hghbitset = 0;

    /* is the first bit equal to 1? */
    if( ip_value & (Uint32)0x1 )
    {
        /* we are done! */
        hghbitset = 0;
    }
    else
    {
        /* we have at least one zero */
        hghbitset = 1;

        /* are the 4 first bits equal to zero? */
        if( (ip_value & (Uint32)0xF) == 0 )
        {
            ip_value >>= 4;
            hghbitset += 4;
        }

        /* are the 2 next bits equal to zero? */
        if( (ip_value & (Uint32)0x3) == 0 )
        {
            ip_value >>= 2;
            hghbitset += 2;
        }

        hghbitset -= (ip_value & (Uint8)0x1);
    }

    return hghbitset;
}

/*
                mrtk_findhghbitsetonleft32

@brief:
            This function finds the first bit set on the left
            in the 32 bits value passed in parameter. If the value passed
            in parameter is zero, the return value will be zero.

Argument:
        ip_value the number on which we find the first bit set

@return:
        Uint32  the index of the first bit set
*/
Uint32 mrtk_findhghbitsetonleft32(Uint32 ip_value)
{
    Uint32 hghbitset = 31;

    /* is the first bit equal to 1? */
    if( ip_value & (Uint32)0x80000000 )
    {
        /* we are done! */
        hghbitset = 31;
    }
    else
    {
        /* There is at least one zero */
        hghbitset = 30;

        /* are the 16 first bits equal to zero? */
        if( (ip_value & (Uint32)0xFFFF0000) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value <<= 16;
            hghbitset -= 16;
        }

        /* are the 8 next bits equal to zero? */
        if( (ip_value & (Uint32)0xFF000000) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value <<=8;
            hghbitset -= 8;
        }

        /* are the 4 next bits equal to zero? */
        if( (ip_value & (Uint32)0xF0000000) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value <<= 4;
            hghbitset -= 4;
        }

        /* are the two next bits equal to zero? */
        if( (ip_value & (Uint32)0xC0000000) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value <<= 2;
            hghbitset -= 2;
        }

        if( (ip_value & (Uint32)0x2) != 0 )
        {
            hghbitset += 1;
        }
    }

    return hghbitset;
}

/*
                mrtk_findhghbitsetonleft8

@brief:
            This function finds the first bit set on the left
            in the 8 bits value passed in parameter. If the value passed
            in parameter is zero, the return value will be zero.

Argument:
        ip_value the number on which we find the first bit set

@return:
        Uint32  the index of the first bit set
*/
Uint32 mrtk_findhghbitsetonleft8(Uint8 ip_value)
{
    Uint32 hghbitset = 7;

    /* is the first bit equal to 1? */
    if( ip_value & (Uint8)0x80 )
    {
        /* we are done! */
        hghbitset = 7;
    }
    else
    {
        /* There is at least one zero */
        hghbitset = 6;

        /* are the 4 next bits equal to zero? */
        if( (ip_value & (Uint8)0xF0) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value <<= 4;
            hghbitset -= 4;
        }

        /* are the two next bits equal to zero? */
        if( (ip_value & (Uint8)0xC0) == 0 )
        {
            /* skip these bits and add it in the counter */
            ip_value <<= 2;
            hghbitset -= 2;
        }

        if( (ip_value & (Uint8)0x2) != 0 )
        {
            hghbitset += 1;
        }
    }

    return hghbitset;
}
/*

                mrtk_InitPrioTable

@brief :
                This function initializes all queues in mrtk

Argument:

@return:

*/
void mrtk_InitPrioTable()
{
    Uint32 i = 0;
    
    for(i = 0; i < NB_GROUP; i ++)
    {
        RdyPrioTable[i] = 0;
        RunPrioTable[i] = 0;
    }
}
