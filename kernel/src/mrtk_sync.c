/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#include "mrtk.h"
#include "mrtk_port.h"
#include "mrtk_sync.h"
#include "mrtk_port_timer.h"

/*
                mrtk_lock
@brief    	allows to take control on to the resource linked to this lockhandler.
			to lock resource shared in multicore processor system
@param
            ipp_lock     pointer to the lock handler.
@return
*/

void mrtk_lock(LockHandler *ipp_lock)
{
#ifdef MRTK_ENABLE_HARDWARE_TEST_SET
    Boolean result = FALSE;

    do
    {
        result = mrtk_HardwareAtomicTest(ipp_lock);

    }while(result == FALSE);

    return;
#else
    Uint16 cpuid = mrtk_GetCpuId();
    Uint16 my_number = 0, number_j = 0,j = 0, choose = 0;
    Boolean my_turn = FALSE;

    mrtk_SetChoosing(ipp_lock, cpuid, 1); /* indicate that we are choosing our number */
    my_number = 1 + mrtk_GetMaximumNumber(ipp_lock);/* get the last number */
    mrtk_SetNumber(ipp_lock, cpuid, my_number);
    mrtk_SetChoosing(ipp_lock, cpuid, 0); /* choosing step terminated */

    for(j  = 0; j < NbProcessors; j++)
    {
    	if( j == cpuid )
    		continue;
    	do
    	{/* wait until, choosing step on this processor is finished */
    		choose = mrtk_GetChoosing(ipp_lock, j);
    	} while( choose != 0 );
    	do
    	{ /* wait until, it is our turn */
    		number_j = mrtk_GetNumber(ipp_lock, j);
    		my_turn = mrtk_IsMyTurn(ipp_lock, j, number_j, cpuid, my_number);
    	}while( number_j != 0 && my_turn == FALSE );
    }
#endif
}

/*
            mrtk_unlock
@brief		allows to release control on to resource linked to this lock handler.
			unlock resource shared in multicore processor system
@param
            ipp_lock     pointer to the lock handler.
                        It must be a pointer to Boolean
@return
*/
void mrtk_unlock(LockHandler *ipp_lock)
{
#ifdef MRTK_ENABLE_HARDWARE_TEST_SET
    mrtk_HardwareAtomicSet(ipp_lock);
#else
    Uint16 cpuid = mrtk_GetCpuId();
    mrtk_SetNumber(ipp_lock, cpuid, 0);/* reset our number */
#endif

}
