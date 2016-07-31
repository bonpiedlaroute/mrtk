/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */

#ifndef MRTK_MSGQUEUE_H_
#define MRTK_MSGQUEUE_H_
#include "mrtk.h"

extern EVENT* mrtk_Qcreate(void ** qbegin, Uint16 size);

extern void* mrtk_Qpend(EVENT* event, Uint32 timeout_ms, EVENT_ERROR* error);

extern void mrtk_Qpost(EVENT* pevent, void * msg, EVENT_ERROR* error);

#endif /* MRTK_MSGQUEUE_H_ */
