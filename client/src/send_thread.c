/*
 * send_thread.c
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "send_thread.h"

HTHREAD g_hSendThread;

///////////////////////////////////////////////////////////////////////////////
// ShouldKeepSending function

BOOL ShouldKeepSending(const char* pszCurLine) {
	if (pszCurLine == NULL || pszCurLine[0] == '\0'
			|| strcasecmp(pszCurLine, PROTOCOL_QUIT_COMMAND) == 0
			|| strcmp(pszCurLine, MSG_TERMINATOR) == 0) {
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// ShowPrompt function

void ShowPrompt() {
	fprintf(stdout, INPUT_PROMPT);
}

///////////////////////////////////////////////////////////////////////////////
// SendThread function

void *SendThread(void *pvData) {
	// Double check to ensure we have a valid socket file descriptor for
	// communications.  If not, then stop.
	if (!IsSocketValid(nClientSocket)) {
		return NULL;
	}

	char szCurLine[MAX_LINE_LENGTH + 1]; // Buffer for the current line inputted by the user

	//ShowPrompt();

	// Continuously run a fgets.  Since the fgets call merely blocks the current thread
	// and not the entire program, we can call fgets here and lines sent from the chat
	// server will still be received by the other thread we have spun up for receiving text
	// and writing it to stdout.
	while (NULL != fgets(szCurLine, MAX_LINE_LENGTH, stdin)) {
		if (szCurLine[0] == '\n') {
			continue;		// skip instances where the user just presses ENTER.
		}

		// If we are here, then there is something to be sent.  Go ahead and send it to
		// the socket.
		if (0 > Send(nClientSocket, szCurLine)) {
			// If we are here, then an error occurred with sending.
			continue;
		}

		sleep(1);	// Force a context switch

		// Ask if we should keep sending, or whether it's time to stop waiting for
		// input.
		if (!ShouldKeepSending(szCurLine))
			break;

		//ShowPrompt();
	}

	// Done with send thread
	return NULL;
}
