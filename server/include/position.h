/*
 * position.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __POSITION_H__
#define __POSITION_H__

typedef struct _tagPOSITION {
	ROOT* pListRoot;

	struct _tagPOSITION* pPrev;
	struct _tagPOSITION* pNext;

	void* pvData;
} POSITION;

POSITION* AddHead(void* pvData);
BOOL AddMember(POSITION** ppListHead, void* pvData);
POSITION* GetHeadPosition(POSITION** ppMember);
POSITION* GetTailPosition(POSITION** ppMember);
BOOL RemoveHead(POSITION** ppListHead);
BOOL RemoveTail(POSITION** ppListHead);

#endif /* __POSITION_H__ */
