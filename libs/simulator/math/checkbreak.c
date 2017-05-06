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
#include "checkbreak.h"

#define N	3
#define MaxAbs(x,y) ((fabs(x) > fabs(y)) ? fabs(x) : fabs(y))

struct _checkbreak {
	control_ *control;
	char units;		/* units of x, 'A' (amps) or 'V' (volts) */
	double maxAngle;	/* maximum change in slope */
	double x[N];	/* x history storage */
	double t[N];	/* time history storage */
	double theta[N];	/* angle history storage */
	int n;			/* current time slot */
};

/*===========================================================================*/

int checkbreakIsBreak(checkbreak_ *r, double x)
{
	ReturnErrIf(r == NULL);
	
	/* Only step time when time is increasing, the simulator can search back
	 * in time for a more appropiate point (i.e. based on next step above)
	 */
	if(r->control->time > r->t[r->n%N]) {
		r->n++;		
	}
	
	/* update x, t, and angle */
	r->x[r->n%N] = x;
	r->t[r->n%N] = r->control->time;
	r->theta[r->n%N] = atan2(r->x[r->n%N] - r->x[(r->n-1)%N], 
			r->t[r->n%N] - r->t[(r->n-1)%N]);
	
	if(fabs(r->theta[r->n%N] - r->theta[(r->n-1)%N]) > r->maxAngle) {
		Debug("Break at %e with angles %e,%e", 
				r->t[r->n%N], r->theta[r->n%N], r->theta[(r->n-1)%N]);
		return 1;
	}	
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int checkbreakInitialize(checkbreak_ *r, double ic)
{
	int i;
	
	ReturnErrIf(r == NULL);
	
	/* Make a local copy, this allows the control value to
		be changed between runs */
	if(r->units == 'V') {
		r->maxAngle = r->control->maxAngleV;
	} else if(r->units == 'A') {
		r->maxAngle = r->control->maxAngleA;
	} else {
		ReturnErr("Unsupported unit type %c, should be 'A' or 'V'", r->units);
	}
	
	for(i = 0; i < N; i++) {
		r->x[i] = ic;
		r->t[i] = 0.0;
	}
	
	r->n = 0;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int checkbreakDestroy(checkbreak_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying Break Check %p", *r);
	
	free(*r);
	*r = NULL;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

checkbreak_ * checkbreakNew(checkbreak_ *r, control_ *control, char units)
{
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(control == NULL);
	ReturnNULLIf((units != 'A') && (units != 'V'));
	
	r = calloc(1, sizeof(checkbreak_));
	ReturnNULLIf(r == NULL, "Malloc Failed");
	
	Debug("Creating Break Check %p", r);
	
	r->units = units;
	r->control = control;
	
	ReturnNULLAndFreeIf(r, checkbreakInitialize(r, 0.0))		
	
	return r;
}

/*===========================================================================*/

