#ifndef __CLIENT_LIST_H__
#define __CLIENT_LIST_H__

#include <../../common_core/common_core/include/common_core.h>

#include "root.h"
#include "position.h"

/**
 * @brief Global variable containing the count of elements in the list.
 * @remarks Always use a mutex when accessing this variable in a multi-
 * threaded application.
 */
extern int g_nClientCount;

/**
 * @brief Callback to be used to compare two elements of the linked list.
 * @param pKey Address of a memory location that holds the search key.
 * @param pData Address of the current element in the list.
 * @returns TRUE if the current element of the list has information matching
 * the search key; FALSE otherwise.
 * @remarks Use this as the function signature for a comparison routine whose
 * address is to be provided to the FindElement function.
 */
typedef BOOL (*LPCOMPARE_ROUTINE)(void*, void*);

/**
 * @brief Callback to be used to deallocate the storage occupied by the
 * current element of the list.
 * @param pData Pointer to the memory storage occupied by the current list
 * element.
 * @remarks Provide the address of a function matching the signature of this
 * routine to the DestroyList function.  The function referenced will be called
 * once for each element in the list. */
typedef void (*LPDEALLOC_ROUTINE)(void*);

/**
 * @brief Callback that does an action for the current item in the list.
 * @param pData Address of the current element in the list.
 * @remarks Use this as the function signature for a function whose address
 * is to be provided to the ForEach function that will specify the actions
 * to be executed for each element in the list.
 */
typedef void (*LPACTION_ROUTINE)(void*);

/**
 * @brief Searches the list for an element containing data that matches the
 * search key, and then returns the address of the POSITION structure that
 * addresses the element's position.
 * @param ppListHead Address of a pointer to the head of the list. This
 * value may be updated by the function.
 * @param pSearchKey Address of the value to be used to match against data
 * in the linked list by the comparer routine.
 * @parm lpfnCompare Address of a function having the signature specified
 * by the LPCOMPARE_ROUTINE type. Functions should match the data in the
 * search key to data in the linked list.
 * @returns Address of a POSITION that locates the found item in the list
 * or NULL if either (a) the item was not found, or (b) a problem occurred.
 * @remarks Iterates through the list starting at the head, calling the
 * function pointed to by lpfnCompare for each one.  Stops when the element
 * is reached for which the function returns TRUE, or the end of the list has
 * been reached.
 */
POSITION* FindElement(POSITION** ppListHead, void* pSearchKey,
        LPCOMPARE_ROUTINE lpfnCompare);
BOOL RemoveElement(POSITION** ppListHead, void* pSearchKey,
        LPCOMPARE_ROUTINE lpfnSearch);
void DestroyList(POSITION** ppListHead,
    LPDEALLOC_ROUTINE lpfnDeallocFunc);

/**
 * @brief Executes an action for each member of a non-empty list.
 * @param listHead Reference to the head node of the list.
 * @param lpfnForEachRoutine Reference to the function to be executed for each
 * element.  This function is passed a reference to the element.
 */
void ForEach(POSITION** ppListHead, LPACTION_ROUTINE lpfnForEachRoutine);

POSITION* GetNext(POSITION* pos);

#endif /* __CLIENT_LIST_H__*/
