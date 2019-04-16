// position.h - Defines the interface to the POSITION data structure.
//

#ifndef __POSITION_H__
#define __POSITION_H__

#ifndef ADD_ELEMENT_HEAD_NULL
#define ADD_ELEMENT_HEAD_NULL \
    "Adding list member has failed.\nlist head is NULL\n"
#endif //ADD_ELEMENT_HEAD_NULL
/**
 * @brief Error message displayed when the allocation of the head of the linked
 * list has failed.
 */
#ifndef FAILED_ALLOC_HEAD
#define FAILED_ALLOC_HEAD \
    "Failed to allocate memory for list head node.\n"
#endif //FAILED_ALLOC_HEAD

#ifndef FAILED_ALLOC_NEW_NODE
#define FAILED_ALLOC_NEW_NODE \
    "Failed to allocate memory for a new linked list node.\n"
#endif //FAILED_ALLOC_NEW_NODE

/**
 * @brief Error message displayed when the allocation of the root of the list
 * has failed.
 */
#ifndef FAILED_ALLOC_ROOT
#define FAILED_ALLOC_ROOT \
    "Failed to allocate memory for list head node.\n"
#endif //FAILED_ALLOC_ROOT

/**
 * @brief Error message that is displayed when a function is given a NULL
 * pointer for its pvData parameter.
 */
#ifndef INVALID_LIST_DATA
#define INVALID_LIST_DATA \
    "The pointer for the data to add to the linked list is an invalid value.\n"
#endif //INVALID_LIST_DATA

/**
 * @brief Defines a node of the linked list and also serves to locate the
 * node.
 */
typedef struct _tagPOSITION {
    ROOT* pListRoot;

    struct _tagPOSITION* pPrev;
    struct _tagPOSITION* pNext;

    void* pvData;
} POSITION;

/**
 * @brief Adds a new element to the head of the linked list.
 * @param pvData Address of the data to place in the new linked list node.
 * @returns Address of a POSITION structure that locates where the new
 * node is in the list.
 * @remarks Changes the list to make sure the new node is at the head.
 */
POSITION* AddHead(void* pvData);

/**
 * @brief Adds a new member to the linked list.
 * @param ppListHead Address of the address of a POSITION structure that
 * denotes the location of the list's head.
 * @param pvData Address of the data to place in the new linked list node.
 * @returns TRUE if the add operation succeeded; FALSE otherwise.
 */
BOOL AddTail(POSITION** ppListHead, void* pvData);

/**
 * @brief Gets the address of a POSITION structure that references the head
 * of the linked list.
 * @param ppMember Address of the POSITION structure that references any
 * existing element of the list.
 * @returns Address of a POSITION structure that references the head of the
 * list.
 * @remarks This function returns NULL if the ppMember parameter is NULL or
 * the head could not be located, maybe because the linked list is currently
 * empty.
 */
POSITION* GetHeadPosition(POSITION** ppMember);

/**
 * @brief Gets the address of a POSITION structure that references the tail
 * of the linked list.
 * @param ppMember Address of the POSITION structure that references any
 * existing element of the list.
 * @returns Address of a POSITION structure that references the tail of the
 * list.
 * @remarks This function returns NULL if the ppMember parameter is NULL or
 * the tail could not be located, maybe because the linked list is currently
 * empty.
 */
POSITION* GetTailPosition(POSITION** ppMember);

/**
 * @brief Removes the element at the head of the linked list.
 * @param ppListHead Address of the POSITION structure that references the head
 * element of the list.
 * @returns TRUE if the remove operation succeeded; FALSE otherwise.
 * @remarks Returns FALSE if the ppListHead value is NULL or if the operation
 * failed.
 */
BOOL RemoveHead(POSITION** ppListHead);

/**
 * @brief Removes the element at the tail of the linked list.
 * @param ppListHead Address of the POSITION structure that references the head
 * element of the list.
 * @returns TRUE if the remove operation succeeded; FALSE otherwise.
 * @remarks Returns FALSE if the ppListHead value is NULL or if the operation
 * failed.
 */
BOOL RemoveTail(POSITION** ppListHead);

#endif /* __POSITION_H__ */
