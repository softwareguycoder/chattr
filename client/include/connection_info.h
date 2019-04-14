/*
 * connection_info.h
 *
 *  Created on: Apr 14, 2019
 *      Author: bhart
 */

#ifndef __CONNECTION_INFO_H__
#define __CONNECTION_INFO_H__

#ifndef MAX_HOSTNAME_LENGTH
#define MAX_HOSTNAME_LENGTH     255
#endif //MAX_HOSTNAME_LENGTH

typedef struct tagCONNECTIONINFO {
    char szHostname[MAX_HOSTNAME_LENGTH + 1];
    int nPort;
} CONNECTIONINFO, *LPCONNECTIONINFO;

LPCONNECTIONINFO CreateConnectionInfo(const char* pszHostname, int nPort);

#endif /* __CONNECTION_INFO_H__ */
