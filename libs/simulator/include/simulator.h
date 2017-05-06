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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#define SIMULATOR_MAJOR_VERSION		2
#define SIMULATOR_MINOR_VERSION		4

typedef struct _simulator simulator_;

int simulatorRunTransient(simulator_ *r,
	double tstep,	/* Seconds */
	double tstop,	/* Seconds */
	double tmax,	/* Seconds (0.0 for none) */
	int restart,	/* Clear out the data and restart the simulator */
	double *data[],
	char **variables[],
	int *numPoints,
	int *numVariables);
int simulatorRunOperatingPoint(simulator_ *r,
	double *data[],
	char **variables[],
	int *numPoints,
	int *numVariables);

int simulatorAddResistor(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	double *resistance); /* Ohms */
	
int simulatorAddCapacitor(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	double *capacitance); /* Farads */
	
int simulatorAddInductor(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	double *inductance); /* Henrys */
	
int simulatorAddNonlinearSource(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	char type,		/* Either 'i' or 'v' */
	char *equation);
	
int simulatorAddSource(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	char type,
	double *dc,		/* Volts / Amps (can be NULL) */
	char stimulus,	/*-------------*
					 | none  | 0x0 |
					 | pulse | 'p' |
					 | sin   | 's' |
					 | exp   | 'e' |
					 | pwl   | 'l' |
					 | pwc   | 'c' |
					 | sffm  | 'f' |
					 *-------------*/
	double *args[7]);
	
int simulatorAddTLine(simulator_ *r,
	char *refdes,
	char *pNodeLeft,
	char *nNodeLeft,
	char *pNodeRight,
	char *nNodeRight,
	double *Z0,
	double *Td,
	double *loss);


int simulatorAddTLineW(simulator_ *r,
	char *refdes,
	char *nodes[], int nNodes,
	int *M,			/* order of approixmation */
	double *len, 	/* Length of the T-Line in inches */
	double **L0,	/* DC inductance matrix per unit length */
	double **C0,	/* DC capacitance matrix per unit length */
	double **R0,	/* DC resistance matrix per unit length */
	double **G0,	/* DC shunt conductance matrix per unit length */
	double **Rs,	/* Skin-effect resistance matrix per unit length */
	double **Gd,	/* Dielectric-loss conductance matrix per unit length */
	double *fgd,		/* Cut-off for the Dielectric Loss */
	double *fK		/* Cut-off frequency */
);

int simulatorAddVoltageControlledCap(simulator_ *r,
	char *refdes,
	char *pNode,	/* positive node */
	char *nNode,	/* negative node */
	char *cNode);	/* control node */
	
int simulatorAddVICurve(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	double **vi,
	int *viLength,
	char viType,
	double **ta,
	int *taLength,
	char taType);

typedef int (*simulatorCallback_)(double *xN, void *private);
int simulatorAddCallbackSource(simulator_ *r,
	char *refdes,
	char *pNode,
	char *nNode,
	char type,
	char *variables[],
	double values[],
	double derivs[],
	int numVariables,
	simulatorCallback_ callback,
	void *private);

int simulatorInfo(void);
int simulatorPrintDevices(simulator_ *r);

int simulatorDestroy(simulator_ **r);
simulator_ * simulatorNew(simulator_ *r);

#endif
