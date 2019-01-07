#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "stdafx.h"

#define OK              0		// The server completed successfully
#define ERROR           -1		// The server encountered an error

#define FALSE           0
#define TRUE            1

#define MIN_NUM_ARGS	3		// The minimum # of cmd line args to pass
#define MAX_LINE_LENGTH 255     // The maximum length of a line

/* key string constants */
#define SOFTWARE_TITLE  	"TCP echo client v1.0\n"
#define COPYRIGHT_MESSAGE	"Copyright (c) 2018 by Brian Hart.\n\n"
#define USAGE_STRING	"Usage: client <host name or IP> <port_num>\n" 	// Usage string

#define RECV_BLOCK_SIZE	1
#define RECV_FLAGS	0

#endif//__CLIENT_H__
