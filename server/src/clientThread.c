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
	if (pszMessage == NULL
			|| strlen(pszMessage) == 0)
	{
		return 0;
	}

	POSITION* pos = GetHeadPosition(&clientList);
	if (pos == NULL)
		return 0;

	int total_bytes_sent = 0;

	do {
		LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT)pos->data;
		if (lpClientStruct == NULL)
			continue;

		char* sendBuffer = NULL;
		sprintf(sendBuffer, "%s: %s", lpClientStruct->pszNickname,
			pszMessage);

		total_bytes_sent += SocketDemoUtils_send(lpClientStruct->sockFD, pszMessage);

	} while ((pos = GetNext(pos)) != NULL);

	return total_bytes_sent;
}

BOOL HandleProtocolCommand(LPCLIENTSTRUCT lpClientStruct, const char* pszBuffer)
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
