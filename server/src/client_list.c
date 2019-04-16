// client_list.c - defines the implementation of a linked list (which we will utilize to keep track of the clients)
// Each time a new client connects, just add it to the linked list
//

#include "stdafx.h"

#include "client_list.h"
#include "client_struct.h"
#include "root.h"
#include "position.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables

int g_nClientCount = 0;

///////////////////////////////////////////////////////////////////////////////
// AddHead function

POSITION* AddHead(void* pvData) {

	POSITION* pListHead = (POSITION*) calloc(sizeof(POSITION), 1);
	if (pListHead == NULL) {
	    HandleError(FAILED_ALLOC_HEAD);
	    return NULL;
	}

	ROOT* pListRoot = (ROOT*) calloc(sizeof(ROOT), 1);
	if (pListRoot == NULL) {
		HandleError(FAILED_ALLOC_ROOT);
		return NULL;
	}

	pListRoot->pHead = pListHead;
	pListRoot->pTail = pListHead;

	pListHead->pListRoot = pListRoot;
	pListHead->pNext = NULL;
	pListHead->pPrev = NULL;

	pListHead->pvData = pvData;

	return pListHead;
}

BOOL AddElement(POSITION** listHead, void* data) {
	if (listHead == NULL || (*listHead) == NULL) {
		HandleError("Adding list member has failed.\nlist head is NULL\n");
		return FALSE;
	}

	POSITION* curr = (POSITION*) calloc(sizeof(POSITION), 1);
	//list* localHead = NULL;

	curr->pListRoot = (*listHead)->pListRoot;
	curr->pNext = NULL;
	curr->pPrev = (*listHead)->pListRoot->pTail;
	curr->pvData = data;

	(*listHead)->pListRoot->pTail->pNext = curr;
	(*listHead)->pListRoot->pTail = curr;

	return TRUE;
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

POSITION* FindElement(POSITION** pos, void* valueToFind,
		LPCOMPARE_ROUTINE lpfnCompare) {
	if (pos == NULL || (*pos) == NULL) {
		HandleError("Finding member has failed. list head is NULL\n");
		return NULL;
	}

	if (valueToFind == NULL)
		return NULL;

	// precautionary measure
	POSITION* curr = (*pos)->pListRoot->pHead;
	if (curr == NULL)
		return NULL;

	do {
		if (lpfnCompare(valueToFind, curr->pvData))
			return curr;

	} while ((curr = curr->pNext) != NULL);

	return NULL;
}

POSITION* GetHeadPosition(POSITION** listMember) {

	if (listMember == NULL || *listMember == NULL)
		HandleError("GetHeadPosition: Must specify starting member.");

	return (*listMember)->pListRoot->pHead;
}

POSITION* GetTailPosition(POSITION** listMember) {

	if (listMember == NULL || *listMember == NULL)
		HandleError("GetTailPosition: Must specify starting member.");

	return (*listMember)->pListRoot->pTail;
}

// returns 1 on success
int RemoveElement(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnSearch) {

	if (listHead == NULL || (*listHead) == NULL) {
		HandleError("Removing member has failed.\nlist head is NULL\n");
		return FALSE;
	}

	//precuationary measure
	POSITION* localHead = (*listHead)->pListRoot->pHead;

	if (localHead == NULL)
		return FALSE;

	POSITION* member = FindElement(listHead, value, lpfnSearch);
	if (member == NULL) {
	    return FALSE;
	}

	if (member == localHead) {
		RemoveHead(listHead);
		return TRUE;
	}
	if (member == localHead->pListRoot->pTail) {
		RemoveTail(listHead);
		return TRUE;
	}

	POSITION* prev = member->pPrev;
	POSITION* next = member->pNext;

	prev->pNext = next;
	next->pPrev = prev;

	(*listHead) = localHead;

	//free(member->data);
	free(member);
	member = NULL;

	return 1;
}

BOOL RemoveHead(POSITION** listHead) {

	if ((*listHead) == NULL)
		return FALSE;

	POSITION* curr = (*listHead);
	POSITION* newHead = curr->pNext;
	POSITION* oldHead = curr;

	if (newHead == NULL) { //head is the only element
		free(curr->pListRoot);
		free(curr);
		(*listHead) = NULL;
		return TRUE;
	}

	newHead->pPrev = NULL;
	(*listHead) = newHead;
	//free(oldHead->next);
	free(oldHead);

	return 1;
}

BOOL RemoveTail(POSITION** listHead) {

	if (listHead == NULL || (*listHead) == NULL)
		return FALSE;

	POSITION* head = (*listHead)->pListRoot->pHead;
	POSITION* oldTail = head->pListRoot->pTail;
	POSITION* newTail = oldTail->pPrev;

	head->pListRoot->pTail = newTail;
	newTail->pNext = NULL;

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

	POSITION* curr = (*listHead)->pListRoot->pHead;
	free(curr->pListRoot);

	do {
		if (lpfnDeallocFunc != NULL)
			lpfnDeallocFunc(curr->pvData);

		//free(curr->data);
		free(curr);
	} while ((curr = GetNext(curr)) != NULL);

	(*listHead) = NULL;

}

void ForEach(POSITION** listHead, LPACTION_ROUTINE lpfnForEachRoutine) {
	if ((*listHead) == NULL)
		return;

	POSITION* curr = (*listHead)->pListRoot->pHead;
	free(curr->pListRoot);

	do {
		if (lpfnForEachRoutine != NULL)
			lpfnForEachRoutine(curr->pvData);
	} while ((curr = GetNext(curr)) != NULL);

	(*listHead) = NULL;
}

POSITION* GetNext(POSITION* pos) {
	if (pos == NULL)
		return NULL;

	return pos->pNext;
}
