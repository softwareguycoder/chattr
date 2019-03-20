///////////////////////////////////////////////////////////////////////////////
// mat.c - Master Acceptor Thread for the chat server
// The server receives text a line at a time and echoes the text back to its
// client only AFTER an entire line has been received.
//
// AUTHOR: Brian Hart
// DATE: 20 Mar 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for
// inspiration
//

/*
 * mat.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "clientThread.h"
#include "list.h"
#include "clientStruct.h"
#include "utils.h"

HTHREAD hMasterThread;

int client_count = 0;

void* MasterAcceptorThread(void* pThreadData)
{
	if (pThreadData == NULL) {
		error("You should have passed the server socket file descriptor to the master acceptor thread!");
		return NULL;
	}

	int* pServerSocketFD = (int*)pThreadData;
	if (pServerSocketFD == NULL){
		error("You should have passed the server socket file descriptor to the master acceptor thread!");
		return NULL;
	}

	int server_socket = *pServerSocketFD;
	if (server_socket <= 0){
		error("Invalid server socket file descriptor passed to the Master Acceptor Thread.");
		return NULL;
	}

	// This thread procedure runs an infinite loop which runs while the server socket
	// is listening for new connections.  This thread's sole mission in life is to
	// wait for incoming client connections, accept them as they come in, and then
	// go back to waiting for more incoming client connections.  Each time a client
	// connection comes in, its IP address where it's coming from is read, and its IP address,
	// file descriptor, and individual thread handle are all bundled up into the
	// CLIENTSTRUCT structure which then is passed to the new thread.

	struct sockaddr_in client_address;
	int client_socket = -1;

	while(1) {

		// We now call the accept function.  This function holds us up
		// until a new client connection comes in, whereupon it returns
		// a file descriptor that represents the socket on our side that
		// is connected to the client.
		if ((client_socket = SocketDemoUtils_accept(server_socket,
				&client_address)) < 0) {

			// Failed to accept

			close(client_socket);
			client_socket = -1;

			continue;
		}

		// if we are here then we have a brand-new client connection
		LPCLIENTSTRUCT lpClientData = CreateClientStruct(
						client_socket,
						inet_ntoa(client_address.sin_addr)
		);

		lpClientData->hClientThread = CreateThreadEx(ClientThread, lpClientData);

		// ALWAYS Use a mutex to touch the linked list of clients!
		LockMutex(hClientListMutex);
		{
			if (client_count == 0) {
				clientList = AddHead(lpClientData);
				if (clientList == NULL)
					error("Failed to initialize the master list of clients.");
			} else if (clientList != NULL) {
				AddMember(&clientList, lpClientData);
			}
		}
		UnlockMutex(hClientListMutex);

		// Increment the count of connected clients
		InterlockedIncrement(&client_count);

		// Get rid of the client address information for the next accept() call
		memset(&client_address, 0, sizeof(client_address));

		if (client_count == 0)
			break;	// stop this loop when there are no more connected clients.
	}

	DestroyMutex(hInterlockMutex);

	return NULL;
}
