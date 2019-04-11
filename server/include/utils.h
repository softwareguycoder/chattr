/*
 * utils.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __UTILS_H__
#define __UTILS_H__

// Mutex for interlocked decrement and increment of ints
extern HMUTEX hInterlockMutex;

/* thread-safe way to increment and decrement integers */
void InterlockedIncrement(int* pn);
void InterlockedDecrement(int* pn);


#endif /* __UTILS_H__ */
