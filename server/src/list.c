// list.c - defines the implementation of a linked list (which we will utilize to keep track of the clients)
// Each time a new client connects, just add it to the linked list
//

#include "list.h"

//typedef int (* compare_func)(void*, void*);

list* initializeList(void* data) {

	list* listHead = (list*) calloc(sizeof(list), 1);

	root* listRoot = (root*) calloc(sizeof(root), 1);

	if (listHead == NULL || listRoot == NULL) {
		perror("Linked list initialization failed.\n");
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

int addMember(list** listHead, void* data) {

	if (listHead == NULL || (*listHead) == NULL) {
		perror("Adding list member has failed.\n"
				"list head is NULL\n");
		return 0;
	}

	list* curr = (list*) calloc(sizeof(list), 1);
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
 * The compare functiopn (compare_func) is user implemented.
 * It takes in two parameters:
 *    1. void* of value 1
 *    2. void* of value 2
 *
 *    The function return 1 on equals, 0 otherwise.
 *
 * find_member traverses the linked list starting at the head
 * and finds the first element of interest by using compare_func.
 *
 * It takes the address of the value its trying to find a a function
 * pointer of type compare_func.  It feeds its first parameter to
 * as a first arguemnt to compare_func and the data field in the
 * linked list structure definition as the second argument to compare_func.
 *
 * On success find_member returns a pointer to the list_entry of interest.
 * On failure it returns NULL.
 */

list* find_member(list** listHead, void* value, compare_func cmp_func) {
	if (listHead == NULL | ((*listHead) == NULL)) {
		perror("Finding member has failed.\n"
				"list head is NULL\n");
		return 0;
	}

	// precautionary measure
	list* curr = (*listHead)->listRoot->head;

	do {
		if (cmp_func(value, curr->data))
			return curr;

	} while ((curr = curr->next) != NULL);

	return NULL;
}

list* getHead(list** listMember) {

	return (*listMember)->listRoot->head;
}

list* getTail(list** listMember) {

	return (*listMember)->listRoot->tail;
}

// returns 1 on success

int removeMember(list** listHead, void* value, compare_func cmp_func) {

	if (listHead == NULL || (*listHead) == NULL) {
		perror("Removing member has failed.\nlist head is NULL\n");
		return 0;
	}
	//precuationary measure
	list* localHead = (*listHead)->listRoot->head;

	if (localHead == NULL)
		return 0;

	list* member = find_member(listHead, value, cmp_func);

	if (member == localHead) {
		removeHead(listHead);
		return 1;
	}
	if (member == localHead->listRoot->tail) {
		removeTail(listHead);
		return 1;
	}

	list* prev = member->prev;
	list* next = member->next;

	prev->next = next;
	next->prev = prev;

	(*listHead) = localHead;

	//free(member->data);
	free(member);

	return 1;
}

int removeHead(list** listHead) {

	if ((*listHead) == NULL)
		return 0;

	list* curr = (*listHead);
	list* newHead = curr->next;
	list* oldHead = curr;

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

int removeTail(list** listHead) {

	if (listHead == NULL || (*listHead) == NULL)
		return 0;

	list* head = (*listHead)->listRoot->head;
	list* oldTail = head->listRoot->tail;
	list* newTail = oldTail->prev;

	head->listRoot->tail = newTail;
	newTail->next = NULL;

	(*listHead) = head;
	//free(oldTail->data);
	free(oldTail);

	return 1;
}

/*
 * doFunc is a user defined function to perform on
 * every data member of the linked list structure
 * as the memory allocated to the linked list structure
 * is freed.
 */
void destroyList(list** listHead, doFunction func) {

	if ((*listHead) == NULL)
		return;

	list* curr = (*listHead)->listRoot->head;
	free(curr->listRoot);

	do {
		if (func != NULL)
			func(curr);

		//free(curr->data);
		free(curr);
	} while ((curr = curr->next) != NULL);

	(*listHead) = NULL;

}

int cmpFunc(void* client_socket, void* client_Structure) {
	int* client_sock = (int*) client_socket;
	clientStruct* client_Struct = (clientStruct*) client_Structure;

	if (*client_sock == client_Struct->sockFD) {
		return 1;

	}

	return 0;

}

clientStruct * createClientStruct(int client_sock, char* clientIPAddr) {

	clientStruct* clientStructPTR = calloc(sizeof(clientStruct), 1);
	clientStructPTR->sockFD = client_sock;
	memcpy(clientStructPTR->ipAddr, clientIPAddr, min(strlen(clientIPAddr),
	IPADDRLEN));

	return clientStructPTR;

}

int min(int a, int b) {

	if (a < b)
		return a;

	return b;

}
