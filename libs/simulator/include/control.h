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

#ifndef CONTROL_H
#define CONTROL_H

typedef enum {
	CONTROL_LU_SUPERLU,
} controlLULibrary_;

typedef enum {
	CONTROL_NIMETHOD_TRAP,		/* Trapazoidal */
	CONTROL_NIMETHOD_GEAR,		/* Gear */
} controlNIMethod_;

typedef struct {
/*-- Old Spice Options --*/
	int itl1;
	int itl4;
	double reltol;
	double vntol;
	double captol;
	double abstol;
	double chgtol;
	double trtol;
	double minstep; /* same as  minbreak in Old Spice */
	double gmin;
	int maxorder;
/*-- New eispice Options --*/
	controlLULibrary_ luLibrary;
	double maxAngleA;
	double maxAngleV;
/*-- Transient Analysis State --*/
	double tstop;
	double tstep;
	int integratorOrder;
	double time;
} control_;

#define ControlIntegratorOrderUp(cntrl) \
		((cntrl->integratorOrder < cntrl->maxorder) ? \
		cntrl->integratorOrder + 1 : cntrl->integratorOrder)
		
#define ControlIntegratorOrderDown(cntrl) \
		((cntrl->integratorOrder > 1) ? \
		cntrl->integratorOrder - 1 : cntrl->integratorOrder)

int controlDestroy(control_ **r);
control_ * controlNew(control_ *r);

#endif
