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
#include <float.h>
#include <log.h>

#include "complex.h"

/*===========================================================================*/

complex_ complexSqrt(complex_ z)
{
	complex_ c;
	double x, y, w, r;
	if ((z.r == 0.0) && (z.i == 0.0)) {
		c.i = 0.0;
		c.r = 0.0;
	} else {
		x = fabs(z.r);
		y = fabs(z.i);
		if (x >= y) {
			r = y/x;
			w = sqrt(x)*sqrt(0.5*(1.0+sqrt(1.0+r*r)));
		} else {
			r=x/y;
			w = sqrt(y)*sqrt(0.5*(r+sqrt(1.0+r*r)));
		}
		if (z.r >= 0.0) {
			c.r = w;
			c.i = z.i/(2.0*w);
		} else {
			c.i = (z.i >= 0) ? w : -w;
			c.r = z.i/(2.0*c.i);
		}
	}
	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexExp(complex_ z)
{
	complex_ c;
	c.r = exp(z.r)*cos(z.i);
	c.i = exp(z.r)*sin(z.i);
	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexMult(complex_ a, complex_ b)
{
	complex_ c;
	c.r = a.r*b.r-a.i*b.i;
	c.i = a.i*b.r+a.r*b.i;
	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexAdd(complex_ a, complex_ b)
{
	complex_ c;
	c.r = a.r + b.r;
	c.i = a.i + b.i;
	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexSub(complex_ a, complex_ b)
{
	complex_ c;
	c.r = a.r - b.r;
	c.i = a.i - b.i;
	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexConj(complex_ a)
{
	complex_ c;
	c.r = a.r;
	c.i = -a.i;
	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexDiv(complex_ a, complex_ b)
{
	complex_ c;

	/* Deal with divide by 0 */
	if((b.r == 0) && (b.i == 0)) {
		b.r = DBL_MIN;
	}

	c.r = ((a.r*b.r) + (a.i*b.i)) / ((b.r*b.r) + (b.i*b.i));
	c.i = ((a.i*b.r) - (a.r*b.i)) / ((b.r*b.r) + (b.i*b.i));

	return c;
}

/*---------------------------------------------------------------------------*/

complex_ complexDivInt(complex_ a, int b)
{
	complex_ c;

	/* Deal with divide by 0 */
	if(b != 0) {
		c.r = a.r / b;
		c.i = a.i / b;
	} else {
		c.r = a.r / DBL_MIN;
		c.i = a.r / DBL_MIN;
	}

	return c;
}

/*===========================================================================*/

