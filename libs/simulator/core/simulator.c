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

#include <log.h>
#include <ctype.h>
#include <data.h>
#include <calc.h>

#include "simulator.h"
#include "device.h"
#include "history.h"

struct _simulator {
	list_ *devices;
	matrix_ *matrix;
	control_ *control;
	int locked;	/* Indicates that the matrix has been initialise */
				/* TODO: Add a re-initialise function to can remove the
				 * no re-run requirement.
				 */
};

/*===========================================================================
 |                                  Analysis                                 |
  ===========================================================================*/

static int simulatorSolve(simulator_ *r, int interationLimit)
{
	int count = 0;
	int linear;

	ReturnErrIf(r == NULL);
	ReturnErrIf(interationLimit < 1);

	ReturnErrIf(matrixSolve(r->matrix));

	while(count++ < interationLimit) {
		linear = 1;
		ReturnErrIf(listExecute(r->devices, (listExecute_)deviceLinearize,
				&linear))
		if(linear)
			break;
		ReturnErrIf(matrixSolveAgain(r->matrix));
	};

	if(count == interationLimit) {
		Warn("Failed to Linearize Circuit");
	}

	return count;
}

/*---------------------------------------------------------------------------*/

#define Min3(x,y,z) ((x < y) ? ((z < x) ? z : x) : ((z < y) ? z : y))

int simulatorRunTransient(simulator_ *r,
		double tstep, double tstop, double tmax, int restart,
		double *data[], char **variables[], int *numPoints, int *numVariables)
{
	int linCount = 0;      /* Linearization loop count */
	double maxStep = 0.0;  /* maximum value of the current step */
	double thisStep = 0.0; /* current step size */
	double lteStep = 0.0;  /* step recomendation based on lte calculations
						   	from devices that use numerical integration */
	double prevTime = 0.0; /* time at previous step */
	int breakPoint = 0;	   /* Indicates currently servicing a break-point */
	double oldMinstep;

	ReturnErrIf(r == NULL);

	/* Set default tmax if it's 0.0 */
	if(tmax == 0.0) {
		tmax = tstop / 50;
		tmax = (tstep < tmax) ? tstep : tmax;
	}

	/* These are used by sources to set some default values */
	r->control->tstep = tstep;
	r->control->tstop = tstop;

	/* The default value of minstep is -1, indicating it hasn't been set */
	oldMinstep = r->control->minstep;
	if(r->control->minstep < 0) {
		r->control->minstep = tstep * 5e-5;
	}

	/* First step is chosen based on tran parameters, the divide by 100
	* is kind of arbitrary, it's taken from Spice 3.
	*/
	maxStep = ((tstep < tmax) ? tstep : tmax) / 100;

	if(restart || (r->control->time == 0.0)) {

		/* Initialize the matrices if they haven't been already */
		if(!r->locked) {
			ReturnErrIf(matrixInitialize(r->matrix, r->control));
			r->locked = -1;
		}

		/* Clear out any data that may be in the matrices */
		ReturnErrIf(matrixClear(r->matrix));

		/* Load Matrix A with Values from Devices */
		ReturnErrIf(listExecute(r->devices, (listExecute_)deviceLoad, NULL));

		/* Solve Matrices */
		linCount = simulatorSolve(r, r->control->itl1);
		ReturnErrIf((linCount < 0) || (linCount > r->control->itl1));

		/* Initialize the Devices for a Time Stepping */
		ReturnErrIf(listExecute(r->devices, (listExecute_)deviceInitStep, NULL));

		/* For first step set integration order to the maximum */
		r->control->integratorOrder = r->control->maxorder;

		/* Set the current time to 0 */
		r->control->time = 0.0;
		prevTime = 0.0;
	} else {
		prevTime = r->control->time;
	}

	while(r->control->time < tstop) {
		/* Record the last step, also record break-point status */
		ReturnErrIf(matrixRecord(r->matrix, r->control->time,
				(breakPoint ? HISTORY_FLAG_BRKPOINT : 0)));

		/* Find out if any devices are going to force the time step to
		 * something smaller, this is a look-ahead function, i.e. before
		 * the results have been calculated, the DeviceMinStep function
		 * below is a look-back function
		 */
		thisStep = maxStep;
		ReturnErrIf(listExecute(r->devices, (listExecute_)deviceNextStep,
				&thisStep));

		while(1) {
			r->control->time = prevTime + thisStep;
			if (r->control->time > tstop) {
				thisStep = tstop - prevTime;
				r->control->time = tstop;
			}
			Debug("time = %e", r->control->time);

			/* Step all of the devices in time, and check to see if any
			 * of them want to declare this step a break-point.
			 */
			breakPoint = 0;
			ReturnErrIf(listExecute(r->devices, (listExecute_)deviceStep,
					&breakPoint));
			if(breakPoint) {
				Debug("Break");
				r->control->integratorOrder = 1;
			}
			/* Devices that use integration are processed seperatally so
			 * they can pick up the break point order change if there was
			 * one.
			 */
			ReturnErrIf(listExecute(r->devices, (listExecute_)deviceIntegrate,
					NULL));

			/* Solve the matrices */
			linCount = simulatorSolve(r, r->control->itl4);
			ReturnErrIf(linCount < 0);

			/* check to see if we reached linearization */
			if(linCount < r->control->itl4) {
				/* get the recomened step size from devices that have ODEs */
				lteStep = tmax; /* need a large seed for the min check */
				ReturnErrIf(listExecute(r->devices,
						(listExecute_)deviceMinStep, &lteStep));
				if(lteStep < (0.9 * thisStep)) {
					Debug("lteStep = %e", lteStep);
					ReturnErrIf(lteStep < (tstep * 1e-9),
							"Timestep %es is too Small at %es",
							lteStep, r->control->time);
					r->control->integratorOrder = 1;
					thisStep = lteStep;
				} else {
					break;
				}
			} else {
				/* if we didn't linearize reduce the timestep  and
				 * integrator order and try again.
				 */
				Warn("Failed to linearize at %gs, trying smaller step-size.",
						r->control->time);
				thisStep = thisStep / 8;
				ReturnErrIf(thisStep < (tstep * 1e-9),
							"Timestep %es is too Small at %es",
							r->control->time, thisStep);
				r->control->integratorOrder =
						ControlIntegratorOrderDown(r->control);
				maxStep = maxStep / 8; /* used to limit next step size */
			}

			/* Reset the solution matrix to the one from the preveious step */
			ReturnErrIf(matrixRecall(r->matrix));
		}

		/* Get ready for the next step */
		/* Increase integration order */
		r->control->integratorOrder = ControlIntegratorOrderUp(r->control);
		/* store the current time */
		prevTime = r->control->time;
		/* Set the maximum size of the next step */
		if(breakPoint) {
			/* if it's a break-point reduce the step size to pick up more
			 * detail.
			 */
			maxStep = Min3(lteStep, 0.1*maxStep, tmax);
		} else {
			maxStep = Min3(lteStep, 2*maxStep, tmax);
		}

	}

	/* Record final step */
	ReturnErrIf(matrixRecord(r->matrix, r->control->time, HISTORY_FLAG_END));

	/* reset minstep value */
	r->control->minstep = oldMinstep;

	/* Make a copy of the results to send back to the caller */
	ReturnErrIf(matrixGetSolution(r->matrix, data, variables, numPoints,
			numVariables));

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorRunOperatingPoint(simulator_ *r,
		double *data[], char **variables[], int *numPoints, int *numVariables)
{
	int linCount;

	ReturnErrIf(r == NULL);

	/* Initialize the matrices if they haven't been already */
	if(!r->locked) {
		ReturnErrIf(matrixInitialize(r->matrix, r->control));
		r->locked = -1;
	}
	/* Clear out any data that may be in the matrices */
	ReturnErrIf(matrixClear(r->matrix));

	/* Load Matrix A with Values from Devices */
	ReturnErrIf(listExecute(r->devices, (listExecute_)deviceLoad, NULL));

	/* Solve Matraces */
	linCount = simulatorSolve(r, r->control->itl1);
	ReturnErrIf((linCount < 0) || (linCount > r->control->itl1));

	/* Store Data (time is 0 and no break-point) */
	ReturnErrIf(matrixRecord(r->matrix, 0.0, HISTORY_FLAG_END));

	/* Make a copy of the results to send back to the caller */
	ReturnErrIf(matrixGetSolution(r->matrix, data, variables, numPoints,
			numVariables));

	return 0;
}

/*===========================================================================
 |                               Device Creation                             |
  ===========================================================================*/

int simulatorAddResistor(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		double *resistance)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	ReturnErrIf(deviceResistorConfig(device, resistance));

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddCapacitor(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		double *capacitance)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	ReturnErrIf(deviceCapacitorConfig(device, capacitance));

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddInductor(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		double *inductance)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	ReturnErrIf(deviceInductorConfig(device, inductance));

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddNonlinearSource(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		char type, char *equation)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	if(tolower(type) == 'i') {
		ReturnErrIf(deviceNonlinearCurrentConfig(device, equation));
	} else if(tolower(type) == 'v') {
		ReturnErrIf(deviceNonlinearVoltageConfig(device, equation));
	} else if(tolower(type) == 'c') {
		ReturnErrIf(deviceNonlinearCapacitorConfig(device, equation));
	} else {
		ReturnErr("Nonlinear Source type must be either i, v or c, not %c", type);
	}

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddCallbackSource(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		char type, char *variables[], double values[], double derivs[],
		int numVariables, simulatorCallback_ callback, void*private)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	if(tolower(type) == 'i') {
		ReturnErrIf(deviceCallbackCurrentConfig(device, variables, values,
				derivs, numVariables, 	callback, private));
	} else if(tolower(type) == 'v') {
		ReturnErrIf(deviceCallbackVoltageConfig(device, variables, values,
				derivs, numVariables, callback, private));
	} else {
		ReturnErr("Call-Back Source type must be either i or v, not %c", type);
	}

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddSource(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		char type, double *dc, char stimulus, double *args[7])
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	/* Configure the Device */
	if(tolower(type) == 'i') {
		ReturnErrIf(deviceCurrentSourceConfig(device, dc, stimulus, args));
	} else if(tolower(type) == 'v') {
		ReturnErrIf(deviceVoltageSourceConfig(device, dc, stimulus, args));
	} else {
		ReturnErr("Nonlinear Source type must be either i or v, not %c", type);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddTLine(simulator_ *r,
		char *refdes,
		char *pNodeLeft, char *nNodeLeft, char *pNodeRight, char *nNodeRight,
		double *Z0, double *Td, double *loss)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew4Pins(r->matrix, r->control, refdes, pNodeLeft,
			nNodeLeft, pNodeRight, nNodeRight);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	ReturnErrIf(deviceTLineConfig(device, Z0, Td, loss));

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddTLineW(simulator_ *r,
	char *refdes,
	char *nodes[], int nNodes,
	int *M, double *len,
	double **L0, double **C0, double **R0, double **G0, double **Rs,
	double **Gd, double *fgd, double *fK)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNewNPins(r->matrix, r->control, refdes, nodes, nNodes);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	ReturnErrIf(deviceTLineWConfig(device, M, len, L0, C0, R0, G0, Rs,
			Gd, fgd, fK));

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*---------------------------------------------------------------------------*/

int simulatorAddVICurve(simulator_ *r,
		char *refdes,
		char *pNode, char *nNode,
		double **vi, int *viLength, char viType,
		double **ta, int *taLength, char taType)
{
	device_ *device;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->locked, "Can't add a device to a simulator that has run.");

	/* Create a new device and add it to the device list*/
	device = deviceNew2Pins(r->matrix, r->control, refdes, pNode, nNode);
	ReturnErrIf(device == NULL);

	/* Configure the Device */
	ReturnErrIf(deviceVICurveConfig(device, vi, viLength, viType, ta, taLength,
			taType));

	/* Add device to the device list */
	ReturnErrIf(listAdd(r->devices, device, (listAdd_)deviceCheckDuplicate))

	return 0;
}

/*===========================================================================
 |                                 Utilities                                 |
  ===========================================================================*/

int simulatorInfo(void)
{
	Info("==================================================================");
	Info("eispice Simulation Engine %i.%i", SIMULATOR_MAJOR_VERSION,
			SIMULATOR_MINOR_VERSION);
	Info("Compiled " __DATE__ " at " __TIME__);
	Info("(c) 2006 Cooper Street Innovations Inc.");
	Info("==================================================================");
	dataInfo();
	Info("==================================================================");
	calcInfo();
	Info("==================================================================");
	LogInfo();
	Info("==================================================================");
	Info("SuperLU (Version 3.0)\n");
	Info("Copyright (c) 2003, The Regents of the University of California,");
	Info("through Lawrence Berkeley National Laboratory (subject to receipt");
	Info("of any required approvals from U.S. Dept. of Energy)");
	Info(" ");
	Info("All rights reserved.");
	Info(" ");
	Info("Redistribution and use in source and binary forms, with or without");
	Info("modification, are permitted provided that the following conditions");
	Info("are met:");
	Info(" ");
	Info("(1) Redistributions of source code must retain the above copyright");
	Info("notice, this list of conditions and the following disclaimer.");
	Info("(2) Redistributions in binary form must reproduce the above");
	Info("copyright notice, this list of conditions and the following");
	Info("disclaimer in the documentation and/or other materials provided");
	Info("with the distribution.");
	Info("(3) Neither the name of Lawrence Berkeley National Laboratory, U.S.");
	Info("Dept. of Energy nor the names of its contributors may be used to");
	Info("endorse or promote products derived from this software without");
	Info("specific prior written permission.");
	Info(" ");
	Info("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS");
	Info("\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT");
	Info("LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS");
	Info("FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE");
	Info("COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,");
	Info("INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING");
	Info(",BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;");
	Info("LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER");
	Info("CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT");
	Info("LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN");
	Info("ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE");
	Info("POSSIBILITY OF SUCH DAMAGE.");
	Info("==================================================================");
	Info("------------------------ Netlib Packages -------------------------");
	Info("Select Functions from the BLAS Library -- Public Domain License");
	Info("http://www.netlib.org/blas/faq.html#2");
	Info("------------------------------------------------------------------");
	Info("Select Functions from the Cephes Library -- Public Domain License");
	Info("Copyright 2000 by Stephen L. Moshier");
	Info("http://www.netlib.org/cephes/readme");
	Info("------------------------------------------------------------------");
	Info("Select Functions from the LAPACK Library");
	Info("Copyright (c) 1992-2007 The University of Tennessee.  All rights");
	Info("reserved. Redistribution and use in source and binary forms, with");
	Info("or without modification, are permitted provided that the following");
	Info("conditions are met:");
	Info(" ");
	Info("(1) Redistributions of source code must retain the above copyright");
	Info("notice, this list of conditions and the following disclaimer.");
	Info(" ");
	Info("(2) Redistributions in binary form must reproduce the above copyright");
	Info("notice, this list of conditions and the following disclaimer listed");
	Info("in this license in the documentation and/or other materials");
	Info("provided with the distribution.");
	Info(" ");
	Info("(3) Neither the name of the copyright holders nor the names of its");
	Info("contributors may be used to endorse or promote products derived from");
	Info("this software without specific prior written permission.");
	Info(" ");
	Info("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS");
	Info("\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT");
	Info("LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS");
	Info("FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE");
	Info("COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,");
	Info("INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING");
	Info(",BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;");
	Info("LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER");
	Info("CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT");
	Info("LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN");
	Info("ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE");
	Info("POSSIBILITY OF SUCH DAMAGE.");
	Info("------------------------------------------------------------------");
	Info("Select Functions from the Toms Library");
	Info("Copyright 1998 Association for Computing Machinery, Inc.");
	Info("http://www.acm.org/pubs/toc/CRnotice.html");
	Info("==================================================================");

	return ((SIMULATOR_MAJOR_VERSION << 16) + SIMULATOR_MINOR_VERSION);
}

/*---------------------------------------------------------------------------*/

int simulatorPrintDevices(simulator_ *r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(listExecute(r->devices, (listExecute_)devicePrint, NULL));
	return 0;
}

/*===========================================================================
 |                          Constructor / Destructor                         |
  ===========================================================================*/

int simulatorDestroy(simulator_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(*r == NULL);
	Debug("Destroying simulator %p", *r);

	if((*r)->devices != NULL) {
		if(listDestroy(&(*r)->devices, (listDestroy_)deviceDestroy)) {
			Warn("Error destroying device list");
		}
	}

	if((*r)->matrix != NULL) {
		if(matrixDestroy(&(*r)->matrix)) {
			Warn("Error destroying matrix");
		}
	}

	if((*r)->control != NULL) {
		if(controlDestroy(&((*r)->control))) {
			Warn("Error destroying control");
		}
	}

	free(*r);
	*r = NULL;
	return 0;
}

/*---------------------------------------------------------------------------*/

simulator_ * simulatorNew(simulator_ *r)
{
	ReturnNULLIf(NULL != 0);
	ReturnNULLIf(r != NULL);
	r = calloc(1, sizeof(simulator_));
	ReturnNULLIf(r == NULL);

	Debug("Creating simulator %p", r);

	r->locked = 0;

	/* Create Device List */
	r->devices = listNew(r->devices);
	ReturnNULLIf(r->devices == NULL);

	/* Create Matrix Object */
	r->matrix = matrixNew(r->matrix);
	ReturnNULLIf(r->matrix == NULL);

	/* Create Control Object */
	r->control = controlNew(r->control);
	ReturnNULLIf(r->control == NULL);

	return r;
}

/*===========================================================================*/

