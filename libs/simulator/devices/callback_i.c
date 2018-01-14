/*
 * Copyright (C) 2006 Cooper Street Vnnovations Vnc.
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
 * Foundation, Vnc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <math.h>
#include <log.h>
#include <data.h>

#include "checkbreak.h"
#include "checklinear.h"
#include "device_internal.h"

/* Pin Designations */
#define K			0
#define J			1
#define NP			2

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

typedef struct {
	char *name;
	row_ *row;
	node_ *nodeJX;
	node_ *nodeMX;
	double G;		/* Value of Differential (Siemens) */
} variable_;

struct _devicePrivate {
	double *time;
	char **vars;
	int numVars;
	deviceCallback_ callback;
	void *private;
	double Ic; 		/* Calculated Value (Amps)*/
	double In; 		/* Present Value (Amps)*/
	double Ieq;		/* Equalization Value (Amps) */
	double IeqCalc;
	checklinear_ *checklinear;
	checkbreak_ *checkbreak;
	variable_ *variables;
	double *derivs;
	double *values;
	node_ *nodeRK;
	node_ *nodeRM;
	node_ *nodeKR;
	node_ *nodeMR;
	row_ *rowR;
	row_ *rowK;
	row_ *rowJ;
	row_ *rowM;
};

/*===========================================================================
 |                             Local Functions                               |
  ===========================================================================*/

static int deviceCallbackCalculate(devicePrivate_ *p)
{
	int i;

	p->In = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(p->In));

	for(i = 0; i < p->numVars; i++) {
		if(!strcmp(p->variables[i].name, "time")) {
			ReturnErrIf(p->time == NULL);
			p->values[i] = *p->time;
		} else {
			p->values[i] = rowGetSolution(p->variables[i].row);
			ReturnErrIf(isnan(p->values[i]), "%s", p->variables[i].name);
		}
	}

	ReturnErrIf(p->callback(&p->Ic, p->private));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceCallbackLoadVariable(devicePrivate_ *p, variable_ *variable,
		double G)
{
	/* Set the differential w.r.t. this variable */
	ReturnErrIf(nodeDataPlus(variable->nodeJX, -(G - variable->G)));
	ReturnErrIf(nodeDataPlus(variable->nodeMX,  (G - variable->G)));
	variable->G = G;

	/* Contribute to the equivelant current calculation */
	p->IeqCalc -= G * rowGetSolution(variable->row);
	ReturnErrIf(isnan(p->IeqCalc));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceCallbackLoad(devicePrivate_ *p)
{
	int i;

	ReturnErrIf(p == NULL);

	p->IeqCalc = p->Ic;
	for(i = 0; i < p->numVars; i++) {
		if(strcmp(p->variables[i].name, "time")) {
			ReturnErrIf(deviceCallbackLoadVariable(p, &p->variables[i],
				p->derivs[i]));
		}
	}

	ReturnErrIf(rowRHSPlus(p->rowJ, p->IeqCalc - p->Ieq));
	ReturnErrIf(rowRHSPlus(p->rowM, -(p->IeqCalc - p->Ieq)));
	p->Ieq = p->IeqCalc;

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceCallbackSetVariable(device_ *r, variable_ *variable,
		char *name)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	variable->name = name;

	if(!strcmp("time", name)) {
		p->time = &r->control->time;
		return 0;
	} else if(((name[0] == 'i') || (name[0] == 'v')) && name[1] == '(') {
		/* if the variable name is i(...) or v(...) link to a row solution */

		/* Find the row associated with the varaible */
		variable->row = matrixFindOrAddRow(r->matrix, 0x0, name);
		ReturnErrIf(variable->row == NULL);
		/* Create a node at the intersection of this row and our row */
		variable->nodeJX = matrixFindOrAddNode(r->matrix, p->rowJ,
				variable->row);
		ReturnErrIf(variable->nodeJX == NULL);
		variable->nodeMX = matrixFindOrAddNode(r->matrix, p->rowM,
				variable->row);
		ReturnErrIf(variable->nodeMX == NULL);

	} else {
		ReturnErr("For the B element, included varaibles must be in the form"
				" i(...), v(...), or time -- not %s", name);
	}

	return 0;
}

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

static int deviceClassStep(device_ *r, int *breakPoint)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->time != NULL) {
		Debug("Stepping %s %s %p", r->class->type, r->refdes, r);

		ReturnErrIf(deviceCallbackCalculate(p));
		ReturnErrIf(deviceCallbackLoad(p));

		/* Check to see if we changed the voltage enough to warrant a break */
		*breakPoint = checkbreakIsBreak(p->checkbreak, p->Ieq);
		ReturnErrIf(*breakPoint < 0);
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassLinearize(device_ *r, int *linear)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Linearizing %s %s %p", r->class->type, r->refdes, r);
	ReturnErrIf(deviceCallbackCalculate(p));

	/* Check convergence */
	*linear = checklinearIsLinear(p->checklinear, p->In, p->Ic);
	ReturnErrIf(*linear < 0);

	/* If not linear update conductances to try again */
	if(!(*linear)) {
		ReturnErrIf(deviceCallbackLoad(p));
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

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
	p->time = &r->control->time;
	p->Ic = 0.0;
	p->In = 0.0;
	p->Ieq = 0.0;
	p->IeqCalc = 0.0;

	/* Modified Nodal Analysis Stamp Current Source
	 *	                     	  		+  /\ - + __  -
	 *	  |_Vk_Vj_Vm_Ir_Vpx_Isy_|_rhs_|	__/Vr\___/  \__
	 *	k | -- -- --  1 --  --  | --  |	k \  / m \__/ j
	 *	j | -- -- -- -- -gx -gy |  Ir |	   \/ 0v
	 *	m | -- -- -- -1  gx gy  | -Ir |   ---------------->
	 *	r |  1 -- -1 -- --  --  | --  |   Ir = f(Vp0, Vp1, ..., Is0, Is1)
	 *
	 *	NOTEs:	- the 0V Voltage source is used as an ammeter
	 *			- gx is the derivative of the function wrt the volatge Vpx
	 *			- all voltages are relative to ground
	 *			- Gy is the derivative of the function wrt the current Isy.
	 *			- Vr is the constant value, i.e.:
	 *				Ir = f(Vx0,Vx1,...,Iy0,Iy1,...)-gx*Vx0-gx*Vx1-gx*Iy0...
	 */

	ReturnErrIf(nodeDataSet(p->nodeRK, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeRM, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeKR, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeMR, -1.0));

	ReturnErrIf(deviceCallbackCalculate(p));
	ReturnErrIf(deviceCallbackLoad(p));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassUnconfig(device_ *r)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->variables != NULL) {
		free(p->variables);
	}

	if(p->checkbreak != NULL) {
		if(checkbreakDestroy(&p->checkbreak)) {
			Warn("Error destroying break check");
		}
	}

	if(p->checklinear != NULL) {
		if(checklinearDestroy(&p->checklinear)) {
			Warn("Error destroying linear check");
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

deviceClass_ deviceCallbackCurrent = {
	.type = "Callback Current Source",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = deviceClassLinearize,
	.initStep = NULL,
	.step = deviceClassStep,
	.minStep = NULL,
	.nextStep = NULL,
	.integrate = NULL,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceCallbackCurrentConfig(device_ *r, char *vars[], double values[],
		double derivs[], int numVars, deviceCallback_ callback, void *private)
{
	devicePrivate_ *p;
	int i;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(callback == NULL);
	ReturnErrIf(r->numPins != NP);
	ReturnErrIf(vars == NULL);
	ReturnErrIf(values == NULL);
	ReturnErrIf(derivs == NULL);
	ReturnErrIf(numVars < 0);

	ReturnErrIf((r->pin[K] == &gndRow) && (r->pin[J] == &gndRow),
			"Source %s has both input nodes shorted to 0!", r->refdes);

	/* Copy in class pointer */
	r->class = &deviceCallbackCurrent;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Copy in arguments */
	p->numVars = numVars;
	p->callback = callback;
	p->private = private;
	p->time = NULL;

	/* Create local variables list */
	p->variables = calloc(numVars, sizeof(variable_));
	ReturnErrIf(p->variables == NULL);

	/* Store arrays used to send data to and get data from the callback */
	p->derivs = derivs;
	p->values = values;

	/* Setup the linear checking object */
	p->checklinear = checklinearNew(p->checklinear, r->control, 'A');
	ReturnErrIf(p->checklinear == NULL);

	/* Setup the break checking object */
	p->checkbreak = checkbreakNew(p->checkbreak, r->control, 'A');
	ReturnErrIf(p->checkbreak == NULL);

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

	/* Fill in the variables array */
	for(i = 0; i < numVars; i++) {
		ReturnErrIf(deviceCallbackSetVariable(r, &p->variables[i], vars[i]));
	}

	return 0;
}

/*===========================================================================*/

