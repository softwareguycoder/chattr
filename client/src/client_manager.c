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

	LogDebug(
			"HandshakeWithServer: Preparing local buffer to store the user's chosen nickname...");

	char szNickname[MAX_NICKNAME_LEN + 1];

	LogDebug("HandshakeWithServer: Local buffer ready for nickname.");

	LogInfo(
			"HandshakeWithServer: Asking the user for their desired chat nickname...");

	GetNickname(szNickname, MAX_NICKNAME_LEN);

	LogInfo("HandshakeWithServer: The user wants to use the nickname '%s'.",
			szNickname);

	LogDebug("HandshakeWithServer: Readying reply buffer...");

	char* pszReplyBuffer = NULL;

	LogDebug("HandshakeWithServer: The reply buffer has been set up.");

	LogInfo("HandshakeWithServer: Beginning chat session...");

	GreetServer();

	LogInfo("HandshakeWithServer: Chat session greeting sent.");

	LogInfo("HandshakeWithServer: Looking for reply from server...");

	int nBytesReceived = ReceiveFromServer((char**) &pszReplyBuffer);

	LogInfo(
			"HandshakeWithServer: Reply from server has been retrieved and is %d bytes long.",
			nBytesReceived);

	LogDebug("HandshakeWithServer: Processing server reply...");

	ProcessReceivedText(pszReplyBuffer, nBytesReceived);

	LogDebug(
			"HandshakeWithServer: The reply has been processed.  Freeing the buffer...");

	free_buffer((void**) &pszReplyBuffer);

	LogDebug(
			"HandshakeWithServer: Memory consumed by reply buffer has been freed.");

	LogInfo(
			"HandshakeWithServer: Telling the server the user's desired nickname...");

	/* We run a loop in case the user's requested value does not satisfy the validation
	 * condition (that is imposed by our protocol) that the chat handle/nickname can be
	 * no longer than a certain number of chars. */
	while (!SetNickname(szNickname)) {
		GetNickname(szNickname, MAX_NICKNAME_LEN);
	}

	LogInfo(
			"HandshakeWithServer: Server has been told that we want the nickname '%s'.",
			szNickname);

	LogInfo("HandshakeWithServer: Looking for reply from server...");

	ReceiveFromServer((char**) &pszReplyBuffer);

	LogInfo(
			"HandshakeWithServer: Reply from server has been retrieved and is %d bytes long.",
			strlen(pszReplyBuffer));

	LogDebug("HandshakeWithServer: Processing server reply...");

	ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

	LogDebug(
			"HandshakeWithServer: The reply has been processed.  Freeing the buffer...");

	free_buffer((void**) &pszReplyBuffer);

	LogDebug(
			"HandshakeWithServer: Memory consumed by reply buffer has been freed.");

	LogInfo(
			"HandshakeWithServer: Printing the usage directions for the user...");

	PrintClientUsageDirections();

	LogInfo("HandshakeWithServer: Usage directions printed.");

	LogDebug("HandshakeWithServer: Done.");
}

///////////////////////////////////////////////////////////////////////////////
// LeaveChatRoom function

void LeaveChatRoom() {
	LogDebug("In LeaveChatRoom");

	LogInfo(
			"LeaveChatRoom: Sending the 'QUIT' command for logging off of the current chat session...");

	int nBytesSent = 0;

	if ((nBytesSent = Send(nClientSocket, PROTOCOL_QUIT_COMMAND)) <= 0) {
		LogError("LeaveChatRoom: Send operation failed.");

		LogDebug("LeaveChatRoom: Done.");

		CleanupClient(ERROR);
	}

	LogInfo("LeaveChatRoom: %d B sent to server.  Operation succeeded.",
			nBytesSent);

	sleep(1);			// force CPU context switch

	LogDebug("LeaveChatRoom: Done.");
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

	LogInfo(
			"ProcessReceivedText: Checking whether nSize parameter is a positive integer...");

	LogDebug("ProcessReceivedText: nSize = %d", nSize);

	if (nSize < MIN_SIZE) {
		LogError(
				"ProcessReceivedText: The size passed is zero or negative, which is an invalid value.");

		LogDebug("ProcessReceivedText: Done.");

		return;
	}

	LogInfo(
			"ProcessReceivedText: The value of nSize is a positive integer.  Proceeding...");

	LogInfo(
			"ProcessReceivedText: Checking whether the pszReceivedText buffer has a value...");

	if (pszReceivedText == NULL || pszReceivedText[0] == '\0') {
		LogError(
				"ProcessReceivedText: No text in the pszReceivedText buffer.  Stopping.");

		LogDebug("ProcessReceivedText: Done.");

		return;
	}

	LogInfo(
			"ProcessReceivedText: Text was present in the pszReceivedText buffer.");

	LogInfo("ProcessReceivedText: Dumping the received text to the console...");

	// For now, just dump all received text to the screen
	fprintf(stdout, "S: %s", pszReceivedText);

	LogInfo(
			"ProcessReceivedText: Received text has been written to the console.");

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

	LogDebug(
			"ReceiveFromServer: Checking whether the pszReplyBuffer parameter is a NULL value...");
	/* Wipe away any existing reply buffer */

	if (ppszReplyBuffer != NULL) {
		LogInfo(
				"ReceiveFromServer: Blanking away any existing text in the reply buffer, so we can reuse it...");

		free_buffer((void**) ppszReplyBuffer);

		LogInfo(
				"ReceiveFromServer: Contents of reply buffer have been blanked.");
	} else {
		LogDebug(
				"ReceiveFromServer: pszReplyBuffer pointer is NULL to start with.");
	}

	/* Do a receive. Cleanup if the operation was not successful. */

	LogInfo("ReceiveFromServer: Attempting to call Receive...");

	int nBytesRead = 0;

	if ((nBytesRead = Receive(nClientSocket, ppszReplyBuffer))
			< 0&& errno != EBADF && errno != EWOULDBLOCK) {
		LogError("ReceiveFromServer: Failed to receive text from server.");

		LogDebug(
				"ReceiveFromServer: Releasing the memory consumed by the receive buffer...");

		free_buffer((void**) ppszReplyBuffer);

		LogDebug(
				"ReceiveFromServer: Memory consumed by the receive buffer has been freed.");

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

BOOL SetNickname(const char* pszNickname) {
	LogDebug("In SetNickname");

	LogInfo(
			"SetNickname: Checking whether the value passed for pszNickname is blank...");

	LogDebug("SetNickname: pszNickname = '%s'", pszNickname);

	if (pszNickname == NULL || pszNickname[0] == '\0' || pszNickname[0] == '\n'
			|| strlen(pszNickname) == 0) {
		LogError(
				"SetNickname: Blank value passed in for pszNickname.  A value is required.  Stopping.");

		fprintf(stderr, "SetNickname: A non-blank nickname is required.");

		LogDebug("SetNickname: Done.");

		CleanupClient(ERROR);
	}

	LogInfo("SetNickname: A non-blank nickname value has been passed.");

	LogInfo(
			"SetNickname: Now checking to see if it's %d characters or less in length...",
			MAX_NICKNAME_LEN);

	if (strlen(pszNickname) > MAX_NICKNAME_LEN) {
		LogError(
				"SetNickname: User wants to set a chat nickname that is greater than %d characters in length.",
				MAX_NICKNAME_LEN);

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr,
					"SetNickname: Nickname must be 15 characters or less.  Please try again.");
		}

		LogDebug("SetNickname: Returning FALSE.");

		LogDebug("SetNickname: Done.");

		return FALSE;
	}

	LogInfo("SetNickname: The nickname supplied is of a valid length.");

	LogInfo(
			"SetNickname: Proceeding to tell the server that this is the nickname that we want...");

	LogDebug(
			"SetNickname: Allocating local buffer of %d B in size to hold formatted NICK command string...",
			5 + MAX_NICKNAME_LEN);

	char szNicknameCommand[5 + MAX_NICKNAME_LEN];

	LogDebug(
			"SetNickname: Local buffer allocated.  Formatting server command string...");

	sprintf(szNicknameCommand, PROTOCOL_NICK_COMMAND, pszNickname);

	char *pszTrimmedNicknameCommand = Trim(szNicknameCommand);

	LogDebug("SetNickname: szNicknameCommand = '%s'",
			pszTrimmedNicknameCommand);

	free(pszTrimmedNicknameCommand);

	LogInfo("SetNickname: Now sending the NICK command to the server...");

	int nBytesSent = 0;

	if ((nBytesSent = Send(nClientSocket, szNicknameCommand)) < 0) {
		LogError("SetNickname: Sending the NICK command to the server failed.");

		LogDebug("SetNickname: Done.");

		CleanupClient(ERROR);
	}

	LogDebug("SetNickname: %d B sent to server.", nBytesSent);

	LogDebug("SetNickname: Returning TRUE.");

	LogDebug("SetNickname: Done.");

	return TRUE; /* TRUE return value means that the user's requested nickname was valid. */
}

///////////////////////////////////////////////////////////////////////////////
// ShouldStopReceiving function: Decides whether the receive thread is to be
// terminated.  Occurs when we see the message "200 Goodbye" from the server.
//

BOOL ShouldStopReceiving(const char* pszReceivedText, int nSize) {
	LogDebug("In ShouldStopReceiving");

	BOOL bResult = FALSE;	// Default return value is FALSE

	LogInfo(
			"ShouldStopReceiving: Checking whether nSize is a positive quantity...");

	LogDebug("ShouldStopReceiving: nSize = %d", nSize);

	if (nSize < MIN_SIZE) {
		LogError(
				"ShouldStopReceiving: nSize is a nonpositive quantity.  This is not valid.  Stopping.");

		LogDebug("ShouldStopReceiving: Returning FALSE.");

		LogDebug("ShouldStopReceiving: Done.");

		return bResult;
	}

	LogInfo("ShouldStopReceiving: nSize is a positive quantity, which is valid.  Continuing...");

	LogInfo("ShouldStopReceiving: Checking whether the text received is the server's goodbye message...");

	// Stop receiving if the server says good bye to us.
	bResult = strcasecmp(pszReceivedText, OK_GOODBYE) == 0;

	if (bResult) {
		LogInfo("ShouldStopReceiving: The goodbye message (or an error reply) has been detected.");

		LogDebug("ShouldStopReceiving: Returning TRUE.");
	} else {
		LogInfo(
				"ShouldStopReceiving: No replies from the server have indicated that we should stop polling.");

		LogDebug("ShouldStopReceiving: Returning FALSE.");
	}

	LogDebug("ShouldStopReceiving: Done.");

	return bResult;
}
