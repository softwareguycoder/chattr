/*
 * client_functions.h
 *
 *  Created on: Apr 11, 2019
 *      Author: bhart
 */

#ifndef __CLIENT_FUNCTIONS_H__
#define __CLIENT_FUNCTIONS_H__

void CleanupClient(int nExitCode);
void ClientCleanupHandler(int signum);
void ConfigureLogFile();
void FormatLogFileName(const char* pszBuffer);
BOOL InitializeApplication();
void InstallSigintHandler();
BOOL IsCommandLineArgumentCountValid(int argc);
int ParsePortNumber(const char* pszPort);

#endif /* __CLIENT_FUNCTIONS_H__ */
