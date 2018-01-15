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

#include "piecewise.h"

typedef struct {
	double x;
	double y;
} xy_;

struct _piecewise {
	char type;
	xy_ *xy;
	xy_ **xyPtr;
	double *m;
	int length;
	int *lengthPtr;
};

/*===========================================================================*/

static int piecewiseSetIndex(piecewise_ *r, int *index, double x0)
{
	/* Used an index stored in the structure because in most cases
	 * the data to be accessed will be close to the last data
	 * accessed, this should speed things up a bit.
	 */
	if(((*index) < 0) || ((*index) > (r->length - 1))) {
		 *index = 0;
	}
	while(((*index) >= 0) && ((*index) < (r->length - 1))) {
		if(r->xy[(*index)].x <= x0) {
			if((*index) == (r->length - 2))
				return 0;
			if(r->xy[(*index)+1].x > x0)
				return 0;
			(*index)++;
		} else {
			if(*index == 0)
				return 0;
			(*index)--;
		}
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

static int piecewiseCSplineCalcValue(piecewise_ *r, int index, double x0,
		double *y0, double *dydx0)
{
	double dx0, dy, dx, a, b, c;

	/* x0 is between r->x[index] and r->x[index + 1] */
	dx0 = x0 - r->xy[index].x;
	dy = r->xy[index + 1].y - r->xy[index].y;
	dx = r->xy[index + 1].x - r->xy[index].x;

	a = dy/dx - dx*(r->m[index + 1] + r->m[index] * 2.0) / 6.0;
	b = 0.5*r->m[index];
	c = (r->m[index + 1] - r->m[index])/(6.0 * dx);

	/* evaluate cubic polynomial */
	*y0 =  r->xy[index].y + dx0*(a + dx0*(b + dx0*c));

	/* derivative of the above equation at x0 (w.r.t x0) */
	*dydx0 = a + dx0*(2*b + 3*c*dx0);

	return 0;
}

/*---------------------------------------------------------------------------*/

static int piecewiseLinearCalcValue(piecewise_ *r, int index, double x0,
		double *y0, 	double *dydx0)
{

	*y0 = r->xy[index].y + r->m[index] * (x0 - r->xy[index].x);
	*dydx0 = r->m[index];

	return 0;
}

/*---------------------------------------------------------------------------*/

static int piecewiseCSplineInit(piecewise_ *r)
{
	double *h, *b, *u, *v, *alpha, *beta;
	int n, i;

	ReturnErrIf(r == NULL);

	n = r->length - 1;

	/* If only two points the second derivative is 0 */
	if(r->length == 2) {
		Warn("Can't do spline interpolation with only two data points. "
				"Doing linear interpolation instead.");
		r->m[0] = 0.0;
		r->m[1] = 0.0;
		return 0;
	}

	h = malloc(sizeof(double) * n);
	ReturnErrIf(h == NULL);
	b = malloc(sizeof(double) * n);
	ReturnErrIf(b == NULL);
	u = malloc(sizeof(double) * n);
	ReturnErrIf(u == NULL);
	v = malloc(sizeof(double) * n);
	ReturnErrIf(v == NULL);
	alpha = malloc(sizeof(double) * n);
	ReturnErrIf(alpha == NULL);
	beta = malloc(sizeof(double) * n);
	ReturnErrIf(beta == NULL);

	/* 2nd derivative calculation based on spline portion of GNU PlotUtils
	 * written by Robert S. Maier.
	 */

	for (i = 0; i < n; i++) {
      h[i] = r->xy[i+1].x - r->xy[i].x;
      b[i] = 6.0 * (r->xy[i+1].y - r->xy[i].y) / h[i];
    }

	for (i = 0; i < n ; i++) {
		alpha[i] = h[i];
		beta[i] = 2.0 * h[i];
	}

	u[1] = beta[0] + beta[1];
	ReturnErrIf(u[1] == 0.0, "Problem of computing spline is signular");

	v[1] = b[1] - b[0];

	for (i = 2; i < n ; i++) {
		u[i] = beta[i] + beta[i-1] - (alpha[i-1] * alpha[i-1] / u[i-1]);
		ReturnErrIf(u[i] == 0.0, "Problem of computing spline is signular");
		v[i] = b[i] - b[i - 1] - (alpha[i - 1] * v[i - 1] / u[i - 1]);
	}

	/* fill in 2nd derivative array using boundary condition of
	 * dy2/dx2 = 0.0 at both ends
	 */
	r->m[0] = 0.0;
	r->m[n] = 0.0;
	for (i = n - 1; i > 0; i--) {
		r->m[i] = (v[i] - alpha[i] * r->m[i + 1]) / u[i];
	}

	free(beta);
	free(alpha);
	free(v);
	free(u);
	free(b);
	free(h);

	return 0;
}

/*---------------------------------------------------------------------------*/

static int piecewiseLinearInit(piecewise_ *r)
{
	int i;

	ReturnErrIf(r == NULL);

	for(i = 0; i < r->length; i++) {
		if((i == (r->length - 1))) {
			r->m[i] = 0;
		} else {
			r->m[i] = (r->xy[i+1].y-r->xy[i].y) / (r->xy[i+1].x-r->xy[i].x);
		}
	}

	return 0;
}

/*===========================================================================*/

int piecewiseGetNextX(piecewise_ *r, int *index, double x0, double *x)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(x == NULL);

	ReturnErrIf(piecewiseSetIndex(r, index, x0));

	if((*index) == (r->length - 1)) {
		*x = HUGE_VAL;
	} else if((index == 0) && (r->xy[0].x > x0)) {
		*x = r->xy[0].x;
	} else {
		*x = r->xy[(*index)+1].x;
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int piecewiseCalcValue(piecewise_ *r, int *index, double x0, double *y0,
		double *dydx0)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->m == NULL)
	ReturnErrIf(y0 == NULL);
	ReturnErrIf(dydx0 == NULL);
	ReturnErrIf(dydx0 == NULL);

	ReturnErrIf(piecewiseSetIndex(r, index, x0));

	if(((*index) == 0) || ((*index) >= r->length-2)) {
		if(r->xy[0].x > x0) {
			/* if x0 is before start return first value */
			*y0 = r->xy[0].y;
			*dydx0 = 0;
			return 0;
		}
		if(r->xy[r->length-1].x < x0) {
			/* if x0 is after end return last value */
			*y0 = r->xy[r->length-1].y;
			*dydx0 = 0;
			return 0;
		}
	}

	switch(r->type) {
	case 'c':
		ReturnErrIf(piecewiseCSplineCalcValue(r, (*index), x0, y0, dydx0));
		break;
	case 'l':
		ReturnErrIf(piecewiseLinearCalcValue(r, (*index), x0, y0, dydx0));
		break;
	default:
		ReturnErr("Unsupported piece-wise type, must be c or l, not %c",
				r->type);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int piecewiseInitialize(piecewise_ *r)
{
	int i;

	ReturnErrIf(r == NULL);

	if(r->m != NULL) {
		free(r->m);
	}

	r->xy = *r->xyPtr;
	r->length = *r->lengthPtr;

	for(i = 0; i < (r->length-1); i++) {
		ReturnErrIf(r->xy[i].x > r->xy[i+1].x, "%e > %e",
				r->xy[i].x, r->xy[i+1].x);
	}

	r->m = malloc(r->length * sizeof(double));
	ReturnErrIf(r->m == NULL);

	if(r->type == 'c') {
		ReturnErrIf(piecewiseCSplineInit(r));
	} else if(r->type == 'l') {
		ReturnErrIf(piecewiseLinearInit(r));
	} else {
		ReturnErr("Unsupported piece-wise type, must be c or l, not %c",
				r->type);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int piecewiseDestroy(piecewise_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying PW %p", (*r));

	if((*r)->m != NULL) {
		free((*r)->m);
	}

	free(*r);
	*r = NULL;

	return 0;
}

/*---------------------------------------------------------------------------*/

piecewise_ * piecewiseNew(piecewise_ *r, double **xy, int *length, char type)
{
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(xy == NULL);
	ReturnNULLIf(length == NULL);
	ReturnNULLIf((type != 'c') && (type != 'l'));

	r = calloc(1, sizeof(piecewise_));
	ReturnNULLIf(r == NULL);

	Debug("Creating PW %p", r);

	r->type = type;
	r->xyPtr = (xy_**)xy;
	r->lengthPtr = length;
	r->xy = *r->xyPtr;
	r->length = *r->lengthPtr;

	return r;
}

/*===========================================================================*/

