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

	log_debug("In BroadcastAll");

	log_info(
			"BroadcastAll: Checking whether the message to broadcast is blank...");

	if (pszMessage == NULL || strlen(pszMessage) == 0) {
		log_error("BroadcastAll: The message to broadcast is blank.");

		log_debug("BroadcastAll: Done.");

		return 0;
	}

	log_info("BroadcastAll: The message to broadcast is not blank.");

	int total_bytes_sent = 0;

	LockMutex(hClientListMutex);
	{
		log_info(
				"BroadcastAll: Checking whether more than zero clients are connected...");

		// If there are zero clients in the list of connected clients, then continuing
		// is pointless, isn't it?
		if (client_count == 0) {
			log_error("BroadcastAll: No clients currently connected.");

			log_info("BroadcastAll: Zero bytes sent.");

			log_debug("BroadcastAll: Done.");

			return 0;
		}

		log_info("BroadcastAll: More than zero clients are connected.");

		log_info(
				"BroadcastAll: Getting the position of the head of the internal client list...");

		POSITION* pos = GetHeadPosition(&clientList);
		if (pos == NULL) {
			log_error(
					"BroadcastAll: No clients registered, or failed to get head of internal list.");

			log_info("BroadcastAll: Zero bytes sent.");

			log_debug("BroadcastAll: Done.");

			return 0;
		}

		log_info(
				"BroadcastAll: Successfully obtained head of internal client list.");

		do {
			if (g_bShouldTerminateClientThread)
				return ERROR;

			log_info("BroadcastAll: Obtaining data about current client...");

			LPCLIENTSTRUCT lpCurrentClientStruct = (LPCLIENTSTRUCT) pos->data;
			if (lpCurrentClientStruct == NULL) {
				log_warning(
						"BroadcastAll: Null reference at current location.");

				log_debug("BroadcastAll: Attempting to continue loop...");

				continue;
			}

			log_info(
					"BroadcastAll: Successfully obtained info for current client.  Sending message...");

			/* Allocate space to hold message */
			char sendBuffer[4096];

			sprintf(sendBuffer, "%s: %s", lpCurrentClientStruct->pszNickname,
					pszMessage);

			int bytes_sent = Send(lpCurrentClientStruct->sockFD,
					pszMessage);

			log_debug("BroadcastAll: %d B sent to client socket descriptor %d.",
					bytes_sent, lpCurrentClientStruct->sockFD);

			total_bytes_sent += bytes_sent;

			log_debug("BroadcastAll: Moving on to next client.");

		} while ((pos = GetNext(pos)) != NULL);

		log_info("BroadcastAll: Done procesing message broadcast.");
	}
	UnlockMutex(hClientListMutex);

	log_info("BroadcastAll: %d bytes sent.", total_bytes_sent);

	log_debug("BroadcastAll: Done.");

	return total_bytes_sent;
}

void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCurrentClientStruct) {
	log_debug("In ForciblyDisconnectClient");

	log_info(
			"ForciblyDisconnectClient: Checking whether lpCurrentClientStruct has a valid reference...");

	if (lpCurrentClientStruct == NULL) {
		log_error(
				"ForciblyDisconnectClient: The required parameter is not supplied.  Nothing to do.");

		log_debug("ForciblyDisconnectClient: Done.");

		return;
	}

	log_info(
			"ForciblyDisconnectClient: lpCurrentClientStruct parameter has a valid value.");

	log_info("ForciblyDisconnectClient: Checking whether the client is still marked as active...");

	log_debug("ForciblyDisconnectClient lpCurrentClientStruct->bConnected = %d",
			lpCurrentClientStruct->bConnected);

	if (lpCurrentClientStruct->bConnected == TRUE) {
		log_info("ForciblyDisconnectClient: Client with socket descriptor %d is still connected. Stopping.",
				lpCurrentClientStruct->sockFD);

		log_debug("ForciblyDisconnectClient: Done.");

		/* Not time to run yet */
		return;
	}

	/* Forcibly close client connections */

	log_info(
			"ForciblyDisconnectClient: Sending the termination reply string...");

	Send(lpCurrentClientStruct->sockFD,
			"503 Server forcibly terminated connection.\n");

	log_info(
			"ForciblyDisconnectClient: Client notified that we will be terminating the connection.");

	log_info(
			"ForciblyDisconnectClient: Calling SocketDemoUtils_close on the clent's socket...");

	CloseSocket(lpCurrentClientStruct->sockFD);

	log_info("ForciblyDisconnectClient: Client socket closed.");

	log_info("%s: <disconnected>", lpCurrentClientStruct->ipAddr);

	if (GetErrorLogFileHandle() != stdout) {
		fprintf(stdout, "%s: <disconnected>\n", lpCurrentClientStruct->ipAddr);
	}

	log_info(
			"ForciblyDisconnectClient: Decrementing the count of connected clients...");

	log_debug("ForciblyDisconnectClient: client_count = %d", client_count);

	InterlockedDecrement(&client_count);

	log_info(
			"ForciblyDisconnectClient: Count of connected clients has been decremented.");

	log_debug("ForciblyDisconnectClient: client_count = %d", client_count);

	log_debug("ForciblyDisconnectClient: Done.");
}

void ReplyToClient(LPCLIENTSTRUCT lpClientStruct, const char* pszBuffer) {
	if (g_bShouldTerminateClientThread)
		return;

	log_debug("In ReplyToClient");

	log_info(
			"ReplyToClient: Checking whether client structure pointer passed is valid...");

	if (lpClientStruct == NULL) {
		log_error(
				"ReplyToCOK_NICK_REGISTEREDlient: NULL value passed for client structure.");

		log_debug("ReplyToClient: Done.");

		return;
	}

	log_info("ReplyToClient: Valid value received for client data structure.");

	log_info(
			"ReplyToClient: Checking whether client socket file descriptor is valid...");

	if (lpClientStruct->sockFD <= 0) {

		log_error(
				"ReplyToClient: The client socket file descriptor has an invalid value.");

		log_debug("ReplyToClient: Done.");

		return;
	}

	log_info("ReplyToClient: The client socket file descriptor is valid.");

	log_info("ReplyToClient: Checking whether the client is connected...");

	log_debug("ReplyToClient: lpClientStruct->bConnected = %d",
			lpClientStruct->bConnected);

	if (lpClientStruct->bConnected == FALSE) {

		log_error(
				"ReplyToClient: The current client is not in a connected state.");

		log_debug("ReplyToClient: Done.");

		return;
	}

	log_info("ReplyToClient: The current client is in a connected state.");

	log_info(
			"ReplyToClient: Checking whether any text is present in the reply buffer...");

	if (pszBuffer == NULL || strlen(pszBuffer) == 0) {
		log_error("ReplyToClient: Reply buffer has a zero length.");

		log_debug("ReplyToClient: Done.");

		return;
	}

	int buffer_size = strlen(pszBuffer);

	log_info("ReplyToClient: Reply buffer contains %d bytes.", buffer_size);

	if (GetLogFileHandle() != stdout) {
		fprintf(stdout, "S: %s", pszBuffer);
	}

	log_info("S: %s", pszBuffer);

	log_info("ReplyToClient: Sending the reply to the client...");

	int bytes_sent = Send(lpClientStruct->sockFD, pszBuffer);
	if (bytes_sent <= 0) {

		log_error("ReplyToClient: Error sending reply.");

		log_debug("ReplyToClient: Done.");

		return;
	}

	log_info("ReplyToClient: Sent %d bytes to client.", bytes_sent);

	log_debug("ReplyToClient: Done.");
}

