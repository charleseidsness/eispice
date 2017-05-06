/* 
 * Copyright (C) 2006 Cooper Street Innovations Inc.
 *	Charles Eidsness    <charles@cooper-street.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 * 02110-1301, USA.
 *
 */

#include <log.h>

#include "data.h"

/*===========================================================================*/

struct _listNode {
	listNode_ *next;
	listNode_ *prev;
	void *data;
};

/*===========================================================================*/

static int listNodeDestroy(listNode_ **r, listDestroy_ f)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying List Node %p", *r);
	
	if(((*r)->data != NULL) && (f != NULL)) {
		ReturnErrIf(f((*r)->data));
	}
	
	free(*r);
	*r = NULL;
	
	return 0;
}

static listNode_ * listNodeNew(listNode_ *r, void *data)
{
	ReturnNULLIf(r != NULL);
	r = calloc(1, sizeof(listNode_));
	ReturnNULLIf(r == NULL);
	r->data = data;
	
	Debug("Creating List Node %p", r);
	
	return r;
}

int listNodeGetPrevious(listNode_ *r, void **data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	if(r->prev == NULL) {
		*data = NULL;
	}
	*data = r->prev->data;
	return 0;
}

int listNodeGetNext(listNode_ *r, void **data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	if(r->next == NULL) {
		*data = NULL;
	}
	*data = r->next->data;
	return 0;
}

int listNodeGetData(listNode_ *r, void **data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	*data = r->data;
	return 0;
}

/*===========================================================================*/

struct _list {
	listNode_ *head;
	listNode_ *tail;
	int numNodes;
	int lock;
};

/*===========================================================================*/

static inline int listLock(list_ *r)
{
	r->lock = -1;
	return 0;
}

static inline int listUnlock(list_ *r)
{
	r->lock = 0;
	return 0;
}

int listGetFirstNode(list_ *r, listNode_ **node)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(node == NULL);
	*node = r->head;
	return 0;
}

int listGetLastNode(list_ *r, listNode_ **node)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(node == NULL);
	*node = r->tail;
	return 0;
}

/*---------------------------------------------------------------------------*/

listFindReturn_ listStringCompare(void *data, void *key)
{
	ReturnErrIf(data == NULL);
	ReturnErrIf(key == NULL);
	
	if(!strcmp((char *)data, (char *)key)) {
		return LIST_FIND_MATCH;
	}
	
	return LIST_FIND_NOTAMATCH;
}

int listFreeData(void *data)
{
	if(data != NULL) {
		free(data);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

int listGetFirst(list_ *r, void **data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	*data = r->head->data;
	return 0;
}

int listGetLast(list_ *r, void **data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	*data = r->tail->data;
	return 0;
}

int listExecute(list_ *r, listExecute_ f, void *private)
{
	listNode_ *ptr;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(f == NULL);
	
	ReturnErrIf(listLock(r));
	for(ptr = r->head; ptr != NULL; ptr = ptr->next) {
		ReturnErrIf(f(ptr->data, private));
	}
	ReturnErrIf(listUnlock(r));
	
	return 0;
}

int listFind(list_ *r, void *key, listFind_ f, void **data)
{
	listNode_ *ptr;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(f == NULL);
	ReturnErrIf(data == NULL);
	
	*data = NULL;
	
	for(ptr = r->head; ptr != NULL; ptr = ptr->next) {
		switch(f(ptr->data, key)) {
		case LIST_FIND_ERR:
			ReturnErrIf("Error from Find Function");
		case LIST_FIND_MATCH:
			*data = ptr->data;
			return 0;
		case LIST_FIND_NOTAMATCH:
			break;
		}
	}
	
	return 0;
}

int listSearch(list_ *r, void *key, listSearch_ f, listNode_ **node,
		void **data)
{
	listNode_ *ptr, *ptrPrev = NULL, *ptrPrevPrev = NULL;
	void *dataNext;
	int prev = 0;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(f == NULL);
	ReturnErrIf(data == NULL);
	ReturnErrIf(node == NULL);
	
	*data = NULL;
	
	if(*node == NULL) {
		*node = r->head;
	}
	
	for(ptr = *node; ptr != NULL; ptr = (prev ? ptr->prev : ptr->next)) {
		/* Some checks so we don't end up in and endless loop */
		ReturnErrIf((ptr == ptrPrevPrev), "Search looped back on itself");
		ReturnErrIf((ptr == ptrPrev), "Broken List");
		ptrPrevPrev = ptrPrev;
		ptrPrev = ptr;
		dataNext = (ptr->next == NULL) ? NULL : ptr->next->data;
		switch(f(ptr->data, dataNext, key)) {
		case LIST_SEARCH_ERR:
			*node = NULL;
			ReturnErrIf("Error from Find Function");
		case LIST_SEARCH_MATCH:
			*data = ptr->data;
			*node = ptr;
			return 0;
		case LIST_SEARCH_PREVIOUS:
			prev = 1;
			break;
		case LIST_SEARCH_NEXT:
			prev = 0;
			break;
		case LIST_SEARCH_NOTONLIST:
			break;
		}
	}
	
	*node = NULL;
	
	return 0;
}

int listLength(list_ *r)
{
	ReturnErrIf(r == NULL);
	return r->numNodes;
}

int listAdd(list_ *r, void *data, listAdd_ f)
{
	listNode_ *newNode = NULL, *ptr;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	ReturnErrIf(r->lock);
	
	if(r->tail == NULL) {
		ReturnErrIf(r->head != NULL);
		newNode = listNodeNew(newNode, data);
		ReturnErrIf(newNode == NULL);
		r->head = newNode;
		r->tail = newNode;
		r->numNodes = 1;
		return 0;
	}
	
	ReturnErrIf(r->head == NULL);
	
	if(f != NULL) {
		for(ptr = r->head; ptr != NULL; ptr = ptr->next) {
			switch(f(ptr->data, data)) {
			case LIST_ADD_ERR:
				ReturnErr("Error from Attach Function");
			case LIST_ADD_BEFORE:
				newNode = listNodeNew(newNode, data);
				ReturnErrIf(newNode == NULL);
				newNode->next = ptr;
				newNode->prev = ptr->prev;
				if(newNode->prev != NULL) {
					ptr->prev->next = newNode;
				} else {
					r->head = newNode;
				}
				ptr->prev = newNode;
				r->numNodes++;
				return 0;
			case LIST_ADD_AFTER:
				newNode = listNodeNew(newNode, data);
				ReturnErrIf(newNode == NULL);
				newNode->next = ptr->next;
				newNode->prev = ptr;
				if(newNode->next != NULL) {
					ptr->next->prev = newNode;
				} else {
					r->tail = newNode;
				}
				ptr->next = newNode;
				r->numNodes++;
				return 0;
			case LIST_ADD_NOWHERE:
				ReturnErr("Not alowd to attach element");
			case LIST_ADD_NOTHERE:
				break;
			}
		}
	}
	
	newNode = listNodeNew(newNode, data);
	ReturnErrIf(newNode == NULL);
	newNode->prev = r->tail;
	r->tail->next = newNode;
	r->tail = newNode;
	r->numNodes++;
	
	return 0;
}

int listClear(list_ *r, listDestroy_ f)
{
	listNode_ *ptr, *ptrNext;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->lock);
	Debug("Clearing List %p", r);
	
	for(ptr = r->head; ptr != NULL; ptr = ptrNext) {
		ptrNext = ptr->next;
		ReturnErrIf(listNodeDestroy(&ptr,f));
		free(ptr);
	}
	
	r->head = NULL;
	r->tail = NULL;
	r->numNodes = 0;
	r->lock = 0;
	
	return 0;
}

int listDestroy(list_ **r, listDestroy_ f)
{
	listNode_ *ptr, *ptrNext;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	ReturnErrIf((*r)->lock);
	Debug("Destroying List %p", *r);
	
	for(ptr = (*r)->head; ptr != NULL; ptr = ptrNext) {
		ptrNext = ptr->next;
		ReturnErrIf(listNodeDestroy(&ptr,f));
		free(ptr);
	}
	
	free(*r);
	*r = NULL;
	
	return 0;
}

list_ * listNew(list_ *r)
{
	ReturnNULLIf(r != NULL);
	r = calloc(1, sizeof(list_));
	ReturnNULLIf(r == NULL);
	
	Debug("Creating List %p", r);
	r->numNodes = 0;
	r->lock = 0;
	
	return r;
}

