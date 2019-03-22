/*
 * clientThreadManager.h
 *
 *  Created on: Mar 22, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_THREAD_MANAGER_H__
#define __CLIENT_THREAD_MANAGER_H__

/**
 * @brief Creates and launches a new thread of execution to handle communications
 * with a particular client.
 * @param lpClientData Reference to an instance of a CLIENTSTRUCT structure that contains
 * data about the specific client to launch a new thread for.
 * @remarks The HTHREAD thread handle of the new thread is saved in the hClientThread
 * member of the CLIENTSTRUCT instance that is passed to this function.  This funciton
 * kills the whole program if the lpClientData parameter (which is required) is NULL.
 */
void LaunchNewClientThread(LPCLIENTSTRUCT lpClientData);

#endif /* __CLIENT_THREAD_MANAGER_H__ */
