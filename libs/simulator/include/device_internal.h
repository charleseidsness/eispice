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

#ifndef DEVICE_INTERNAL_H
#define DEVICE_INTERNAL_H

#include <data.h>

#include "device.h"

/* Device Class Defintions */
typedef int (*deviceUnconfig_)(device_ *r);
typedef int (*devicePrint_)(device_ *r);
typedef int (*deviceLoad_)(device_ *r);
typedef int (*deviceLinearize_)(device_ *r, int *linear);
typedef int (*deviceInitStep_)(device_ *r);
typedef int (*deviceStep_)(device_ *r, int *breakPoint);
typedef int (*deviceIntegrate_)(device_ *r);
typedef int (*deviceMinStep_)(device_ *r, double *minStep);
typedef int (*deviceNextStep_)(device_ *r, double *nextStep);

typedef struct _deviceClass deviceClass_;
struct _deviceClass {
	char *type;
	/* Utilities */
	deviceUnconfig_ unconfig;
	devicePrint_ print;
	/* Operating Point */
	deviceLoad_ load;
	deviceLinearize_ linearize;
	/* Transient Analysis */
	deviceInitStep_ initStep;
	deviceStep_ step;
	deviceMinStep_ minStep;
	deviceNextStep_ nextStep;
	deviceIntegrate_ integrate;
};

/* Basic Data Structure Defintion */
typedef struct _devicePrivate devicePrivate_;
struct _device {
	char *refdes;
	matrix_ *matrix;
	control_ *control;
	row_ **pin;
	int numPins;
	deviceClass_ *class;
	devicePrivate_ *private;
};

#endif
