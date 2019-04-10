/*
 * receive_thread.h
 *
 *  Created on: Apr 10, 2019
 *      Author: bhart
 */

#ifndef __RECEIVE_THREAD_H__
#define __RECEIVE_THREAD_H__

extern HTHREAD g_hReceiveThread;

void *ReceiveThread(void *pvData);

#endif /* __RECEIVE_THREAD_H__ */
