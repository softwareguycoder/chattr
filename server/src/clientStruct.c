/*
 * clientStruct.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "utils.h"

#include "clientStruct.h"

LPCLIENTSTRUCT createClientStruct(int nClientSocket, const char* pszClientIPAddress) {

	if (pszClientIPAddress == NULL || strlen(pszClientIPAddress) == 0)
		return NULL;

	CLIENTSTRUCT* clientStructPTR = calloc(sizeof(CLIENTSTRUCT), 1);
	clientStructPTR->sockFD = nClientSocket;
	memcpy(clientStructPTR->ipAddr, pszClientIPAddress,
			min(strlen(pszClientIPAddress), IPADDRLEN));

	return clientStructPTR;

}
