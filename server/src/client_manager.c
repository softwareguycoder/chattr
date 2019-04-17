/*
 * client_manager.c
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include <client_thread_functions.h>
#include "stdafx.h"
#include "server.h"

#include "client_manager.h"
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
			if (g_bShouldTerminateClientThread)
				return ERROR;

			LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT) pos->pvData;
			if (lpCS == NULL) {
				continue;
			}

			if (!IsSocketValid(lpCS->nSocket)) {
				continue;
			}

			if (lpCS->bConnected == FALSE) {
				/* client has not issued the HELO command yet, so it does
				 * not get included on broadcasts */
				continue;
			}

			int nBytesSent = Send(lpCS->nSocket, pszMessage);

			nTotalBytesSent += nBytesSent;
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

	if (pszMessage == NULL || strlen(pszMessage) == 0) {
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

	LogInfo("S: %s", pszMessage);

	fprintf(stdout, "S: %s", pszMessage);

	LockMutex(g_hClientListMutex);
	{
		POSITION* pos = GetHeadPosition(&g_pClientList);
		if (pos == NULL) {
			// There is nothing in the linked list of clients; nothing to do.
			return 0;
		}

		do {
			if (g_bShouldTerminateClientThread)
				return ERROR;

			LPCLIENTSTRUCT lpCurrClient = (LPCLIENTSTRUCT) pos->pvData;
			if (lpCurrClient == NULL) {
				// Cannot access the current list member; skip to next one.
				continue;
			}

			// Double-check that the current client list entry's
			// socket is still valid.
			if (!IsSocketValid(lpCurrClient->nSocket)) {
				continue;
			}

			// If we have the client list entry for the sender, skip it,
			// since this function does not broadcast back to the sender.
			if (lpCurrClient->nSocket == lpSendingClient->nSocket) {
				continue;
			}

			// If the current client has not registered as a new chatter
			// yet, then skip it.
			if (lpCurrClient->bConnected == FALSE) {
				continue;
			}

			// If we are here, then we know for sure that the current
			// client is active and connected and is chatting.  Send the
			// sending client's message to them.

			int nBytesSent = Send(lpCurrClient->nSocket, pszMessage);

			nTotalBytesSent += nBytesSent;
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

void DisconnectClient(void* pClientStruct) {
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

	/* Forcibly close client connections */
	Send(lpCS->nSocket, ERROR_FORCED_DISCONNECT);
	CloseSocket(lpCS->nSocket);

	LogInfo("C[%s:%d]: <disconnected>", lpCS->szIPAddress, lpCS->nSocket);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "C[%s:%d]: <disconnected>\n", lpCS->szIPAddress,
				lpCS->nSocket);
	}

	/* set the client socket file descriptor to now have a value of -1,
	 * since its socket has been closed and we've said good bye.  This will
	 * prevent any other socket functions from working on this now dead socket.
	 */
	lpCS->nSocket = INVALID_SOCKET_HANDLE;

	/* Decrement the count of connected clients */
	InterlockedDecrement(&g_nClientCount);

	/* Client nicknames are allocated with malloc() */
	FreeBuffer((void**) &(lpCS->pszNickname));
}

int ReplyToClient(LPCLIENTSTRUCT lpCS, const char* pszBuffer) {
	if (g_bShouldTerminateClientThread) {
		return 0;		// Zero bytes sent.
	}

	if (lpCS == NULL) {
		// The lpCS pointer indicates to whom to send the reply. If it's
		// NULL, then we have nothing to do.
		return 0;
	}

	/* Double-check that we have a valid endpoint file descriptor. */
	if (!IsSocketValid(lpCS->nSocket)) {
		return 0;
	}

	if (lpCS->bConnected == FALSE) {
		// Can't reply to this client because it's not in a connected state.
		// Nothing to do.
		return 0;
	}

	/* Check whether there's any text in the reply buffer */
	if (pszBuffer == NULL || strlen(pszBuffer) == 0) {
		return 0;
	}

	LogInfo("S: %s", pszBuffer);

    /* Per the protocol, replies to clients are supposed to be
    terminated with a newline; therefore, it's safe to assume that
    the text in pszBuffer is already terminated with one.  This is
    why there is no newline character in the format string below. */
	fprintf(stdout, "S: %s", pszBuffer);

	int nBytesSent = Send(lpCS->nSocket, pszBuffer);
	if (nBytesSent <= 0) {
		// No bytes sent to the client.  Nothing more to do.
		return 0;
	}

	return nBytesSent;
}

