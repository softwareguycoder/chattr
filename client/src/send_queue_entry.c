/*
 * send_queue_entry.c
 *
 *  Created on: Apr 9, 2019
 *      Author: bhart
 */

#include "stdafx.h"
#include "client.h"

#include "send_queue_entry.h"

LPSENDQUEUEENTRY CreateSendQueueEntry(const char* message) {
	if (message == 0 || message[0] == '\0') {
		CleanupClient(ERROR);
	}

	/* max length of message, including newline, is MAX_MESSAGE_LEN */
	if (MAX_MESSAGE_LEN < strlen(message)) {
		fprintf(stderr, "CreateSendQueueEntry: Message must be %d characters or fewer in length.",
				MAX_MESSAGE_LEN - 1);

		CleanupClient(ERROR);
	}

	LPSENDQUEUEENTRY result = (LPSENDQUEUEENTRY)malloc(1*sizeof(SENDQUEUEENTRY));
	if (result == NULL) {
		CleanupClient(ERROR);
	}

	uuid_generate(result->id);	// give this entry a unique identifier

	// Set all the time fields in this structure to the same value to start with.
	// This way, they all have valid values.  When the message is actually queued, and then
	// delivered, at these points, the relevant fields will be updated further.
	result->createDateTime = result->deliveredDateTime = result->queuedDateTime = time(NULL);
	result->status = CREATED;

	/* put the value in the 'message' parameter into the message member of the
	 * LPSENDQUEUEENTRY structure, and truncate it at MAX_MESSAGE_LEN chars. */
	strncpy(result->message, message, MAX_MESSAGE_LEN);

	char uuid_value[37];
	uuid_unparse(result->id, uuid_value);

	log_info("CreateSendQueueEntry: Created send queue entry '%s'.", uuid_value);

	return result;
}

void FreeSendQueueEntry(void* pSendQueueEntry){
	log_debug("In FreeSendQueueEntry");

		log_info(
				"FreeSendQueueEntry: Checking whether supplied SENDQUEUEENTRY pointer is NULL...");

		if (pSendQueueEntry == NULL) {
			log_warning(
					"FreeSendQueueEntry: The send queue entry structure has already been freed.  Nothing to do.");

			log_debug("FreeSendQueueEntry: Done.");

			return;
		}

		log_info("FreeSendQueueEntry: The pSendQueueEntry pointer references a valid memory address.");

		log_info("FreeSendQueueEntry: Freeing the SENDQUEUEENTRY pointer...");

		free(pSendQueueEntry);
		pSendQueueEntry = NULL;

		log_info("FreeSendQueueEntry: The memory has been released back to the system.");

		log_debug("FreeSendQueueEntry: Done.");
}
