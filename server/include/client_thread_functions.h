/*
 * clientThreadManager.h
 *
 *  Created on: Mar 22, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_THREAD_MANAGER_H__
#define __CLIENT_THREAD_MANAGER_H__

#include "client_struct.h"

#ifndef FAILED_GET_CLIENTSTRUCT_FROM_USER_STATE
#define FAILED_GET_CLIENTSTRUCT_FROM_USER_STATE \
    "client thread: Failed to get client information from user state.\n"
#endif //FAILED_GET_CLIENTSTRUCT_FROM_USER_STATE

/**
 * @brief Indicates whether a client thread should be terminated.
 */
extern BOOL g_bShouldTerminateClientThread;

/**
 * @brief Extracts the address of a CLIENTSTRUCT instance from the user state
 * bag passed to the client thread.
 * @param pvClientThreadUserState Address of a storage location containing user
 * state data that had been passed to the client thread upon its creation.
 * @returns Pointer to a CLIENTSTRUCT instance containing information on the
 * client who sent the communications, or NULL if it can't be obtained.
 */
LPCLIENTSTRUCT GetSendingClientInfo(void* pvClientThreadUserState);

/**
 * @brief Checks received data for protocol-specific commands and handles them.
 * @param lpSendingClient Address of a CLIENTSTRUCT instance that contains
 * information on the client who sent the command.
 * @param pszBuffer Address of a character array containing the text that was
 * received.
 * @returns TRUE if the text received contained a protocol command and no
 * further processing is needed; FALSE otherwise.
 */
BOOL HandleProtocolCommand(LPCLIENTSTRUCT lpSendingClient, char* pszBuffer);

/**
 * @brief Callback that is called for each entry in the client list to kill
 * client threads that are no longer needed for communications.
 * @param pClientStruct Address of an instance of CLIENTSTRUCT containing
 * information about the thread that is being used to communicate with the
 * client at the current element in the list.
 */
void KillClientThread(void* pClientStruct);

/**
 * @brief Creates and launches a new thread of execution to handle
 * communications with a particular client.
 * @param lpCS Reference to an instance of a CLIENTSTRUCT structure
 * that contains data about the specific client to launch a new thread for.
 * @remarks The HTHREAD thread handle of the new thread is saved in the
 * hClientThread member of the CLIENTSTRUCT instance that is passed to this
 * function.  This funciton kills the whole program if the lpClientData
 * parameter (which is required) is NULL.
 */
void LaunchNewClientThread(LPCLIENTSTRUCT lpCS);

/**
 * @brief Takes the specified chat message and prepends the nickname of the
 * sending client to it
 * @param pszChatMessage Address of the character array containing the chat
 * message.
 * @param lpSendingClient Reference to a CLIENTSTRUCT instance containing data
 * on the client who sent the chat message.
 * @remarks When a particular chatter in a chat room sends a message, the
 * other people want to know who sent the message, so we prepend the message
 * with the sender's chat handle (aka nickname) prior to sending to the other
 * clients.
 */
void PrependNicknameAndBroadcast(const char* pszChatMessage,
        LPCLIENTSTRUCT lpSendingClient);
/**
 * @brief Semaphore that gets signaled to indicate that this client thread
 * should terminate in an orderly fashion.
 * @param signum Signal code corresponding to the signal that triggered this
 * semaphore.
 */
void TerminateClientThread(int signum);

#endif /* __CLIENT_THREAD_MANAGER_H__ */
