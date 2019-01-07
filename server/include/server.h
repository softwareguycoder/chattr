/*
 * server.h
 *
 *  Created on: Oct 8, 2018
 *      Author: bhart
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include "stdafx.h"

#define MIN_NUM_ARGS		2		// The minimum # of cmd line args to pass
#define BACKLOG_SIZE		128		// Max number of client connections

/* key string constants */
#define SOFTWARE_TITLE  	"TCP echo server v1.0\n"
#define COPYRIGHT_MESSAGE	"Copyright (c) 2018 by Brian Hart.\n\n"
#define USAGE_STRING		"Usage: server <port_num>\n" 	// Usage string

#define RECV_BLOCK_SIZE	1
#define RECV_FLAGS	0

void cleanup_handler(int s);
void install_sigint_handler();

#endif /* __SERVER_H__ */
