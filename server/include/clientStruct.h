/*
 * clientStruct.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef INCLUDE_CLIENTSTRUCT_H_
#define INCLUDE_CLIENTSTRUCT_H_

#include "stdafx.h"

typedef struct _tagCLIENTSTRUCT {

	char ipAddr[IPADDRLEN];
	char * userName;
	int sockFD;

} CLIENTSTRUCT, *LPCLIENTSTRUCT;

CLIENTSTRUCT* createClientStruct(int client_sock, const char* pszClientIPAddress);

#endif /* INCLUDE_CLIENTSTRUCT_H_ */
