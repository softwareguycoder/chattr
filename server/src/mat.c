///////////////////////////////////////////////////////////////////////////////
// mat.c - Master Acceptor Thread for the chat server
//
// AUTHOR: Brian Hart
// CREATED DATE: 3 Feb 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for
// inspiration
//

#include "stdafx.h"
#include "server.h"

#include "client_thread_functions.h"
#include "mat.h"
#include "mat_functions.h"
#include "server_functions.h"

BOOL g_bShouldTerminateMasterThread = FALSE;

///////////////////////////////////////////////////////////////////////////////
// MasterAcceptorThread thread procedure

void* MasterAcceptorThread(void* pThreadData) {
    SetThreadCancelState(PTHREAD_CANCEL_ENABLE);
    SetThreadCancelType(PTHREAD_CANCEL_DEFERRED);

    RegisterEvent(TerminateMasterThread);

    // Extract the file descriptor of the server's TCP endpoint from
    // the user state passed to this thread.  The GetServerSocketFileDescriptor
    // function's return value is guaranteed to be valid.
    int nServerSocket = GetServerSocketFileDescriptor(pThreadData);

    // This thread procedure runs an infinite loop which runs while the server
    // socket is listening for new connections.  This thread's sole mission in
    // life is to wait for incoming client connections, accept them as they come
    // in, and then go back to waiting for more incoming client connections.

    while (1) {
        if (!IsSocketValid(nServerSocket)) {
            break;
        }

        // If we have been signaled to stop, then abort
        if (g_bShouldTerminateMasterThread) {
            return NULL;
        }

        MakeServerEndpointReusable(nServerSocket);

        // We now call the accept function.  This function holds us up
        // until a new client connection comes in, whereupon it returns
        // a file descriptor that represents the socket on our side that
        // is connected to the client.  The output of the function called
        // below is guaranteed to be valid.
        LPCLIENTSTRUCT lpCS = WaitForNewClientConnection(nServerSocket);
        if (lpCS == NULL) {
            continue;
        }

        // Add the info for the newly connected client to the list we maintain
        AddNewlyConnectedClientToList(lpCS);

        // Launch a new thread to handle the communications with this client
        LaunchNewClientThread(lpCS);

        if (IsClientCountZero()) {
            break;   // No more clients are connected
        }
    }

    fprintf(stdout, "Master thread ending.\n");

    return NULL;
}
