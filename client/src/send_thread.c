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
// ShouldKeepSending function - Examines the current line that is supposed to
// contain the data that was just sent, and determines if it was, basically,
// the QUIT command that is supposed to terminate communications.
//

BOOL ShouldKeepSending(const char* pszCurLine) {
	if (pszCurLine == NULL || pszCurLine[0] == '\0'
			|| strcasecmp(pszCurLine, PROTOCOL_QUIT_COMMAND) == 0) {
		// The QUIT command has been issued, so we should stop sending.
		return FALSE;
	}

	// If we are here, then we can keep the sending thread alive.
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// SendThread function

void *SendThread(void *pvData) {
	LogDebug("In SendThread");

	LogInfo("SendThread: Beginning sending thread...");

	LogInfo(
			"SendThread: Checking whether the sending socket file descriptor is valid...");

	// Double check to ensure we have a valid socket file descriptor for
	// communications.  If not, then stop.
	if (!IsSocketValid(nClientSocket)) {
		LogError(
				"SendThread: The sending socket descriptor is invalid, or the socket is now closed.  Stopping.");

		LogDebug("SendThread: Done.");

		return NULL;
	}

	LogInfo(
			"SendThread: The sending socket file descriptor has a valid value.");

	LogInfo("SendThread: Waiting for user input...");

	char szCurLine[MAX_LINE_LENGTH + 1]; // Buffer for the current line inputted by the user

	//ShowPrompt();

	// Continuously run a fgets.  Since the fgets call merely blocks the current thread
	// and not the entire program, we can call fgets here and lines sent from the chat
	// server will still be received by the other thread we have spun up for receiving text
	// and writing it to stdout.
	while (NULL != fgets(szCurLine, MAX_LINE_LENGTH, stdin)) {
		LogInfo("SendThread: Flushing stdin...");

		FlushStdin();	// Get everything off the stdin

		LogInfo("SendThread: Standard input has been flushed.");

		LogInfo("SendThread: Checking the user input...");

		if (szCurLine[0] == '\n') {
			LogInfo(
					"SendThread: The user just typed an empty line.  Waiting for more user input...");

			continue;		// skip instances where the user just presses ENTER.
		}

		LogInfo(
				"SendThread: The user typed something and pressed ENTER.  Sending to the chat server...");

		// If we are here, then there is something to be sent.  Go ahead and send it to
		// the socket.
		if (0 > Send(nClientSocket, szCurLine)) {
			LogInfo("SendThread: A problem occurred with the send operation.");

			LogInfo("SendThread: Giving up and waiting for more user input...");

			// If we are here, then an error occurred with sending.
			continue;
		}

		/*LogInfo("SendThread: Chat message sent to server.  Sending message terminator...");

		 if (0 > Send(nClientSocket, MSG_TERMINATOR)) {
		 // If we are here, then an error occurred with sending.
		 continue;
		 }

		 LogInfo("SendThread: Message terminator sent.")*/

		sleep(1);	// Force a context switch

		LogInfo(
				"SendThread: Asking whether we can continue checking for chat messages to send...");

		// Ask if we should keep sending, or whether it's time to stop waiting for
		// input.
		if (!ShouldKeepSending(szCurLine)) {
			LogInfo(
					"SendThread: The user has sent something to the server that matches the criteria for terminating the chat session.  Stopping.");
			break;
		}

		//ShowPrompt();
	}

	LogInfo("SendThread: Finished looking for user input.");

	LogDebug("SendThread: Done.");

	// Done with send thread
	return NULL;
}
