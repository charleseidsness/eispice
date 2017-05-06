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
#include <data.h>

#include "history.h"

struct _history {
	double time;
	double *data;
	int length;
	unsigned int flag;
};

/*===========================================================================*/

listSearchReturn_ historySearch(history_ *r, history_ *next, double *time)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(time == NULL);
	
	/* At end of list */
	if((r->time <= *time) && (next == NULL)) {
		return LIST_SEARCH_MATCH;
	}
	
	/* between points */
	if((r->time <= *time) && (next->time > *time)) {
		return LIST_SEARCH_MATCH;
	}
	
	/* If not a match return which way we have to move to find a match */
	if(r->time > *time) {
		return LIST_SEARCH_PREVIOUS;
	} else if(r->time < *time) {
		return LIST_SEARCH_NEXT;
	}
	
	return LIST_SEARCH_ERR;
}

/*---------------------------------------------------------------------------*/

listSearchReturn_ historySearchBreak(history_ *r, history_ *next, double *time)
{
	ReturnErrIf(r == NULL);
	
	if(r->flag & HISTORY_FLAG_BRKPOINT) {
		return LIST_SEARCH_MATCH;
	}
	
	if(next == NULL) {
		return LIST_SEARCH_NOTONLIST;
	}
	
	return LIST_SEARCH_NEXT;
}

/*---------------------------------------------------------------------------*/

int historyRecall(history_ *r, double *data, int length)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	ReturnErrIf(length != r->length);
	memcpy(data, r->data, length*sizeof(double));
	return 0;
}

/*---------------------------------------------------------------------------*/

int historyGetTime(history_ *r, double *time)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(time == NULL);
	*time = r->time;
	return 0;
}

/*---------------------------------------------------------------------------*/

int historyGetData(history_ *r, double *data, int index)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	ReturnErrIf(index < 0);
	ReturnErrIf(index >= r->length);
	*data = r->data[index];
	return 0;
}

/*---------------------------------------------------------------------------*/

int historyGetLength(history_ *r)
{
	ReturnErrIf(r == NULL);
	return r->length;
}

/*---------------------------------------------------------------------------*/

int historyGetAllData(history_ *r, double *data, int length)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	ReturnErrIf(length != (r->length + 1));
	data[0] = r->time;
	memcpy(&data[1], r->data, r->length*sizeof(double));
	return 0;
}

/*===========================================================================
 |                          Constructor / Destructor                         |
  ===========================================================================*/

int historyDestroy(history_ *r)
{
	ReturnErrIf(r == NULL);
	
	if(r->data != NULL) {
		free(r->data);
	}
	
	free(r);
	
	return 0;
}

/*---------------------------------------------------------------------------*/

history_ * historyNew(double time, double *data, int length, 
		unsigned int flag)
{
	history_ *r;
	
	ReturnNULLIf(data == NULL);
	ReturnNULLIf(length < 0);
	
	r = malloc(sizeof(history_));
	ReturnNULLIf(r == NULL);
	
	r->time = time;
	r->length = length;
	r->flag = flag;
	r->data = malloc(length*sizeof(double));
	ReturnNULLIf(r->data == NULL);
	memcpy(r->data, data, length*sizeof(double));
	
	return r;
}

/*===========================================================================*/

