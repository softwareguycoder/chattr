/*
 * position.h
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#ifndef __POSITION_H__
#define __POSITION_H__

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
BOOL AddElement(POSITION** ppListHead, void* pvData);

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
POSITION* GetTailPosition(POSITION** ppMember);
BOOL RemoveHead(POSITION** ppListHead);
BOOL RemoveTail(POSITION** ppListHead);

#endif /* __POSITION_H__ */
