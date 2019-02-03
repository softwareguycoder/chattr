#ifndef INCLUDE_LIST_H_
#define INCLUDE_LIST_H_

/* Forward declaration of POSITION node structure */
typedef struct _tagPOSITION POSITION;

/* Forward declaration of ROOT structure */
typedef struct _tagROOT ROOT;

typedef BOOL (*LPCOMPARE_ROUTINE)(void*, void*);
typedef void (*LPDEALLOC_ROUTINE)(void*);

/* Callback that is called for everyone in the list */
typedef void* (*LPACTION_ROUTINE)(void*);

POSITION* AddHead(void* data);
int AddMember(POSITION** listHead, void* data);
POSITION* FindMember(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnCompare);
POSITION* GetHeadPosition(POSITION** listMember);
POSITION* GetTailPosition(POSITION** listMember);
int RemoveElement(POSITION** listHead, void* value,
		LPCOMPARE_ROUTINE lpfnSearch);
BOOL RemoveHead(POSITION** listHead);
BOOL RemoveTail(POSITION** listHead);
void DestroyList(POSITION** listHead, LPDEALLOC_ROUTINE lpfnDeallocFunc);

#endif /* INCLUDE_LIST_H_*/
