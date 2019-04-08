/*
 * clientThread.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_list_manager.h"
#include "client_struct.h"
#include "client_thread.h"
#include "server_symbols.h"
#include "utils.h"

BOOL g_bShouldTerminateClientThread = FALSE;

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

void TerminateClientThread(int s) {
	log_debug("In TerminateClientThread");

	log_info(
			"TerminateClientThread: Checking whether SIGSEGV signal received...");

	log_debug("TerminateClientThread: s = %d", s);

	if (SIGSEGV != s) {
		log_error(
				"TerminateClientThread: Different signal received, stopping.");

		log_debug("TerminateClientThread: Done.");

		return;
	}

	log_info(
			"TerminateClientThread: SIGSEGV signal detected.  Setting global terminate flag...");

	g_bShouldTerminateClientThread = TRUE;

	log_info("TerminateClientThread: Terminate flag set.");

	/* Re-associate this function with the signal */
	RegisterEvent(TerminateClientThread);
}

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

	log_info("ReplyToClient: Reply buffer contains %d bytes.",
			strlen(pszBuffer));

	if (get_log_file_handle() != stdout) {
		log_info("S: %s", pszBuffer);
	}

	fprintf(stdout, "S: %s", pszBuffer);

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

BOOL HandleProtocolCommand(LPCLIENTSTRUCT lpClientStruct, char* pszBuffer) {
	if (g_bShouldTerminateClientThread)
		return TRUE;

	log_debug("In HandleProtocolCommand");

	log_info(
			"HandleProtocolCommand: Checking whether client structure pointer passed is valid...");

	if (lpClientStruct == NULL) {
		log_error(
				"HandleProtocolCommand: NULL value passed for client structure.");

		log_debug("HandleProtocolCommand: Returning FALSE.");

		log_debug("HandleProtocolCommand: Done.");

		return FALSE;
	}

	log_info(
			"HandleProtocolCommand: Valid value received for client data structure.");

	log_info("Checking whether any text is present...");

	if (pszBuffer == NULL || strlen(pszBuffer) == 0) {
		log_error("HandleProtocolCommand: Input buffer has a zero length.");

		log_debug("HandleProtocolCommand: Returning FALSE.");

		log_debug("HandleProtocolCommand: Done.");

		return FALSE;
	}

	log_info("HandleProtocolCommand: Input buffer contains %d bytes.",
			strlen(pszBuffer));

	if (get_log_file_handle() != stdout) {
		fprintf(stdout, "C[%s]: %s", lpClientStruct->ipAddr, pszBuffer);
	}

	/* per protocol, HELO command is client saying hello to the server.  It does not matter
	 * whether a client socket has connected; that socket has to say HELO first, so that
	 * then that client is marked as being allowed to receive stuff. */
	if (strcasecmp(pszBuffer, "HELO\n") == 0) {
		log_info("HandleProtocolCommand: HELO command being processed.");

		/* mark the current client as connected */
		lpClientStruct->bConnected = TRUE;

		/* Reply OK to the client */
		ReplyToClient(lpClientStruct, OK_FOLLOW_WITH_NICK_REPLY);

		log_debug("HandleProtocolCommand: Returning TRUE.");

		return TRUE; /* command successfully handled */
	}

	log_info(
			"HandleProtocolCommand: Checking whether the client is connected...");

	log_debug("HandleProtocolCommand: lpClientStruct->bConnected = %d",
			lpClientStruct->bConnected);

	if (lpClientStruct->bConnected == FALSE) {

		log_error(
				"HandleProtocolCommand: The current client is not in a connected state.");

		log_debug("HandleProtocolCommand: Returning FALSE.");

		log_debug("HandleProtocolCommand: Done.");

		return FALSE;
	}

	log_info(
			"HandleProtocolCommand: The current client is in a connected state.");

	// NOTE: We do not append a newline to this fprintf call since we expect, per protocol,
	// that everything clients send us is terminated with a CRLF
	fprintf(stdout, "C[%s]: %s", lpClientStruct->ipAddr, pszBuffer);

	log_info(
			"HandleProtocolCommand: Checking for multi-line input termination signal...");

	if (strcasecmp(pszBuffer, ".\n") == 0) {
		log_info(
				"HandleProtocolCommand: Completion signal for multi-line input received.");

		log_debug("HandleProtocolCommand: Returning TRUE.");

		log_debug("HandleProtocolCommand: Done.");

		return TRUE;	// completion of a chat message.
	}

	log_info("HandleProtocolCommand: Multi-line termination signal not found.");

	/* per protocol, the NICK command establishes the user's chat nickname */

	// StartsWith function is declared/defined in utils.h/.c
	if (StartsWith(pszBuffer, "NICK ")) {
		log_info("HandleProtocolCommand: NICK command being processed.");

		// let's parse this command with lpClientStructstrtok.  Protocol spec says this command is
		// NICK <chat-nickname>\n with no spaces allowed in the <chat-nickname>
		char* pszNickname = strtok(pszBuffer, " ");
		if (pszNickname != NULL) {
			/* the first call to strtok just gives us the word "NICK" which
			 * we will just throw away.  */
			pszNickname = strtok(NULL, " ");
			if (pszNickname == NULL || strlen(pszNickname) == 0) {

				log_error(
						"HandleProtocolCommand: Did not receive a client nickname.");

				ReplyToClient(lpClientStruct, ERROR_NO_NICK_RECEIVED);

				log_debug("HandleProtocolCommand: Returning FALSE.");

				log_debug("HandleProtocolCommand: Done.");

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

			log_info("HandleProtocolCommand: Client %d nickname set to %s.",
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
	}

	/* per protocol, client says bye bye server */
	if (strcasecmp(pszBuffer, "QUIT\n") == 0) {

		log_info("HandleProtocolCommand: Processing QUIT command.");

		ReplyToClient(lpClientStruct, OK_GOODBYE);

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
			RemoveElement(&clientList, &(lpClientStruct->sockFD),
					FindClientBySocket);

			// remove the client data structure from memory
			free(lpClientStruct);
			lpClientStruct = NULL;
		}
		UnlockMutex(hClientListMutex);

		// now decrement the count of connected clients
		InterlockedDecrement(&client_count);

		return TRUE;
	}

	log_debug("HandleProtocolCommand: Done.");

	return FALSE;
}

void CheckTerminateFlag(LPCLIENTSTRUCT lpCurrentClientStruct) {
	log_debug("In CheckTerminateFlag");

	log_info(
			"CheckTerminateFlag: Checking whether we've been passed a valid client structure reference...");

	if (NULL == lpCurrentClientStruct) {
		log_error(
				"CheckTerminateFlag: A null pointer has been passed for the client structure.  Stopping.");

		log_debug("CheckTerminateFlag: Done.");

		return;
	}

	log_info(
			"CheckTerminateFlag: A valid client structure reference has been passed.");

	log_info(
			"CheckTerminateFlag: Checking whether the terminate flag has been set...");

	log_debug("CheckTerminateFlag: g_bShouldTerminateClientThread = %d",
			g_bShouldTerminateClientThread);

	if (g_bShouldTerminateClientThread) {
		log_warning(
				"CheckTerminateFlag: The client terminate flag has been set.");

		log_info("CheckTerminateFlag: Forcibly disconnecting the client...");

		ForciblyDisconnectClient(lpCurrentClientStruct);

		log_info("CheckTerminateFlag: Disconnected.");

		log_debug("CheckTerminateFlag: Done.");

		return;
	}

	log_info("CheckTerminateFlag: The terminate flag is not set.");

	log_debug("CheckTerminateFlag: Done.");
}

void *ClientThread(void* pData) {
	log_debug("In ClientThread");

	RegisterEvent(TerminateClientThread);

	log_info("ClientThread: Checking whether user state was passed...");

	if (pData == NULL) {

		log_error("ClientThread: No user state passed.");

		log_debug("ClientThread: Done.");

		return NULL;
	}

	log_info("ClientThread: Valid user state information was passed.");

	LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT) pData;

	log_info("ClientThread: Setting up recv loop...");

	while (1) {
		CheckTerminateFlag(lpClientStruct);

		// Receive all the line sof text that the client wants to send,
		// and put them all into a buffer.
		char* buf = NULL;
		int bytes = 0;

		// just call SocketDemoUtils_recv over and over again until
		// all the data has been read that the client wants to send.
		// Clients should send a period on one line by itself to indicate
		// the termination of a chat message; a protocol command terminates
		// with a linefeed.

		log_debug("ClientThread: Calling SocketDemoUtils_recv...");

		if ((bytes = Receive(lpClientStruct->sockFD, &buf)) > 0) {

			CheckTerminateFlag(lpClientStruct);

			log_info("%s: %d B received.", lpClientStruct->ipAddr, bytes);

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

			log_debug("lpClientStruct->bConnected = %d",
					lpClientStruct->bConnected);

			log_info(
					"ClientThread: Checking whether client with socket descriptor %d (%s) is connected...",
					lpClientStruct->sockFD, lpClientStruct->ipAddr);

			/* If the client has closed the connection, bConnected will
			 * be FALSE.  This is our signal to stop looking for further input. */
			if (lpClientStruct->bConnected == FALSE) {

				log_info(
						"ClientThread: Client has terminated connection.  Decrementing count of connected clients...");

				InterlockedDecrement(&client_count);

				log_info("ClientThread: Count of connected clients: %d",
						client_count);

				log_info("ClientThread: Stopping receive loop.");

				break;
			}
		}
	}

	CheckTerminateFlag(lpClientStruct);

	log_debug("%s: Done.", lpClientStruct->ipAddr);

	return NULL;
}
