/*
 * position.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __POSITION_H__
#define __POSITION_H__

typedef struct _tagPOSITION {
	ROOT* listRoot;

	struct _tagPOSITION* prev;
	struct _tagPOSITION* next;

	void* data;
} POSITION;

POSITION* AddHead(void* data);
BOOL AddMember(POSITION** listHead, void* data);
POSITION* GetHeadPosition(POSITION** listMember);
POSITION* GetTailPosition(POSITION** listMember);
BOOL RemoveHead(POSITION** listHead);
BOOL RemoveTail(POSITION** listHead);

#endif /* __POSITION_H__ */
