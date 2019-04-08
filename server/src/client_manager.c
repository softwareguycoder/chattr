/*
 * client_manager.h
 *
 *  Created on: Apr 8, 2019
 *      Author: bhart
 */


void ForciblyDisconnectClient(LPCLIENTSTRUCT lpCurrentClientStruct) {
	log_debug("In ForciblyDisconnectClient");

	log_info(
			"ForciblyDisconnectClient: Checking whether lpCurrentClientStruct has a valid reference...");

	if (lpCurrentClientStruct == NULL) {
		log_error(
				"ForciblyDisconnectClient: The required parameter is not supplied.  Nothing to do.");

		log_debug("ForciblyDisconnectClient: Done.");

		return;
	}

	log_info(
			"ForciblyDisconnectClient: lpCurrentClientStruct parameter has a valid value.");

	/* Forcibly close client connections */

	log_info(
			"ForciblyDisconnectClient: Sending the termination reply string...");

	Send(lpCurrentClientStruct->sockFD,
			"503 Server forcibly terminated connection.\n");

	log_info(
			"ForciblyDisconnectClient: Client notified that we will be terminating the connection.");

	log_info(
			"ForciblyDisconnectClient: Calling SocketDemoUtils_close on the clent's socket...");

	CloseSocket(lpCurrentClientStruct->sockFD);

	log_info("ForciblyDisconnectClient: Client socket closed.");

	log_info("%s: <disconnected>", lpCurrentClientStruct->ipAddr);

	if (get_error_log_file_handle() != stdout) {
		fprintf(stdout, "%s: <disconnected>\n", lpCurrentClientStruct->ipAddr);
	}

	log_info(
			"ForciblyDisconnectClient: Decrementing the count of connected clients...");

	log_debug("ForciblyDisconnectClient: client_count = %d", client_count);

	InterlockedDecrement(&client_count);

	log_info(
			"ForciblyDisconnectClient: Count of connected clients has been decremented.");

	log_debug("ForciblyDisconnectClient: client_count = %d", client_count);

	log_debug("ForciblyDisconnectClient: Done.");
}
