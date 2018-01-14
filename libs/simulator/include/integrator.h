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

#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "control.h"

typedef struct _integrator integrator_;

int integratorNextStep(integrator_ *r, double x0, double *h);
int integratorIntegrate(integrator_ *r, double x0, double *dydx0, double *y0);
int integratorIntegrateFunction(integrator_ *r, double x0, double *dydx0,
		double *y0);

int integratorInitialize(integrator_ *r, double ic, double *ydtdx);

int integratorDestroy(integrator_ **r);
integrator_ * integratorNew(integrator_ *r, control_ *control, double *ydtdx,
		char units);

#endif
