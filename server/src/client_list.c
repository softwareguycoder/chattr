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
    if (pvData == NULL) {
        HandleError(INVALID_LIST_DATA);
    }

    /* Create a new node to serve as the new head. */
	POSITION* pListHead = (POSITION*) calloc(1, sizeof(POSITION));
	if (pListHead == NULL) {
	    HandleError(FAILED_ALLOC_HEAD);
	}

	/* Create a new root structure to bear information about the
	 * head and tail. */
	ROOT* pListRoot = (ROOT*) calloc(1, sizeof(ROOT));
	if (pListRoot == NULL) {
		HandleError(FAILED_ALLOC_ROOT);
	}

	/* Set the root structure to point to the newly-created node as
	 * both the head and the tail (since there is just one node in the
	 * list right now). */
	pListRoot->pHead = pListHead;
	pListRoot->pTail = pListHead;

	/* Set the pListRoot of the newly-created head element to reference
	 * back to the root. */
	pListHead->pListRoot = pListRoot;

	/* Since this is the first element, there is nothing for the pNext and
	 * pPrev pointers to reference right now. */
	pListHead->pNext = NULL;
	pListHead->pPrev = NULL;

	/* Initialize the pvData member of the new node with the data
	 * pointer passed to this function. */
	pListHead->pvData = pvData;

	/* Return the address of the newly-created node. */
	return pListHead;
}

///////////////////////////////////////////////////////////////////////////////
// AddTail function

BOOL AddTail(POSITION** ppListHead, void* pvData) {
    if (pvData == NULL){
        HandleError(INVALID_LIST_DATA);
    }

	if (ppListHead == NULL || (*ppListHead) == NULL) {
		HandleError(ADD_ELEMENT_HEAD_NULL);
	}

	POSITION* pNewTail = (POSITION*) calloc(1, sizeof(POSITION));
	if (pNewTail == NULL) {
	    HandleError(FAILED_ALLOC_NEW_NODE);
	}

	/* Set the pListRoot member of the new node to point at
	 * the address referenced by the list head */
	pNewTail->pListRoot = (*ppListHead)->pListRoot;

	/* Set the pNext pointer of the new node to NULL since this node
	 * is going on the tail of the list. */
	pNewTail->pNext = NULL;

	/* Set the pPrev pointer of the new node to the address of the current
	 * list tail since this new node is now the tail. */
	pNewTail->pPrev = (*ppListHead)->pListRoot->pTail;

	/* Set the new node to reference the data passed to this function. */
	pNewTail->pvData = pvData;

	/* Set the (current) tail of the linked list to point at this
	 * node as the next element after it. */
	(*ppListHead)->pListRoot->pTail->pNext = pNewTail;

	/* Set the newly-created node to be the new tail of the list. */
	(*ppListHead)->pListRoot->pTail = pNewTail;

	/* Return success. */
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

POSITION* FindElement(POSITION** ppListHead, void* pSearchKey,
		LPCOMPARE_ROUTINE lpfnCompare) {
	if (ppListHead == NULL || (*ppListHead) == NULL) {
		HandleError(FAILED_SEARCH_NULL_HEAD);
	}

	if (pSearchKey == NULL) {
		HandleError(FAILED_SEARCH_NULL_KEY);
	}

	if (lpfnCompare == NULL) {
	    HandleError(FAILED_SEARCH_NULL_COMPARER);
	}

	// precautionary measure -- double-check that the list
	// root element's head has been initialized
	POSITION* pos = (*ppListHead)->pListRoot->pHead;
	if (pos == NULL){
	    HandleError(FAILED_SEARCH_NULL_HEAD);
	}

	// Iterate through the list elements and run the compare
	// routine for each.  Once the compare routine returns
	// TRUE, stop and return a pointer to the current element.
	do {
		if (lpfnCompare(pSearchKey, pos->pvData))
			return pos;

	} while ((pos = pos->pNext) != NULL);

	/* If we are here, then the desired element could not be located. */

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// GetHeadPosition function

POSITION* GetHeadPosition(POSITION** ppMember) {

	if (ppMember == NULL || *ppMember == NULL)
		HandleError("GetHeadPosition: Must specify starting member.");

	return (*ppMember)->pListRoot->pHead;
}

POSITION* GetTailPosition(POSITION** ppMember) {

	if (ppMember == NULL || *ppMember == NULL)
		HandleError("GetTailPosition: Must specify starting member.");

	return (*ppMember)->pListRoot->pTail;
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
