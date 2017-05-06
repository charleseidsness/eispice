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

#include "control.h"
#include "checklinear.h"

#define MaxAbs(x,y) ((fabs(x) > fabs(y)) ? fabs(x) : fabs(y))

struct _checklinear {
	char state; /* a = not linear, b = one linear step, c = linear */
	control_ *control;
	char units;
	double lastV;
};

/*===========================================================================*/

int checklinearIsLinear(checklinear_ *r, double V, double calcedV)
{
	double e;
	ReturnErrIf(r == NULL);
	
	/* Error of Previous Value vs. Present Value */
	if(r->units == 'V') {
		e = r->control->reltol * MaxAbs(r->lastV, V) + r->control->vntol;
	} else if(r->units == 'A') {
		e = r->control->reltol * MaxAbs(r->lastV, V) + r->control->abstol;
	} else if(r->units == 'F') {
		e = r->control->reltol * MaxAbs(r->lastV, V) + r->control->captol;
	} else {
		ReturnErr("Unsupported units type");
	}
	/* If error is greater than value changed that it's not linear */
	if(fabs(r->lastV - V) > e) {
		r->state = 'a';
		r->lastV = V;
		return 0;
	}
	
	/* Error of Calculated Value vs. Present Value */
	if(r->units == 'V') {
		e = r->control->reltol * MaxAbs(calcedV, V) + r->control->vntol;
	} else if(r->units == 'A') {
		e = r->control->reltol * MaxAbs(calcedV, V) + r->control->abstol;
	} else if(r->units == 'F') {
		e = r->control->reltol * MaxAbs(calcedV, V) + r->control->captol;
	} else {
		ReturnErr("Unsupported units type");
	}
	/* If error is greater than value changed that it's not linear */
	if(fabs(calcedV - V) > e) {
		r->state = 'a';
		r->lastV = V;
		return 0;
	}
	
	r->lastV = V;
	
	/* Need to have two passing steps in a row to declare it's linear */
	if(r->state == 'b') {
		r->state = 'c';
		return 1;
	} else if(r->state == 'c'){
		return 1;
	} else {
		r->state = 'b';
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int checklinearInitialize(checklinear_ *r, double ic)
{
	ReturnErrIf(r == NULL);
	r->state = 'a';
	r->lastV = ic;
	return 0;
}

/*---------------------------------------------------------------------------*/

int checklinearDestroy(checklinear_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying Linear Check %p", *r);
	
	free(*r);
	*r = NULL;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

checklinear_ * checklinearNew(checklinear_ *r, control_ *control, char units)
{
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(control == NULL);
	ReturnNULLIf((units != 'A') && (units != 'V') && (units != 'F'));
	
	r = malloc(sizeof(checklinear_));
	ReturnNULLIf(r == NULL, "Malloc Failed");
	
	Debug("Creating Linear Check %p", r);
	
	r->units = units;
	r->control = control;
	r->state = 'a';
	r->lastV = 0.0;
	
	return r;
}

/*===========================================================================*/

