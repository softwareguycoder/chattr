/*
 * server.h
 *
 *  Created on: Oct 8, 2018
 *      Author: bhart
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include "stdafx.h"

#include "list.h"

extern POSITION* clientList;

// Lock object for the global mutex
extern HMUTEX hClientListMutex;

#define MIN_NUM_ARGS		2		// The minimum # of cmd line args to pass
#define BACKLOG_SIZE		128		// Max number of client connections

/* key string constants */
#define SOFTWARE_TITLE  	"Chattr TCP chat server v1.0\n"
#define COPYRIGHT_MESSAGE	"Copyright (c) 2018-19 by Brian Hart.\n\n"
#define USAGE_STRING		"Usage: server <port_num>\n" 	// Usage string

#define RECV_BLOCK_SIZE	1
#define RECV_FLAGS	0

BOOL FindClientBySocket(void* pClientSocketFd, void* pClientStruct);
void FreeClient(void* pClientStruct);

void ServerCleanupHandler(int s);
void install_sigint_handler();

#endif /* __SERVER_H__ */
