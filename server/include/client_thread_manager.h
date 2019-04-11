/*
 * clientThreadManager.h
 *
 *  Created on: Mar 22, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_THREAD_MANAGER_H__
#define __CLIENT_THREAD_MANAGER_H__

#include "client_struct.h"

/**
 * @brief Indicates whether a client thread should be terminated.
 */
extern BOOL g_bShouldTerminateClientThread;

/**
 * @brief Creates and launches a new thread of execution to handle communications
 * with a particular client.
 * @param lpCS Reference to an instance of a CLIENTSTRUCT structure
 * that contains data about the specific client to launch a new thread for.
 * @remarks The HTHREAD thread handle of the new thread is saved in the
 * hClientThread member of the CLIENTSTRUCT instance that is passed to this
 * function.  This funciton kills the whole program if the lpClientData
 * parameter (which is required) is NULL.
 */
void LaunchNewClientThread(LPCLIENTSTRUCT lpCS);

/**
 * @brief Semaphore that gets signaled to indicate that this client thread
 * should terminate in an orderly fashion.
 * @param signum Signal code corresponding to the signal that triggered this
 * semaphore.
 */
void TerminateClientThread(int signum);

#endif /* __CLIENT_THREAD_MANAGER_H__ */
