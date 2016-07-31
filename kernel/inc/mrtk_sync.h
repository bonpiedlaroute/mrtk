/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK_SYNC_H
#define MRTK_SYNC_H
#include "mrtk_port.h"

extern void mrtk_lock(LockHandler *ipp_lock);
extern void mrtk_unlock(LockHandler *ipp_lock);

extern void mrtk_SetChoosing(LockHandler *lockhandler,Uint16 cpuid, Uint16 value);
extern Uint16 mrtk_GetChoosing(LockHandler *lockhandler, Uint16 j);
extern Uint16 mrtk_GetMaximumNumber(LockHandler *lockhandler);
extern void mrtk_SetNumber(LockHandler *lockhandler, Uint16 cpuid, Uint16 my_number);
extern Uint16 mrtk_GetNumber(LockHandler *lockhandler, Uint16 j);
extern Boolean mrtk_IsMyTurn(LockHandler* lockhandler, Uint16 j, Uint16 number_j, Uint16 cpuid, Uint16 my_number);
#endif
