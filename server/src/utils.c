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
