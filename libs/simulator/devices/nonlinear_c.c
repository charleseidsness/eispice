/* 
 * Copyright (C) 2007 Cooper Street Innovations Inc.
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
#include <data.h>
#include <log.h>

#include "integrator.h"
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
	/* Capacitor */
	double G;		/* Conductance (siemen) */
	double Ieq;		/* Equalization Current (Amp) */
	integrator_ *integrator;
	row_ *rowK;
	row_ *rowJ;
	node_ *nodeKK;
	node_ *nodeJK;
	node_ *nodeKJ;
	node_ *nodeJJ;
	/* Control Voltage (use an indepedant voltage to calculate the
		capacitance at each step) */
	char *equation;
	double *time;
	double Cc; 		/* Calculated Value (Farads)*/
	double Cn; 		/* Present Value (Farads)*/
	double Ceq;		/* Equalization Value (Farads) */
	double CeqCalc;
	checklinear_ *checklinear;
	calc_ *calc;
	list_ *variables;
	row_ *rowR;
	row_ *rowC;
	node_ *nodeRC;
	node_ *nodeCR;	
};

/*===========================================================================
 |                             Local Functions                               |
  ===========================================================================*/

static int deviceNonlinearCalculate(devicePrivate_ *p)
{
	p->Cn = rowGetSolution(p->rowC);
	ReturnErrIf(isnan(p->Cn));
	ReturnErrIf(calcSolve(p->calc, &p->Cc));
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
	p->CeqCalc -= R * rowGetSolution(r->row);
	ReturnErrIf(isnan(p->CeqCalc));
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceNonlinearLoad(devicePrivate_ *p)
{
	ReturnErrIf(p == NULL);
	
	p->CeqCalc = p->Cc;
	ReturnErrIf(listExecute(p->variables, 
			(listExecute_)deviceNonlinearLoadVariable, p));
	
	ReturnErrIf(rowRHSPlus(p->rowR, p->CeqCalc - p->Ceq));
	p->Ceq = p->CeqCalc;
	
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
		
		/* Get the pointer to the pointer to the row solution to
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
	*linear = checklinearIsLinear(p->checklinear, p->Cn, p->Cc);
	ReturnErrIf(*linear < 0);
	
	/* If not linear update conductances to try again */
	if(!(*linear)) {
		ReturnErrIf(deviceNonlinearLoad(p));
	}
	
	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassMinStep(device_ *r, double *minStep)
{
	devicePrivate_ *p;
	double v0;
	
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);
	
	Debug("Calc Min Step %s %s %p", r->class->type, r->refdes, r);
	
	v0 = rowGetSolution(p->rowK) - rowGetSolution(p->rowJ);
	ReturnErrIf(isnan(v0));
	
	ReturnErrIf(integratorNextStep(p->integrator, v0, minStep));
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassIntegrate(device_ *r)
{
	devicePrivate_ *p;
	double Ieq, G, v0;
	
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);
	
	Debug("Integrating %s %s %p", r->class->type, r->refdes, r);
		
	/* Modified Nodal Analysis Stamp
	 *				          +    Gn    -
	 *	  |_Vk__Vj_|_rhs_|	  +--/\/\/\--+
	 *	k | Gn -Gn | Ieq |	k_|    __    |_j
	 *	j | -Gn Gn |-Ieq |	  |__ /  \___|
	 *	   					      \__/
	 *	        				   Ieq
	 */
	
	v0 = rowGetSolution(p->rowK) - rowGetSolution(p->rowJ);
	ReturnErrIf(isnan(v0));
	
	ReturnErrIf(integratorIntegrate(p->integrator, v0, &G, &Ieq));
	
	/* Set-up Matrices based on MNA Stamp Above*/
	ReturnErrIf(nodeDataPlus(p->nodeKK, G - p->G));
	ReturnErrIf(nodeDataPlus(p->nodeKJ, -(G - p->G)));
	ReturnErrIf(nodeDataPlus(p->nodeJK, -(G - p->G)));
	ReturnErrIf(nodeDataPlus(p->nodeJJ, G - p->G));
	p->G = G;
	ReturnErrIf(rowRHSPlus(p->rowK, Ieq - p->Ieq));
	ReturnErrIf(rowRHSPlus(p->rowJ, -(Ieq - p->Ieq)));
	p->Ieq = Ieq;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassInitStep(device_ *r)
{
	devicePrivate_ *p;
	double v0;
	
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);
	
	Debug("Initializing Stepping %s %s %p", r->class->type, r->refdes, r);
	
	v0 = rowGetSolution(p->rowK) - rowGetSolution(p->rowJ);
	ReturnErrIf(isnan(v0));
	
	ReturnErrIf(integratorInitialize(p->integrator, v0, &p->Cc));
	p->G = 0;
	p->Ieq = 0;
	
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
	p->Cc = 0.0;
	p->Cn = 0.0;
	p->Ceq = 0.0;
	p->CeqCalc = 0.0;
	
	/* Modified Nodal Analysis Stamp (Open)
	 *	                  	+      -
	 *	  |_Vk_Vj_|_rhs_|	--o  o--
	 *	k | -- -- | --  |	k      j
	 *	j | -- -- | --  |
	 */
	
	/* Modified Nodal Analysis Stamp Control Voltage Source
	 *	                     	    +  /\  -
	 *	  |_Vc_Ir_Vpx_Isy_|_rhs_|	__/Vr\__
	 *	c | --  1 --  --  | --  |	k \  / GND
	 *	r |  1 -- -gx -gy | Vr  |	   \/ Vr = f(Vp0, Vp1, ..., Is0, Is1)
	 *								------->
	 *	                    	   		   Ir
	 *	NOTEs:	- gx is the derivative of the function wrt the volatge Vpx 
	 *			- all voltages are relative to ground
	 *			- Ry is the derivative of the function wrt the current Isy.
	 *			- Vr is the constant value, i.e.: 
	 *				Vr = f(Vx0,Vx1,...,Iy0,Iy1,...)-gx*Vx0-gx*Vx1-gx*Iy0...
	 */
	
	ReturnErrIf(nodeDataSet(p->nodeRC, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeCR, 1.0));	
	
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
	
	Debug("Unconfiging %s %s %p", r->class->type, r->refdes, r);
	
	if(p->calc != NULL) {
		ReturnErrIf(calcDestroy(&p->calc));
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
	
	if(p->integrator != NULL) {
		if(integratorDestroy(&p->integrator)) {
			Warn("Error destroying integrator");
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

	Info("%s -- %s %s -> %s; C = %s", r->class->type, r->refdes, 
			rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			p->equation);
	
	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceVoltageControlledCap = {
	.type = "Voltage Controlled Capacitor",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = deviceClassLinearize,
	.initStep = deviceClassInitStep,
	.step = deviceClassStep,
	.minStep = deviceClassMinStep,
	.nextStep = NULL,
	.integrate = deviceClassIntegrate,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

/* Capacitance for the integrator until it is properly initialsied. */
double icc = 0.0;

int deviceNonlinearCapacitorConfig(device_ *r, char *equation)
{
	devicePrivate_ *p;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);
	ReturnErrIf(equation == NULL);
	
	/* Copy in class pointer */
	r->class = &deviceVoltageControlledCap;
	
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
	p->checklinear = checklinearNew(p->checklinear, r->control, 'F');
	ReturnErrIf(p->checklinear == NULL);
	
	/* Create numerical intergartion object */
	p->integrator = integratorNew(p->integrator, r->control, &icc, 'F');
	ReturnErrIf(p->integrator == NULL);
	
	/* Create required nodes and rows (see MNA stamp above) */
	p->rowK = r->pin[K];
	ReturnErrIf(p->rowK == NULL);
	p->rowJ = r->pin[J];
	ReturnErrIf(p->rowJ == NULL);
	p->nodeKK = matrixFindOrAddNode(r->matrix, p->rowK, p->rowK);
	ReturnErrIf(p->nodeKK == NULL);
	p->nodeJK = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowK);
	ReturnErrIf(p->nodeJK == NULL);
	p->nodeKJ = matrixFindOrAddNode(r->matrix, p->rowK, p->rowJ);
	ReturnErrIf(p->nodeKJ == NULL);
	p->nodeJJ = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowJ);
	ReturnErrIf(p->nodeJJ == NULL);
	
	p->rowR = matrixFindOrAddRow(r->matrix, 'i', r->refdes);
	ReturnErrIf(p->rowR == NULL);
	p->rowC = matrixFindOrAddRow(r->matrix, 'v', r->refdes);
	ReturnErrIf(p->rowC == NULL);
	p->nodeRC = matrixFindOrAddNode(r->matrix, p->rowR, p->rowC);
	ReturnErrIf(p->nodeRC == NULL);
	p->nodeCR = matrixFindOrAddNode(r->matrix, p->rowC, p->rowR);
	ReturnErrIf(p->nodeCR == NULL);	
	
	/* Parse the Equation using The Calc Library  */
	p->calc = calcNew(p->calc, equation, 
			(calcGetVarPtr_)deviceNonlinearGetVariable, r, &r->control->gmin);
	ReturnErrIf(p->calc == NULL, "Bad B equation: \n%s", equation);
	
	return 0;
}

/*===========================================================================*/
