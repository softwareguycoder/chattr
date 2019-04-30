/*
 * client_functions.h
 *
 *  Created on: Apr 11, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_FUNCTIONS_H__
#define __CLIENT_FUNCTIONS_H__

#include "connection_info.h"

void ConfigureLogFile();
void ConnectToChatServer(LPCONNECTIONINFO lpConnectionInfo);
void CleanupClient(int nExitCode);
void ClearNickname();
void ClientCleanupHandler(int signum);
void CreateReceiveThread();
void CreateSendThread();
void FormatLogFileName(char* pszBuffer);
BOOL HasChatSessionBegun();
BOOL InitializeApplication();
void InstallSigintHandler();
BOOL IsCommandLineArgumentCountValid(int argc);
void ParseCommandLine(char* argv[], char** ppszHostname, int* pnPort);
int ParsePortNumber(const char* pszPort);
void PrintSoftwareTitleAndCopyright();

#endif /* __CLIENT_FUNCTIONS_H__ */
