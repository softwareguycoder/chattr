/*
 * position.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef INCLUDE_POSITION_H_
#define INCLUDE_POSITION_H_

typedef struct _tagPOSITION {
	ROOT* listRoot;

	struct _tagPOSITION* prev;
	struct _tagPOSITION* next;

	void* data;
} POSITION;

#endif /* INCLUDE_POSITION_H_ */
