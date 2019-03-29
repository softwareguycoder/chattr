///////////////////////////////////////////////////////////////////////////////
// clientThreadManager.c - Routines to manage threads that are used to service
// communications to and from this server's clients.
//
// AUTHOR: Brian Hart
// CREATED DATE: 22 Mar 2019
// LAST UPDATED: 22 Mr 2019
//
// Shout-out to <https://gist.githubusercontent.com/suyash/2488ff6996c98a8ee3a8
// 4fe3198a6f85/raw/ba06e891060b277867a6f9c7c2afa20da431ec91/server.c> and
// <http://www.linuxhowtos.org/C_C++/socket.htm> for
// inspiration
//

/*
 * clientThreadManager.c
 *
 *  Created on: 22 Mar 2019
 *      Author: bhart
 */

#include <client_thread.h>
#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "clientThreadManager.h"
#include "list.h"
#include "utils.h"

///////////////////////////////////////////////////////////////////////////////
// Client thread management routines

void LaunchNewClientThread(LPCLIENTSTRUCT lpClientData) {
	log_debug("In LaunchNewClientThread");

	log_info(
			"LaunchNewClientThread: Checking whether the 'lpClientData' has a NULL reference...");

	if (lpClientData == NULL) {

		log_error(
				"LaunchNewClientThread: Required parameter 'lpClientData' has a NULL reference.  Stopping.");

		log_debug("LaunchNewClientThread: Done.");

		exit(ERROR);
	}

	log_info(
			"LaunchNewClientThread: The 'lpClientData' parameter has a valid reference.");

	log_info(
			"LaunchNewClientThread: Creating client thread to handle communications with that client...");

	HTHREAD hClientThread = CreateThreadEx(ClientThread, lpClientData);

	if (INVALID_HANDLE_VALUE == hClientThread) {
		log_error("Failed to create new client communication thread.");

		log_debug("LaunchNewClientThread: Done.");

		exit(ERROR);
	}

	// Save the handle to the newly-created thread in the CLIENTSTRUCT instance.
	lpClientData->hClientThread = hClientThread;

	log_info("LaunchNewClientThread: Successfully created new client thread.");

	log_debug("LaunchNewClientThread: Done.");
}

///////////////////////////////////////////////////////////////////////////////
