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

void DoBroadcast(LPCLIENTSTRUCT lpCurrentClient,
		const char* pszMessage, int* pnTotalBytesSent) {
	if (lpCurrentClient == NULL || pnTotalBytesSent == NULL) {
		return;
	}

	if (IsNullOrWhiteSpace(pszMessage)) {
		return;
	}

	int nBytesSent = 0;

	if ((nBytesSent = SendToClient(lpCurrentClient, pszMessage)) > 0) {
		*pnTotalBytesSent += nBytesSent;
	}
}

int BroadcastToAllClients(const char* pszMessage) {
	if (g_bShouldTerminateClientThread) {
		return ERROR;
	}

	if (IsNullOrWhiteSpace(pszMessage)) {
		// The message to broadcast is blank; nothing to do.
		return 0;
	}

	int nTotalBytesSent = 0;

	LogInfo(SERVER_DATA_FORMAT, pszMessage);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, SERVER_DATA_FORMAT, pszMessage);
	}

	LockMutex(GetClientListMutex());
	{
		// If there are zero clients in the list of connected clients,
		// then continuing is pointless, isn't it?
		if (GetElementCount(g_pClientList) == 0) {
			// No clients are connected; nothing to do.
			return 0;
		}

		MoveToHeadPosition(&g_pClientList);

		do {
			DoBroadcast(
				(LPCLIENTSTRUCT)(g_pClientList->pvData),
				pszMessage, &nTotalBytesSent);
		} while ((g_pClientList = g_pClientList->pNext) != NULL);

	}
	UnlockMutex(GetClientListMutex());

	return nTotalBytesSent;
}

///////////////////////////////////////////////////////////////////////////////
// BroadcastToAllClientsExceptSender function: Sends a chat message to everyone
// in the room except the client who sent it.
//

int BroadcastToAllClientsExceptSender(const char* pszMessage,
		LPCLIENTSTRUCT lpSendingClient) {
	int nTotalBytesSent = 0;

	if (g_bShouldTerminateClientThread)
		return ERROR;

	if (IsNullOrWhiteSpace(pszMessage)) {
		// Chat message to broadcast is blank; nothing to do.
		return nTotalBytesSent;
	}

	if (lpSendingClient == NULL) {
		// The data structure reference for the sending client is NULL;
		// as this information is necessary to carry out this function's task,
		// there's nothing to do.
		return nTotalBytesSent;
	}

	LogInfo(SERVER_DATA_FORMAT, pszMessage);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, SERVER_DATA_FORMAT, pszMessage);
	}

	LockMutex(GetClientListMutex());
	{
		if (GetElementCount(g_pClientList) == 0) {
			return nTotalBytesSent;	// Nothing to do.
		}

		MoveToHeadPosition(&g_pClientList);

		do {

			int nBytesSent = 0;

			LPCLIENTSTRUCT lpCurrentClient
				= (LPCLIENTSTRUCT) g_pClientList->pvData;
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

		} while ((g_pClientList = g_pClientList->pNext) != NULL);
	}
	UnlockMutex(GetClientListMutex());

	// Return the total bytes sent to the caller
	return nTotalBytesSent;
}

///////////////////////////////////////////////////////////////////////////////
// ForciblyDisconnectClient function - used when the server console's user
// kills the server, to sever connections with its clients.
//

void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCS) {
	//fprintf(stdout, "In ForciblyDisconnectClient...\n");

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

	if (lpCS->bConnected == FALSE) {
		// Nothing to do if the client is already marked as
		// not connected
		return;
	}

	/* Remove this client from the list of clients */
	/*if (!RemoveElement(&g_pClientList, &(lpCS->clientID), FindClientByID)) {
	 return;
	 }*/

	fprintf(stdout, "S: %s", ERROR_FORCED_DISCONNECT);

	/* Forcibly close client connections */
	Send(lpCS->nSocket, ERROR_FORCED_DISCONNECT);
	CloseSocket(lpCS->nSocket);

	LogInfo(CLIENT_DISCONNECTED, lpCS->szIPAddress, lpCS->nSocket);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, CLIENT_DISCONNECTED, lpCS->szIPAddress, lpCS->nSocket);
	}

	/* set the client socket file descriptor to now have a value of -1,
	 * since its socket has been closed and we've said good bye.  This will
	 * prevent any other socket functions from working on this now dead socket.
	 */
	lpCS->nSocket = INVALID_SOCKET_VALUE;
	lpCS->bConnected = FALSE;

	/* Client nicknames are allocated with malloc() and are a max of 15
	 * alpha numeric chars (plus null term) long; blank out any
	 * value currently in the structure */
	if (lpCS->pszNickname != NULL) {
		memset((char*) (lpCS->pszNickname), 0, MAX_NICKNAME_LEN + 1);
	}

	//fprintf(stdout, "ForciblyDisconnectClient: Done.\n");
}

int ReplyToClient(LPCLIENTSTRUCT lpCS, const char* pszBuffer) {
	int nBytesSent = SendToClient(lpCS, pszBuffer);
	if (nBytesSent <= 0) {
		FreeSocketMutex();

		CleanupServer(ERROR);

		return -1;
	}

	// Asume buffer terminates in a newline.  Report what the server
	// is sending to the console and the log file.  Only log the server
	// as successfully having sent a message if and only if a message was
	// actually sent!
	fprintf(stdout, SERVER_DATA_FORMAT, pszBuffer);

	if (GetLogFileHandle() != stdout) {
		LogInfo(SERVER_DATA_FORMAT, pszBuffer);
	}

	return nBytesSent;
}
