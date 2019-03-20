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
	char* pszNickname;
	int sockFD;
	HTHREAD hClientThread;	/* handle to the thread this client is chatting on */
	int bytesReceived;
	int bytesSent;
	BOOL bConnected;	/* is this client connected? */
} CLIENTSTRUCT, *LPCLIENTSTRUCT;

LPCLIENTSTRUCT CreateClientStruct(int nClientSocket, const char* pszClientIPAddress);

#endif /* INCLUDE_CLIENTSTRUCT_H_ */
