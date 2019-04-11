/*
 * receive_thread.c
 *
 *  Created on: Apr 10, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "client_manager.h"
#include "receive_thread.h"

BOOL 	g_bShouldTerminateReceiveThread = FALSE;

HTHREAD g_hReceiveThread = INVALID_HANDLE_VALUE;

void *ReceiveThread(void *pvData) {
	// Keep track of total bytes received
	int nTotalBytesReceived = 0;

	// Start polling the server endpoint for any data it has for us.
	while(1) {
		char *pszReceiveBuffer = NULL;
		int nBytesReceived = 0;

		// Ask socket for data.  If it has none, then just loop again or
		// keep waiting if this is a blocking socket.
		if ((nBytesReceived = Receive(nClientSocket, (char**)&pszReceiveBuffer))
				> 0) {

			// Data was actually received from the server.  Tally the total
			// bytes received.
			nTotalBytesReceived += nBytesReceived;

			// Handle the data received from the server.
			ProcessReceivedText(pszReceiveBuffer, nBytesReceived);

			// Ask whether we should stop receiving (perhaps the QUIT command
			// was sent, or server disconnected us forcibly from its end)
			if (ShouldStopReceiving(pszReceiveBuffer, nBytesReceived)) {
				break;
			}

			/* If we get to here, we have not been told to stop receiving, so
			 * keep polling. */
		} else if (errno != EWOULDBLOCK && errno != EBADF) {
			// An unknown error occurred.
			perror("ReceiveThread");
			break;
		}
	}

	// Done polling.
	return NULL;
}

void TerminateReceiveThread(int signum) {
	LogDebug("In TerminateReceiveThread");

	LogInfo("TerminateReceiveThread: Checking whether signum is SIGSEGV...");

	LogDebug("TerminateReceiveThread: signum = %d", signum);

	if (SIGSEGV != signum) {
		LogError("TerminateReceiveThread: signum is not equal to SIGSEGV.  Stopping.");

		LogDebug("TerminateReceiveThread: Done.");

		return;
	}

	LogInfo("TerminateReceiveThread: SIGSEGV signal received.  Marking ReceiveThread for termination...");

	LogDebug("TerminateReceiveThread: Setting g_bShouldTerminateReceiveThread to TRUE...");

	g_bShouldTerminateReceiveThread = TRUE;

	LogDebug("TerminateReceiveThread: Set g_bShouldTerminateReceiveThread equal to TRUE.");

	LogInfo("TerminateReceiveThread: Re-registering ourselves as a SIGSEGV event handler...");

	RegisterEvent(TerminateReceiveThread);

	LogInfo("TerminateReceiveThread: We've re-registered ourselves as a SIGSEGV event handler.");

	LogDebug("TerminateReceiveThread: Done.");
}
