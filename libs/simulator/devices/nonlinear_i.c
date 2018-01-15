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
#include <calc.h>
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
	char *equation;
	double *time;
	double Ic; 		/* Calculated Value (Amps)*/
	double In; 		/* Present Value (Amps)*/
	double Ieq;		/* Equalization Value (Amps) */
	double IeqCalc;
	checklinear_ *checklinear;
	checkbreak_ *checkbreak;
	calc_ *calc;
	list_ *variables;
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

static int deviceNonlinearCalculateInitial(devicePrivate_ *p)
{
	p->In = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(p->In));

	ReturnErrIf(calcSolve(p->calc, &p->Ic));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceNonlinearCalculate(devicePrivate_ *p)
{
	p->In = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(p->In));

	/* If there's no current through the device then it must be an open
	 * and this device will never converge so shut it off by setting the
	 * calculated current to 0 Amps.
	 */

	if(p->In == 0.0) {
		Debug("Open Nonlinear Current Source.");
		p->Ic = 0.0;
		return 0;
	}

	ReturnErrIf(calcSolve(p->calc, &p->Ic));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceNonlinearLoadVariable(variable_ *r, devicePrivate_ *p)
{
	double G;

	/* Calculate the differential w.r.t. this variable */
	ReturnErrIf(calcDiff(p->calc, r->name, &G));
	ReturnErrIf(nodeDataPlus(r->nodeJX, -(G - r->G)));
	ReturnErrIf(nodeDataPlus(r->nodeMX,  (G - r->G)));
	r->G = G;

	/* Contribute to the equivelant current calculation */
	p->IeqCalc -= G * rowGetSolution(r->row);
	ReturnErrIf(isnan(p->IeqCalc));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceNonlinearLoad(devicePrivate_ *p)
{
	ReturnErrIf(p == NULL);

	p->IeqCalc = p->Ic;
	ReturnErrIf(listExecute(p->variables,
			(listExecute_)deviceNonlinearLoadVariable, p));

	ReturnErrIf(rowRHSPlus(p->rowJ, p->IeqCalc - p->Ieq));
	ReturnErrIf(rowRHSPlus(p->rowM, -(p->IeqCalc - p->Ieq)));
	p->Ieq = p->IeqCalc;

	return 0;
}

/*---------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------
 * Call-Back sent to Calc Library, called during equation parsing,
 * Calc is asking for pointers to pointers to the values of variables
 * in the equation.
 * ------------------------------------------------------------------------- */

static double ** deviceNonlinearGetVariable(char *name, device_ *r)
{
	devicePrivate_ *p;
	variable_ *variable;
	double **ptr;

	ReturnNULLIf(r == NULL);
	p = r->private;
	ReturnNULLIf(p == NULL);

	if(!strcmp("time", name)) {
		p->time = &r->control->time;
		ptr = &p->time;
	} else if(((name[0] == 'i') || (name[0] == 'v')) && name[1] == '(') {
		/* if the variable name is i(...) or v(...) link to a row solution */

		/* Create new variable element */
		variable = calloc(1, sizeof(variable_));
		ReturnNULLIf(variable == NULL);

		/* Find the row associated with the varaible */
		variable->row = matrixFindOrAddRow(r->matrix, 0x0, name);
		ReturnNULLIf(variable->row == NULL);
		/* Create a node at the intersection of this row and our row */
		variable->nodeJX = matrixFindOrAddNode(r->matrix, p->rowJ,
				variable->row);
		ReturnNULLIf(variable->nodeJX == NULL);
		variable->nodeMX = matrixFindOrAddNode(r->matrix, p->rowM,
				variable->row);
		ReturnNULLIf(variable->nodeMX == NULL);

		variable->name = name;
		ReturnNULLIf(listAdd(p->variables, variable, NULL));

		/* Get the pointer to ther pointer to the row solution to
		 * return to calculon.
		 */
		ptr = rowGetSolutionPtr(variable->row);
		ReturnNULLIf(ptr == NULL);
	} else {
		ReturnNULL("For the B element, included varaibles must be in the form"
			" i(...), v(...), or time -- not %s", name);
	}

	return ptr;
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

		ReturnErrIf(deviceNonlinearCalculateInitial(p));
		ReturnErrIf(deviceNonlinearLoad(p));

		/* Check to see if we changed the current enough to warrant a break */
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
	ReturnErrIf(deviceNonlinearCalculate(p));

	/* Check convergence */
	*linear = checklinearIsLinear(p->checklinear, p->In, p->Ic);
	ReturnErrIf(*linear < 0);

	/* If not linear update conductances to try again */
	if(!(*linear)) {
		ReturnErrIf(deviceNonlinearLoad(p));
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
	p->Ic = 0.0;
	p->In = 0.0;
	p->Ieq = 0.0;
	p->IeqCalc = 0.0;

	/* Modified Nodal Analysis Stamp Current Source
	 *	                     	       	+  /\ - + __  -
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
	 *			- Ir is the constant value, i.e.:
	 *				Ir = f(Vx0,Vx1,...,Iy0,Iy1,...)-gx*Vx0-gx*Vx1-gx*Iy0...
	 */

	ReturnErrIf(nodeDataSet(p->nodeRK, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeRM, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeKR, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeMR, -1.0));

	ReturnErrIf(deviceNonlinearCalculateInitial(p));
	ReturnErrIf(deviceNonlinearLoad(p));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassUnconfig(device_ *r)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->calc != NULL) {
		ReturnErrIf(calcDestroy(&p->calc));
	}

	if(p->checkbreak != NULL) {
		if(checkbreakDestroy(&p->checkbreak)) {
			Warn("Error destroying break check");
		}
	}

	if(p->variables != NULL) {
		if(listDestroy(&p->variables, listFreeData)) {
			Warn("Error destroying variable list");
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

	Info("%s -- %s %s -> %s; I = %s", r->class->type, r->refdes,
			rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			p->equation);

	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceNonlinearCurrent = {
	.type = "Nonlinear Current Source",
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

int deviceNonlinearCurrentConfig(device_ *r, char *equation)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);
	ReturnErrIf(equation == NULL);

	/* Copy in class pointer */
	r->class = &deviceNonlinearCurrent;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Copy in arguments */
	p->equation = equation;
	p->time = NULL;

	/* Create local variables list */
	p->variables = listNew(p->variables);
	ReturnErrIf(p->variables == NULL);

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

	/* Parse the Equation using The Calc Library  */
	p->calc = calcNew(p->calc, equation,
			(calcGetVarPtr_)deviceNonlinearGetVariable, r, &r->control->gmin);
	ReturnErrIf(p->calc == NULL, "Bad B equation: \n%s", equation);

	return 0;
}

/*===========================================================================*/

