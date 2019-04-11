/*
 * stdafx.h
 *
 *  Created on: Oct 8, 2018
 *      Author: bhart
 */

#ifndef __STDAFX_H__
#define __STDAFX_H__

#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

// Bringing in libraries defined by us
#include <../../inetsock_core/inetsock_core/include/inetsock_core.h>
#include <../../conversion_core/conversion_core/include/conversion_core.h>
#include <../../debug_core/debug_core/include/debug_core.h>
#include <../../../threading_core/threading_core/include/threading_core.h>
#include <../../../mutex_core/mutex_core/include/mutex_core.h>
#include <../../../mutex_core/mutex_core/include/interlocked_operations.h>
#include <../../common_core/common_core/include/common_core.h>

#endif /* __STDAFX_H__ */
