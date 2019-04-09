/*
 * send_thread.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef __SEND_THREAD_H__
#define __SEND_THREAD_H__

#include <../../../threading_core/threading_core/include/threading_core.h>

/* global handle to the send thread */
extern HTHREAD hSendThread;

void *SendThread(void *pData);

#endif /* __SEND_THREAD_H__ */
