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

BOOL g_bAskForNicknameAgain = FALSE; /* file-scope global; used here only */

BOOL g_bChattersHaveBeenListed = FALSE;

/* Are we in the process of receiving the list of chatters? */
BOOL g_bReceivingChatterList = FALSE;

POSITION* g_pChatterList; /* list of chatters */

char g_szNickname[MAX_NICKNAME_LEN + 1]; /* global-scope */

///////////////////////////////////////////////////////////////////////////////
// Internal file-scope-only functions

///////////////////////////////////////////////////////////////////////////////
// Publicly-exposed functions (to rest of the program)

///////////////////////////////////////////////////////////////////////////////
// GetNicknameFromUser function
//

BOOL GetNicknameFromUser(char* pszNickname) {
	if (pszNickname == NULL) {
		fprintf(stderr, "GetNickname expects the address of storage that"
				" will receive the chat nickname the user types.\n");

		// Nickname invalid.
		CleanupClient(ERROR);
	}

	ClearNickname();

	// Prompt the user to input their desired chat handle.  Remove whitespace.
	int nGetLineResult = GetLineFromUser(NICKNAME_PROMPT, pszNickname,
	MAX_NICKNAME_LEN);

	if (IsNicknameValid(nGetLineResult, pszNickname)) {
		strcpy(g_szNickname, pszNickname);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// GreetServer function: carries out the first step of the chat protocol, which
// consists of isusing the HELO command to the server
//

void GreetServer() {
	if (0 >= Send(g_nClientSocket, PROTOCOL_HELO_COMMAND)) {
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
// HandleAdminOrChatMessage function

void HandleAdminOrChatMessage(const char* pszReceivedText) {
	if (IsNullOrWhiteSpace(pszReceivedText)) {
		return;
	}

	const int RECEIVED_TEXT_SIZE = strlen(pszReceivedText) + 1;

	// Format the text that should be dumped to the console.
	char szTextToDump[RECEIVED_TEXT_SIZE];
	memset(szTextToDump, 0, RECEIVED_TEXT_SIZE*sizeof(char));

	// strip off the '!' char in front
	memmove(szTextToDump, pszReceivedText + 1,
			(RECEIVED_TEXT_SIZE - 1)*sizeof(char));

	// If we are currently in the middle of receiving the list of chatters
	// from the program, this function will be called as the lines of output
	// begin with '!' and it's this function that is called in that case, to
	// handle admin/chat output.  Also, during the operation, the
	// g_bReceivingChatterList variable will have the value of TRUE.  We
	// put the names returned into a linked list.
	if (g_bReceivingChatterList) {
		char* pszChatterName = (char*) malloc(
				RECEIVED_TEXT_SIZE * sizeof(char));
		if (pszChatterName == NULL) {
			fprintf(stderr, FAILED_ALLOC_CHATTER_NAME);
			return;
		}

		memset(pszChatterName, 0, RECEIVED_TEXT_SIZE * sizeof(char));
		strcpy(pszChatterName, szTextToDump);
		AddElementToTail(&g_pChatterList, pszChatterName);
		return;
	}

	LogInfo("S: !%s", szTextToDump);
	if (GetLogFileHandle() != stdout
			&& !IsMultilineResponseTerminator(pszReceivedText)) {
		fprintf(stdout, "%s", szTextToDump);
	}

	memset(szTextToDump, 0, (RECEIVED_TEXT_SIZE) * sizeof(char));
}
///////////////////////////////////////////////////////////////////////////////
// HandleIncorrectNicknameSubmitted function

void HandleIncorrectNicknameSubmitted(char* pszNickname, int nNicknameSize,
		char* pszReplyBuffer) {
	if (nNicknameSize <= 0) {
		return;
	}

	while (g_bAskForNicknameAgain) {
		g_bAskForNicknameAgain = FALSE; // reset if nickname still in use

		// whoops, server did not like the nickname we used
		memset(pszNickname, 0, nNicknameSize);

		PromptUserForNickname(pszNickname);

		// Tell the server what nickname the user wants.
		SetNickname(pszNickname);

		ReceiveFromServer((char**) &pszReplyBuffer);

		ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

		FreeBuffer((void**) &pszReplyBuffer);
	}
}

void HandleProtocolReply(const char* pszReplyMessage) {
	if (IsNullOrWhiteSpace(pszReplyMessage)) {
		LogError(ERROR_INVALID_PTR_ARG);

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr, ERROR_INVALID_PTR_ARG);
		}
	}

	if (IsMultilineResponseTerminator(pszReplyMessage)) {
		if (g_bReceivingChatterList) {
			g_bReceivingChatterList = FALSE;

			if (GetElementCount(g_pChatterList) == 0) {
				fprintf(stdout, NO_OTHER_CHATTERS);
			} else {
				DoForEach(g_pChatterList, PrintChatterName);
				ClearList(&g_pChatterList, DefaultFree);

				fprintf(stdout, BLANK_LINE);
			}
		}
		return; /* stop processing lines of text that terminate a
		 multiline response. */
	}

	/* start of a list of chatters */
	if (StartsWith(pszReplyMessage, "203 ")) {
		g_bReceivingChatterList = TRUE;
		return;
	}

	if (StartsWith(pszReplyMessage, "502 ")) {
		/* indicates that client tried to connect to the server,
		 * however the maximum allowed number of clients are already
		 * connected, or its database is full. */
		LogError(ERROR_MAX_CONNECTIONS_EXCEEDED);

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr, ERROR_MAX_CONNECTIONS_EXCEEDED);
		}

		CleanupClient(ERROR);
		return;
	}

	if (StartsWith(pszReplyMessage, "504 ")) {
		// Nickname already taken
		fprintf(stderr, ERROR_NICKNAME_ALREADY_IN_USE);
		g_bAskForNicknameAgain = TRUE;
		return;
	}

	if (StartsWith(pszReplyMessage, "505 ")) {
		// Nickname cannot be changed.
		fprintf(stderr, CHANGING_NICKNAME_NOT_ALLOWED);
		g_bAskForNicknameAgain = FALSE;
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
	memset(szNickname, 0, MAX_NICKNAME_LEN + 1);

	PromptUserForNickname(szNickname);

	char* pszReplyBuffer = NULL;

	/* Begin the chat session */
	GreetServer();

	int nBytesReceived = ReceiveFromServer((char**) &pszReplyBuffer);

	ProcessReceivedText(pszReplyBuffer, nBytesReceived);

	FreeBuffer((void**) &pszReplyBuffer);

	// Tell the server what nickname the user wants.
	SetNickname(szNickname);

	ReceiveFromServer((char**) &pszReplyBuffer);

	ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

	FreeBuffer((void**) &pszReplyBuffer);

	// Handle the case where *we* validated and submitted a nickname
	// to the server, and the server rejected it (more than likely, because
	// another chatter already was using that handle)
	HandleIncorrectNicknameSubmitted(szNickname, MAX_NICKNAME_LEN + 1,
			pszReplyBuffer);

	// Tell the user how to chat.
	PrintClientUsageDirections();

	PrintChattersInRoom();
}

///////////////////////////////////////////////////////////////////////////////
// IsAdminOrChatMessage function

BOOL IsAdminOrChatMessage(const char* pszReceivedText) {
	if (IsNullOrWhiteSpace(pszReceivedText)) {
		return FALSE;
	}
	return pszReceivedText[0] == '!';
}

BOOL IsMultilineResponseTerminator(const char* pszMessage) {
	if (IsNullOrWhiteSpace(pszMessage)) {
		return FALSE;
	}
	return strcmp(pszMessage, ".\n") == 0;
}

///////////////////////////////////////////////////////////////////////////////
// LeaveChatRoom function - Sends the QUIT command to the server (per the
// protocol) to tell the server that the user wants to leave the chat room.
// Normally, the client's user just types "QUIT" and presses ENTER to do this;
// however, the client's software might also have to leave the chat room
// automatically; say, if an error occurrs elsewhere

void LeaveChatRoom() {

	int nBytesSent = 0;

	if ((nBytesSent = Send(g_nClientSocket, PROTOCOL_QUIT_COMMAND)) <= 0) {
		// Error sending QUIT command.
		CleanupClient(ERROR);
	}

	LogInfo("C: %s", PROTOCOL_QUIT_COMMAND);

	// Send successful.

	/* clear out the current nickname value */
	ClearNickname();

	sleep(1); // force CPU context switch to trigger threads to do their stuff
}

///////////////////////////////////////////////////////////////////////////////
// PrintChatterName function

void PrintChatterName(void* pvChatterName) {
	if (pvChatterName == NULL) {
		return;
	}

	char* pszChatterName = (char*) pvChatterName;
	if (IsNullOrWhiteSpace(pszChatterName)) {
		return;
	}

	const int CHATTER_NAME_SIZE = strlen(pszChatterName) + 1;

	char szTextToDump[CHATTER_NAME_SIZE];
	memset(szTextToDump, 0, CHATTER_NAME_SIZE*sizeof(char));

	Trim(szTextToDump, CHATTER_NAME_SIZE, pszChatterName);

	// strip off the '!' char in front
	if (StartsWith(pszChatterName, "!")) {
		memmove(szTextToDump, pszChatterName + 1, CHATTER_NAME_SIZE - 1);
	}

	// Dump the chatter's name
	fprintf(stdout, "%s\n", szTextToDump);
}

///////////////////////////////////////////////////////////////////////////////
// PrintChattersInRoom function

void PrintChattersInRoom() {
	// Request a list of chatters currently in the room from the server
	fprintf(stdout, OTHER_CHATTERS_IN_ROOM_ARE);

	// Send the LIST command to the server.  The response will cause
	// other code in this software to print the list to STDOUT.
	Send(g_nClientSocket, PROTOCOL_LIST_COMMAND);

	ProcessMultilineResponse();

	g_bChattersHaveBeenListed = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// PrintClientUsageDirections function - Tells the user of the client how to
// actually chat with other people by printing a message to the screen.
//

void PrintClientUsageDirections() {
	if (IsNullOrWhiteSpace(g_szNickname)) {
		fprintf(stderr, "ERROR: Chat session has not been set up yet.\n");

		exit(ERROR);
	}

	PrintSoftwareTitleAndCopyright();

	/* Print some usage directions */
	fprintf(stdout, CHAT_USAGE_MESSAGE, g_szNickname);
}

///////////////////////////////////////////////////////////////////////////////
// ProcessMultilineResponse function

void ProcessMultilineResponse() {
	char* pszReplyBuffer = NULL;

	ReceiveFromServer(&pszReplyBuffer);

	while (!IsMultilineResponseTerminator(pszReplyBuffer)) {
		ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

		FreeBuffer((void**) &pszReplyBuffer);

		ReceiveFromServer(&pszReplyBuffer);
	}

	ProcessReceivedText(pszReplyBuffer, strlen(pszReplyBuffer));

	FreeBuffer((void**) &pszReplyBuffer);
}

///////////////////////////////////////////////////////////////////////////////
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

	// For now, just dump all received text to the screen. If the text begins
	// with an exclamation mark (bang) then strip off the bang first.  If not,
	// then it's a direct reply by the server to a command.
	if (IsAdminOrChatMessage(pszReceivedText)) {
		HandleAdminOrChatMessage(pszReceivedText);
	} else {
		// If we are here, it's more likely that the server sent a protocol
		// specific reply to a command that we issued.  It's not necessary to
		// show this on the screen; it can just be sent to the log file.
		if (GetLogFileHandle() != stdout) {
			LogInfo(SERVER_DATA_FORMAT, pszReceivedText);
		}

		HandleProtocolReply(pszReceivedText);
	}
}

///////////////////////////////////////////////////////////////////////////////
// PromptUserForNickname function

void PromptUserForNickname(char* pszNicknameBuffer) {
	if (pszNicknameBuffer == NULL) {
		LogError(ERROR_INVALID_PTR_ARG);

		if (GetErrorLogFileHandle() != stderr) {
			fprintf(stderr, ERROR_INVALID_PTR_ARG);
		}

		return;
	}

	/* We run a loop in case the user's requested value does not satisfy
	 * the validation condition (that is imposed by our protocol) that the
	 * chat handle/nickname can be  no longer than a certain number of
	 * chars and must be alphanumeric and cannot contain spaces or special
	 * characters (not to mention, cannot be blank). */

	while (!GetNicknameFromUser(pszNicknameBuffer)) {
		/* blank out the nickname for the next try, so that
		 * buffer-packing cannot occur */
		memset(pszNicknameBuffer, 0, MAX_NICKNAME_LEN + 1);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ReceiveFromServer function - Does a one-off, synchronous receive (not a
// polling loop) of a specific message from the server.  Blocks the calling
// thread until the message has arrived.
//

int ReceiveFromServer(char** ppszReplyBuffer) {
// Check whether we have a valid endpoint for talking with the server.
	if (!IsSocketValid(g_nClientSocket)) {
		fprintf(stderr,
				"chattr: Failed to receive the line of text back from the "
						"server.\n");

		CleanupClient(ERROR);
	}

	/* Wipe away any existing reply buffer */
	if (ppszReplyBuffer != NULL) {
		if (*ppszReplyBuffer != NULL) {
			memset(*ppszReplyBuffer, 0, strlen(*ppszReplyBuffer));
		}

		FreeBuffer((void**) ppszReplyBuffer);
	}

	/* Do a receive. Cleanup if the operation was not successful. */
	int nBytesRead = 0;

	if ((nBytesRead = Receive(g_nClientSocket, ppszReplyBuffer))
			< 0&& errno != EBADF && errno != EWOULDBLOCK) {
		FreeBuffer((void**) ppszReplyBuffer);

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

	if ((nBytesSent = Send(g_nClientSocket, szNicknameCommand)) < 0) {
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

	if (bResult) {
		if (GetLogFileHandle() != stdout) {
			LogDebug("ShouldStopReceiving: returning TRUE.");
		}
	}

	return bResult;
}
