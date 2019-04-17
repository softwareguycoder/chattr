/*
 * clientThread.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_manager.h"
#include "client_struct.h"
#include "client_thread.h"
#include "client_thread_functions.h"
#include "client_list_manager.h"

#include "server_functions.h"



void PrependNicknameAndBroadcast(const char* pszChatMessage,
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
		return;	// Nickname is blank, but we can't work with that since we need a value here.
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

void *ClientThread(void* pData) {
	/* Be sure to register the termination semaphore so we can be
	 * signalled to stop if necessary */
	RegisterEvent(TerminateClientThread);

	/* Valid user state data consisting of a reference to the CLIENTSTRUCT
	 * instance giving information for this client must be passed. */
	if (pData == NULL) {
		return NULL;
	}

	LPCLIENTSTRUCT lpSendingClient = (LPCLIENTSTRUCT) pData;

	while (1) {
		/* Check whether the client's socket endpoint is valid. */
		if (!IsSocketValid(lpSendingClient->nSocket)) {
			// Nothing to do.
			break;
		}

		// Receive all the lines of text that the client wants to send,
		// and put them all into a buffer.
		char* pszData = NULL;
		int nReceived = 0;

		char szLineBuffer[MAX_LINE_LENGTH + 1];

		// just call Receive over and over again until
		// all the data has been read that the client wants to send.
		// Clients should send a period on one line by itself to indicate
		// the termination of a chat message; a protocol command terminates
		// with a linefeed.

		LogDebug("ClientThread: Calling Receive...");

		if ((nReceived = Receive(lpSendingClient->nSocket, &pszData)) > 0) {
			/* Inform the server console's user how many bytes we got. */
			LogInfo("C[%s:%d]: %d B received.", lpSendingClient->szIPAddress,
					lpSendingClient->nSocket, nReceived);
			if (GetLogFileHandle() != stdout) {
				fprintf(stdout, "C[%s:%d]: %d B received.\n",
						lpSendingClient->szIPAddress, lpSendingClient->nSocket,
						nReceived);
			}

			/* Save the total bytes received from this client */
			lpSendingClient->bytesReceived += nReceived;

			/* Check if the termination semaphore has been signalled, and
			 * stop this loop if so. */
			if (g_bShouldTerminateClientThread) {
				g_bShouldTerminateClientThread = FALSE;
				break;
			}

			strcat(szLineBuffer, pszData);

			if (!Contains(szLineBuffer, "\n")) {
			   continue;
			}

			LogInfo("C[%s:%d]: %s", lpSendingClient->szIPAddress,
					lpSendingClient->nSocket, szLineBuffer);

			// Log what the client sent us to the server's interactive
			// console
			if (GetLogFileHandle() != stdout) {
				fprintf(stdout, "C[%s:%d]: %s", lpSendingClient->szIPAddress,
						lpSendingClient->nSocket, szLineBuffer);
			}

			/* first, check if we have a protocol command.  If so, skip to
			 * next loop. We know if this is a protocol command rather than a
			 * chat message because the HandleProtocolCommand returns a value
			 * of TRUE in this case. */
			if (HandleProtocolCommand(lpSendingClient, szLineBuffer))
				continue;

			/* IF we are here, then the pszData was not found to contain a protocol-
			 * required command string; rather, this is simply text.  We prepend the
			 * 'chat handle' of the person who sent the message and then send it to
			 * all the chatters except the person who sent the message.
			 */
			PrependNicknameAndBroadcast(szLineBuffer, lpSendingClient);

			/* TODO: Add other protocol handling here */

			/* If the client has closed the connection, bConnected will
			 * be FALSE.  This is our signal to stop looking for further input. */
			if (lpSendingClient->bConnected == FALSE
					|| !IsSocketValid(lpSendingClient->nSocket)) {

				// Decrement the count of connected clients
				InterlockedDecrement(&g_nClientCount);

				break;
			}
		}
	}

	// reset the termination semaphore
	if (g_bShouldTerminateClientThread) {
		g_bShouldTerminateClientThread = FALSE;
	}

	// done
	return NULL;
}
