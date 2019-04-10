/*
 * receive_thread.c
 *
 *  Created on: Apr 10, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "client_manager.h"

HTHREAD g_hReceiveThread;

void *ReceiveThread(void *pvData) {
	int nTotalBytesReceived = 0;
	while(1) {
		char *pszReceiveBuffer = NULL;
		int nBytesReceived = 0;

		if ((nBytesReceived = Receive(nClientSocket, &pszReceiveBuffer)) > 0) {
			nTotalBytesReceived += nBytesReceived;
			ProcessReceivedText(pszReceiveBuffer, nBytesReceived);

			if (ShouldStopReceiving(pszReceiveBuffer, nBytesReceived)) {
				break;
			}
		} else if (errno != EWOULDBLOCK) {
			break;
		}
	}

	return NULL;
}
