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

#include "device_internal.h"

/* Pin Designations */
#define K			0
#define J			1
#define NP			2

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

struct _devicePrivate {
	double *R;		/* Resistance (Ohms) */
	node_ *nodeKK;
	node_ *nodeKJ;
	node_ *nodeJK;
	node_ *nodeJJ;
};

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

static int deviceClassLoad(device_ *r)
{
	devicePrivate_ *p;
	double g;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);
	
	Debug("Loading %s %s %p", r->class->type, r->refdes, r);
	
	/* Modified Nodal Analysis Stamp
	 *	  |_Vk_Vj_|		+        -
	 *	k | G  -G |		--/\/\/\--
	 *	j |-G   G |		k        j
	 */
	
	g = 1/(*p->R); /* maybe this can be moved to an init fuction someday */
	ReturnErrIf(nodeDataPlus(p->nodeKK, g));
	ReturnErrIf(nodeDataPlus(p->nodeKJ, -g));
	ReturnErrIf(nodeDataPlus(p->nodeJK, -g));
	ReturnErrIf(nodeDataPlus(p->nodeJJ, g));
	
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
	
	Info("%s -- %s %s -> %s; R = %gOhms", r->class->type, r->refdes, 
			rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			*p->R);
	
	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceResistor = {
	.type = "Resistor",
	.unconfig = NULL,
	.load = deviceClassLoad,
	.linearize = NULL,
	.initStep = NULL,
	.step = NULL,
	.minStep = NULL,
	.nextStep = NULL,
	.integrate = NULL,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceResistorConfig(device_ *r, double *resistance)
{
	devicePrivate_ *p;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins != NP);
	
	/* Copy in class pointer */
	r->class = &deviceResistor;
	
	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);
	
	/* Allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;
	
	/* Copy in parameter pointers */
	p->R = resistance;
	ReturnErrIf(p->R == NULL);
	
	/* Create required nodes and rows (see MNA stamp above) */
	p->nodeKK = matrixFindOrAddNode(r->matrix, r->pin[K], r->pin[K]);
	ReturnErrIf(p->nodeKK == NULL);
	p->nodeKJ = matrixFindOrAddNode(r->matrix, r->pin[K], r->pin[J]);
	ReturnErrIf(p->nodeKJ == NULL);
	p->nodeJK = matrixFindOrAddNode(r->matrix, r->pin[J], r->pin[K]);
	ReturnErrIf(p->nodeJK == NULL);
	p->nodeJJ = matrixFindOrAddNode(r->matrix, r->pin[J], r->pin[J]);
	ReturnErrIf(p->nodeJJ == NULL);
	
	return 0;
}

/*===========================================================================*/

