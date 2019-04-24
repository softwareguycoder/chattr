///////////////////////////////////////////////////////////////////////////////
// client_thread_functions.c - Routines to manage threads that are used to
// service communications to and from this server's clients.
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

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_manager.h"
#include "client_list_manager.h"
#include "client_thread.h"
#include "client_thread_functions.h"
#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables

BOOL g_bShouldTerminateClientThread = FALSE;

///////////////////////////////////////////////////////////////////////////////
// BroadcastChatMessage function

void BroadcastChatMessage(const char* pszChatMessage,
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

///////////////////////////////////////////////////////////////////////////////
// EndChatSession function

BOOL EndChatSession(LPCLIENTSTRUCT lpSendingClient) {
    if (lpSendingClient == NULL) {
        return FALSE;
    }

    char szReplyBuffer[BUFLEN];

    sprintf(szReplyBuffer, NEW_CHATTER_LEFT, lpSendingClient->pszNickname);

    /* Give ALL connected clients the heads up that this particular chatter
     * is leaving the chat room (i.e., Elvis has left the building) */
    BroadcastToAllClientsExceptSender(szReplyBuffer, lpSendingClient);

    /* Tell the client who told us they want to quit, "Good bye sucka!" */
    ReplyToClient(lpSendingClient, OK_GOODBYE);

    // Mark this client as no longer being connected.
    lpSendingClient->bConnected = FALSE;

    // Save off the value of the thread handle of the client thread for
    // this particular client
    HTHREAD hClientThread = lpSendingClient->hClientThread;

    // Accessing the linked list -- make sure and use the mutex
    // to close the socket, to remove the client struct from the
    // list of clients, AND to decrement the global reference count
    // of connected clients
    LockMutex(g_hClientListMutex);
    {
        /* Inform the interactive user of the server of a client's
         * disconnection */
        LogInfoToFileAndScreen(CLIENT_DISCONNECTED,
                lpSendingClient->szIPAddress, lpSendingClient->nSocket);

        // Remove the client from the client list
        if (!RemoveElement(&g_pClientList, &(lpSendingClient->nSocket),
                FindClientBySocket)) {
            return FALSE;   // Failed to remove the client from the list
        }

        KillThread(hClientThread);
        sleep(1);   // force CPU context switch to trigger semaphore

        /* Close the TCP endpoint that led to the client, but do it
         * AFTER we have removed the client from the linked list! */
        CloseSocket(lpSendingClient->nSocket);
        lpSendingClient->nSocket = INVALID_SOCKET_HANDLE;

        // remove the client data structure from memory
        free(lpSendingClient);
        lpSendingClient = NULL;
    }
    UnlockMutex(g_hClientListMutex);

    // now decrement the count of connected clients
    InterlockedDecrement(&g_nClientCount);

    LockMutex(g_hClientListMutex);
    {
        if (g_nClientCount == 0) {
            LogInfoToFileAndScreen("Client count has dropped to zero.\n");
        }
    }
    UnlockMutex(g_hClientListMutex);

    // If we are here, the client count is still greater than zero, so
    // tell the caller the command has been handled
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GetSendingClientInfo function

LPCLIENTSTRUCT GetSendingClientInfo(void* pvClientThreadUserState) {
    if (pvClientThreadUserState == NULL) {
        HandleError(FAILED_GET_CLIENTSTRUCT_FROM_USER_STATE);
    }

    return (LPCLIENTSTRUCT) pvClientThreadUserState;
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

    if (IsNullOrWhiteSpace(pszBuffer)) {
        // Buffer containing the command we are handling is blank.
        // Nothing to do.
        return FALSE;
    }

    /* per protocol, HELO command is client saying hello to the server.
     * It does not matter whether a client socket has connected; that socket
     * has to say HELO first, so that then that client is marked as being
     * allowed to receive stuff. */
    if (strcasecmp(pszBuffer, PROTOCOL_HELO_COMMAND) == 0) {
        ProcessHeloCommand(lpSendingClient);

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

    // StartsWith function is declared/defined in utils.h/.c
    if (StartsWith(pszBuffer, PROTOCOL_NICK_COMMAND)) {
        return RegisterClientNickname(lpSendingClient, pszBuffer);
    }

    char szReplyBuffer[BUFLEN];

    /* per protocol, client says bye bye server by sending the QUIT
     * command */
    if (StartsWith(pszBuffer, PROTOCOL_QUIT_COMMAND)) {
        return EndChatSession(lpSendingClient);
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

    LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT) pClientStruct;

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

void ProcessHeloCommand(LPCLIENTSTRUCT lpSendingClient) {
    if (NULL == lpSendingClient) {
        return;
    }

    /* mark the current client as connected */
    lpSendingClient->bConnected = TRUE;

    /* Reply OK to the client */
    ReplyToClient(lpSendingClient, OK_FOLLOW_WITH_NICK_REPLY);
}

///////////////////////////////////////////////////////////////////////////////
// ReceiveFromServer function - Does a one-off, synchronous receive (not a
// polling loop) of a specific message from the server.  Blocks the calling
// thread until the message has arrived.
//

int ReceiveFromClient(LPCLIENTSTRUCT lpSendingClient, char** ppszReplyBuffer) {
    if (lpSendingClient == NULL) {
        fprintf(stderr, ERROR_NO_SENDING_CLIENT_SPECIFIED);

        CleanupServer(ERROR);
    }

    // Check whether we have a valid endpoint for talking with the server.
    if (!IsSocketValid(lpSendingClient->nSocket)) {
        fprintf(stderr, FAILED_RECEIVE_TEXT_FROM_CLIENT);

        CleanupServer(ERROR);
    }

    if (ppszReplyBuffer == NULL) {
        fprintf(stderr, FAILED_RECEIVE_TEXT_FROM_CLIENT);

        CleanupServer(ERROR);
    }

    /* Wipe away any existing reply buffer by filling it with null
     * terminators. If the reply buffer is not allocated, then that is
     * fine. */
    if (*ppszReplyBuffer != NULL) {
        memset(*ppszReplyBuffer, 0, strlen(*ppszReplyBuffer));
    }

    /* Do a receive. Cleanup if the operation was not successful. */
    int nBytesReceived = 0;

    if ((nBytesReceived = Receive(lpSendingClient->nSocket, ppszReplyBuffer))
            <= 0 && errno != EBADF && errno != EWOULDBLOCK) {
        FreeBuffer((void**) ppszReplyBuffer);

        fprintf(stderr, FAILED_RECEIVE_TEXT_FROM_CLIENT);

        CleanupServer(ERROR);
    }

    /* Inform the server console's user how many bytes we got. */
    LogInfoToFileAndScreen(CLIENT_BYTES_RECD_FORMAT,
            lpSendingClient->szIPAddress, lpSendingClient->nSocket,
            nBytesReceived);

    /* Save the total bytes received from this client */
    lpSendingClient->nBytesReceived += nBytesReceived;

    // Log what the client sent us to the server's interactive
    // console and the log file, unless they're the same, then
    // just send the output to the console.
    LogInfoToFileAndScreen(CLIENT_DATA_FORMAT, lpSendingClient->szIPAddress,
            lpSendingClient->nSocket, *ppszReplyBuffer);

    // Return the number of received bytes
    return nBytesReceived;
}

BOOL RegisterClientNickname(LPCLIENTSTRUCT lpSendingClient, char* pszBuffer) {
    if (lpSendingClient == NULL) {
        return FALSE;   // command not handled
    }

    if (IsNullOrWhiteSpace(pszBuffer)) {
        return FALSE;   // command not handled
    }

    /* per protocol, the NICK command establishes the user's chat nickname */
    char szReplyBuffer[BUFLEN];

    // let's parse this command with lpClientStructstrtok.
    // Protocol spec says this command is NICK <chat-nickname>\n with no spaces
    // allowed in the <chat-nickname>.  The nickname can only be 15 or less
    // characters long.  Nicknames can only be alphanumeric.
    char* pszNickname = strtok(pszBuffer, " ");
    if (pszNickname != NULL) {
        /* the first call to strtok just gives us the word "NICK" which
         * we will just throw away.  */
        pszNickname = strtok(NULL, " ");
        if (pszNickname == NULL || strlen(pszNickname) == 0) {
            // Tell the client they are wrong for sending a blank
            // value for the nickname
            ReplyToClient(lpSendingClient, ERROR_NO_NICK_RECEIVED);
            return FALSE;   // command not handled
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

        /** Tell ALL connected clients  (except the one that just
         * joined) that there's a new connected client. */
        BroadcastToAllClientsExceptSender(szReplyBuffer, lpSendingClient);

        return TRUE;    // command handled successfully
    }

    return FALSE;   // command not handled
}

int SendToClient(LPCLIENTSTRUCT lpCurrentClient, const char* pszMessage) {
    if (g_bShouldTerminateClientThread) {
        return ERROR;
    }

    if (lpCurrentClient == NULL) {
        return ERROR;
    }

    if (IsNullOrWhiteSpace(pszMessage)) {
        return ERROR;
    }

    if (!IsSocketValid(lpCurrentClient->nSocket)) {
        return ERROR;
    }

    if (lpCurrentClient->bConnected == FALSE) {
        /* client has not issued the HELO command yet, so it does
         * not get included on broadcasts */
        return ERROR;
    }

    return Send(lpCurrentClient->nSocket, pszMessage);
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
