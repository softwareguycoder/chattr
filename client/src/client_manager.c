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
#include "nickname_validation.h"

///////////////////////////////////////////////////////////////////////////////
// GetNickname function: Prompts the user for a nickname, and places the value
// typed into the buffer pointed to by the nickname parameter.
//

BOOL GetNickname(char* pszNickname) {
	if (pszNickname == NULL) {
		fprintf(stderr, "GetNickname expects the address of storage that"
				" will receive the chat nickname the user types.\n");

		// Nickname invalid.
		CleanupClient(ERROR);
	}

	// Prompt the user to input their desired chat handle.  Remove whitespace.
	int nGetLineResult = GetLineFromUser(NICKNAME_PROMPT, pszNickname,
			MAX_NICKNAME_LEN);

	return IsNicknameValid(nGetLineResult, pszNickname);
}

///////////////////////////////////////////////////////////////////////////////
// GreetServer function: carries out the first step of the chat protocol, which
// consists of isusing the HELO command to the server
//

void GreetServer() {
	if (0 >= Send(nClientSocket, PROTOCOL_HELO_COMMAND)) {
		// Error sending HELO command.
	    LogError("GreetServer: Failed to send HELO command.");

		CleanupClient(ERROR);
	}

	// If we are here, then the send operation was successful.

	if (GetLogFileHandle() != stdout) {
	    LogInfo(CLIENT_DATA_FORMAT, PROTOCOL_HELO_COMMAND);
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

	/* We run a loop in case the user's requested value does not satisfy
	 * the validation condition (that is imposed by our protocol) that the
	 * chat handle/nickname can be  no longer than a certain number of
	 * chars and must be alphanumeric and cannot contain spaces or special
	 * characters (not to mention, cannot be blank). */

	while (!GetNickname(szNickname)) {
		/* blank out the nickname for the next try, so that
		 * buffer-packing cannot occur */
		memset(szNickname, 0, MAX_NICKNAME_LEN + 1);
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

    LogInfo("C: %s", PROTOCOL_QUIT_COMMAND);

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
	if (IsNullOrWhiteSpace(pszReceivedText)) {
		return;
	}

	// Format the text that should be dumped to the console.
	char szTextToDump[strlen(pszReceivedText) + 1];

	// For now, just dump all received text to the screen. If the text begins
	// with an exclamation mark (bang) then strip off the bang first.  If not,
	// then it's a direct reply by the server to a command.
	if (pszReceivedText[0] == '!') {
		memmove(szTextToDump, pszReceivedText + 1, strlen(pszReceivedText));
		LogInfo("%s", szTextToDump);
		if (GetLogFileHandle() != stdout) {
			fprintf(stdout, "%s", szTextToDump);
		}
	} else {
	    // If we are here, it's more likely that the server sent a protocol
	    // specific reply to a command that we issued.  It's not necessary to
	    // show this on the screen; it can just be sent to the log file.
	    if (GetLogFileHandle() != stdout) {
	        LogInfo(SERVER_DATA_FORMAT, pszReceivedText);
	    }
	}
}

///////////////////////////////////////////////////////////////////////////////
// ReceiveFromServer function - Does a one-off, synchronous receive (not a
// polling loop) of a specific message from the server.  Blocks the calling
// thread until the message has arrived.
//

int ReceiveFromServer(char** ppszReplyBuffer) {
	// Check whether we have a valid endpoint for talking with the server.
	if (!IsSocketValid(nClientSocket)) {
		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the server.");

		CleanupClient(ERROR);
	}

	/* Wipe away any existing reply buffer */
	if (ppszReplyBuffer != NULL) {
		free_buffer((void**) ppszReplyBuffer);
	}

	/* Do a receive. Cleanup if the operation was not successful. */
	int nBytesRead = 0;

	if ((nBytesRead = Receive(nClientSocket, ppszReplyBuffer))
			< 0&& errno != EBADF && errno != EWOULDBLOCK) {
		free_buffer((void**) ppszReplyBuffer);

		fprintf(stderr, "chattr: Failed to receive the line of text back from "
				"the server.");

		CleanupClient(ERROR);
	}

	// Return the number of received bytes
	return nBytesRead;
}

///////////////////////////////////////////////////////////////////////////////
// SetNickname function: Sets the user's chat handle or nickname to the desired
// value.
//

BOOL SetNickname(const char* pszNickname) {
	// Make sure the input is not blank.
	if (IsNullOrWhiteSpace(pszNickname)) {
		fprintf(stderr, "SetNickname: A non-blank nickname is required.\n");
		return FALSE;
	}

	if (strlen(pszNickname) > MAX_NICKNAME_LEN) {
		fprintf(stderr, "SetNickname: Nickname must be %d characters or less.  "
				"Please try again.\n", MAX_NICKNAME_LEN);
		return FALSE;
	}

	if (!IsAlphaNumeric(pszNickname)) {
		fprintf(stderr, "SetNickname: Nickname can only have letters and"
				"numbers.  No spaces or special chars allowed.\n");
		return FALSE;
	}
	// Make a buffer to format the command string.  It must be
	// "NICK <value>\n", so we format 6 chars (N-I-C-K, plus space, plus
	// newline) and then send it off to the server.
	char szNicknameCommand[6 + MAX_NICKNAME_LEN];

	sprintf(szNicknameCommand, PROTOCOL_NICK_COMMAND, pszNickname);

	int nBytesSent = 0;

	if ((nBytesSent = Send(nClientSocket, szNicknameCommand)) < 0) {
		fprintf(stderr, "chattr: Failed to send NICK command.\n");

		CleanupClient(ERROR);
	}

	// If we are here, then the command was sent successfully
	if (GetLogFileHandle() != stdout) {
	    LogInfo(CLIENT_DATA_FORMAT, szNicknameCommand);
	}

	/* TRUE return value means that the
	 * user's requested nickname was valid. */

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ShouldStopReceiving function: Decides whether the receive thread is to be
// terminated.  Occurs when we see the message "200 Goodbye" from the server.
//

BOOL ShouldStopReceiving(const char* pszReceivedText, int nSize) {
	BOOL bResult = FALSE;	// Default return value is FALSE

	if (nSize < MIN_SIZE) {
		// Can't do anything with a message zero or negative bytes long.
		return bResult;
	}

	// Stop receiving if the server says good bye to us.
	bResult = strcasecmp(pszReceivedText, OK_GOODBYE) == 0
			|| strcasecmp(pszReceivedText, ERROR_FORCED_DISCONNECT) == 0;

	return bResult;
}
