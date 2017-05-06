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

#ifndef DEVICE_H
#define DEVICE_H

#include <data.h>

#include "matrix.h"
#include "control.h"

typedef struct _device device_;

int deviceResistorConfig(device_ *r, double *resistance);
int deviceInductorConfig(device_ *r, double *inductance);
int deviceCapacitorConfig(device_ *r, double *capacitance);

int deviceTLineConfig(device_ *r, double *Z0, double *Td, double *loss);
int deviceTLineWConfig(device_ *r, int *M, double *len,
		double **L0, double **C0, double **R0, double **G0, double **Rs,
		double **Gd, double *fgd, double *fK);

int deviceCurrentSourceConfig(device_ *r, double *dc, char type, 
		double *args[7]);
int deviceVoltageSourceConfig(device_ *r, double *dc, char type, 
		double *args[7]);

int deviceNonlinearVoltageConfig(device_ *r, char *equation);
int deviceNonlinearCurrentConfig(device_ *r, char *equation);
int deviceNonlinearCapacitorConfig(device_ *r, char *equation);

int deviceVICurveConfig(device_ *r, double **vi, int *viLength, char viType,
		double **ta, int *taLength, char taType);

typedef int (*deviceCallback_)(double *xN, void *private);
int deviceCallbackVoltageConfig(device_ *r, char *vars[], double values[],
		double derivs[], int numVars, deviceCallback_ callback, void *private);
int deviceCallbackCurrentConfig(device_ *r, char *vars[], double values[],
		double derivs[], int numVars, deviceCallback_ callback, void *private);

/* Operating Point */
int deviceLoad(device_ *r, void *data);
int deviceLinearize(device_ *r, int *linear);

/* Transient Analysis */
int deviceInitStep(device_ *r, void *data);
int deviceStep(device_ *r, int *breakPoint);
int deviceMinStep(device_ *r, double *minStep);
int deviceNextStep(device_ *r, double *nextStep);
int deviceIntegrate(device_ *r, void *data);

listAddReturn_ deviceCheckDuplicate(device_ *old, device_ *new);

int devicePrint(device_ *r, void *data);
int deviceDestroy(device_ *r);
device_ * deviceNew2Pins(matrix_ *matrix, control_ *control, char *refdes, 
		char *pNode, char *nNode);
device_ * deviceNew3Pins(matrix_ *matrix, control_ *control, char *refdes, 
		char *pNode, char *nNode, char *cNode);
device_ * deviceNew4Pins(matrix_ *matrix, control_ *control, char *refdes,
		char *pNodeLeft, char *nNodeLeft, char *pNodeRight, char *nNodeRight);
device_ * deviceNewNPins(matrix_ *matrix, control_ *control, char *refdes,
		char *nodes[], int nNodes);

#endif
