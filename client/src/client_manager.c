///////////////////////////////////////////////////////////////////////////////
// client_manager.c - Main client functionality manager functions
//
// This file contains all the routines that are centeral to the user interface
// and operation of the chat client program.
//
// AUTHOR: Brian Hart
// DATE: 8 Apr 2019
//
// Shout out to https://gist.github.com/DnaBoss/a5e1ea07de5ef3525937 for
// code that provided inspiration
//

#include "stdafx.h"
#include "client.h"

#include "client_manager.h"

///////////////////////////////////////////////////////////////////////////////
// GetNickname function: Prompts the user for a nickname, and places the value
// typed into the buffer pointed to by the nickname parameter.
//

void GetNickname(char* pszNickname, int nSize) {
	LogDebug("In GetNickname");

	LogInfo(
			"GetNickname: Checking whether a valid address was supplied for the 'pszNickname' parameter...");

	if (pszNickname == NULL) {
		LogError(
				"GetNickname: NULL value supplied for the pszNickname value. Stopping.");

		LogDebug("GetNickname: Done.");

		exit(ERROR);
	}

	LogInfo("GetNickname: The nickname parameter has a valid memory address.");

	LogInfo("GetNickname: Checking whether size is a positive value...");

	if (nSize < MIN_SIZE) {
		LogError("GetNickname: size is a non-positive value.  Stopping.");

		LogDebug("GetNickname: Done.");

		exit(ERROR);
	}

	LogInfo("GetNickname: size is a positive value.");

	LogInfo("GetNickname: Prompting the user for the user's chat nickname...");

	if (OK != get_line(NICKNAME_PROMPT, pszNickname, nSize)) {
		LogError("GetNickname: Failed to get user nickname.");

		LogDebug("GetNickname: Done.");

		exit(ERROR);
	}

	LogDebug("GetNickname: result = '%s'", pszNickname);

	LogDebug("GetNickname: Done.");

	return;
}

///////////////////////////////////////////////////////////////////////////////
// GreetServer function: carries out the first step of the chat protocol, which
// consists of isusing the HELO command to the server
//

void GreetServer() {
	LogDebug("In GreetServer");

	LogInfo("GreetServer: Greeting the server...");

	if (0 >= Send(nClientSocket, PROTOCOL_HELO_COMMAND)) {
		LogError("GreetServer: Error sending data.  Stopping.");

		LogDebug("GreetServer: Done.");

		CleanupClient(ERROR);
	}

	LogInfo("GreetServer: Server greeted successfully.");

	LogDebug("GreetServer: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// HandshakeWithServer function: Carries out the handshaking with the chat
// server, per protocol.  This consists of (1) sending the HELO command and
// getting an "OK" response; (2) if that succeeds, then prompts the user
// to type a nickname or 'chat handle' which is then sent to the server using
// the NICK command of the protocol.  For more details, see protocol_spec.txt
//

void HandshakeWithServer() {
	LogDebug("In HandshakeWithServer");

	LogDebug("HandshakeWithServer: Preparing local buffer to store the user's chosen nickname...");

	char szNickname[MAX_MESSAGE_LEN];

	LogDebug("HandshakeWithServer: Local buffer ready for nickname.");

	LogInfo("HandshakeWithServer: Asking the user for their desired chat nickname...");

	GetNickname(szNickname, MAX_MESSAGE_LEN);

	LogInfo("HandshakeWithServer: The user wants to use the nickname '%s'.", szNickname);

	LogDebug("HandshakeWithServer: Readying reply buffer...");

	char* pszReplyBuffer = NULL;

	LogDebug("HandshakeWithServer: The reply buffer has been set up.");

	LogInfo("HandshakeWithServer: Beginning chat session...");

	GreetServer();

	LogInfo("HandshakeWithServer: Chat session greeting sent.");

	LogInfo("HandshakeWithServer: Looking for reply from server...");

	int nBytesReceived = ReceiveFromServer((char**)&pszReplyBuffer);

	LogInfo("HandshakeWithServer: Reply from server has been retrieved and is %d bytes long.",
			nBytesReceived);

	LogDebug("HandshakeWithServer: Processing server reply...");

	ProcessReceivedText(pszReplyBuffer, nBytesReceived);

	LogDebug("HandshakeWithServer: The reply has been processed.  Freeing the buffer...");

	free_buffer((void**)&pszReplyBuffer);

	LogDebug("HandshakeWithServer: Memory consumed by reply buffer has been freed.");

	LogInfo("HandshakeWithServer: Telling the server the user's desired nickname...");

	SetNickname(szNickname);

	LogInfo("HandshakeWithServer: Server has been told that we want the nickname '%s'.",
			szNickname);

	LogInfo("HandshakeWithServer: Looking for reply from server...");

	ReceiveFromServer((char**)&pszReplyBuffer);

	LogInfo("HandshakeWithServer: Reply from server has been retrieved and is %d bytes long.",
			strlen(pszReplyBuffer));

	LogDebug("HandshakeWithServer: Processing server reply...");

	ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

	LogDebug("HandshakeWithServer: The reply has been processed.  Freeing the buffer...");

	free_buffer((void**)&pszReplyBuffer);

	LogDebug("HandshakeWithServer: Memory consumed by reply buffer has been freed.");

	LogInfo("HandshakeWithServer: Printing the usage directions for the user...");

	PrintClientUsageDirections();

	LogInfo("HandshakeWithServer: Usage directions printed.");

	LogDebug("HandshakeWithServer: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// LeaveChatRoom function

void LeaveChatRoom() {
	if (0 >= Send(nClientSocket, PROTOCOL_QUIT_COMMAND)) {
		CleanupClient(ERROR);
	}

	/* mock up a receive operation on the socket by
	 * just sleeping */
	sleep(1);
}

///////////////////////////////////////////////////////////////////////////////
// PrintClientUsageDirections function

void PrintClientUsageDirections() {
	LogDebug("In PrintClientUsageDirections");

	LogInfo(
			"PrintClientUsageDirections: Printing the usage directions for the user...");

	/* Print some usage directions */
	fprintf(stdout, USAGE_MESSAGE);

	LogInfo("PrintClientUsageDirections: Usage directions printed.");

	LogDebug("PrintClientUsageDirections: Done.");
}

////////////////////////////////////////////////////////////////////////////////
// ProcessReceivedText functon

void ProcessReceivedText(const char* pszReceivedText, int nSize) {
	LogDebug("In ProcessReceivedText");

	LogInfo("ProcessReceivedText: Checking whether nSize parameter is a positive integer...");

	LogDebug("ProcessReceivedText: nSize = %d", nSize);

	if (nSize < MIN_SIZE) {
		LogError("ProcessReceivedText: The size passed is zero or negative, which is an invalid value.");

		LogDebug("ProcessReceivedText: Done.");

		return;
	}

	LogInfo("ProcessReceivedText: The value of nSize is a positive integer.  Proceeding...");

	LogInfo("ProcessReceivedText: Checking whether the pszReceivedText buffer has a value...");

	if (pszReceivedText == NULL || pszReceivedText[0] == '\0') {
		LogError("ProcessReceivedText: No text in the pszReceivedText buffer.  Stopping.");

		LogDebug("ProcessReceivedText: Done.");

		return;
	}

	LogInfo("ProcessReceivedText: Text was present in the pszReceivedText buffer.");

	LogInfo("ProcessReceivedText: Dumping the received text to the console...");

	// For now, just dump all received text to the screen
	fprintf(stdout, "S: %s", pszReceivedText);

	LogInfo("ProcessReceivedText: Received text has been written to the console.");

	LogDebug("ProcessReceivedText: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// ReceiveFromServer function

int ReceiveFromServer(char** ppszReplyBuffer) {
	LogDebug("In ReceiveFromServer");

	LogInfo(
			"ReceiveFromServer: Checking whether the nClientSocket value passed refers to a valid socket...");

	LogDebug("ReceiveFromServer: nClientSocket = %d", nClientSocket);

	if (!IsSocketValid(nClientSocket)) {
		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		LogError(
				"ReceiveFromServer: The nClientSocket value passed is not a valid socket file descriptor.");

		LogDebug(
				"ReceiveFromServer: Releasing the memory of the socket mutex...");

		FreeSocketMutex();

		LogDebug("ReceiveFromServer: Memory consumed by socket mutex freed.");

		LogDebug("ReceiveFromServer: Done.");

		exit(ERROR);
	}

	LogDebug("ReceiveFromServer: Checking whether the pszReplyBuffer parameter is a NULL value...");
	/* Wipe away any existing reply buffer */

	if (ppszReplyBuffer != NULL) {
		LogInfo("ReceiveFromServer: Blanking away any existing text in the reply buffer, so we can reuse it...");

		free_buffer((void**)ppszReplyBuffer);

		LogInfo("ReceiveFromServer: Contents of reply buffer have been blanked.");
	} else {
		LogDebug("ReceiveFromServer: pszReplyBuffer pointer is NULL to start with.");
	}

	/* Do a receive. Cleanup if the operation was not successful. */

	LogInfo("ReceiveFromServer: Attempting to call Receive...");

	int nBytesRead = 0;

	if ((nBytesRead = Receive(nClientSocket, ppszReplyBuffer)) < 0
			&& errno != EBADF && errno != EWOULDBLOCK) {
		LogError("ReceiveFromServer: Failed to receive text from server.");

		LogDebug("ReceiveFromServer: Releasing the memory consumed by the receive buffer...");

		free_buffer((void**)ppszReplyBuffer);

		LogDebug("ReceiveFromServer: Memory consumed by the receive buffer has been freed.");

		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		LogDebug(
				"ReceiveFromServer: Releasing the memory of the socket mutex...");

		FreeSocketMutex();

		LogDebug("ReceiveFromServer: Memory consumed by socket mutex freed.");

		LogDebug("ReceiveFromServer: Done.");

		exit(ERROR);
	} else {
		LogInfo("ReceiveFromServer: Received %d B from server.", nBytesRead);
	}

	return nBytesRead;
}

///////////////////////////////////////////////////////////////////////////////
// SetNickname function: Sets the user's chat handle or nickname to the desired
// value

void SetNickname(const char* pszNickname) {
	LogDebug("In SetNickname");

	// TODO: Add logging to SetNickname

	char szNicknameCommand[512];

	sprintf(szNicknameCommand, PROTOCOL_NICK_COMMAND, pszNickname);

	if (0 >= Send(nClientSocket, szNicknameCommand)) {
		CleanupClient(ERROR);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ShouldStopReceiving function: Decides whether the receive thread is to be
// terminated.  Occurs when we see the message "200 Goodbye" from the server.
//

BOOL ShouldStopReceiving(const char* pszReceivedText, int nSize) {
	if (nSize < MIN_SIZE) {
		return FALSE;
	}

	return strcasecmp(pszReceivedText, OK_GOODBYE) == 0;
}
