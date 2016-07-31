/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk_utils.h"

/*

                    mrtk_strlen
    @brief: This function computes the lenght of a string


    Argument: 
        ip_name   the string to compute the lenght

    return: 
        the lenght of the string

*/
Uint32 mrtk_strlen(Uint8* ip_name)
{
    Uint32 i = 0;

    if( ip_name != NULL)
    {
        while( *ip_name != '\0' )
        {
            i++;
            ip_name++;
        }

    }

    return i;
}
