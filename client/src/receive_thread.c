/*
 * receive_thread.c
 *
 *  Created on: Apr 10, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "client_manager.h"

BOOL 	g_bShouldTerminateReceiveThread = FALSE;

HTHREAD g_hReceiveThread = INVALID_HANDLE_VALUE;

void *ReceiveThread(void *pvData) {
	LogDebug("In ReceiveThread");

	int nTotalBytesReceived = 0;

	LogInfo("ReceiveThread: Starting polling loop...");

	while(1) {
		char *pszReceiveBuffer = NULL;
		int nBytesReceived = 0;

		LogInfo("ReceiveThread: Checking if there is data waiting for us...");

		if ((nBytesReceived = Receive(nClientSocket, (char**)&pszReceiveBuffer)) > 0) {
			LogInfo("ReceiveThread: %d bytes received.", nBytesReceived);

			nTotalBytesReceived += nBytesReceived;

			LogInfo("ReceiveThread: %d total bytes received so far.",
					nTotalBytesReceived);

			LogInfo("ReceiveThread: Processing the received text..");

			ProcessReceivedText(pszReceiveBuffer, nBytesReceived);

			LogInfo("ReceiveThread: The received text has been dealt with.");

			LogInfo("ReceiveThread: Checking whether we need to stop checking for new data...");

			if (ShouldStopReceiving(pszReceiveBuffer, nBytesReceived)) {
				LogWarning("ReceiveThread: We've been told to stop polling for more data.");

				LogDebug("ReceiveThread: Breaking receive loop...");

				break;
			}


			/* If we get to here, we have not been told to stop receiving, so
			 * keep polling. */
		} else if (errno != EWOULDBLOCK) {
			perror("ReceiveThread");

			LogError("ReceiveThread: Problem encountered in receive thread.");

			LogDebug("ReceiveThread: Stopping receive loop.");

			break;
		}

		LogInfo("ReceiveThread: We have NOT been told to stop polling.  Going to next receive loop iteration...");
	}

	LogDebug("ReceiveThread: Done.");

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
