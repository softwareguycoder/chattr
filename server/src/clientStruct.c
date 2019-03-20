/*
 * clientStruct.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "utils.h"

#include "clientStruct.h"

LPCLIENTSTRUCT CreateClientStruct(int nClientSocket, const char* pszClientIPAddress) {

	if (pszClientIPAddress == NULL
			|| strlen(pszClientIPAddress) == 0)
		return NULL;

	LPCLIENTSTRUCT lpClientStruct = (LPCLIENTSTRUCT)calloc(sizeof(CLIENTSTRUCT), 1);

	memset(lpClientStruct, 0, sizeof(CLIENTSTRUCT));

	lpClientStruct->sockFD = nClientSocket;

	memcpy(lpClientStruct->ipAddr, pszClientIPAddress,
			min(strlen(pszClientIPAddress), IPADDRLEN));

	lpClientStruct->bConnected = TRUE;

	return lpClientStruct;

}
