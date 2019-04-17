///////////////////////////////////////////////////////////////////////////////
// clientThreadManager.c - Routines to manage threads that are used to service
// communications to and from this server's clients.
//
// AUTHOR: Brian Hart
// CREATED DATE: 22 Mar 2019
// LAST UPDATED: 22 Mr 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for
// inspiration
//

/*
 * clientThreadManager.c
 *
 *  Created on: 22 Mar 2019
 *      Author: bhart
 */

#include <client_thread_functions.h>
#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_list.h"
#include "client_thread.h"
#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables

BOOL g_bShouldTerminateClientThread = FALSE;

///////////////////////////////////////////////////////////////////////////////
// GetSendingClientInfo function

LPCLIENTSTRUCT GetSendingClientInfo(void* pvClientThreadUserState) {
    if (pvClientThreadUserState == NULL) {
        HandleError(FAILED_GET_CLIENTSTRUCT_FROM_USER_STATE);
    }

    return (LPCLIENTSTRUCT)pvClientThreadUserState;
}

///////////////////////////////////////////////////////////////////////////////
// HandleProtocolCommand function - Deals with things we receive which appear
// to be commands that are specific to the chat protocol itself.
//

BOOL HandleProtocolCommand(LPCLIENTSTRUCT lpSendingClient, char* pszBuffer) {
    if (g_bShouldTerminateClientThread) {
        return TRUE;    // Means, "yes this protocol command got handled"
    }

    if (lpSendingClient == NULL) {
        // We do not have info referring to who sent this command, so stop.
        return FALSE;
    }

    if (pszBuffer == NULL || strlen(pszBuffer) == 0) {
        // Buffer containing the command we are handling is blank.
        // Nothing to do.
        return FALSE;
    }

    /* per protocol, HELO command is client saying hello to the server.
     * It does not matter whether a client socket has connected; that socket
     * has to say HELO first, so that then that client is marked as being
     * allowed to receive stuff. */
    if (strcasecmp(pszBuffer, PROTOCOL_HELO_COMMAND) == 0) {
        /* mark the current client as connected */
        lpSendingClient->bConnected = TRUE;

        /* Reply OK to the client */
        ReplyToClient(lpSendingClient, OK_FOLLOW_WITH_NICK_REPLY);

        return TRUE; /* command successfully handled */
    }

    /* Check whether the sending client is in the connected state.
     * We do not do this check earlier, since just in case the client sends
     * the HELO command, that is the only command a non-connected client
     * can even send. Otherwise, do not accept any further protocol commands
     * until a client has said HELO ("Hello!") to us. */
    if (lpSendingClient->bConnected == FALSE) {
        return FALSE;
    }

    if (strcasecmp(pszBuffer, MSG_TERMINATOR) == 0) {
        /* Signal for end of multi-line input received.  However, we
         * do not define this for the chat server (chat messages can only be one
         * line). */
        return FALSE;
    }

    /* per protocol, the NICK command establishes the user's chat nickname */
    char szReplyBuffer[BUFLEN];

    // StartsWith function is declared/defined in utils.h/.c
    if (StartsWith(pszBuffer, PROTOCOL_NICK_COMMAND)) {
        // let's parse this command with lpClientStructstrtok.  Protocol spec says this command is
        // NICK <chat-nickname>\n with no spaces allowed in the <chat-nickname>
        char* pszNickname = strtok(pszBuffer, " ");
        if (pszNickname != NULL) {
            /* the first call to strtok just gives us the word "NICK" which
             * we will just throw away.  */
            pszNickname = strtok(NULL, " ");
            if (pszNickname == NULL || strlen(pszNickname) == 0) {
                // Tell the client they are wrong for sending a blank
                // value for the nickname
                ReplyToClient(lpSendingClient, ERROR_NO_NICK_RECEIVED);
                return FALSE;
            }

            // Allocate a buffer to hold the nickname but not including the LF
            // on the end of the command string coming from the client
            lpSendingClient->pszNickname = (char*) malloc(
                    (strlen(pszNickname) - 1) * sizeof(char));

            // Copy the contents of the buffer referenced by pszNickname to that
            // referenced by lpClientStruct->pszNickname
            strncpy(lpSendingClient->pszNickname, pszNickname,
                    strlen(pszNickname) - 1);

            // Now send the user a reply telling them OK your nickname is <bla>
            sprintf(szReplyBuffer, OK_NICK_REGISTERED,
                    lpSendingClient->pszNickname);

            ReplyToClient(lpSendingClient, szReplyBuffer);

            /* Now, tell everyone (except the new guy)
             * that a new chatter has joined! Yay!! */

            sprintf(szReplyBuffer, NEW_CHATTER_JOINED,
                    lpSendingClient->pszNickname);

            /** Tell ALL connected clients that there's a new
             *  connected client. */
            BroadcastToAllClientsExceptSender(szReplyBuffer, lpSendingClient);
        }

        /* Return TRUE to signify command handled */
        return TRUE;
    }

    /* per protocol, client says bye bye server by sending the QUIT
     * command */
    if (StartsWith(pszBuffer, PROTOCOL_QUIT_COMMAND)) {
        sprintf(szReplyBuffer, NEW_CHATTER_LEFT, lpSendingClient->pszNickname);

        /* Give ALL connected clients the heads up that this particular chatter
         * is leaving the chat room (i.e., Elvis has left the building) */
        BroadcastToAllClientsExceptSender(szReplyBuffer, lpSendingClient);

        /* Tell the client who told us they want to quit, "Good bye sucka!" */
        ReplyToClient(lpSendingClient, OK_GOODBYE);

        // Mark this client as no longer being connected.
        lpSendingClient->bConnected = FALSE;

        // Save off the value of the thread handle of the client thread for this particular
        // client
        HTHREAD hClientThread = lpSendingClient->hClientThread;

        // Accessing the linked list -- make sure and use the mutex
        // to close the socket, to remove the client struct from the
        // list of clients, AND to decrement the global reference count
        // of connected clients
        LockMutex(g_hClientListMutex);
        {
            /* Inform the interactive user of the server of a client's
             * disconnection */
            LogInfo("C[%s:%d]: <disconnected>\n", lpSendingClient->szIPAddress,
                    lpSendingClient->nSocket);

            if (GetLogFileHandle() != stdout) {
                fprintf(stdout, "C[%s:%d]: <disconnected>\n",
                        lpSendingClient->szIPAddress, lpSendingClient->nSocket);
            }

            // Remove the client from the client list
            RemoveElement(&g_pClientList, &(lpSendingClient->nSocket),
                    FindClientBySocket);

            /* Close the TCP endpoint that led to the client, but do it
             * AFTER we have removed the client from the linked list! */
            close(lpSendingClient->nSocket);
            lpSendingClient->nSocket = -1;

            // remove the client data structure from memory
            free(lpSendingClient);
            lpSendingClient = NULL;
        }
        UnlockMutex(g_hClientListMutex);

        // now decrement the count of connected clients
        InterlockedDecrement(&g_nClientCount);

        // Check if the count of connected clients has dropped to zero.
        // If so, then shut down the server.  Inform the server's interactive
        // user.
        if (g_nClientCount == 0) {
            fprintf(stdout,
                    "Client count has dropped to zero.  Stopping server...\n");
            CleanupServer(OK);

            return TRUE;
        } else {
            LockMutex(g_hClientListMutex);
            {
                // If we are here, do not kill the server, but this client's
                // particular thread needs to stop
                if (INVALID_HANDLE_VALUE != hClientThread) {
                    KillThread(hClientThread);
                    sleep(1);   // force CPU context switch to trigger semaphore
                }
            }
            UnlockMutex(g_hClientListMutex);
        }

        // If we are here, the client count is still greater than zero, so
        // tell the caller the command has been handled
        return TRUE;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// KillClientThread function - A callback that is run for each element in the
// client list in order to kill each client's thread.
//

void KillClientThread(void* pClientStruct) {
    if (pClientStruct == NULL) {
        return;
    }

    LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT)pClientStruct;

    if (lpCS->hClientThread == INVALID_HANDLE_VALUE) {
        return;
    }

    KillThread(lpCS->hClientThread);

    sleep(1); /* force a CPU context switch so the semaphore can work */
}

///////////////////////////////////////////////////////////////////////////////
// Client thread management routines

void LaunchNewClientThread(LPCLIENTSTRUCT lpCS) {
	if (lpCS == NULL) {
		CleanupServer(ERROR);
	}

	HTHREAD hClientThread = CreateThreadEx(ClientThread, lpCS);

	if (INVALID_HANDLE_VALUE == hClientThread) {
		fprintf(stderr, FAILED_LAUNCH_CLIENT_THREAD);

		CleanupServer(ERROR);
	}

	// Save the handle to the newly-created thread in the CLIENTSTRUCT instance.
	lpCS->hClientThread = hClientThread;
}

///////////////////////////////////////////////////////////////////////////////
// PrependNicknameAndBroadcast function

void PrependNicknameAndBroadcast(const char* pszChatMessage,
        LPCLIENTSTRUCT lpSendingClient) {
    if (pszChatMessage == NULL || pszChatMessage[0] == '\0') {
        return;
    }

    if (lpSendingClient == NULL || !IsSocketValid(lpSendingClient->nSocket)) {
        return;
    }

    if (lpSendingClient->pszNickname == NULL
            || lpSendingClient->pszNickname[0] == '\0') {
        return;
    }

    if (lpSendingClient->bConnected == FALSE) {
        return;
    }

    const int NICKNAME_PREFIX_SIZE = strlen(lpSendingClient->pszNickname) + 4;

    if (NICKNAME_PREFIX_SIZE == 4) {
        return; // Nickname is blank, but we can't work with that since we need a value here.
    }

    // Make a buffer for putting a bang, the nickname, a colon, and then a space into.
    // Clients look for strings prefixed with a bang (!) and strip the bang and do not
    // show an "S: " before it in their UIs.
    char szNicknamePrefix[NICKNAME_PREFIX_SIZE];

    sprintf(szNicknamePrefix, "!%s: ", lpSendingClient->pszNickname);

    char *pszMessageToBroadcast = NULL;

    PrependTo(&pszMessageToBroadcast, szNicknamePrefix, pszChatMessage);

    if (pszMessageToBroadcast != NULL) {
        // Send the message to be broadcast to all the connected
        // clients except for the sender (per the requirements)
        BroadcastToAllClientsExceptSender(pszMessageToBroadcast,
                lpSendingClient);
    }
}


void TerminateClientThread(int signum) {
	// If signum is not equal to SIGSEGV, then ignore this semaphore
	if (SIGSEGV != signum) {
		return;
	}

	g_bShouldTerminateClientThread = TRUE;

	/* Re-associate this function with the signal */
	RegisterEvent(TerminateClientThread);
}


///////////////////////////////////////////////////////////////////////////////
