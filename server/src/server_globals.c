// server_globals.c - Implementations of server global-scope variables
//

#include "stdafx.h"

#include "server_globals.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables and their starting values

POSITION* g_pClientList = NULL;
HMUTEX g_hClientListMutex = INVALID_HANDLE_VALUE;
HTHREAD g_hMasterThread = INVALID_HANDLE_VALUE;

int g_nServerSocket = 0;

///////////////////////////////////////////////////////////////////////////////
