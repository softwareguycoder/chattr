/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "client_struct.h"
#include "utils.h"
#include "client_manager.h"
#include "client_list_manager.h"

BOOL g_bShouldTerminateClientThread = FALSE;

int BroadcastAll(const char* pszMessage) {

	if (g_bShouldTerminateClientThread)
		return ERROR;

	LogDebug("In BroadcastAll");

	LogInfo(
			"BroadcastAll: Checking whether the message to broadcast is blank...");

	if (pszMessage == NULL || strlen(pszMessage) == 0) {
		LogError("BroadcastAll: The message to broadcast is blank.");

		LogDebug("BroadcastAll: Done.");

		return 0;
	}

	LogInfo("BroadcastAll: The message to broadcast is not blank.");

	int total_bytes_sent = 0;

	LockMutex(hClientListMutex);
	{
		LogInfo(
				"BroadcastAll: Checking whether more than zero clients are connected...");

		// If there are zero clients in the list of connected clients, then continuing
		// is pointless, isn't it?
		if (client_count == 0) {
			LogError("BroadcastAll: No clients currently connected.");

			LogInfo("BroadcastAll: Zero bytes sent.");

			LogDebug("BroadcastAll: Done.");

			return 0;
		}

		LogInfo("BroadcastAll: More than zero clients are connected.");

		LogInfo(
				"BroadcastAll: Getting the position of the head of the internal client list...");

		POSITION* pos = GetHeadPosition(&clientList);
		if (pos == NULL) {
			LogError(
					"BroadcastAll: No clients registered, or failed to get head of internal list.");

			LogInfo("BroadcastAll: Zero bytes sent.");

			LogDebug("BroadcastAll: Done.");

			return 0;
		}

		LogInfo(
				"BroadcastAll: Successfully obtained head of internal client list.");

		do {
			if (g_bShouldTerminateClientThread)
				return ERROR;

			LogInfo("BroadcastAll: Obtaining data about current client...");

			LPCLIENTSTRUCT lpCurrentClientStruct = (LPCLIENTSTRUCT) pos->data;
			if (lpCurrentClientStruct == NULL) {
				LogWarning("BroadcastAll: Null reference at current location.");

				LogDebug("BroadcastAll: Attempting to continue loop...");

				continue;
			}

			LogInfo(
					"BroadcastAll: Successfully obtained info for current client.  Sending message...");

			/* Allocate space to hold message */
			char sendBuffer[4096];

			sprintf(sendBuffer, "%s: %s", lpCurrentClientStruct->pszNickname,
					pszMessage);

			int bytes_sent = Send(lpCurrentClientStruct->sockFD, pszMessage);

			LogDebug("BroadcastAll: %d B sent to client socket descriptor %d.",
					bytes_sent, lpCurrentClientStruct->sockFD);

			total_bytes_sent += bytes_sent;

			LogDebug("BroadcastAll: Moving on to next client.");

		} while ((pos = GetNext(pos)) != NULL);

		LogInfo("BroadcastAll: Done procesing message broadcast.");
	}
	UnlockMutex(hClientListMutex);

	LogInfo("BroadcastAll: %d bytes sent.", total_bytes_sent);

	LogDebug("BroadcastAll: Done.");

	return total_bytes_sent;
}

/* Serves as a routine that can be called from the ForEach function declared
 * in client_list.h -- we can call this for every client in the client list and
 * tell them bye bye...  */
void DisconnectClient(void* pClientStruct) {
	if (pClientStruct == NULL) {
		return;
	}

	ForciblyDisconnectClient((LPCLIENTSTRUCT) pClientStruct);
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

	if (!IsSocketValid(lpCurrentClientStruct->sockFD)) {
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

	Send(lpCurrentClientStruct->sockFD, ERROR_FORCED_DISCONNECT);

	LogInfo(
			"ForciblyDisconnectClient: Client notified that we will be terminating the connection.");

	LogInfo(
			"ForciblyDisconnectClient: Calling CloseSocket on the clent's socket...");

	CloseSocket(lpCurrentClientStruct->sockFD);

	LogInfo("ForciblyDisconnectClient: Client socket closed.");

	LogInfo("C[%s:%d]: <disconnected>", lpCurrentClientStruct->ipAddr,
			lpCurrentClientStruct->sockFD);

	if (GetErrorLogFileHandle() != stdout) {
		fprintf(stdout, "C[%s:%d]: <disconnected>\n", lpCurrentClientStruct->ipAddr,
				lpCurrentClientStruct->sockFD);
	}

	LogInfo(
			"ForciblyDisconnectClient: Invalidating the client's socket file descriptor...");

	/* set the client socket file descriptor to now have a value of -1, since its socket has been
	 * closed and we've said good bye.  This will prevent any other socket functions from working on
	 * this now dead socket. */
	lpCurrentClientStruct->sockFD = -1;

	LogInfo(
			"ForciblyDisconnectClient: Client socket file descriptor has been invalidated.");

	LogInfo(
			"ForciblyDisconnectClient: Decrementing the count of connected clients...");

	LogDebug("ForciblyDisconnectClient: client_count = %d", client_count);

	InterlockedDecrement(&client_count);

	LogInfo(
			"ForciblyDisconnectClient: Count of connected clients has been decremented.");

	LogDebug("ForciblyDisconnectClient: client_count = %d", client_count);

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

	if (lpClientStruct->sockFD <= 0) {

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

	int bytes_sent = Send(lpClientStruct->sockFD, pszBuffer);
	if (bytes_sent <= 0) {

		LogError("ReplyToClient: Error sending reply.");

		LogDebug("ReplyToClient: Done.");

		return;
	}

	LogInfo("ReplyToClient: Sent %d bytes to client.", bytes_sent);

	LogDebug("ReplyToClient: Done.");
}

