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
	double *L;		/* Inductance (Henry) */
	double R;		/* Resistance (Ohm) */
	double Veq;		/* Equalization Voltage (Volt) */
	integrator_ *integrator;
	row_ *rowK;
	row_ *rowJ;
	row_ *rowR;
	node_ *nodeRK;
	node_ *nodeRJ;
	node_ *nodeKR;
	node_ *nodeJR;
	node_ *nodeRR;
};

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

static int deviceClassMinStep(device_ *r, double *minStep)
{
	devicePrivate_ *p;
	double i0;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Calc Min Step %s %s %p", r->class->type, r->refdes, r);

	i0 = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(i0));

	ReturnErrIf(integratorNextStep(p->integrator, i0, minStep));

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassIntegrate(device_ *r)
{
	devicePrivate_ *p;
	double Veq, R, i0;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Integrating %s %s %p", r->class->type, r->refdes, r);

	/* Modified Nodal Analysis Stamp
	 *
	 *	  |_Vk_Vj_Ir_|_rhs_|
	 *	k | -- --  1 | --  |	 +    /\     Rn    -
	 *	j | -- -- -1 | --  |	 k__ /  \__/\/\/\__j
	 *	r | 1  -1 -Rn|-Veq |	     \  /
	 *	        				      \/ Veq
	 */

	i0 = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(i0));

	ReturnErrIf(integratorIntegrate(p->integrator, i0, &R, &Veq));

	/* Set-up Matrices based on MNA Stamp Above*/
	ReturnErrIf(nodeDataPlus(p->nodeRR, -(R - p->R)));
	p->R = R;
	ReturnErrIf(rowRHSPlus(p->rowR, -(Veq - p->Veq)));
	p->Veq = Veq;

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassInitStep(device_ *r)
{
	devicePrivate_ *p;
	double i0;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Initializing Stepping %s %s %p", r->class->type, r->refdes, r);

	/* Set Initial Conditions (based on Opertaing Point results) */
	i0 = rowGetSolution(p->rowR);
	ReturnErrIf(isnan(i0));

	ReturnErrIf(integratorInitialize(p->integrator, i0, p->L));

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
	p->R = 0;
	p->Veq = 0;

	/* Modified Nodal Analysis Stamp (Short)
	 *	                     	+      -
	 *	  |_Vk_Vj_Ir_|_rhs_|	________
	 *	k | -- --  1 | --  |	k      j
	 *	j | -- -- -1 | --  |
	 *	r |  1 -1 -- | --  |	------->
	 *	                    	   Ir
	 */

	ReturnErrIf(nodeDataSet(p->nodeRK, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeRJ, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeKR, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeJR, -1.0));

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

	Info("%s -- %s %s -> %s; L = %gH", r->class->type, r->refdes,
			rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			*p->L);

	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceInductor = {
	.type = "Inductor",
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

int deviceInductorConfig(device_ *r, double *inductance)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);

	/* Copy in class pointer */
	r->class = &deviceInductor;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Parse arguments */
	p->L = inductance;
	ReturnErrIf(p->L == NULL);

	/* Create numerical integration object */
	p->integrator = integratorNew(p->integrator, r->control, p->L, 'A');
	ReturnErrIf(p->integrator == NULL);

	/* Create required nodes and rows (see MNA stamp above) */
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
	p->nodeRR = matrixFindOrAddNode(r->matrix, p->rowR, p->rowR);
	ReturnErrIf(p->nodeJR == NULL);

	return 0;
}

/*===========================================================================*/

