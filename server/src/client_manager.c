/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "client_struct.h"
#include "client_manager.h"
#include "client_list_manager.h"
#include "client_thread_manager.h"

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

	if (pszMessage == NULL || strlen(pszMessage) == 0) {
		// The message to broadcast is blank; nothing to do.
		return 0;
	}

	int nTotalBytesSent = 0;

	LockMutex(g_hClientListMutex);
	{
		// If there are zero clients in the list of connected clients, then continuing
		// is pointless, isn't it?
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

			LPCLIENTSTRUCT lpCS = (LPCLIENTSTRUCT) pos->data;
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

			LPCLIENTSTRUCT lpCurrClient = (LPCLIENTSTRUCT) pos->data;
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
	LogDebug("In DisconnectClient");

	LogInfo("DisconnectClient: Checking whether the pClientStruct is NULL...");

	if (pClientStruct == NULL) {
		LogError(
				"DisconnectClient: The pClientStruct parameter is NULL.  Stopping.");

		LogDebug("DisconnectClient: Done.");

		return;
	}

	LogInfo(
			"DisconnectClient: The pClientStruct parameter has a valid reference.");

	LogInfo(
			"DisconnectClient: Calling ForciblyDisconnectClient for the client referred to by pClientStruct...");

	ForciblyDisconnectClient((LPCLIENTSTRUCT) pClientStruct);

	LogInfo("DisconnectClient: Finished calling ForciblyDisconnectClient.");

	LogDebug("DisconnectClient: Done.");
}

void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCurrentClientStruct) {
	LogDebug("In ForciblyDisconnectClient");

	LogInfo(
			"ForciblyDisconnectClient: Checking whether lpCurrentClientStruct has a valid reference...");

	if (lpCurrentClientStruct == NULL) {
		LogError(
				"ForciblyDisconnectClient: The required parameter is not supplied.  Nothing to do.");

		LogDebug("ForciblyDisconnectClient: Done.");

		return;
	}

	LogInfo(
			"ForciblyDisconnectClient: lpCurrentClientStruct parameter has a valid value.");

	/* Check whether there is still a valid socket file descriptor available for the client endpoint... */

	LogInfo(
			"ForciblyDisconnectClient: Checking whether the client's socket file descriptor is still a valid value...");

	if (!IsSocketValid(lpCurrentClientStruct->nSocket)) {
		LogError(
				"ForciblyDisconnectClient: The client's socket file descriptor is not a valid value.  We are unable to proceed.");

		LogDebug("ForciblyDisconnectClient: Done.");

		return;
	}

	LogInfo(
			"ForciblyDisconnectClient: The client socket file descriptor is a valid value.");

	/* Forcibly close client connections */

	LogInfo(
			"ForciblyDisconnectClient: Sending the termination reply string...");

	Send(lpCurrentClientStruct->nSocket, ERROR_FORCED_DISCONNECT);

	LogInfo(
			"ForciblyDisconnectClient: Client notified that we will be terminating the connection.");

	LogInfo(
			"ForciblyDisconnectClient: Calling CloseSocket on the clent's socket...");

	CloseSocket(lpCurrentClientStruct->nSocket);

	LogInfo("ForciblyDisconnectClient: Client socket closed.");

	LogInfo("C[%s:%d]: <disconnected>", lpCurrentClientStruct->szIPAddress,
			lpCurrentClientStruct->nSocket);

	if (GetErrorLogFileHandle() != stdout) {
		fprintf(stdout, "C[%s:%d]: <disconnected>\n",
				lpCurrentClientStruct->szIPAddress, lpCurrentClientStruct->nSocket);
	}

	LogInfo(
			"ForciblyDisconnectClient: Invalidating the client's socket file descriptor...");

	/* set the client socket file descriptor to now have a value of -1, since its socket has been
	 * closed and we've said good bye.  This will prevent any other socket functions from working on
	 * this now dead socket. */
	lpCurrentClientStruct->nSocket = -1;

	LogInfo(
			"ForciblyDisconnectClient: Client socket file descriptor has been invalidated.");

	LogInfo(
			"ForciblyDisconnectClient: Decrementing the count of connected clients...");

	LogDebug("ForciblyDisconnectClient: client_count = %d", g_nClientCount);

	InterlockedDecrement(&g_nClientCount);

	LogInfo(
			"ForciblyDisconnectClient: Count of connected clients has been decremented.");

	LogDebug("ForciblyDisconnectClient: client_count = %d", g_nClientCount);

	LogDebug("ForciblyDisconnectClient: Done.");
}

void ReplyToClient(LPCLIENTSTRUCT lpClientStruct, const char* pszBuffer) {
	if (g_bShouldTerminateClientThread)
		return;

	LogDebug("In ReplyToClient");

	LogInfo(
			"ReplyToClient: Checking whether client structure pointer passed is valid...");

	if (lpClientStruct == NULL) {
		LogError(
				"ReplyToCOK_NICK_REGISTEREDlient: NULL value passed for client structure.");

		LogDebug("ReplyToClient: Done.");

		return;
	}

	LogInfo("ReplyToClient: Valid value received for client data structure.");

	LogInfo(
			"ReplyToClient: Checking whether client socket file descriptor is valid...");

	if (lpClientStruct->nSocket <= 0) {

		LogError(
				"ReplyToClient: The client socket file descriptor has an invalid value.");

		LogDebug("ReplyToClient: Done.");

		return;
	}

	LogInfo("ReplyToClient: The client socket file descriptor is valid.");

	LogInfo("ReplyToClient: Checking whether the client is connected...");

	LogDebug("ReplyToClient: lpClientStruct->bConnected = %d",
			lpClientStruct->bConnected);

	if (lpClientStruct->bConnected == FALSE) {

		LogError(
				"ReplyToClient: The current client is not in a connected state.");

		LogDebug("ReplyToClient: Done.");

		return;
	}

	LogInfo("ReplyToClient: The current client is in a connected state.");

	LogInfo(
			"ReplyToClient: Checking whether any text is present in the reply buffer...");

	if (pszBuffer == NULL || strlen(pszBuffer) == 0) {
		LogError("ReplyToClient: Reply buffer has a zero length.");

		LogDebug("ReplyToClient: Done.");

		return;
	}

	int buffer_size = strlen(pszBuffer);

	LogInfo("ReplyToClient: Reply buffer contains %d bytes.", buffer_size);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "S: %s", pszBuffer);
	}

	LogInfo("S: %s", pszBuffer);

	LogInfo("ReplyToClient: Sending the reply to the client...");

	int bytes_sent = Send(lpClientStruct->nSocket, pszBuffer);
	if (bytes_sent <= 0) {

		LogError("ReplyToClient: Error sending reply.");

		LogDebug("ReplyToClient: Done.");

		return;
	}

	LogInfo("ReplyToClient: Sent %d bytes to client.", bytes_sent);

	LogDebug("ReplyToClient: Done.");
}

