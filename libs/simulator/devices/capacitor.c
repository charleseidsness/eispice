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

#include "integrator.h"
#include "device_internal.h"

/* Pin Designations */
#define K			0
#define J			1
#define NP			2

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

struct _devicePrivate {
	double *C;		/* Capacitance (Farad) */
	double G;		/* Conductance (siemen) */
	double Ieq;		/* Equalization Current (Amp) */
	integrator_ *integrator;
	row_ *rowK;
	row_ *rowJ;
	node_ *nodeKK;
	node_ *nodeJK;
	node_ *nodeKJ;
	node_ *nodeJJ;
};

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

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
	
	/* Set Initial Conditions (based on Opertaing Point results) */
	v0 = rowGetSolution(p->rowK) - rowGetSolution(p->rowJ);
	ReturnErrIf(isnan(v0));
	
	ReturnErrIf(integratorInitialize(p->integrator, v0, p->C));	
	
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
	p->G = 0;
	p->Ieq = 0;
	
	/* Modified Nodal Analysis Stamp (Open)
	 *	                  	+      -
	 *	  |_Vk_Vj_|_rhs_|	--o  o--
	 *	k | -- -- | --  |	k      j
	 *	j | -- -- | --  |
	 */
	
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
	
	Info("%s -- %s %s -> %s; C = %gF", r->class->type, r->refdes, 
			rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			*p->C);
	
	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceCapacitor = {
	.type = "Capacitor",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = NULL,
	.initStep = deviceClassInitStep,
	.step = NULL,
	.minStep = deviceClassMinStep,
	.nextStep = NULL,
	.integrate = deviceClassIntegrate,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceCapacitorConfig(device_ *r, double *capacitance)
{
	devicePrivate_ *p;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);
	
	/* Copy in class pointer */
	r->class = &deviceCapacitor;
	
	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);
	
	/* allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;
	
	/* Parse arguments */
	p->C = capacitance;
	ReturnErrIf(p->C == NULL);
	
	/* Create numerical integration object */
	p->integrator = integratorNew(p->integrator, r->control, p->C, 'V');
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
	
	return 0;
}

/*===========================================================================*/

