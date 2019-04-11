/*
 * server_functions.h
 *
 *  Created on: Apr 11, 2019
 *      Author: bhart
 */

#ifndef __SERVER_FUNCTIONS_H__
#define __SERVER_FUNCTIONS_H__

BOOL CheckCommandLineArgs(int argc, char *argv[]);

/**
 * @brief Frees resources consumed by the server and exits the application with the specified code.
 * @param nExitCode Exit code to supply to the operating system when this program is terminated.
 */
void CleanupServer(int nExitCode);
void ConfigureLogFile();
void CreateClientListMutex();
void DestroyClientListMutex();
BOOL InitializeApplication();
void InstallSigintHandler();
void PrintSoftwareTitleAndCopyright();
void QuitServer();
void ServerCleanupHandler(int signum);
struct sockaddr_in* SetUpServerOnPort(const char* pszPortNum);

#endif /* __SERVER_FUNCTIONS_H__ */
