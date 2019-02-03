/*
 * stdafx.h
 *
 *  Created on: Oct 8, 2018
 *      Author: bhart
 */

#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifndef IPADDRLEN
#define IPADDRLEN   20
#endif //IPADDRLEN

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

#include <../../inetsock_core/inetsock_core/include/inetsock_core.h>
#include <../../conversion_core/conversion_core/include/conversion_core.h>
#include <../../debug_core/debug_core/include/debug_core.h>


#endif /* __STDAFX_H__ */
