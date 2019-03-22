///////////////////////////////////////////////////////////////////////////////
// clientThreadManager.c - Routines to manage threads that are used to service
// communications to and from this server's clients.
//
// AUTHOR: Brian Hart
// CREATED DATE: 22 Mar 2019
// LAST UPDATED: 22 Mar 2019
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
#include "clientThread.h"
#include "list.h"
#include "clientStruct.h"
#include "utils.h"
