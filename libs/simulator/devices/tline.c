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
#include <data.h>

#include "checkbreak.h"
#include "history.h"
#include "history_interp.h"
#include "device_internal.h"

/* Pin Designations */
#define K			0
#define J			1
#define L			2
#define M			3
#define NP			4

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

struct _devicePrivate {
	double *Z0;		/* Characteristic Impedance (Ohms) */
	double *Td;		/* Delay (seconds) */
	double *loss;	/* Loss = (loss tangent) * (length) (unit-less) */
	double Vr;		/* Input Equalization Voltage (Volts) */
	double Vs;		/* Output Equalization Voltage (Volts) */
	checkbreak_ *checkbreakR;
	checkbreak_ *checkbreakS;
	historyInterp_ *historyInterp;
	double IrIC;		/* Initial Conditions */
	double IsIC;		/* Initial Conditions */
	double VkIC;		/* Initial Conditions */
	double VjIC;		/* Initial Conditions */
	double VlIC;		/* Initial Conditions */
	double VmIC;		/* Initial Conditions */
	row_ *rowK;
	row_ *rowJ;
	row_ *rowL;
	row_ *rowM;
	char *rowRName;
	row_ *rowR;
	char *rowSName;
	row_ *rowS;
	node_ *nodeRK;
	node_ *nodeRJ;
	node_ *nodeKR;
	node_ *nodeJR;
	node_ *nodeRL;
	node_ *nodeRM;
	node_ *nodeSL;
	node_ *nodeSM;
	node_ *nodeLS;
	node_ *nodeMS;
	node_ *nodeSK;
	node_ *nodeSJ;
	node_ *nodeRR;
	node_ *nodeSS;
	node_ *nodeKS;
	node_ *nodeJS;
	node_ *nodeLR;
	node_ *nodeMR;
};

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

/* TODO: Take a look at adding a next step object to this class. */

/*---------------------------------------------------------------------------*/

static int deviceClassInitStep(device_ *r)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Initializing Stepping %s %s %p", r->class->type, r->refdes, r);

	/* Remove explict km -> lm link */
	ReturnErrIf(nodeDataClear(p->nodeRL));
	ReturnErrIf(nodeDataClear(p->nodeRM));
	ReturnErrIf(nodeDataClear(p->nodeSK));
	ReturnErrIf(nodeDataClear(p->nodeSJ));
	ReturnErrIf(nodeDataClear(p->nodeKS));
	ReturnErrIf(nodeDataClear(p->nodeJS));
	ReturnErrIf(nodeDataClear(p->nodeLR));
	ReturnErrIf(nodeDataClear(p->nodeMR));

	/* set initial conditions */
	p->IrIC = 0.0;
	p->IsIC = 0.0;
	p->VkIC = rowGetSolution(p->rowK);
	p->VjIC = rowGetSolution(p->rowJ);
	p->VlIC = rowGetSolution(p->rowL);
	p->VmIC = rowGetSolution(p->rowM);

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassStep(device_ *r, int *breakPoint)
{
	devicePrivate_ *p;
	double Vl, Vm, Vj, Vk, Vs, Vr; /* Voltage at nodes (V) */
	double Ir, Is; /* Current through sources (A) */
	double tp; /* Time minus Tline's Delay (s) */

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Stepping %s %s %p", r->class->type, r->refdes, r);

	/* Modified Nodal Analysis Stamp
	 *	                     		 	     /\     Zo    -
	 *	  |_Vk_Vj_Ir_Vl_Vm_Is_|_rhs_|	k__ /  \__/\/\/\__j
	 *	k | -- --  1 -- -- -- | --  |	    \  /
	 *	j | -- -- -1 -- -- -- | --  |	     \/ Vr = Vlm(t-Td) + Zo*Ilm(t-Td)
	 *	r | 1  -1 -Zo-- -- -- | Vr  |  +    /\     Zo    -
	 *	l | -- -- -- -- --  1 | --  |  l__ /  \__/\/\/\__m
	 *	m | -- -- -- -- -- -1 | --  |      \  /
	 *	s | -- -- --  1 -1 -Zo| Vs  |       \/ Vs = Vkj(t-Td) + Zo*Ikj(t-Td)
	 */

	/* tp (time previous) is the time at which the waveform entered
	 * the tranmision line
	 */
	tp = r->control->time - (*p->Td);
	ReturnErrIf(isnan(tp));

	if(tp <= 0) {
		Ir = p->IrIC;
		Is = p->IsIC;
		Vk = p->VkIC;
		Vj = p->VjIC;
		Vl = p->VlIC;
		Vm = p->VmIC;
	} else {

		ReturnErrIf(historyInterpSetTime(p->historyInterp, tp));

		ReturnErrIf(historyInterpGetData(p->historyInterp,
				rowGetIndex(p->rowR), &Ir));
		ReturnErrIf(historyInterpGetData(p->historyInterp,
				rowGetIndex(p->rowS), &Is));
		ReturnErrIf(historyInterpGetData(p->historyInterp,
				rowGetIndex(p->rowK), &Vk));
		ReturnErrIf(historyInterpGetData(p->historyInterp,
				rowGetIndex(p->rowJ), &Vj));
		ReturnErrIf(historyInterpGetData(p->historyInterp,
				rowGetIndex(p->rowL), &Vl));
		ReturnErrIf(historyInterpGetData(p->historyInterp,
				rowGetIndex(p->rowM), &Vm));

	}

	/* For a derivation of these equations refer to "Qucs Technical Papers"
		pages 105-107 */
	if(*p->loss == HUGE_VAL) {
		Vr = (Vl - Vm) + (*p->Z0)*Is;
		Vs = (Vk - Vj) + (*p->Z0)*Ir;
	} else {
		Vr = exp(-(*p->loss)/2)*((Vl - Vm) + (*p->Z0)*Is);
		Vs = exp(-(*p->loss)/2)*((Vk - Vj) + (*p->Z0)*Ir);
	}

	ReturnErrIf(rowRHSPlus(p->rowR, Vr - p->Vr));
	p->Vr = Vr;

	ReturnErrIf(rowRHSPlus(p->rowS, Vs - p->Vs));
	p->Vs = Vs;

	/* Check to see if we changed the voltage enough to warrant a break */
	*breakPoint = checkbreakIsBreak(p->checkbreakR, Vr);
	ReturnErrIf(*breakPoint < 0);
	if(!(*breakPoint)) {
		*breakPoint = checkbreakIsBreak(p->checkbreakS, Vs);
		ReturnErrIf(*breakPoint < 0);
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
	ReturnErrIf(checkbreakInitialize(p->checkbreakR, 0.0));
	ReturnErrIf(checkbreakInitialize(p->checkbreakS, 0.0));
	ReturnErrIf(historyInterpInitialize(p->historyInterp));
	p->Vr = 0.0;
	p->Vs = 0.0;
	p->IrIC = 0.0;
	p->IsIC = 0.0;
	p->VkIC = 0.0;
	p->VjIC = 0.0;
	p->VlIC = 0.0;
	p->VmIC = 0.0;

	/* These short the output to the input for opertaing point analysis
	 * they're removed during the first timestep.
	 */
	ReturnErrIf(nodeDataSet(p->nodeRL, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeRM, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeSK, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeSJ, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeKS, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeJS, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeLR, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeMR, 1.0));

	/* Modified Nodal Analysis Stamp
	 *	                     		 	     /\     gmin   -
	 *	  |_Vk_Vj_Ir__Vl_Vm_Is_|_rhs_|	k__ /  \__/\/\/\__j
	 *	k | -- --  1  -- -- -1 | --  |	    \  /
	 *	j | -- -- -1  -- --  1 | --  |	     \/ Vm = Vlm
	 *	r | 1  -1 -gm -1  1 -- | --  | +     /\     gmin   -
	 *	l | -- -- -1  -- --  1 | --  |  l__ /  \__/\/\/\__m
	 *	m | -- --  1  -- -- -1 | --  |      \  /
	 *	s |  1 -1  -- 1 -1 -gm | --  |       \/ Vs = Vkj
	 */

	ReturnErrIf(nodeDataSet(p->nodeRK, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeRJ, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeKR, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeJR, -1.0));

	ReturnErrIf(nodeDataSet(p->nodeSL, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeSM, -1.0));
	ReturnErrIf(nodeDataSet(p->nodeLS, 1.0));
	ReturnErrIf(nodeDataSet(p->nodeMS, -1.0));

	ReturnErrIf(nodeDataPlus(p->nodeRR, -(*p->Z0)));
	ReturnErrIf(nodeDataPlus(p->nodeSS, -(*p->Z0)));

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

	if(p->rowRName != NULL) {
		free(p->rowRName);
	}

	if(p->rowSName != NULL) {
		free(p->rowSName);
	}

	if(p->historyInterp != NULL) {
		if(historyInterpDestroy(&p->historyInterp)) {
			Warn("Error destroying history interpolation");
		}
	}

	if(p->checkbreakS != NULL) {
		if(checkbreakDestroy(&p->checkbreakS)) {
			Warn("Error destroying break check");
		}
	}

	if(p->checkbreakR != NULL) {
		if(checkbreakDestroy(&p->checkbreakR)) {
			Warn("Error destroying break check");
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

	Info("%s -- %s %s -> %s, %s -> %s; Z0 = %gOhms, Td = %gs, loss = %g",
			r->class->type, r->refdes,
			rowGetName(r->pin[K]), rowGetName(r->pin[L]),
			rowGetName(r->pin[J]), rowGetName(r->pin[M]),
			*p->Z0, *p->Td, *p->loss);

	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceTLine = {
	.type = "Transmission Line",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = NULL,
	.initStep = deviceClassInitStep,
	.step = deviceClassStep,
	.minStep = NULL,
	.nextStep = NULL,
	.integrate = NULL,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceTLineConfig(device_ *r, double *Z0, double *Td, double *loss)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);

	ReturnErrIf((r->pin[K] == &gndRow) && (r->pin[J] == &gndRow),
			"T-Line %s has both input nodes shorted to 0!", r->refdes);
	ReturnErrIf((r->pin[L] == &gndRow) && (r->pin[M] == &gndRow),
			"T-Line %s has both output nodes shorted to 0!", r->refdes);

	/* Copy in class pointer */
	r->class = &deviceTLine;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Copy in parameter pointers */
	p->Z0 = Z0;
	ReturnErrIf(p->Z0 == NULL);
	p->Td = Td;
	ReturnErrIf(p->Td == NULL);
	p->loss = loss;
	ReturnErrIf(p->loss == NULL);

	/* Create required nodes and rows (see MNA stamp above)
	 * Note: local names are created to differentiate the two v-sources
	 */
	p->rowRName = malloc(strlen(r->refdes) + 5);
	ReturnErrIf(p->rowRName == NULL);
	strcpy(p->rowRName, r->refdes);
	strcat(p->rowRName, "#in1");
	p->rowSName = malloc(strlen(r->refdes) + 5);
	ReturnErrIf(p->rowSName == NULL);
	strcpy(p->rowSName, r->refdes);
	strcat(p->rowSName, "#in2");

	/* Create required nodes and rows (see MNA stamp above) */
	p->rowK = r->pin[K];
	ReturnErrIf(p->rowK == NULL);
	p->rowJ = r->pin[J];
	ReturnErrIf(p->rowJ == NULL);
	p->rowR = matrixFindOrAddRow(r->matrix, 'i', p->rowRName);
	ReturnErrIf(p->rowR == NULL);
	p->rowM = r->pin[M];
	ReturnErrIf(p->rowM == NULL);
	p->rowL = r->pin[L];
	ReturnErrIf(p->rowL == NULL);
	p->rowS = matrixFindOrAddRow(r->matrix, 'i', p->rowSName);
	ReturnErrIf(p->rowS == NULL);
	p->nodeRK = matrixFindOrAddNode(r->matrix, p->rowR, p->rowK);
	ReturnErrIf(p->nodeRK == NULL);
	p->nodeRJ = matrixFindOrAddNode(r->matrix, p->rowR, p->rowJ);
	ReturnErrIf(p->nodeRJ == NULL);
	p->nodeKR = matrixFindOrAddNode(r->matrix, p->rowK, p->rowR);
	ReturnErrIf(p->nodeKR == NULL);
	p->nodeJR = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowR);
	ReturnErrIf(p->nodeJR == NULL);
	p->nodeRL = matrixFindOrAddNode(r->matrix, p->rowR, p->rowL);
	ReturnErrIf(p->nodeRL == NULL);
	p->nodeRM = matrixFindOrAddNode(r->matrix, p->rowR, p->rowM);
	ReturnErrIf(p->nodeRM == NULL);
	p->nodeSL = matrixFindOrAddNode(r->matrix, p->rowS, p->rowL);
	ReturnErrIf(p->nodeSL == NULL);
	p->nodeSM = matrixFindOrAddNode(r->matrix, p->rowS, p->rowM);
	ReturnErrIf(p->nodeSM == NULL);
	p->nodeLS = matrixFindOrAddNode(r->matrix, p->rowL, p->rowS);
	ReturnErrIf(p->nodeLS == NULL);
	p->nodeMS = matrixFindOrAddNode(r->matrix, p->rowM, p->rowS);
	ReturnErrIf(p->nodeMS == NULL);
	p->nodeSK = matrixFindOrAddNode(r->matrix, p->rowS, p->rowK);
	ReturnErrIf(p->nodeSK == NULL);
	p->nodeSJ = matrixFindOrAddNode(r->matrix, p->rowS, p->rowJ);
	ReturnErrIf(p->nodeSJ == NULL);
	p->nodeRR = matrixFindOrAddNode(r->matrix, p->rowR, p->rowR);
	ReturnErrIf(p->nodeRR == NULL);
	p->nodeSS = matrixFindOrAddNode(r->matrix, p->rowS, p->rowS);
	ReturnErrIf(p->nodeSS == NULL);
	p->nodeKS = matrixFindOrAddNode(r->matrix, p->rowK, p->rowS);
	ReturnErrIf(p->nodeKS == NULL);
	p->nodeJS = matrixFindOrAddNode(r->matrix, p->rowJ, p->rowS);
	ReturnErrIf(p->nodeJS == NULL);
	p->nodeLR = matrixFindOrAddNode(r->matrix, p->rowL, p->rowR);
	ReturnErrIf(p->nodeLR == NULL);
	p->nodeMR = matrixFindOrAddNode(r->matrix, p->rowM, p->rowR);
	ReturnErrIf(p->nodeMR == NULL);

	p->historyInterp = historyInterpNew(p->historyInterp,
			matrixGetHistory(r->matrix));
	ReturnErrIf(p->historyInterp == NULL);

	p->checkbreakR = checkbreakNew(p->checkbreakR, r->control, 'V');
	ReturnErrIf(p->checkbreakR == NULL);

	p->checkbreakS = checkbreakNew(p->checkbreakS, r->control, 'V');
	ReturnErrIf(p->checkbreakS == NULL);

	return 0;
}

/*===========================================================================*/

