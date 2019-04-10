/*
 * send_queue_entry.h
 *
 *  Created on: Apr 9, 2019
 *      Author: bhart
 */

#ifndef __SEND_QUEUE_ENTRY_H__
#define __SEND_QUEUE_ENTRY_H__

#include "client_symbols.h"

#include <uuid/uuid.h>

#include <../../common_core/common_core/include/common_core.h>

/**
 * @brief Message delivery status.  The values are UNKNOWN, QUEUED, and DELIVERED.
 * @remarks UNKNOWN messages are message entries that have just been created.  QUEUED
 * messages are waiting to go out.  DELIVERED messages have already been sent.  They are
 * held in the linked list so that the iteration can continue over the list indefinitely. */
typedef enum {
	CREATED,
	QUEUED,
	DELIVERED
} MESSAGE_STATUS;

/**
 * @brief Structure that contains something to be sent to the server.
 * @remarks We implement the sending of messages by queueing the lines to be sent
 * so that a 'sending thread' can constantly iterate over the list of messages
 * and pump them to the server.  The struct below wraps a single message. */
typedef struct _tagSENDQUEUEENTRY {
	uuid_t sendQueueEntryID;
	char message[MAX_MESSAGE_LEN];
	MESSAGE_STATUS status;
	time_t createDateTime;
	time_t queuedDateTime;
	time_t deliveredDateTime;
} SENDQUEUEENTRY,  *LPSENDQUEUEENTRY;

/**
 * @brief Creates an instance of a SENDQUEUEENTRY structure and fills it with the
 * message that is to be sent.
 * @param message Address of the start of the next line to be sent to the server.
 * @remarks New instances of structures start out with the status member set to UNKNOWN.
 */
LPSENDQUEUEENTRY CreateSendQueueEntry(const char* message);

/**
 * @brief Releases the memory allocated for a send queue entry structure pointer back to the system.
 * @param pSendQueueEntry Pointer to a SENDQUEUEENTRY instance whose memory is to be freed.
 */
void FreeSendQueueEntry(void* pSendQueueEntry);

#endif /* __SEND_QUEUE_ENTRY_H__ */
