#ifndef __CLIENT_LIST_H__
#define __CLIENT_LIST_H__

#include <../../common_core/common_core/include/common_core.h>

#include "root.h"
#include "position.h"

extern int g_nClientCount;

typedef BOOL (*LPCOMPARE_ROUTINE)(void*, void*);
typedef void (*LPDEALLOC_ROUTINE)(void*);

/* Callback that is called for everyone in the list */
typedef void (*LPACTION_ROUTINE)(void*);

POSITION* FindMember(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnCompare);
int RemoveElement(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnSearch);
void DestroyList(POSITION** listHead, LPDEALLOC_ROUTINE lpfnDeallocFunc);
void ForEach(POSITION** listHead, LPACTION_ROUTINE lpfnForEachRoutine);
POSITION* GetNext(POSITION* pos);

#endif /* __CLIENT_LIST_H__*/
