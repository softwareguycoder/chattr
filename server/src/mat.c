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
	log_debug("In MasterAcceptorThread");

	if (pThreadData == NULL) {
		log_error("MasterAcceptorThread: You should have passed the server socket file descriptor to the master acceptor thread!");
		return NULL;
	}

	int* pServerSocketFD = (int*)pThreadData;
	if (pServerSocketFD == NULL){
		log_error("MasterAcceptorThread: You should have passed the server socket file descriptor to the master acceptor thread!");
		return NULL;
	}

	int server_socket = *pServerSocketFD;
	if (server_socket <= 0){
		log_error("MasterAcceptorThread: Invalid server socket file descriptor passed to the Master Acceptor Thread.");
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

		log_info("MasterAcceptorThread: Waiting for a new client connection...")

		// We now call the accept function.  This function holds us up
		// until a new client connection comes in, whereupon it returns
		// a file descriptor that represents the socket on our side that
		// is connected to the client.
		if ((client_socket = SocketDemoUtils_accept(server_socket,
				&client_address)) < 0) {

			log_error("MasterAcceptorThread: Error accepting new connection.");

			// Failed to accept

			close(client_socket);
			client_socket = -1;

			continue;
		}

		log_info("MasterAcceptorThread: Processing new client connection...");

		// if we are here then we have a brand-new client connection
		LPCLIENTSTRUCT lpClientData = CreateClientStruct(			log_info("MasterAcceptorThread: Checking whether count of connected clients has dropped to zero...")


						client_socket,
						inet_ntoa(client_address.sin_addr)
		);

		lpClientData->hClientThread = CreateThreadEx(ClientThread, lpClientData);

		// ALWAYS Use a mutex to touch the linked list of clients!
		LockMutex(hClientListMutex);
		{
			log_info("MasterAcceptorThread: Registering client in client list...");

			log_debug("MasterAcceptorThread: Count of registered clients is currently %d.",
					client_count);

			if (client_count == 0) {
				log_debug("MasterAcceptorThread: Adding client info to head of internal client list...");

				clientList = AddHead(lpClientData);
				if (clientList == NULL)
					log_error("MasterAcceptorThread: Failed to initialize the master list of clients.");
			} else if (clientList != NULL) {
				log_debug("MasterAcceptorThread: Adding client info to internal client list...");

				AddMember(&clientList, lpClientData);
			}
		}
		UnlockMutex(hClientListMutex);

		log_debug("MasterAcceptorThread: Attempting to increment the count of connected clients...");

		// Increment the count of connected clients
		InterlockedIncrement(&client_count);

		LockMutex(hClientListMutex);
		{
			log_debug("MasterAcceptorThread: Connected clients: %d.", client_count);
		}
		UnlockMutex(hClientListMutex);

		// Get rid of the client address information in anticipation
		// of the next accept() call
		memset(&client_address, 0, sizeof(client_address));

		LockMutex(hClientListMutex);
		{
			if (client_count == 0)
				break;	// stop this loop when there are no more connected clients.
		}
		UnlockMutex(hClientListMutex);			log_info("MasterAcceptorThread: Checking whether count of connected clients has dropped to zero...")


	}

	log_info("MasterAcceptorThread: The count of connected clients has dropped to zero.");

	DestroyMutex(hInterlockMutex);

	return NULL;
}
