/*
 * send_queue_entry.c
 *
 *  Created on: Apr 9, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "send_queue_entry.h"

LPSENDQUEUEENTRY CreateSendQueueEntry(const char* pszMessage) {
	LogDebug("In CreateSendQueueEntry");

	LogInfo(
			"CreateSendQueueEntry: Checking whether the message passed is NULL or blank...");

	if (pszMessage == NULL || pszMessage[0] == '\0') {
		LogError("CreateSendQueueEntry: Message passed was blank.  Stopping.");

		LogDebug("CreateSendQueueEntry: Done.");

		CleanupClient(ERROR);
	}

	LogInfo("CreateSendQueueEntry: The message passed is non-blank.");

	LogInfo(
			"CreateSendQueueEntry: Checking whether the message is longer than %d chars...",
			MAX_MESSAGE_LEN - 1);

	/* max length of message, including newline, is MAX_MESSAGE_LEN */
	if (strlen(pszMessage) > MAX_MESSAGE_LEN) {
		LogError(
				"CreateSendQueueEntry: Message must be %d characters or fewer in length.");

		if (stderr != GetErrorLogFileHandle()) {
			fprintf(stderr,
					"CreateSendQueueEntry: Message must be %d characters or fewer in length.",
					MAX_MESSAGE_LEN - 1);
		}

		LogDebug("CreateSendQueueEntry: Done.");

		CleanupClient(ERROR);
	}

	LogInfo("CreateSendQueueEntry: The message is of a valid length.");

	LogInfo(
			"CreateSendQueueEntry: Allocating memory for a new SENDQUEUEENTRY structure instance...");

	LPSENDQUEUEENTRY lpNewSendQueueEntry = (LPSENDQUEUEENTRY) malloc(
			1 * sizeof(SENDQUEUEENTRY));
	if (lpNewSendQueueEntry == NULL) {
		LogError(
				"CreateSendQueueEntry: Failed to allocate sufficient memory for a new SENDQUEUEENTRY instance...");

		LogDebug("CreateSendQueueEntry: Done.");

		CleanupClient(ERROR);
	}

	LogInfo(
			"CreateSendQueueEntry: Successfully allocated memory for a new SENDQUEUEENTRY structure instance.");

	LogInfo(
			"CreateSendQueueEntry: Initializing the new SENDQUEUEENTRY structure instance with message information...");

	uuid_generate(lpNewSendQueueEntry->sendQueueEntryID);	// give this entry a universally-unique identifier so we can find it later

	// Set all the time fields in this structure to the same value to start with.
	// This way, they all have valid values.  When the message is actually queued, and then
	// delivered, at these points, the relevant fields will be updated further.
	lpNewSendQueueEntry->createDateTime = lpNewSendQueueEntry->deliveredDateTime =
			lpNewSendQueueEntry->queuedDateTime = time(NULL);
	lpNewSendQueueEntry->status = CREATED;

	/* put the value in the 'message' parameter into the message member of the
	 * LPSENDQUEUEENTRY structure, and truncate it at MAX_MESSAGE_LEN chars. */
	strncpy(lpNewSendQueueEntry->message, pszMessage, MAX_MESSAGE_LEN);

	char szSendQueueEntryID[37];
	uuid_unparse(lpNewSendQueueEntry->sendQueueEntryID, szSendQueueEntryID);

	LogInfo("CreateSendQueueEntry: Created send queue entry '%s'.", szSendQueueEntryID);

	LogDebug("CreateSendQueueEntry: Done.");

	return lpNewSendQueueEntry;
}

void FreeSendQueueEntry(void* pSendQueueEntry) {
	LogDebug("In FreeSendQueueEntry");

	LogInfo(
			"FreeSendQueueEntry: Checking whether supplied SENDQUEUEENTRY pointer is NULL...");

	if (pSendQueueEntry == NULL) {
		LogWarning(
				"FreeSendQueueEntry: The send queue entry structure has already been freed.  Nothing to do.");

		LogDebug("FreeSendQueueEntry: Done.");

		return;
	}

	LogInfo(
			"FreeSendQueueEntry: The pSendQueueEntry pointer references a valid memory address.");

	LogInfo("FreeSendQueueEntry: Freeing the SENDQUEUEENTRY pointer...");

	free(pSendQueueEntry);
	pSendQueueEntry = NULL;

	LogInfo(
			"FreeSendQueueEntry: The memory has been released back to the system.");

	LogDebug("FreeSendQueueEntry: Done.");
}
