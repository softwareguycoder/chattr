/*
 * clientThread.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "clientStruct.h"
#include "clientThread.h"
#include "utils.h"

int BroadcastAll(const char* pszMessage) {

	log_debug("In BroadcastAll");

	log_info("BroadcastAll: Checking whether the message to broadcast is blank...");

	if (pszMessage == NULL
			|| strlen(pszMessage) == 0)
	{
		log_error("BroadcastAll: The message to broadcast is blank.");

		log_debug("BroadcastAll: Done.");

		return 0;
	}

	log_info("BroadcastAll: The message to broadcast is not blank.");

	int total_bytes_sent = 0;

	LockMutex(hClientListMutex);
	{
		log_info("BroadcastAll: Checking whether more than zero clients are connected...");

		// If there are zero clients in the list of connected clients, then continuing
		// is pointless, isn't it?
		if (client_count == 0){
			log_error("BroadcastAll: No clients currently connected.");

			log_info("BroadcastAll: Zero bytes sent.");

			log_debug("BroadcastAll: Done.");

			return 0;
		}

		log_info("BroadcastAll: More than zero clients are connected.");

		log_info("BroadcastAll: Getting the position of the head of the internal client list...");

		POSITION* pos = GetHeadPosition(&clientList);
		if (pos == NULL) {
			log_error("BroadcastAll: No clients registered, or failed to get head of internal list.");

			log_info("BroadcastAll: Zero bytes sent.");

			log_debug("BroadcastAll: Done.");

			return 0;
		}

		log_info("BroadcastAll: Successfully obtained head of internal client list.");

		do {
			log_info("BroadcastAll: Obtaining data about current client...");

			LPCLIENTSTRUCT lpCurrentClientStruct = (LPCLIENTSTRUCT)pos->data;
			if (lpCurrentClientStruct == NULL){
				log_warning("BroadcastAll: Null reference at current location.");

				log_debug("BroadcastAll: Attempting to continue loop...");

				continue;
			}

			log_info("BroadcastAll: Successfully obtained info for current client.  Sending message...");

			char* sendBuffer = NULL;
			sprintf(sendBuffer, "%s: %s", lpCurrentClientStruct->pszNickname,
				pszMessage);

			int bytes_sent = SocketDemoUtils_send(lpCurrentClientStruct->sockFD, pszMessage);

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

BOOL HandleProtocolCommand(LPCLIENTSTRUCT lpClientStruct, char* pszBuffer)
{
	if (lpClientStruct == NULL)
		return FALSE;

	if (lpClientStruct->bConnected == FALSE)
		return FALSE;

	if (pszBuffer == NULL
			|| strlen(pszBuffer) == 0)
	{
		return FALSE;
	}

	if (strcmp(pszBuffer, ".\n"))
		return TRUE;	// completion of a chat message.

	/* per protocol, this command establishes the user's chat nickname */

	if (StartsWith(pszBuffer, "NICK ")) {
		// let's parse this command with strtok.  Protocol spec says this command is
		// NICK <chat-nickname>\n with no spaces allowed in the <chat-nickname>
		char* pszNickname = strtok(pszBuffer, " ");
		if (pszNickname != NULL) {
			/* the first call to strtok just gives us the word "NICK" which
			 * we will just throw away.  */
			lpClientStruct->pszNickname = strtok(NULL, " ");
		}
	}

	/* per protocol, client says bye bye server */
	if (strcasecmp(pszBuffer, "QUIT\n") == 0) {
		// Mark this client as no longer being connected.
		lpClientStruct->bConnected = FALSE;

		// Accessing the linked list -- make sure and use the mutex
		// to close the socket, to remove the client struct from the
		// list of clients, AND to decrement the global reference count
		// of connected clients
		LockMutex(hClientListMutex);
		{
			close(lpClientStruct->sockFD);
			lpClientStruct->sockFD = -1;

			// Remove the client from the client list
			RemoveElement(&clientList,
					&(lpClientStruct->sockFD), FindClientBySocket);

			// remove the client data structure from memory
			free(lpClientStruct);
			lpClientStruct = NULL;
		}
		UnlockMutex(hClientListMutex);

		// now decrement the count of connected clients
		InterlockedDecrement(&client_count);

		return TRUE;
	}

	return FALSE;
}

void *ClientThread(void* pData)
{
	if (pData == NULL)
		return NULL;

	LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT)pData;

	while(1) {
		// Receive all the line sof text that the client wants to send,
		// and put them all into a buffer.
		char* buf = NULL;
		int bytes = 0;

		// just call SocketDemoUtils_recv over and over again until
		// all the data has been read that the client wants to send.
		// Clients should send a period on one line by itself to indicate
		// the termination of a chat message; a protocol command terminates
		// with a linefeed.
		if ((bytes = SocketDemoUtils_recv(lpClientStruct->sockFD, &buf)) > 0){
			/* first, check if we have a protocol command.  If so, skip to next loop */
			if (HandleProtocolCommand(lpClientStruct, buf))
				continue;

			lpClientStruct->bytesReceived += bytes;
			fprintf(stdout, "C: %s", buf);

			/* throw everything that a client sends us (besides a protocol
			 * command, that is) to all the clients */
			BroadcastAll(buf);

			/* TODO: Add other protocol handling here */
		}
	}

	return NULL;
}
