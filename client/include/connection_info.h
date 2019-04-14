/*
 * connection_info.h
 *
 *  Created on: Apr 14, 2019
 *      Author: bhart
 */

#ifndef __CONNECTION_INFO_H__
#define __CONNECTION_INFO_H__

#ifndef FAILED_ALLOCATE_CONNECTIONINFO
#define FAILED_ALLOCATE_CONNECTIONINFO \
    "chattr: Out of memory.\n"
#endif //FAILED_ALLOCATE_CONNECTIONINFO

#ifndef HOSTNAME_LENGTH_INVALID
#define HOSTNAME_LENGTH_INVALID \
    "chattr: Hostname may not be longer than 255 characters.\n"
#endif //HOSTNAME_LENGTH_INVALID

#ifndef FAILED_RESOLVE_HOSTNAME
#define FAILED_RESOLVE_HOSTNAME \
    "chattr: Failed to validate hostname in DNS.\n"
#endif //FAILED_RESOLVE_HOSTNAME

#ifndef HOSTNAME_VALUE_REQUIRED
#define HOSTNAME_VALUE_REQUIRED \
    "chattr: A value for the hostname is required.\n"
#endif //HOSTNAME_VALUE_REQUIRED

#ifndef MAX_HOSTNAME_LENGTH
#define MAX_HOSTNAME_LENGTH     255
#endif //MAX_HOSTNAME_LENGTH

/**
 * @brief Encapsulates information about a connection to a server.
 */
typedef struct tagCONNECTIONINFO {
    char szHostname[MAX_HOSTNAME_LENGTH + 1];
    int nPort;
} CONNECTIONINFO, *LPCONNECTIONINFO;

/**
 * @brief Creates an instance of a CONNECTIONINFO structure and initializes
 * its members with the given values.
 * @param pszHostname Pointer to a character buffer containing the hostname
 * to connect to.
 * @param nPort Integer value specifying the port number to use.
 * @return Address of memory containing the new instance of the CONNECTIONINFO
 * structure, or NULL if an error or validation failure occurred.
 */
LPCONNECTIONINFO CreateConnectionInfo(const char* pszHostname, int nPort);

/**
 * @brief Removes an instance of a CONNECTIONINFO structure from memory.
 * @param lpConnectionInfo Address of the CONNECTIONINFO structure instance
 * to be freed.
 */
void FreeConnectionInfo(LPCONNECTIONINFO lpConnectionInfo);

/**
 * @brief Determines whether the connection parameters supplied have valid
 * values.
 * @param pszHostname Hostname of the destination server (or IP address).
 * @param nPort Port number of the server to which you want to connect.
 * @return TRUE if the information meets validation criteria; FALSE otherwise.
 */
BOOL IsConnectionInfoValid(const char* pszHostname, int nPort);

#endif /* __CONNECTION_INFO_H__ */
