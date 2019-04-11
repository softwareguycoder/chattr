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

#include "stdafx.h"
#include "server.h"

#include "mat.h"
#include "client_list.h"
#include "client_thread.h"
#include "client_thread_manager.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables

BOOL g_bShouldTerminateClientThread = FALSE;

///////////////////////////////////////////////////////////////////////////////
// Client thread management routines

void LaunchNewClientThread(LPCLIENTSTRUCT lpCS) {
	if (lpCS == NULL) {
		fprintf(stderr, FAILED_LAUNCH_CLIENT_THREAD);

		CleanupServer(ERROR);
	}

	HTHREAD hClientThread = CreateThreadEx(ClientThread, lpCS);

	if (INVALID_HANDLE_VALUE == hClientThread) {
		fprintf(stderr, FAILED_LAUNCH_CLIENT_THREAD);

		CleanupServer(ERROR);
	}

	// Save the handle to the newly-created thread in the CLIENTSTRUCT instance.
	lpCS->hClientThread = hClientThread;
}

void TerminateClientThread(int signum) {
	// If signum is not equal to SIGSEGV, then ignore this semaphore
	if (SIGSEGV != signum) {
		return;
	}

	g_bShouldTerminateClientThread = TRUE;

	/* Re-associate this function with the signal */
	RegisterEvent(TerminateClientThread);
}


///////////////////////////////////////////////////////////////////////////////
