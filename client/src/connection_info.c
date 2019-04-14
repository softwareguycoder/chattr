// connection_info.c - Contains the definitions of functions that can be used
// to create/free instances of the CONNECTIONINFO structure and validate the
// information therein.

#include "stdafx.h"
#include "client.h"

#include "connection_info.h"

///////////////////////////////////////////////////////////////////////////////
// CreateConnectionInfo function - Creates a new instance of a CONNECTIONINFO
// structure and initializes its members
//

LPCONNECTIONINFO CreateConnectionInfo(const char* pszHostname, int nPort) {
    if (!IsConnectionInfoValid(pszHostname, nPort)) {
        exit(ERROR);
    }

    LPCONNECTIONINFO lpResult = (LPCONNECTIONINFO) malloc(
            1 * sizeof(CONNECTIONINFO));
    if (lpResult == NULL) {
        fprintf(stderr,
        FAILED_ALLOCATE_CONNECTIONINFO);
        exit(ERROR);
    }

    /* Set all structure members to value of zero */
    memset((void*) lpResult, 0, sizeof(CONNECTIONINFO));

    strncpy(lpResult->szHostname, pszHostname,
            MAX_HOSTNAME_LENGTH);

    lpResult->nPort = nPort;

    return lpResult;
}

///////////////////////////////////////////////////////////////////////////////
// FreeConnectionInfo function

void FreeConnectionInfo(LPCONNECTIONINFO lpConnectionInfo) {
    FreeBuffer((void**)&lpConnectionInfo);
}

///////////////////////////////////////////////////////////////////////////////
// IsConnectionInfoValid function

BOOL IsConnectionInfoValid(const char* pszHostname, int nPort) {
    if (IsNullOrWhiteSpace(pszHostname)) {
        fprintf(stderr, HOSTNAME_VALUE_REQUIRED);

        return FALSE;
    }

    if (strlen(pszHostname) > MAX_HOSTNAME_LENGTH) {
        fprintf(stderr, HOSTNAME_LENGTH_INVALID);

        return FALSE;
    }

    if (!IsHostnameValid(pszHostname)) {
        fprintf(stderr, FAILED_RESOLVE_HOSTNAME);

        return FALSE;
    }

    if (!IsUserPortNumberValid(nPort)) {
        fprintf(stderr, PORT_NUMBER_NOT_VALID);

        return FALSE;
    }

    return TRUE;
}
