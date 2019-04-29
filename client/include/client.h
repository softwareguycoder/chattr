#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "stdafx.h"

#include "client_symbols.h"

extern int g_nClientSocket;

/**
 * @brief Holds the current value of the client's nickname.
 */
extern char g_szNickname[MAX_NICKNAME_LEN + 1];

#endif//__CLIENT_H__
