// client_manager.c - Implementation of functionality to drive communication
// of this server with its clients.

#include "stdafx.h"
#include "server.h"

#include "client_manager.h"
#include "client_list_manager.h"
#include "client_thread_functions.h"
#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// BroadcastToAllClients: Sends the indicated message to all the clients,
// including the one who sent it.  This is mostly used for server-administrative
// messages which are of general interest, such as 'New chatter joined' or
// "this chatter left the room" etc.
//

int BroadcastToAllClients(const char* pszMessage) {
    if (g_bShouldTerminateClientThread) {
        return ERROR;
    }

    if (IsNullOrWhiteSpace(pszMessage)) {
        // The message to broadcast is blank; nothing to do.
        return 0;
    }

    int nTotalBytesSent = 0;

    LogInfoToFileAndScreen(SERVER_DATA_FORMAT, pszMessage);

    LockMutex(g_hClientListMutex);
    {
        // If there are zero clients in the list of connected clients,
        // then continuing is pointless, isn't it?
        if (g_nClientCount == 0) {
            // No clients are connected; nothing to do.
            return 0;
        }

        POSITION* pos = GetHeadPosition(&g_pClientList);
        if (pos == NULL) {
            // No clients are connected; nothing to do.
            return 0;
        }

        do {

            int nBytesSent = 0;

            LPCLIENTSTRUCT lpCurrentClient = (LPCLIENTSTRUCT) pos->pvData;
            if (lpCurrentClient == NULL) {
                continue;
            }

            if ((nBytesSent = SendToClient(lpCurrentClient, pszMessage)) > 0) {
                nTotalBytesSent += nBytesSent;
            }

        } while ((pos = GetNext(pos)) != NULL);
    }
    UnlockMutex(g_hClientListMutex);

    return nTotalBytesSent;
}

///////////////////////////////////////////////////////////////////////////////
// BroadcastToAllClientsExceptSender function: Sends a chat message to everyone
// in the room except the client who sent it.
//

int BroadcastToAllClientsExceptSender(const char* pszMessage,
        LPCLIENTSTRUCT lpSendingClient) {

    if (g_bShouldTerminateClientThread)
        return ERROR;

    if (IsNullOrWhiteSpace(pszMessage)) {
        // Chat message to broadcast is blank; nothing to do.
        return 0;
    }

    if (lpSendingClient == NULL) {
        // The data structure reference for the sending client is NULL;
        // as this information is necessary to carry out this function's task,
        // there's nothing to do.
        return 0;
    }

    int nTotalBytesSent = 0;

    LogInfoToFileAndScreen(SERVER_DATA_FORMAT, pszMessage);

    LockMutex(g_hClientListMutex);
    {
        POSITION* pos = GetHeadPosition(&g_pClientList);
        if (pos == NULL) {
            // There is nothing in the linked list of clients; nothing to do.
            return 0;
        }

        do {

            int nBytesSent = 0;

            LPCLIENTSTRUCT lpCurrentClient = (LPCLIENTSTRUCT) pos->pvData;
            if (lpCurrentClient == NULL) {
                continue;
            }

            // If we have the client list entry for the sender, skip it,
            // since this function does not broadcast back to the sender.
            if (lpCurrentClient->nSocket == lpSendingClient->nSocket) {
                continue;
            }

            if ((nBytesSent = SendToClient(lpCurrentClient, pszMessage)) > 0) {
                nTotalBytesSent += nBytesSent;
            }

        } while ((pos = GetNext(pos)) != NULL);
    }
    UnlockMutex(g_hClientListMutex);

    // Return the total bytes sent to the caller
    return nTotalBytesSent;
}

///////////////////////////////////////////////////////////////////////////////
// DisconnectClient function - A callback that is called for every currently-
// connected client in the client list, to disconnect them when the server
// is exited by the server console's user.
//

void ForceDisconnectionOfClient(void* pClientStruct) {
    if (pClientStruct == NULL) {
        // Null value for the pClientStruct parameter; nothing to do.
        return;
    }

    // Forcibly disconnect this client
    ForciblyDisconnectClient((LPCLIENTSTRUCT) pClientStruct);
}

///////////////////////////////////////////////////////////////////////////////
// ForciblyDisconnectClient function - used when the server console's user
// kills the server, to sever connections with its clients.
//

void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCS) {
    // lpCS is the reference to the structure containing
    // information for the client whose connection you want to sever

    if (lpCS == NULL) {
        // Null value provided for the client structure; nothing to do.
        return;
    }

    /* Check whether there is still a valid socket file descriptor
     * available for the client endpoint... */
    if (!IsSocketValid(lpCS->nSocket)) {
        // Invalid socket file descriptor available for this client; nothing
        // to do.
        return;
    }

    /* Remove this client from the list of clients */
    if (!RemoveElement(&g_pClientList, &(lpCS->nSocket), FindClientBySocket)) {
        return;
    }

    /* Decrement the count of connected clients */
    InterlockedDecrement(&g_nClientCount);

    /* Forcibly close client connections */
    Send(lpCS->nSocket, ERROR_FORCED_DISCONNECT);
    CloseSocket(lpCS->nSocket);

    LogInfoToFileAndScreen("C[%s:%d]: <disconnected>", lpCS->szIPAddress,
            lpCS->nSocket);

    /* set the client socket file descriptor to now have a value of -1,
     * since its socket has been closed and we've said good bye.  This will
     * prevent any other socket functions from working on this now dead socket.
     */
    lpCS->nSocket = INVALID_SOCKET_HANDLE;
    lpCS->bConnected = FALSE;

    /* Client nicknames are allocated with malloc() */
    FreeBuffer((void**) &(lpCS->pszNickname));

    /* Release the storage associated with the client structure */
    FreeClient((void*) lpCS);
}

int ReplyToClient(LPCLIENTSTRUCT lpCS, const char* pszBuffer) {
    return SendToClient(lpCS, pszBuffer);
}
