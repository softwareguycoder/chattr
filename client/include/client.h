#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "stdafx.h"

#include "client_symbols.h"

extern int nClientSocket;

void CleanupClient(int nExitCode);
void ClientCleanupHandler(int signum);
void InstallSigintHandler();

#endif//__CLIENT_H__
