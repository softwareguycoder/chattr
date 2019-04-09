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
#include <netdb.h>


// Note: you need to do a apt-get install uuid-dev to build this code!
#include <uuid/uuid.h>

#include <../../inetsock_core/inetsock_core/include/inetsock_core.h>
#include <../../conversion_core/conversion_core/include/conversion_core.h>
#include <../../debug_core/debug_core/include/debug_core.h>
#include <../../console_core/console_core/include/console_core.h>
#include <../../../threading_core/threading_core/include/threading_core.h>

#endif//__STDAFX_H__
