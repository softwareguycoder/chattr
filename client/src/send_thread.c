/*
 * send_thread.c
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "send_thread.h"

HTHREAD hSendThread;

///////////////////////////////////////////////////////////////////////////////
// ShouldKeepSending function

BOOL ShouldKeepSending(const char* cur_line) {
	if (cur_line == NULL || cur_line[0] == '\0'
			|| strcasecmp(cur_line, PROTOCOL_QUIT_COMMAND) == 0
			|| strcmp(cur_line, MSG_TERMINATOR) == 0) {
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

void *SendThread(void *pData) {
	// Check if NULL was passed for the user state; if this is so, stop.
	if (pData == NULL) {
		return NULL;
	}

	// Try to get the socket file descriptor of the client connection from
	// the user state.
	int *pClientSockFd = (int*) pData;
	int client_socket = *pClientSockFd;

	// Double check to ensure we have a valid socket file descriptor for
	// communications.  If not, then stop.
	if (!IsSocketValid(client_socket)) {
		return NULL;
	}

	char cur_line[MAX_LINE_LENGTH + 1]; // Buffer for the current line inputted by the user

	//ShowPrompt();

	// Continuously run a fgets.  Since the fgets call merely blocks the current thread
	// and not the entire program, we can call fgets here and lines sent from the chat
	// server will still be received by the other thread we have spun up for receiving text
	// and writing it to stdout.
	while (NULL != fgets(cur_line, MAX_LINE_LENGTH, stdin)) {
		if (cur_line[0] == '\n') {
			continue;		// skip instances where the user just presses ENTER.
		}

		// If we are here, then there is something to be sent.  Go ahead and send it to
		// the socket.
		if (0 > Send(client_socket, cur_line)) {
			// If we are here, then an error occurred with sending.
			continue;
		}

		sleep(1);	// Force a context switch

		// Ask if we should keep sending, or whether it's time to stop waiting for
		// input.
		if (!ShouldKeepSending(cur_line))
			break;

		//ShowPrompt();
	}

	// Done with send thread
	return NULL;
}
