/*
 * send_thread.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#ifndef __SEND_THREAD_H__
#define __SEND_THREAD_H__

/**
 * @brief Runs the functionality for the sending thread.
 * @param pData Address of user state data for the thread.
 * @remarks Sending queued chat messages in a thread allows the GUI to respond
 * anytime the user types a line and presses ENTER.
 */
void *SendThread(void *pData);

#endif /* __SEND_THREAD_H__ */
