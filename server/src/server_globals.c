// server_globals.c - Implementations of server global-scope variables
//

#include "stdafx.h"

#include "server_globals.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables and their starting values

BOOL g_bDiagnosticMode = FALSE;
POSITION* g_pClientList = NULL;
HMUTEX g_hClientListMutex = INVALID_HANDLE_VALUE;
HTHREAD g_hMasterThread = INVALID_HANDLE_VALUE;
int g_nServerPort = 9000;
int g_nServerSocket = 0;

///////////////////////////////////////////////////////////////////////////////
// GetClientListMutex function

HMUTEX GetClientListMutex() {
	return g_hClientListMutex;
}

///////////////////////////////////////////////////////////////////////////////
// GetServerPort function

int GetServerPort() {
	return g_nServerPort;
}

///////////////////////////////////////////////////////////////////////////////
// IsDiagnosticMode function

BOOL IsDiagnosticMode() {
	return g_bDiagnosticMode;
}

///////////////////////////////////////////////////////////////////////////////
// SetClientListMutex function

void SetClientListMutex(HMUTEX value) {
	g_hClientListMutex = value;
}

///////////////////////////////////////////////////////////////////////////////
// SetDiagnosticMode function

void SetDiagnosticMode(BOOL value) {
	g_bDiagnosticMode = value;
}

///////////////////////////////////////////////////////////////////////////////
// SetServerPort function

void SetServerPort(int value) {
	g_nServerPort = value;
}
