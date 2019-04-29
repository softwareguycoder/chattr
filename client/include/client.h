#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "stdafx.h"

#include "client_symbols.h"

/**
 * @brief Flag indicating whether we are connected to a server.
 */
extern BOOL g_bConnected;

/**
 * @brief Socket file descriptor for the connection to the server.
 */
extern int g_nClientSocket;

/**
 * @brief Holds the current value of the client's nickname.
 */
extern char g_szNickname[MAX_NICKNAME_LEN + 1];

#endif//__CLIENT_H__
