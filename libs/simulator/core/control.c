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

/*===========================================================================
 |                          Constructor / Destructor                         |
  ===========================================================================*/

int controlDestroy(control_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(*r == NULL);
	Debug("Destroying Control %p", *r);
	
	free(*r);
	*r = NULL;
	return 0;
}

/*---------------------------------------------------------------------------*/

control_ * controlNew(control_ *r)
{
	ReturnNULLIf(r != NULL);
	r = malloc(sizeof(control_));
	ReturnNULLIf(r == NULL);
	
	Debug("Creating Control %p", r);
	
	/*-- Old Spice Options --*/
	r->itl1 = 100;
	r->itl4 = 10;
	r->reltol = 0.001;
	r->vntol = 1e-6;
	r->abstol = 1e-12;
	r->captol = 1e-18;
	r->chgtol = 1e-14;
	r->trtol = 1;
	r->minstep = -1.0;
	r->gmin = 1e-15;
	r->maxorder = 2;
	
	/*-- New eispice Options --*/
	r->luLibrary = CONTROL_LU_SUPERLU;
	r->maxAngleA = M_PI/3;
	r->maxAngleV = M_PI/3;
	
	/*-- Transient Analysis State --*/
	r->tstop = 0.0;
	r->tstep = 0.0;
	r->integratorOrder = 1;
	r->time = 0.0;
	
	return r;
}

/*===========================================================================*/

