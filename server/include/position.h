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

POSITION* AddHead(void* data)
BOOL AddMember(POSITION** listHead, void* data);
POSITION* GetHeadPosition(POSITION** listMember);
POSITION* GetTailPosition(POSITION** listMember);
BOOL RemoveHead(POSITION** listHead);
BOOL RemoveTail(POSITION** listHead);

#endif /* INCLUDE_POSITION_H_ */
