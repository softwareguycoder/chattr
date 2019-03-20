/*
 * utils.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

// Mutex for interlocked decrement and increment of ints
extern HMUTEX hInterlockMutex;

/* thread-safe way to increment and decrement integers */
void InterlockedIncrement(int* pn);
void InterlockedDecrement(int* pn);

int min(int a, int b);

/**
 * @brief Checks to see whether one string begins with another.
 * @param str String to be examined.
 * @param startsWith The prefix to be checked.
 * @returns TRUE if the string in str begins with the string in startsWith.
 */
BOOL StartsWith(const char *str, const char *startsWith);

#endif /* INCLUDE_UTILS_H_ */
