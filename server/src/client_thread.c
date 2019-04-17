/*
 * clientThread.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_manager.h"
#include "client_struct.h"
#include "client_thread.h"
#include "client_thread_functions.h"
#include "client_list_manager.h"

#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// ClientThread thread procedure

void *ClientThread(void* pData) {
    /* Be sure to register the termination semaphore so we can be
     * signalled to stop if necessary */
    RegisterEvent(TerminateClientThread);

    /* Valid user state data consisting of a reference to the CLIENTSTRUCT
     * instance giving information for this client must be passed. */
    LPCLIENTSTRUCT lpSendingClient = GetSendingClientInfo(pData);

    while (1) {
        /* Check whether the client's socket endpoint is valid. */
        if (!IsSocketValid(lpSendingClient->nSocket)) {
            // Nothing to do.
            break;
        }

        // Receive all the lines of text that the client wants to send,
        // and put them all into a buffer.
        char* pszData = NULL;
        int nBytesReceived = 0;

        lpSendingClient->nBytesReceived =
        ZERO_BYTES_TOTAL_RECEIVED;

        if ((nBytesReceived = ReceiveFromClient(lpSendingClient->nSocket,
                &pszData)) > 0) {

            /* Inform the server console's user how many bytes we got. */
            LogInfoToFileAndScreen(CLIENT_BYTES_RECD_FORMAT,
                    lpSendingClient->szIPAddress, lpSendingClient->nSocket,
                    nBytesReceived);

            /* Save the total bytes received from this client */
            lpSendingClient->nBytesReceived += nBytesReceived;

            /* Check if the termination semaphore has been signalled, and
             * stop this loop if so. */
            if (g_bShouldTerminateClientThread) {
                g_bShouldTerminateClientThread = FALSE;
                break;
            }

            // Log what the client sent us to the server's interactive
            // console and the log file, unless they're the same, then
            // just send the output to the console.
            LogInfoToFileAndScreen(CLIENT_DATA_FORMAT,
                    lpSendingClient->szIPAddress, lpSendingClient->nSocket,
                        pszData);

            /* first, check if we have a protocol command.  If so, skip to
             * next loop. We know if this is a protocol command rather than a
             * chat message because the HandleProtocolCommand returns a value
             * of TRUE in this case. */
            if (HandleProtocolCommand(lpSendingClient, pszData))
                continue;

            /* IF we are here, then the pszData was not found to contain a protocol-
             * required command string; rather, this is simply text.  We prepend the
             * 'chat handle' of the person who sent the message and then send it to
             * all the chatters except the person who sent the message.
             */
            PrependNicknameAndBroadcast(pszData, lpSendingClient);

            /* TODO: Add other protocol handling here */

            /* If the client has closed the connection, bConnected will
             * be FALSE.  This is our signal to stop looking for further input. */
            if (lpSendingClient->bConnected == FALSE
                    || !IsSocketValid(lpSendingClient->nSocket)) {

                // Decrement the count of connected clients
                InterlockedDecrement(&g_nClientCount);

                break;
            }
        }
    }

    // reset the termination semaphore
    if (g_bShouldTerminateClientThread) {
        g_bShouldTerminateClientThread = FALSE;
    }

    // done
    return NULL;
}
