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
#include "integrator.h"

#define MaxAbs(x,y) ((fabs(x) > fabs(y)) ? fabs(x) : fabs(y))

#define N	4	/* Number of stored timesteps (i.e. max NI order plus 1) */

/* DD = Divided Difference */
#define DD1(xnp1, xn, hn) \
	((hn == 0.0) ? 0.0 : ((xnp1 - xn) / \
		(hn)))
#define DD2(xnp1, xn, x1, hn, h1) \
	((DD1(xnp1, xn, hn) - DD1(xn, x1, h1)) / \
		(hn + h1))
#define DD3(xnp1, xn, x1, x2, hn, h1, h2) \
	((DD2(xnp1, xn, x1, hn, h1) - \
		DD2(xn, x1, x2, h1, h2)) / \
		(hn + h1 + h2))

struct _integrator {
	control_ *control;
	char units;
	double abstol;
	double *ydtdx;	/* y*dt/dx (i.e. Capcitance or Inductance) */
	double y0;		/* last y value */
	double dydx0;	/* last slope */
	double t[N];		/* Time (seconds) */
	double h[N];		/* Delta time (seconds) */
	double x[N];		/* x-value (Volts / Amps) */
	double y[N];		/* y-value (Volts / Amps) */
	double f[N];		/* value of ydtdx at each time-point */
	int n;			/* Time-point index i.e. index = n%N */
};

/*===========================================================================*/

int integratorNextStep(integrator_ *r, double x0, double *h)
{
	double y0, ey = 0.0, eyp, e, dd;

	ReturnErrIf(r == NULL);
	ReturnErrIf(h == NULL);
	ReturnErrIf(isnan(x0));

	y0 =  r->dydx0 * x0 - r->y0;

	/* Calculate y error */
	ey = r->control->reltol * MaxAbs(r->y[r->n%N], y0) + r->abstol;

	/* Calculate y' error */
	eyp = r->control->reltol * MaxAbs(MaxAbs(x0, r->x[r->n%N]) *
			(*r->ydtdx / r->h[r->n%N]), r->control->chgtol);

	e = MaxAbs(eyp, ey);

	/* Calculate Estimated Error of Numerical Intergration */
	if(r->control->integratorOrder < 2) { /* Backward-Euler */
		dd = DD2(((*r->ydtdx) * x0),
				(r->f[r->n%N] * r->x[r->n%N]),
				(r->f[(r->n-1)%N] * r->x[(r->n-1)%N]),
				r->h[r->n%N],
				r->h[(r->n-1)%N]);
		dd *= 1.0/2.0;
		*h = r->control->trtol * e / MaxAbs(dd , r->abstol);
	} else { /* Trapazoidal */
		dd = DD3(((*r->ydtdx) * x0),
				(r->f[r->n%N] * r->x[(r->n)%N]),
				(r->f[(r->n-1)%N] * r->x[(r->n-1)%N]),
				(r->f[(r->n-2)%N] * r->x[(r->n-2)%N]),
				r->h[r->n%N],
				r->h[(r->n-1)%N],
				r->h[(r->n-2)%N]);
				/* See page 309 of "The Spice Book" */
		dd *= 1.0/12.0;
		*h = sqrtf(r->control->trtol * e / MaxAbs(dd , r->abstol));
	}

	Debug("h = %e, e = %e, dd = %e", *h, e, dd);
	ReturnErrIf(isnan(*h));

	return 0;
}

/*---------------------------------------------------------------------------*/

int integratorIntegrate(integrator_ *r, double x0, double *dydx0, double *y0)
{
	double t0, mult;

	ReturnErrIf(r == NULL);
	ReturnErrIf(dydx0 == NULL);
	ReturnErrIf(y0 == NULL);
	ReturnErrIf(isnan(x0));

	/* Only step time when time is increasing, the simulator can search back
	 * in time for a more appropiate point (i.e. based on next step above)
	 */
	t0 = r->control->time;
	ReturnNaNIf(isnan(t0));
	if(t0 > r->t[(r->n+1)%N]) {
		r->n++;
	}

	/* Set New Time */
	r->h[r->n%N] = t0 - r->t[r->n%N];
	r->t[(r->n+1)%N] = t0;

	/* Record X, Y, and F*/
	r->x[r->n%N] = x0;
	r->y[r->n%N] = r->dydx0 * x0 - r->y0;
	r->f[r->n%N] = (*r->ydtdx);

	/* Calculate Next dydx0 and y0 Using Numerical Integration */

	if(r->control->integratorOrder < 2) { /* Backward-Euler */
		*dydx0 = r->f[(r->n-1)%N] / r->h[r->n%N];
		*y0 = (*dydx0) * r->x[r->n%N];
	} else { /* Trapazoidal */
		*dydx0 = 2.0 * r->f[r->n%N] / r->h[r->n%N];
		mult = r->f[r->n%N]/r->f[(r->n-1)%N];
		if(isnan(mult)) {
			mult = 1/r->control->gmin;
		}
		/* y multiplier compensates for a changing function value,
				i.e. not your basic trapazoidal integration, refer to
				"Electronic Circuit and System Simulation Methods" by
				Pillage, Rohrer, and Visweswariah page 310 for more
				details */
		*y0 = (*dydx0) * r->x[r->n%N] + mult*r->y[r->n%N];

	}


	/* Store values for next time */
	r->dydx0 = *dydx0;
	r->y0 = *y0;

	return 0;
}

/*---------------------------------------------------------------------------*/

int integratorInitialize(integrator_ *r, double ic, double *ydtdx)
{
	int i;
	ReturnErrIf(r == NULL);
	ReturnErrIf(ydtdx == NULL);

	r->ydtdx = ydtdx;

	r->y0 = 0.0;
	r->dydx0 = 0.0;
	r->n = 0;

	for(i = 0; i < N; i++) {
		r->t[i] = 0.0;
		r->h[i] = r->control->tstop;
		r->y[i] = 0.0;
		r->f[i] = *ydtdx;
		r->x[i] = ic;
		r->f[i] = (*r->ydtdx);
	}

	if(r->units == 'V') {
		r->abstol = r->control->vntol;
	} else if(r->units == 'A') {
		r->abstol = r->control->abstol;
	} else if(r->units == 'F') {
		r->abstol = r->control->captol;
	} else {
		ReturnErr("Unsupported units type");
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int integratorDestroy(integrator_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying NI %p", (*r));

	free(*r);
	*r = NULL;

	return 0;
}

/*---------------------------------------------------------------------------*/

integrator_ * integratorNew(integrator_ *r, control_ *control, double *ydtdx,
		char units)
{
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(control == NULL);
	ReturnNULLIf(ydtdx == NULL);
	ReturnNULLIf((units != 'A') && (units != 'V') && (units != 'F'));

	r = calloc(1, sizeof(integrator_));
	ReturnNULLIf(r == NULL);

	Debug("Creating NI %p", r);

	r->control = control;
	r->units = units;

	ReturnNULLAndFreeIf(r, integratorInitialize(r, 0.0, ydtdx));

	return r;
}

/*===========================================================================*/

