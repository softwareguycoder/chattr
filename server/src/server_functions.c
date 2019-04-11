// server_functions.c - File to contain the various application-specific
// functions used by the server's main function
//

#include "stdafx.h"
#include "server.h"

#include "server_functions.h"

///////////////////////////////////////////////////////////////////////////////
// CleanupServer function

void CleanupServer(int nExitCode) {
	LogDebug("In CleanupServer");

	LogDebug("CleanupServer: exitCode = %d", nExitCode);

	// Handle the case where the user presses CTRL+C in the terminal
	// by performing an orderly shut down of the server and freeing
	// operating system resources.

	LogInfo("CleanupServer: Obtaining a lock on the client list mutex...");

	LockMutex(hClientListMutex);
	{
		LogInfo("CleanupServer: Lock established.");

		LogInfo(
				"CleanupServer: Checking whether the count of connected clients is greater than zero...");

		if (nClientCount > 0) {
			LogInfo(
					"CleanupServer: The count of connected clients is greater than zero.");

			LogInfo("CleanupServer: Forcibly disconnecting each client...");

			ForEach(&clientList, DisconnectClient);

			LogInfo("CleanupServer: Disconnection operation completed.");
		} else {
			LogInfo(
					"CleanupServer: Zero clients are currently connected.  Nothing to do.");
		}

		LogInfo("CleanupServer: Releasing the client list lock...");
	}
	UnlockMutex(hClientListMutex);

	LogInfo("CleanupServer: Client list lock released.");

	LogInfo("CleanupServer: Calling QuitServer...");

	QuitServer();

	LogInfo("CleanupServer: Finished calling QuitServer.");

	LogInfo("CleanupServer: Closing the log file...");

	CloseLogFileHandles();

	/* beyond this point, we cannot utlize the log_* functions */

	exit(nExitCode);	// terminate program
}

