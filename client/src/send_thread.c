// send_thread.c - Contains the implementation of the client's sending thread
//

#include "stdafx.h"
#include "client.h"

#include "send_thread_functions.h"
#include "send_thread.h"


///////////////////////////////////////////////////////////////////////////////
// SendThread function

void *SendThread(void *pvData) {
	// Double check to ensure we have a valid socket file descriptor for
	// communications.  If not, then stop.
	if (!IsSocketValid(g_nClientSocket)) {
		return NULL;
	}

	// Poll stdin for user input.  Once we get some, send it off to the server
	// and then poll again.  Keep going until the ShouldStopSending() function
	// tells us to stop.

	// Buffer for the current line inputted by the user
	char szCurLine[MAX_LINE_LENGTH + 1];

	// Continuously run a fgets.  Since the fgets call merely blocks the
	// current thread and not the entire program, we can call fgets here and
	// lines sent from the chat server will still be received by the other
	// thread we have spun up for receiving text and written to stdout.

	while (NULL != fgets(szCurLine, MAX_LINE_LENGTH, stdin)) {
		// Get everything off the stdin
		FlushStdin();

		if (g_bShouldTerminateSendThread) {
            g_bShouldTerminateSendThread = FALSE;
            break;
        }

		if (IsNullOrWhiteSpace(szCurLine)) {
			continue;		// skip instances where the user just presses ENTER
							// or just types spaces
		}

		// If we are here, then there is something to be sent.  Go ahead and
		// send it to the socket.  Just skip the current input if an error
		// occurs.
		if (0 > Send(g_nClientSocket, szCurLine)) {
			// If we are here, then an error occurred with sending.
			continue;
		}

		// If we are here, then the send operation occurred successfully.
		// Log the communications with the server in the log file
		if (GetLogFileHandle() != stdout) {
		    LogInfo(CLIENT_DATA_FORMAT, szCurLine);
		}

		sleep(1);	// Force a context switch to allow the receive thread to
					// detect any server replies

		// Ask if we should keep sending, or whether it's time to
		// stop waiting for input.
		if (!ShouldKeepSending(szCurLine)) {
			break;
		}

        if (g_bShouldTerminateSendThread) {
            g_bShouldTerminateSendThread = FALSE;
            break;
        }
	}

	if (GetLogFileHandle() != stdout) {
	    LogInfo("Send thread shutting down.");
	}

	// Done with send thread
	return NULL;
}
