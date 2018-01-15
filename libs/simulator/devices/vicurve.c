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
 * You should have recevied a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <math.h>
#include <log.h>

#include "checkbreak.h"
#include "piecewise.h"
#include "checklinear.h"
#include "device_internal.h"

/* Pin Designations */
#define K			0
#define J			1
#define NP			2

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

struct _devicePrivate {
	double G;		/* Value of Differential (Siemens) */
	double Ieq;		/* Equalization Current (Amps) */
	double a;		/* Multiplier (determined by pw) */
	checklinear_ *checklinear;
	checkbreak_ *checkbreak;
	piecewise_ *ta;
	int taIndex;
	piecewise_ *vi;
	int viIndex;
	node_ *nodeRK;
	node_ *nodeRM;
	node_ *nodeKR;
	node_ *nodeMR;
	node_ *nodeJJ;
	node_ *nodeJM;
	node_ *nodeMJ;
	node_ *nodeMM;
	row_ *rowR;
	row_ *rowK;
	row_ *rowJ;
	row_ *rowM;
};

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

static int deviceClassNextStep(device_ *r, double *nextStep)
{
	double nextTime, time;

	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->ta != NULL) {
		time = r->control->time;
		ReturnErrIf(piecewiseGetNextX(p->ta, &p->taIndex, time, &nextTime));
		*nextStep = nextTime - time;
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassStep(device_ *r, int *breakPoint)
{
	double dadt;
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Stepping %s %s %p", r->class->type, r->refdes, r);
	if(p->ta != NULL) {
		ReturnErrIf(piecewiseCalcValue(p->ta, &p->taIndex,
				r->control->time, &p->a, &dadt));

		/* Check to see if we changed the voltage enough to warrant a break */
		*breakPoint = checkbreakIsBreak(p->checkbreak, p->a);
		ReturnErrIf(*breakPoint < 0);
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassLinearize(device_ *r, int *linear)
{
	double G, Ieq, v0, i0, gc, ic;
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Linearizing %s %s %p", r->class->type, r->refdes, r);

	i0 = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(i0));

	v0 = rowGetSolution(p->rowK) - rowGetSolution(p->rowJ);
	ReturnErrIf(isnan(v0));

	/* If there's no current through the device then it must be an
	 * open, so calculated current is forced to 0Amps
	 */
	if(i0 == 0.0) {
		Debug("Open current source");
		ic = 0.0;
	} else {
		ReturnErrIf(piecewiseCalcValue(p->vi, &p->viIndex, v0, &ic, &gc));
		ic *= p->a;
		gc *= p->a;
	}

	/* Check convergence */
	*linear = checklinearIsLinear(p->checklinear, i0, ic);
	ReturnErrIf(*linear < 0);

	/* If not linear update conductances to try again */
	if(!(*linear)) {
		/*Warn("%s failed to linearize\n%e %e %e %e",
				r->refdes, v0, i0, gc, ic);*/

		if (gc < r->control->gmin) {
			gc = r->control->gmin;
		}

		Ieq = (ic - gc * v0);
		G = gc;

		ReturnErrIf(nodeDataPlus(p->nodeJJ, G - p->G));
		ReturnErrIf(nodeDataPlus(p->nodeJM, -(G - p->G)));
		ReturnErrIf(nodeDataPlus(p->nodeMJ, -(G - p->G)));
		ReturnErrIf(nodeDataPlus(p->nodeMM, G - p->G));
		p->G = G;

		ReturnErrIf(rowRHSPlus(p->rowJ, Ieq - p->Ieq));
		ReturnErrIf(rowRHSPlus(p->rowM, -(Ieq - p->Ieq)));
		p->Ieq = Ieq;

	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassLoad(device_ *r)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Loading %s %s %p", r->class->type, r->refdes, r);

	/* Initialise / Reset State Data */
	ReturnErrIf(checklinearInitialize(p->checklinear, 0.0));
	ReturnErrIf(checkbreakInitialize(p->checkbreak, 0.0));
	if(p->ta != NULL) {
		ReturnErrIf(piecewiseInitialize(p->ta));
		ReturnErrIf(piecewiseCalcValue(p->ta,&p->taIndex, 0.0, &p->a, &p->G));
	} else {
		p->a = 1.0;
	}
	ReturnErrIf(piecewiseInitialize(p->vi));
	ReturnErrIf(piecewiseCalcValue(p->vi, &p->viIndex, 0.0, &p->Ieq, &p->G));
	p->G *= p->a;
	p->Ieq *= p->a;

	/* Modified Nodal Analysis Stamp
	 *	                      		+  /\ - + __  -
	 *	  |_Vk_Vj_Vm_Ir_|_rhs_|		__/Vr\___/  \__
	 *	k | -- -- --  1 | --  |		k \  / m \__/ j
	 *	j | -- -g  g -- |  Ir |	   	   \/ 0v
	 *	m | --  g -g -1 | -Ir |   ---------------->
	 *	r |  1 -- -1 -- | --  |   Ir = f(Vp0, Vp1, ..., Is0, Is1)
	 *
	 *	NOTEs:	- the 0V Voltage source is used as an ammeter
	 *			- g is the derviatvie of the function wrt the volatge Vkj
	 */

	ReturnErrIf(nodeDataSet(p->nodeRK, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeRM, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeKR, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeMR, -1.0));

	ReturnErrIf(nodeDataPlus(p->nodeJJ, p->G));
	ReturnErrIf(nodeDataPlus(p->nodeJM, -(p->G)));
	ReturnErrIf(nodeDataPlus(p->nodeMJ, -(p->G)));
	ReturnErrIf(nodeDataPlus(p->nodeMM, p->G));

	ReturnErrIf(rowRHSPlus(p->rowJ, p->Ieq));
	ReturnErrIf(rowRHSPlus(p->rowM, -(p->Ieq)));

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassUnconfig(device_ *r)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->checklinear != NULL) {
		if(checklinearDestroy(&p->checklinear)) {
			Warn("Error destroying linear check");
		}
	}

	if(p->checkbreak != NULL) {
		if(checkbreakDestroy(&p->checkbreak)) {
			Warn("Error destroying break check");
		}
	}

	if(p->vi != NULL) {
		if(piecewiseDestroy(&p->vi)) {
			Warn("Error destroying piecewise");
		}
	}

	if(p->ta != NULL) {
		if(piecewiseDestroy(&p->ta)) {
			Warn("Error destroying piecewise");
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassPrint(device_ *r)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Printing %s %s %p", r->class->type, r->refdes, r);

	Info("%s -- %s %s -> %s", r->class->type, r->refdes,
			rowGetName(r->pin[K]), rowGetName(r->pin[J]));

	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceVICurve = {
	.type = "VI Curve",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = deviceClassLinearize,
	.initStep = NULL,
	.step = deviceClassStep,
	.minStep = NULL,
	.nextStep = deviceClassNextStep,
	.integrate = NULL,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceVICurveConfig(device_ *r, double **vi, int *viLength, char viType,
		double **ta, int *taLength, char taType)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);

	/* Copy in class pointer */
	r->class = &deviceVICurve;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* Allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Copy in parameter pointers */
	p->vi = piecewiseNew(p->vi, vi, viLength, viType);
	ReturnErrIf(p->vi == NULL);
	if(ta != NULL) {
		if((*ta != NULL) && (taLength != NULL)) {
			p->ta = piecewiseNew(p->ta, ta, taLength, taType);
			ReturnErrIf(p->ta == NULL);
		}
	}

	p->checklinear = checklinearNew(p->checklinear, r->control, 'A');
	ReturnErrIf(p->checklinear == NULL);
	p->checkbreak = checkbreakNew(p->checkbreak, r->control, 'A');
	ReturnErrIf(p->checkbreak == NULL)

	/* Create required nodes and rows (see MNA stamps above) */
	p->rowR = matrixFindOrAddRow(r->matrix, 'i', r->refdes);
	ReturnErrIf(p->rowR == NULL);
	p->rowM = matrixFindOrAddRow(r->matrix, 'v', r->refdes);
	ReturnErrIf(p->rowM == NULL);
	p->rowK = r->pin[K];
	ReturnErrIf(p->rowK == NULL);
	p->rowJ = r->pin[J];
	ReturnErrIf(p->rowJ == NULL);
	p->nodeRK = matrixFindOrAddNode(r->matrix, p->rowR, p->rowK);
	ReturnErrIf(p->nodeRK == NULL);
	p->nodeRM = matrixFindOrAddNode(r->matrix, p->rowR, p->rowM);
	ReturnErrIf(p->nodeRM == NULL);
	p->nodeKR = matrixFindOrAddNode(r->matrix, p->rowK, p->rowR);
	ReturnErrIf(p->nodeKR == NULL);
	p->nodeMR = matrixFindOrAddNode(r->matrix, p->rowM, p->rowR);
	ReturnErrIf(p->nodeMR == NULL);
	p->nodeJJ = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowJ);
	ReturnErrIf(p->nodeJJ == NULL);
	p->nodeJM = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowM);
	ReturnErrIf(p->nodeJM == NULL);
	p->nodeMJ = matrixFindOrAddNode(r->matrix, p->rowM, p->rowJ);
	ReturnErrIf(p->nodeMJ == NULL);
	p->nodeMM = matrixFindOrAddNode(r->matrix, p->rowM, p->rowM);
	ReturnErrIf(p->nodeMM == NULL);

	return 0;
}

/*===========================================================================*/

