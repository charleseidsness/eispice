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

#ifndef DATA_H
#define DATA_H

#define DATA_MAJOR_VERSION		2
#define DATA_MINOR_VERSION		0

int dataInfo(void);

/*============================================================================
 |                                   List                                     |
  ============================================================================*/


typedef struct _listNode listNode_;

int listNodeGetPrevious(listNode_ *r, void **data);
int listNodeGetNext(listNode_ *r, void **data);
int listNodeGetData(listNode_ *r, void **data);

typedef struct _list list_;

int listGetFirstNode(list_ *r, listNode_ **node);
int listGetLastNode(list_ *r, listNode_ **node);

typedef int (*listExecute_)(void *data, void *private);
int listExecute(list_ *r, listExecute_ f, void *private);

int listGetFirst(list_ *r, void **data);
int listGetLast(list_ *r, void **data);

typedef enum {
	LIST_FIND_ERR = -1,	/* Error */
	LIST_FIND_MATCH,		/* Data matches key, return data */
	LIST_FIND_NOTAMATCH	/* Data doesn't match key, keep looking */
} listFindReturn_;
typedef listFindReturn_ (*listFind_)(void *data, void *key);
listFindReturn_ listStringCompare(void *data, void *key);
int listFind(list_ *r,  void *key, listFind_ f, void **data);

typedef enum {
	LIST_SEARCH_ERR = -1,	/* Error */
	LIST_SEARCH_MATCH,		/* Data matches key, return data */
	LIST_SEARCH_PREVIOUS,		/* Data doesn't match key, move backward */
	LIST_SEARCH_NEXT,		/* Data doesn't match key, move forward */
	LIST_SEARCH_NOTONLIST		/* Data not on list, return NULL */
} listSearchReturn_;
typedef listSearchReturn_ (*listSearch_)(void *data, void *dataNext,
		void *key);
int listSearch(list_ *r, void *key, listSearch_ f, listNode_ **node,
		void **data);

typedef enum {
	LIST_ADD_ERR = -1,	/* Error */
	LIST_ADD_BEFORE,		/* Attach before the node presented */
	LIST_ADD_AFTER,		/* Attach after the node presented */
	LIST_ADD_NOWHERE,	/* Stop searching, just don't attach */
	LIST_ADD_NOTHERE		/* Don't attach yet, keep searching */
} listAddReturn_;
typedef listAddReturn_ (*listAdd_)(void *data, void *new);
int listAdd(list_ *r, void *data, listAdd_ f);

int listLength(list_ *r);

int listFreeData(void *data);
typedef int (*listDestroy_)(void *data);

int listClear(list_ *r, listDestroy_ f);
int listDestroy(list_ **r, listDestroy_ f);

list_ * listNew(list_ *r);

/*============================================================================
 |                                  Hash                                      |
  ============================================================================*/

typedef struct _hash hash_;

typedef int (*hashExecute_)(char *key, void *record, void *private);
int hashExecute(hash_ *r, hashExecute_ f, void *private);

int hashFind(hash_ *h, char *key, void **record);
int hashFindPointer(hash_ *h, char *key, void ***record);

int hashAddPointer(hash_ *h, char *key, void *record, void **recordPtr);
int hashAdd(hash_ *h, char *key, void *record);

int hashLength(hash_ *h, unsigned int *length);

int hashRemove(hash_ *h, char *key, void **record);

int hashFreeKeyAndRecord(char *key, void *record);
typedef int (*hashDestroy_)(char *key, void *record);
int hashDestroy(hash_ **h, hashDestroy_ f);

hash_ * hashNew(hash_ *h, unsigned int capacity);

/*============================================================================
 |                               Double Hash                                  |
  ============================================================================*/

typedef struct _dblhash dblhash_;

int dblhashFindPtr(dblhash_ *h, char *key, double **value);
int dblhashFind(dblhash_ *h, char *key, double *value);

int dblhashAddPtr(dblhash_ *h, char *key, char freeMem, double value,
		double **valuePtr);
int dblhashAdd(dblhash_ *h, char *key, char freeMem, double value);

int dblhashRemove(dblhash_ *h, char *key);

int dblhashLength(dblhash_ *h, unsigned int *length);

int dblhashDestroy(dblhash_ **h);
dblhash_ * dblhashNew(dblhash_ *h, unsigned int capacity);

#endif
