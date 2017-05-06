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

#include <math.h>
#include <log.h>

#include "row.h"
#include "history.h"
#include "history_interp.h"

#define MaxAbs(x,y) ((fabs(x) > fabs(y)) ? fabs(x) : fabs(y))

struct _historyInterp {
	list_ *history;
	listNode_ *historyNode; /* This is used to store the history of the last
						   	history search, to speed up the next one */
	history_ *prevData;
	history_ *nextData;
	double time;
};

/*===========================================================================*/

int historyInterpSetTime(historyInterp_ *r, double time)
{
	ReturnErrIf(r == NULL);
	r->time = time;
	
	/* first get the point before the time */
	ReturnErrIf(listSearch(r->history, &r->time, (listSearch_)historySearch, 
			&r->historyNode, (void*)&r->prevData));
	ReturnErrIf(r->historyNode == NULL);
	
	/* first get the point after the time */
	ReturnErrIf(listNodeGetNext(r->historyNode, (void*)&r->nextData));
	
	/* If we're at the end of the history list use previous two
	 * points for interpolation
	 */
	if(r->nextData == NULL) {
		r->nextData = r->prevData;
		ReturnErrIf(listNodeGetPrevious(r->historyNode, (void*)&r->prevData));
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int historyInterpGetData(historyInterp_ *r, int index, double *data)
{
	double tP, tN, xP, xN;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->prevData == NULL);
	ReturnErrIf(r->nextData == NULL);
	ReturnErrIf(data == NULL);
	ReturnErrIf(index < 0);
	
	if(index != rowGetIndex(&gndRow)) {
		ReturnErrIf(historyGetTime(r->prevData, &tP));
		ReturnErrIf(historyGetTime(r->nextData, &tN));
		
		ReturnErrIf(historyGetData(r->prevData, &xP, index-1));
		ReturnErrIf(historyGetData(r->nextData, &xN, index-1));
		
		/* Linear Interpolation */
		*data = ( (xN-xP) / (tN-tP) ) * (r->time - tP) + xP;
	} else {
		*data = 0;
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int historyInterpInitialize(historyInterp_ *r)
{
	ReturnErrIf(r == NULL);
	r->historyNode = NULL;
	r->prevData = NULL;
	r->nextData = NULL;
	r->time = 0.0;
	return 0;
}

/*---------------------------------------------------------------------------*/

int historyInterpDestroy(historyInterp_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying History Interpolation %p", *r);
	
	free(*r);
	*r = NULL;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

historyInterp_ * historyInterpNew(historyInterp_ *r, list_ *history)
{
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(history == NULL);
	
	r = malloc(sizeof(historyInterp_));
	ReturnNULLIf(r == NULL, "Malloc Failed");
	
	Debug("Creating History Interpolation %p", r);
	
	r->history = history;
	r->historyNode = NULL;
	r->prevData = NULL;
	r->nextData = NULL;
	r->time = 0.0;
	
	return r;
}

/*===========================================================================*/

