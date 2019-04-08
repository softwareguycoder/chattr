/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "client_manager.h"

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

			char* sendBuffer = NULL;
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

	if (get_error_log_file_handle() != stdout) {
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
