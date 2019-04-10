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
#include "client_list_manager.h"
#include "server_symbols.h"
#include "utils.h"

void TerminateClientThread(int s) {
	if (g_bShouldTerminateClientThread) {
		return;
	}

	LogDebug("In TerminateClientThread");

	LogInfo(
			"TerminateClientThread: Checking whether SIGSEGV signal received...");

	LogDebug("TerminateClientThread: s = %d", s);

	if (SIGSEGV != s) {
		LogError(
				"TerminateClientThread: Different signal received, stopping.");

		LogDebug("TerminateClientThread: Done.");

		return;
	}

	LogInfo(
			"TerminateClientThread: SIGSEGV signal detected.  Setting global terminate flag...");

	g_bShouldTerminateClientThread = TRUE;

	LogInfo("TerminateClientThread: Terminate flag set.");

	/* Re-associate this function with the signal */
	RegisterEvent(TerminateClientThread);
}

BOOL HandleProtocolCommand(LPCLIENTSTRUCT lpClientStruct, char* pszBuffer) {
	if (g_bShouldTerminateClientThread)
		return TRUE;

	LogDebug("In HandleProtocolCommand");

	LogInfo(
			"HandleProtocolCommand: Checking whether client structure pointer passed is valid...");

	if (lpClientStruct == NULL) {
		LogError(
				"HandleProtocolCommand: NULL value passed for client structure.");

		LogDebug("HandleProtocolCommand: Returning FALSE.");

		LogDebug("HandleProtocolCommand: Done.");

		return FALSE;
	}

	LogInfo(
			"HandleProtocolCommand: Valid value received for client data structure.");

	LogInfo("Checking whether any text is present...");

	if (pszBuffer == NULL || strlen(pszBuffer) == 0) {
		LogError("HandleProtocolCommand: Input buffer has a zero length.");

		LogDebug("HandleProtocolCommand: Returning FALSE.");

		LogDebug("HandleProtocolCommand: Done.");

		return FALSE;
	}

	LogInfo("HandleProtocolCommand: Input buffer contains %d bytes.",
			strlen(pszBuffer));

	if (stdout != GetLogFileHandle()) {
		// NOTE: We do not append a newline to this fprintf call since we expect, per protocol,
		// that everything clients send us is terminated with a CRLF
		fprintf(stdout, "C[%s:%d]: %s", lpClientStruct->ipAddr,
				lpClientStruct->sockFD, pszBuffer);
	} else {
		LogInfo("C[%s:%d]: %s", lpClientStruct->ipAddr, lpClientStruct->sockFD,
				pszBuffer);
	}

	/* per protocol, HELO command is client saying hello to the server.  It does not matter
	 * whether a client socket has connected; that socket has to say HELO first, so that
	 * then that client is marked as being allowed to receive stuff. */
	if (strcasecmp(pszBuffer, "HELO\n") == 0) {
		LogInfo("HandleProtocolCommand: HELO command being processed.");

		/* mark the current client as connected */
		lpClientStruct->bConnected = TRUE;

		/* Reply OK to the client */
		ReplyToClient(lpClientStruct, OK_FOLLOW_WITH_NICK_REPLY);

		LogDebug("HandleProtocolCommand: Returning TRUE.");

		LogDebug("HandleProtocolCommand: Done.");

		return TRUE; /* command successfully handled */
	}

	LogInfo(
			"HandleProtocolCommand: Checking whether the client is connected...");

	LogDebug("HandleProtocolCommand: lpClientStruct->bConnected = %d",
			lpClientStruct->bConnected);

	if (lpClientStruct->bConnected == FALSE) {

		LogError(
				"HandleProtocolCommand: The current client is not in a connected state.");

		LogDebug("HandleProtocolCommand: Returning FALSE.");

		LogDebug("HandleProtocolCommand: Done.");

		return FALSE;
	}

	LogInfo(
			"HandleProtocolCommand: The current client is in a connected state.");

	LogInfo(
			"HandleProtocolCommand: Checking for multi-line input termination signal...");

	if (strcasecmp(pszBuffer, ".\n") == 0) {
		LogInfo(
				"HandleProtocolCommand: Completion signal for multi-line input received.");

		LogDebug("HandleProtocolCommand: Returning TRUE.");

		LogDebug("HandleProtocolCommand: Done.");

		return TRUE;	// completion of a chat message.
	}

	LogInfo("HandleProtocolCommand: Multi-line termination signal not found.");

	/* per protocol, the NICK command establishes the user's chat nickname */

	// StartsWith function is declared/defined in utils.h/.c
	if (StartsWith(pszBuffer, "NICK ")) {
		LogInfo("HandleProtocolCommand: NICK command being processed.");

		// let's parse this command with lpClientStructstrtok.  Protocol spec says this command is
		// NICK <chat-nickname>\n with no spaces allowed in the <chat-nickname>
		char* pszNickname = strtok(pszBuffer, " ");
		if (pszNickname != NULL) {
			/* the first call to strtok just gives us the word "NICK" which
			 * we will just throw away.  */
			pszNickname = strtok(NULL, " ");
			if (pszNickname == NULL || strlen(pszNickname) == 0) {

				LogError(
						"HandleProtocolCommand: Did not receive a client nickname.");

				ReplyToClient(lpClientStruct, ERROR_NO_NICK_RECEIVED);

				LogDebug("HandleProtocolCommand: Returning FALSE.");

				LogDebug("HandleProtocolCommand: Done.");

				return FALSE;
			}

			// Allocate a buffer to hold the nickname but not including the LF on
			// the end of the command string coming from the client
			lpClientStruct->pszNickname = (char*) malloc(
					(strlen(pszNickname) - 1) * sizeof(char));

			// Copy the contents of the buffer referenced by pszNickname to that
			// referenced by lpClientStruct->pszNickname
			strncpy(lpClientStruct->pszNickname, pszNickname,
					strlen(pszNickname) - 1);

			LogInfo("HandleProtocolCommand: Client %d nickname set to %s.",
					lpClientStruct->sockFD, lpClientStruct->pszNickname);

			char replyBuffer[BUFLEN];

			sprintf(replyBuffer, OK_NICK_REGISTERED,
					lpClientStruct->pszNickname);

			ReplyToClient(lpClientStruct, replyBuffer);

			/* Now, tell everyone that a new chatter has joined! */

			sprintf(replyBuffer, NEW_CHATTER_JOINED,
					lpClientStruct->pszNickname);

			BroadcastAll(replyBuffer);
		}

		LogDebug("HandleProtocolCommand: Returning TRUE.");

		LogDebug("HandleProtocolCommand: Done.");

		/* Return TRUE to signify command handled */
		return TRUE;
	}

	/* per protocol, client says bye bye server */
	if (StartsWith(pszBuffer, "QUIT")) {

		LogInfo("HandleProtocolCommand: Processing QUIT command.");

		LogInfo("HandleProtocolCommand: Telling client goodbye...");

		ReplyToClient(lpClientStruct, OK_GOODBYE);

		LogInfo(
				"HandleProtocolCommand: We said goodbye to the client.  Marking it as no longer connected...");

		// Mark this client as no longer being connected.
		lpClientStruct->bConnected = FALSE;

		LogDebug("HandleProtocolCommand: lpClientStruct->bConnected = FALSE");

		LogInfo(
				"HandleProtocolCommand: Client marked as no longer connected.");

		LogInfo(
				"HandleProtocolCommand: Removing client from the list of active clients...");

		LogInfo(
				"HandleProtocolCommand: Attempting to get the lock on the list of clients...");

		// Save off the value of the thread handle of the client thread for this particular
		// client
		HTHREAD hClientThread = INVALID_HANDLE_VALUE;

		// Accessing the linked list -- make sure and use the mutex
		// to close the socket, to remove the client struct from the
		// list of clients, AND to decrement the global reference count
		// of connected clients
		LockMutex(hClientListMutex);
		{
			LogInfo("HandleProtocolCommand: Client list mutex lock obtained.");

			LogInfo(
					"HandleProtocolCommand: Reporting the client disconnection to the console...");

			fprintf(stdout, "C[%s:%d]: <disconnected>\n",
					lpClientStruct->ipAddr, lpClientStruct->sockFD);

			LogInfo(
					"HandleProtocolCommand: Reported client disconnection to console.");

			LogInfo(
					"HandleProtocolCommand: Removing client from the active client list...");

			close(lpClientStruct->sockFD);
			lpClientStruct->sockFD = -1;

			hClientThread = lpClientStruct->hClientThread;

			// Remove the client from the client list
			RemoveElement(&clientList, &(lpClientStruct->sockFD),
					FindClientBySocket);

			// remove the client data structure from memory
			free(lpClientStruct);
			lpClientStruct = NULL;

			LogInfo(
					"HandleProtocolCommand: Client information removed from active client list.");

			LogInfo("HandleProtocolCommand: Releasing client list lock...");
		}
		UnlockMutex(hClientListMutex);

		LogInfo("HandleProtocolCommand: Client list lock released.");

		LogInfo(
				"HandleProtocolCommand: Decrementing the count of connected clients...");

		// now decrement the count of connected clients
		InterlockedDecrement(&client_count);

		LogDebug("HandleProtocolCommand: client_count = %d", client_count);

		LogInfo(
				"HandleProtocolCommand: Checking whether client count has dropped to zero.");

		if (client_count == 0) {
			LogInfo(
					"HandleProtocolCommand: Client count has dropped to zero.  Stopping server...");

			CleanupServer(OK);
			return TRUE;
		} else {
			LockMutex(hClientListMutex);
			{
				LogInfo(
						"HandleProtocolCommand: There are still greater than zero clients connected.");

				LogInfo(
						"HandleProtocolCommand: Trying to kill just the thread servicing this particular client...");

				// If we are here, do not kill the server, but this client's particular thread
				// needs to stop
				if (INVALID_HANDLE_VALUE != hClientThread) {
					LogInfo(
							"HandleProtocolCommand: A valid thread handle has been found for the client we just disconnected from.");

					KillThread(hClientThread);
					sleep(1);
				}
			}
			UnlockMutex(hClientListMutex);
		}

		LogInfo(
				"HandleProtocolCommand: Client count is above zero. Not quitting.");

		LogDebug("HandleProtocolCommand: Done.");

		return TRUE;
	}

	LogDebug("HandleProtocolCommand: Done.");

	return FALSE;
}

void CheckTerminateFlag(LPCLIENTSTRUCT lpCurrentClientStruct) {
	LogDebug("In CheckTerminateFlag");

	LogInfo(
			"CheckTerminateFlag: Checking whether we've been passed a valid client structure reference...");

	if (NULL == lpCurrentClientStruct) {
		LogError(
				"CheckTerminateFlag: A null pointer has been passed for the client structure.  Stopping.");

		LogDebug("CheckTerminateFlag: Done.");

		return;
	}

	LogInfo(
			"CheckTerminateFlag: A valid client structure reference has been passed.");

	LogInfo(
			"CheckTerminateFlag: Checking whether the terminate flag has been set...");

	LogDebug("CheckTerminateFlag: g_bShouldTerminateClientThread = %d",
			g_bShouldTerminateClientThread);

	if (g_bShouldTerminateClientThread) {
		LogInfo("CheckTerminateFlag: Checking whether the client's socket file descriptor is still a valid value...");

		if (IsSocketValid(lpCurrentClientStruct->sockFD)) {
			LogInfo("CheckTerminateFlag: The client's socket file descriptor is still a valid value.");

			LogInfo(
					"CheckTerminateFlag: Forcibly disconnecting the client...");

			ForciblyDisconnectClient(lpCurrentClientStruct);

			LogInfo("CheckTerminateFlag: Disconnected.");
		} else {
			LogInfo("CheckTerminateFlag: We no longer have a valid socket file descriptor for the client.");
		}

		LogInfo(
				"CheckTerminateFlag: The client terminate flag has been set.");

		LogDebug("CheckTerminateFlag: Done.");

		return;
	}

	LogInfo("CheckTerminateFlag: The terminate flag is not set.");

	LogDebug("CheckTerminateFlag: Done.");
}

void *ClientThread(void* pData) {
	LogDebug("In ClientThread");

	RegisterEvent(TerminateClientThread);

	LogInfo("ClientThread: Checking whether user state was passed...");

	if (pData == NULL) {

		LogError("ClientThread: No user state passed.");

		LogDebug("ClientThread: Done.");

		return NULL;
	}

	LogInfo("ClientThread: Valid user state information was passed.");

	LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT) pData;

	LogInfo("ClientThread: Setting up recv loop...");

	while (1) {
		CheckTerminateFlag(lpClientStruct);

		if (g_bShouldTerminateClientThread){
			g_bShouldTerminateClientThread = FALSE;
			break;
		}

		// Receive all the lines of text that the client wants to send,
		// and put them all into a buffer.
		char* buf = NULL;
		int bytes = 0;

		// just call SocketDemoUtils_recv over and over again until
		// all the data has been read that the client wants to send.
		// Clients should send a period on one line by itself to indicate
		// the termination of a chat message; a protocol command terminates
		// with a linefeed.

		LogDebug("ClientThread: Calling SocketDemoUtils_recv...");

		if ((bytes = Receive(lpClientStruct->sockFD, &buf)) > 0) {

			CheckTerminateFlag(lpClientStruct);

			if (g_bShouldTerminateClientThread){
				g_bShouldTerminateClientThread = FALSE;
				break;
			}

			LogInfo("%s: %d B received.", lpClientStruct->ipAddr, bytes);

			lpClientStruct->bytesReceived += bytes;

			//fprintf(stdout, "C: %s", buf);

			/* first, check if we have a protocol command.  If so, skip to next loop.
			 * We know if this is a protocol command rather than a chat message because
			 * the HandleProtocolCommand returns a value of TRUE in this case. */
			if (HandleProtocolCommand(lpClientStruct, buf))
				continue;

			/* throw everything that a client sends us (besides a protocol
			 * command, that is) to all the clients */
			BroadcastAll(buf);

			/* TODO: Add other protocol handling here */

			LogDebug("lpClientStruct->bConnected = %d",
					lpClientStruct->bConnected);

			LogInfo(
					"ClientThread: Checking whether client with socket descriptor %d (%s) is connected...",
					lpClientStruct->sockFD, lpClientStruct->ipAddr);

			/* If the client has closed the connection, bConnected will
			 * be FALSE.  This is our signal to stop looking for further input. */
			if (lpClientStruct->bConnected == FALSE
					|| !IsSocketValid(lpClientStruct->sockFD)) {

				LogInfo(
						"ClientThread: Client has terminated connection.  Decrementing count of connected clients...");

				InterlockedDecrement(&client_count);

				LogInfo("ClientThread: Count of connected clients: %d",
						client_count);

				LogInfo("ClientThread: Stopping receive loop.");

				break;
			}
		}
	}

	CheckTerminateFlag(lpClientStruct);

	if (g_bShouldTerminateClientThread){
		g_bShouldTerminateClientThread = FALSE;
	}

	LogDebug("ClientThread: Done.");

	return NULL;
}
