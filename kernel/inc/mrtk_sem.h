/*
 *      Copyright (c) 2016 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef KERNEL_INC_MRTK_SEM_H_
#define KERNEL_INC_MRTK_SEM_H_

/**
 * @brief	create a semaphore, with a counter value
 *
 * @param [in]	Uint8 counter	value to initialize the semaphore with
 *
 * @return		EVENT*			semaphore handler, or NULL in case of issues
 */
EVENT* mrtk_SemCreate(Uint8 counter);

/**
 * @brief	wait for a semaphore
 *
 * @param	EVENT* pevent		semaphore event handler
 * @param	Uint32 timeout_ms	wait until the specify time to acquire the semaphore
 * @param	EVENT_ERROR* error	NO_ERROR if everything is ok
 *
 * @return	NONE
 */
void mrtk_SemPend(EVENT* pevent, Uint32 timeout_ms, EVENT_ERROR* error);

/**
 * @brief	release a semaphore
 *
 * @param 	EVENT* pevent		semaphore event handler
 * @param	EVENT_ERROR* error	NO_ERROR if everything is ok
 *
 * @return	NONE
 */
void mrtk_SemPost(EVENT* pevent, EVENT_ERROR* error);

#endif /* KERNEL_INC_MRTK_SEM_H_ */
