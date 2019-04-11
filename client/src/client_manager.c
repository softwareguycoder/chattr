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
#include "client_functions.h"

#include "client_manager.h"

///////////////////////////////////////////////////////////////////////////////
// GetNickname function: Prompts the user for a nickname, and places the value
// typed into the buffer pointed to by the nickname parameter.
//

BOOL GetNickname(char* pszNickname, int nSize) {
	if (pszNickname == NULL || pszNickname[0] == '\0') {
		CleanupClient(ERROR);
	}

	if (nSize < MIN_SIZE) {
		// nSize is not a positive value.  This can't be right.
		CleanupClient(ERROR);
	}

	// Prompt the user to input their desired chat handle.
	int nGetLineResult = GetLineFromUser(NICKNAME_PROMPT, pszNickname, nSize);
	if (nGetLineResult != OK) {
		fprintf(stderr, "chattr: Please type a value for the nickname that is "
				"%d characters or less.", MAX_NICKNAME_LEN);

		/* If we are here, just fall through to SetNickname if the TOO_LONG code
		 * got returned by GetLineFromUser.  This will make the SetNickname
		 * function (called right after this one) complain to the user that
		 * their requested nickname exceeds the maximum allowed number of
		 * characters, and will give them another chance to put a better
		 * nickname in. */
		if (nGetLineResult == TOO_LONG)
			return FALSE;
		else
			CleanupClient(ERROR);
	}

	// Successfully obtained a valid nickname.
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// GreetServer function: carries out the first step of the chat protocol, which
// consists of isusing the HELO command to the server
//

void GreetServer() {
	if (0 >= Send(nClientSocket, PROTOCOL_HELO_COMMAND)) {
		// Error sending HELO command.
		CleanupClient(ERROR);
	}
}

///////////////////////////////////////////////////////////////////////////////
// HandshakeWithServer function: Carries out the handshaking with the chat
// server, per protocol.  This consists of (1) sending the HELO command and
// getting an "OK" response; (2) if that succeeds, then prompts the user
// to type a nickname or 'chat handle' which is then sent to the server using
// the NICK command of the protocol.  For more details, see protocol_spec.txt
//

void HandshakeWithServer() {
	char szNickname[MAX_NICKNAME_LEN + 1];

	/* We run a loop in case the user's requested value does not satisfy the validation
	 * condition (that is imposed by our protocol) that the chat handle/nickname can be
	 * no longer than a certain number of chars. */

	while (!GetNickname(szNickname, MAX_NICKNAME_LEN)) {
		fprintf(stderr,
				"ERROR: Please choose a nickname that is %d characters or fewer in length.\n",
				MAX_NICKNAME_LEN);
	}

	char* pszReplyBuffer = NULL;

	/* Begin the chat session */
	GreetServer();

	int nBytesReceived = ReceiveFromServer((char**) &pszReplyBuffer);

	ProcessReceivedText(pszReplyBuffer, nBytesReceived);

	free_buffer((void**) &pszReplyBuffer);

	// Tell the server what nickname the user wants.
	SetNickname(szNickname);

	ReceiveFromServer((char**) &pszReplyBuffer);

	ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

	free_buffer((void**) &pszReplyBuffer);

	// Tell the user how to chat.
	PrintClientUsageDirections();
}

///////////////////////////////////////////////////////////////////////////////
// LeaveChatRoom function - Sends the QUIT command to the server (per the
// protocol) to tell the server that the user wants to leave the chat room.
// Normally, the client's user just types "QUIT" and presses ENTER to do this;
// however, the client's software might also have to leave the chat room
// automatically; say, if an error occurrs elsewhere

void LeaveChatRoom() {

	int nBytesSent = 0;

	if ((nBytesSent = Send(nClientSocket, PROTOCOL_QUIT_COMMAND)) <= 0) {
		// Error sending QUIT command.
		CleanupClient(ERROR);
	}

	// Send successful.

	sleep(1); // force CPU context switch to trigger threads to do their stuff
}

///////////////////////////////////////////////////////////////////////////////
// PrintClientUsageDirections function - Tells the user of the client how to
// actually chat with other people by printing a message to the screen.
//

void PrintClientUsageDirections() {
	/* Print some usage directions */
	fprintf(stdout, USAGE_MESSAGE);
}

////////////////////////////////////////////////////////////////////////////////
// ProcessReceivedText functon - Deals with text that is received from the
// server.  Basically this is just to interpret server response codes in accor-
// dance with the protocol and print messages to the screen.
//

void ProcessReceivedText(const char* pszReceivedText, int nSize) {
	if (nSize < MIN_SIZE) {
		// How can we have negative or zero bytes of text?
		return;
	}

	// Double-check that the received text is not blank.
	if (pszReceivedText == NULL || pszReceivedText[0] == '\0') {
		return;
	}

	// Format the text that should be dumped to the console.
	char szTextToDump[strlen(pszReceivedText) + 1];

	// For now, just dump all received text to the screen. If the text begins
	// with an exclamation mark (bang) then strip off the bang first.  If not,
	// then it's a direct reply by the server to a command.
	if (pszReceivedText[0] == '!') {
		memmove(szTextToDump, pszReceivedText+1, strlen(pszReceivedText));
		fprintf(stdout, "%s", szTextToDump);
	} else {
		fprintf(stdout, "S: %s", pszReceivedText);
	}
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

	LogInfo(
			"ShouldStopReceiving: nSize is a positive quantity, which is valid.  Continuing...");

	LogInfo(
			"ShouldStopReceiving: Checking whether the text received is the server's goodbye message...");

	// Stop receiving if the server says good bye to us.
	bResult = strcasecmp(pszReceivedText, OK_GOODBYE) == 0
			|| strcasecmp(pszReceivedText, ERROR_FORCED_DISCONNECT) == 0;

	if (bResult) {
		LogInfo(
				"ShouldStopReceiving: The goodbye message (or an error reply) has been detected.");

		LogDebug("ShouldStopReceiving: Returning TRUE.");
	} else {
		LogInfo(
				"ShouldStopReceiving: No replies from the server have indicated that we should stop polling.");

		LogDebug("ShouldStopReceiving: Returning FALSE.");
	}

	LogDebug("ShouldStopReceiving: Done.");

	return bResult;
}
