/*
 * utils.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include "stdafx.h"

#include "utils.h"

// Internal mutex for interlocked decrement and increment of ints
HMUTEX hInterlockMutex;

BOOL StartsWith(const char *str, const char *startsWith)
{
    size_t prefixLength = strlen(startsWith);
    size_t stringLength = strlen(str);
    return stringLength < prefixLength ? FALSE : strncmp(startsWith, str, prefixLength) == 0;
}

void InterlockedIncrement(int *pn)
{
	if (INVALID_HANDLE_VALUE == hInterlockMutex){
		hInterlockMutex = CreateMutex();
	}

	LockMutex(hInterlockMutex);

	if (pn == NULL)
		return;

	(*pn)++;

	UnlockMutex(hInterlockMutex);
}

void InterlockedDecrement(int *pn)
{
	if (INVALID_HANDLE_VALUE == hInterlockMutex){
		hInterlockMutex = CreateMutex();
	}

	LockMutex(hInterlockMutex);

	if (pn == NULL)
		return;

	(*pn)--;

	UnlockMutex(hInterlockMutex);
}

int min(int a, int b) {
	if (a < b)
		return a;

	return b;
}
