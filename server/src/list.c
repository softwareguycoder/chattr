// list.c - defines the implementation of a linked list (which we will utilize to keep track of the clients)
// Each time a new client connects, just add it to the linked list
//

#include "stdafx.h"
#include "utils.h"

#include "list.h"
#include "root.h"
#include "position.h"

#include "clientStruct.h"

POSITION* AddHead(void* data) {

	POSITION* listHead = (POSITION*) calloc(sizeof(POSITION), 1);

	ROOT* listRoot = (ROOT*) calloc(sizeof(ROOT), 1);

	if (listHead == NULL || listRoot == NULL) {
		error("Linked list initialization failed.\n");
		return NULL;
	}

	listRoot->head = listHead;
	listRoot->tail = listHead;

	listHead->listRoot = listRoot;
	listHead->next = NULL;
	listHead->prev = NULL;

	listHead->data = data;

	return listHead;
}

int AddMember(POSITION** listHead, void* data) {

	if (listHead == NULL || (*listHead) == NULL) {
		error("Adding list member has failed.\n"
				"list head is NULL\n");
		return FALSE;
	}

	POSITION* curr = (POSITION*) calloc(sizeof(POSITION), 1);
	//list* localHead = NULL;

	curr->listRoot = (*listHead)->listRoot;
	curr->next = NULL;
	curr->prev = (*listHead)->listRoot->tail;
	curr->data = data;

	(*listHead)->listRoot->tail->next = curr;
	(*listHead)->listRoot->tail = curr;

	return 1;

}

/*
 * The compare functiopn (LPCOMPARE_ROUTINE) is user implemented.
 * It takes in two parameters:
 *    1. void* of value 1
 *    2. void* of value 2
 *
 *    The function return 1 on equals, 0 otherwise.
 *
 * find_member traverses the linked list starting at the head
 * and finds the first element of interest by using LPCOMPARE_ROUTINE.
 *
 * It takes the address of the value its trying to find a a function
 * pointer of type LPCOMPARE_ROUTINE.  It feeds its first parameter to
 * as a first arguemnt to LPCOMPARE_ROUTINE and the data field in the
 * linked list structure definition as the second argument to the
 * function pointed to by lpfnCompare.
 *
 * On success find_member returns a pointer to the list_entry of interest.
 * On failure it returns NULL.
 */

POSITION* FindMember(POSITION** pos, void* valueToFind,
		LPCOMPARE_ROUTINE lpfnCompare) {
	if (pos == NULL || (*pos) == NULL) {
		error("Finding member has failed. list head is NULL\n");
		return NULL;
	}

	if (valueToFind == NULL)
		return NULL;

	// precautionary measure
	POSITION* curr = (*pos)->listRoot->head;
	if (curr == NULL)
		return NULL;

	do {
		if (lpfnCompare(valueToFind, curr->data))
			return curr;

	} while ((curr = curr->next) != NULL);

	return NULL;
}

POSITION* GetHeadPosition(POSITION** listMember) {

	if (listMember == NULL || *listMember == NULL)
		error("GetHeadPosition: Must specify starting member.");

	return (*listMember)->listRoot->head;
}

POSITION* GetTailPosition(POSITION** listMember) {

	if (listMember == NULL || *listMember == NULL)
		error("GetTailPosition: Must specify starting member.");

	return (*listMember)->listRoot->tail;
}

// returns 1 on success
int RemoveElement(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnSearch) {

	if (listHead == NULL || (*listHead) == NULL) {
		error("Removing member has failed.\nlist head is NULL\n");
		return FALSE;
	}
	//precuationary measure
	POSITION* localHead = (*listHead)->listRoot->head;

	if (localHead == NULL)
		return FALSE;

	POSITION* member = FindMember(listHead, value, lpfnSearch);

	if (member == localHead) {
		RemoveHead(listHead);
		return 1;
	}
	if (member == localHead->listRoot->tail) {
		RemoveTail(listHead);
		return 1;
	}

	POSITION* prev = member->prev;
	POSITION* next = member->next;

	prev->next = next;
	next->prev = prev;

	(*listHead) = localHead;

	//free(member->data);
	free(member);

	return 1;
}

BOOL RemoveHead(POSITION** listHead) {

	if ((*listHead) == NULL)
		return FALSE;

	POSITION* curr = (*listHead);
	POSITION* newHead = curr->next;
	POSITION* oldHead = curr;

	if (newHead == NULL) { //head is the only element
		free(curr->listRoot);
		free(curr);
		(*listHead) = NULL;
		return 1;
	}

	newHead->prev = NULL;
	(*listHead) = newHead;
	//free(oldHead->next);
	free(oldHead);

	return 1;
}

BOOL RemoveTail(POSITION** listHead) {

	if (listHead == NULL || (*listHead) == NULL)
		return FALSE;

	POSITION* head = (*listHead)->listRoot->head;
	POSITION* oldTail = head->listRoot->tail;
	POSITION* newTail = oldTail->prev;

	head->listRoot->tail = newTail;
	newTail->next = NULL;

	(*listHead) = head;
	//free(oldTail->data);
	free(oldTail);

	return TRUE;
}

/*
 * lpfnDeallocFunc is a user defined function to perform on
 * every data member of the linked list structure
 * as the memory allocated to the linked list structure
 * is freed.
 */
void DestroyList(POSITION** listHead, LPDEALLOC_ROUTINE lpfnDeallocFunc) {

	if ((*listHead) == NULL)
		return;

	POSITION* curr = (*listHead)->listRoot->head;
	free(curr->listRoot);

	do {
		if (lpfnDeallocFunc != NULL)
			lpfnDeallocFunc(curr);

		//free(curr->data);
		free(curr);
	} while ((curr = curr->next) != NULL);

	(*listHead) = NULL;

}
