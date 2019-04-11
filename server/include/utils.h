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

/**
 * @brief Tells which of the two integer values passed is the smaller of the two.
 * @param a The first integer value to be checked.
 * @param b The second integer value to be checked.
 * @returns If a < b, then a is returned. If a = b, a is returned.  If b < a, then b is returned.
 * @remarks This function compares two values and returns the value which is the smaller
 * of the two.  If they are equal, then both are returned.
 */
int min(int a, int b);

#endif /* __UTILS_H__ */
