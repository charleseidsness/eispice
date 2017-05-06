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
	node_ *nodeRX;
	double R;		/* Value of Differential (Ohms) */
} variable_;

struct _devicePrivate {
	char *equation;
	double *time;
	double Vc; 		/* Calculated Value (Volts)*/
	double Vn; 		/* Present Value (Volts)*/
	double Veq;		/* Equalization Value (Volts) */
	double VeqCalc;
	checklinear_ *checklinear;
	checkbreak_ *checkbreak;
	calc_ *calc;
	list_ *variables;
	node_ *nodeRK;
	node_ *nodeRJ;
	node_ *nodeKR;
	node_ *nodeJR;
	row_ *rowR;
	row_ *rowK;
	row_ *rowJ;
};

/*===========================================================================
 |                             Local Functions                               |
  ===========================================================================*/

static int deviceNonlinearCalculate(devicePrivate_ *p)
{
	p->Vn = rowGetSolution(p->rowK) - rowGetSolution(p->rowJ);
	ReturnErrIf(isnan(p->Vn));
	ReturnErrIf(calcSolve(p->calc, &p->Vc));
	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceNonlinearLoadVariable(variable_ *r, devicePrivate_ *p)
{
	double R;
	
	/* Calculate the differential w.r.t. this variable */
	ReturnErrIf(calcDiff(p->calc, r->name, &R));
	ReturnErrIf(nodeDataPlus(r->nodeRX, -(R - r->R)));
	r->R = R;
	
	/* Contribute to the equivelant voltage calculation */
	p->VeqCalc -= R * rowGetSolution(r->row);
	ReturnErrIf(isnan(p->VeqCalc));
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceNonlinearLoad(devicePrivate_ *p)
{
	ReturnErrIf(p == NULL);
	
	p->VeqCalc = p->Vc;
	ReturnErrIf(listExecute(p->variables, 
			(listExecute_)deviceNonlinearLoadVariable, p));
	
	ReturnErrIf(rowRHSPlus(p->rowR, p->VeqCalc - p->Veq));
	p->Veq = p->VeqCalc;
	
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
		variable->nodeRX = matrixFindOrAddNode(r->matrix, p->rowR, 
				variable->row);
		ReturnNULLIf(variable->nodeRX == NULL);
		
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
		
		ReturnErrIf(deviceNonlinearCalculate(p));
		ReturnErrIf(deviceNonlinearLoad(p));
		
		/* Check to see if we changed the voltage enough to warrant a break */
		*breakPoint = checkbreakIsBreak(p->checkbreak, p->Veq);
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
	*linear = checklinearIsLinear(p->checklinear, p->Vn, p->Vc);
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
	p->Vc = 0.0;
	p->Vn = 0.0;
	p->Veq = 0.0;
	p->VeqCalc = 0.0;
	
	/* Modified Nodal Analysis Stamp Voltage Source
	 *	                     	        +  /\  -
	 *	  |_Vk_Vj_Ir_Vpx_Isy_|_rhs_|	__/Vr\__
	 *	k | -- --  1 --  --  | --  |	k \  / j
	 *	j | -- -- -1 --  --  | --  |	   \/ Vr = f(Vp0, Vp1, ..., Is0, Is1)
	 *	r |  1 -1 -- -gx -gy | Vr  |	------->
	 *	                    	   		   Ir
	 *	NOTEs:	- gx is the derivative of the function wrt the volatge Vpx 
	 *			- all voltages are relative to ground
	 *			- Ry is the derivative of the function wrt the current Isy.
	 *			- Vr is the constant value, i.e.: 
	 *				Vr = f(Vx0,Vx1,...,Iy0,Iy1,...)-gx*Vx0-gx*Vx1-gx*Iy0...
	 */
	
	ReturnErrIf(nodeDataSet(p->nodeRK, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeRJ, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeKR, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeJR, -1.0));
	
	ReturnErrIf(deviceNonlinearCalculate(p));
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
	
	Info("%s -- %s %s -> %s; V = %s", r->class->type, r->refdes, 
			rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			p->equation);
	
	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceNonlinearVoltage = {
	.type = "Nonlinear Voltage Source",
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

int deviceNonlinearVoltageConfig(device_ *r, char *equation)
{
	devicePrivate_ *p;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);
	ReturnErrIf(equation == NULL);
	
	ReturnErrIf((r->pin[K] == &gndRow) && (r->pin[J] == &gndRow),
			"Source %s has both input nodes shorted to 0!", r->refdes);
	
	/* Copy in class pointer */
	r->class = &deviceNonlinearVoltage;
	
	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);
	
	/* Allocate space for private data */
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
	p->checklinear = checklinearNew(p->checklinear, r->control, 'V');
	ReturnErrIf(p->checklinear == NULL);
	
	/* Setup the break checking object */
	p->checkbreak = checkbreakNew(p->checkbreak, r->control, 'V');
	ReturnErrIf(p->checkbreak == NULL);
	
	/* Create required nodes and rows (see MNA stamps above) */
	p->rowR = matrixFindOrAddRow(r->matrix, 'i', r->refdes);
	ReturnErrIf(p->rowR == NULL);
	p->rowK = r->pin[K];
	ReturnErrIf(p->rowK == NULL);
	p->rowJ = r->pin[J];
	ReturnErrIf(p->rowJ == NULL);
	p->nodeRK = matrixFindOrAddNode(r->matrix, p->rowR, p->rowK);
	ReturnErrIf(p->nodeRK == NULL);
	p->nodeRJ = matrixFindOrAddNode(r->matrix, p->rowR, p->rowJ);
	ReturnErrIf(p->nodeRJ == NULL);
	p->nodeKR = matrixFindOrAddNode(r->matrix, p->rowK, p->rowR);
	ReturnErrIf(p->nodeKR == NULL);
	p->nodeJR = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowR);
	ReturnErrIf(p->nodeJR == NULL);
	
	/* Parse the Equation using The Calc Library  */
	p->calc = calcNew(p->calc, equation, 
			(calcGetVarPtr_)deviceNonlinearGetVariable, r, &r->control->gmin);
	ReturnErrIf(p->calc == NULL, "Bad B equation: \n%s", equation);
	
	return 0;
}

/*===========================================================================*/

