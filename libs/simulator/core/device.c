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
#include <data.h>

#include "device_internal.h"

/*===========================================================================
 |                               Device Control                              |
  ===========================================================================*/

int devicePrint(device_ *r, void *data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data != NULL);
	ReturnErrIf(r->class == NULL);
	if(r->class->print != NULL) {
		ReturnErrIf(r->class->print(r));
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceLoad(device_ *r, void *data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data != NULL);
	ReturnErrIf(r->class == NULL);
	if(r->class->load != NULL) {
		ReturnErrIf(r->class->load(r));
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceLinearize(device_ *r, int *linear)
{
	int localLinear = 1;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(linear == NULL);
	ReturnErrIf(r->class == NULL);
	
	if(r->class->linearize != NULL) {
			ReturnErrIf(r->class->linearize(r, &localLinear));
			*linear = (*linear) && (localLinear);
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceInitStep(device_ *r, void *data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data != NULL);
	ReturnErrIf(r->class == NULL);
	if(r->class->initStep != NULL) {
		ReturnErrIf(r->class->initStep(r));
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceStep(device_ *r, int *breakPoint)
{
	int localBreakPoint = 0;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(breakPoint == NULL);
	ReturnErrIf(r->class == NULL);
	
	if(r->class->step != NULL) {
		ReturnErrIf(r->class->step(r, &localBreakPoint));
		*breakPoint = (*breakPoint) || (localBreakPoint);
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceMinStep(device_ *r, double *minStep)
{
	double localMinStep = 0.0;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(minStep == NULL);
	ReturnErrIf(r->class == NULL);
	
	if(r->class->minStep != NULL) {
			ReturnErrIf(r->class->minStep(r, &localMinStep));
			if((localMinStep > 0.0) && (localMinStep < *minStep)) {
				*minStep = localMinStep;
			}
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceNextStep(device_ *r, double *nextStep)
{
	double localNextStep = 0.0;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(nextStep == NULL);
	ReturnErrIf(r->class == NULL);
	
	if(r->class->nextStep != NULL) {
			ReturnErrIf(r->class->nextStep(r, &localNextStep));
			if((localNextStep > r->control->minstep) && 
					(localNextStep < *nextStep)) {
				*nextStep = localNextStep;
			}
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int deviceIntegrate(device_ *r, void *data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data != NULL);
	ReturnErrIf(r->class == NULL);
	if(r->class->integrate != NULL) {
		ReturnErrIf(r->class->integrate(r));
	}
	
	return 0;
}

/*===========================================================================
 |                               Device Utilities                            |
  ===========================================================================*/ 

listAddReturn_ deviceCheckDuplicate(device_ *old, device_ *new)
{
	ReturnErrIf(old == NULL);
	ReturnErrIf(new == NULL);
	
	if(!strcmp(old->refdes, new->refdes)) {
		Error("%s is listed twice", new->refdes);
		return LIST_ADD_NOWHERE;
	}
	
	return LIST_ADD_NOTHERE;
}

/*===========================================================================
 |                          Constructor / Destructor                         |
  ===========================================================================*/

int deviceDestroy(device_ *r)
{
	ReturnErrIf(r == NULL);
	Debug("Destroying Device %p", r);
	
	if(r->class != NULL) {
		if(r->class->unconfig != NULL) {
			ReturnErrIf(r->class->unconfig(r));
		}
	}
	
	if(r->pin != NULL)
		free(r->pin);
	
	if(r->private != NULL)
		free(r->private);
	
	if(r->refdes != NULL)
		free(r->refdes);
	
	free(r);
	
	return 0;
}

/*---------------------------------------------------------------------------*/

device_ * deviceNew2Pins(matrix_ *matrix, control_ *control, char *refdes, 
		char *pNode, char *nNode)
{
	device_ *r = NULL;
	
	ReturnNULLIf(refdes == NULL);
	ReturnNULLIf(pNode == NULL);
	ReturnNULLIf(nNode == NULL);
	
	/* Allocate Memory */
	r = calloc(1, sizeof(device_));
	ReturnNULLIf(r == NULL);
	r->refdes = malloc(strlen(refdes) + 1);
	ReturnNULLIf(r->refdes == NULL);
	strcpy(r->refdes, refdes);
	r->matrix = matrix;
	r->control = control;
	
	/* Add Row Pointers for Pins */
	r->numPins = 2;
	r->pin = calloc(2, sizeof(row_*));
	ReturnNULLIf(r->pin == NULL);
	r->pin[0] = matrixFindOrAddRow(r->matrix, 'v', pNode);
	ReturnNULLIf(r->pin[0] == NULL, "%s.%i", refdes, 0);
	r->pin[1] = matrixFindOrAddRow(r->matrix, 'v', nNode);
	ReturnNULLIf(r->pin[1] == NULL, "%s.%i", refdes, 1);
	
	return r;
}

/*---------------------------------------------------------------------------*/

device_ * deviceNew3Pins(matrix_ *matrix, control_ *control, char *refdes, 
		char *pNode, char *nNode, char *cNode)
{
	device_ *r = NULL;
	
	ReturnNULLIf(refdes == NULL);
	ReturnNULLIf(pNode == NULL);
	ReturnNULLIf(nNode == NULL);
	ReturnNULLIf(cNode == NULL);
	
	/* Allocate Memory */
	r = calloc(1, sizeof(device_));
	ReturnNULLIf(r == NULL);
	r->refdes = malloc(strlen(refdes) + 1);
	ReturnNULLIf(r->refdes == NULL);
	strcpy(r->refdes, refdes);
	r->matrix = matrix;
	r->control = control;
	
	/* Add Row Pointers for Pins */
	r->numPins = 3;
	r->pin = calloc(3, sizeof(row_*));
	ReturnNULLIf(r->pin == NULL);
	r->pin[0] = matrixFindOrAddRow(r->matrix, 'v', pNode);
	ReturnNULLIf(r->pin[0] == NULL, "%s.%i", refdes, 0);
	r->pin[1] = matrixFindOrAddRow(r->matrix, 'v', nNode);
	ReturnNULLIf(r->pin[1] == NULL, "%s.%i", refdes, 1);
	r->pin[2] = matrixFindOrAddRow(r->matrix, 'v', cNode);
	ReturnNULLIf(r->pin[2] == NULL, "%s.%i", refdes, 2);
	
	return r;
}

/*---------------------------------------------------------------------------*/

device_ * deviceNew4Pins(matrix_ *matrix, control_ *control, char *refdes,
		char *pNodeLeft, char *nNodeLeft, char *pNodeRight, char *nNodeRight)
{
	device_ *r = NULL;
	
	ReturnNULLIf(refdes == NULL);
	ReturnNULLIf(pNodeLeft == NULL);
	ReturnNULLIf(nNodeLeft == NULL);
	ReturnNULLIf(pNodeRight == NULL);
	ReturnNULLIf(nNodeRight == NULL);
	
	/* Allocate Memory */
	r = calloc(1, sizeof(device_));
	ReturnNULLIf(r == NULL);
	r->refdes = malloc(strlen(refdes) + 1);
	ReturnNULLIf(r->refdes == NULL);
	strcpy(r->refdes, refdes);
	r->matrix = matrix;
	r->control = control;
	
	/* Add Row Pointers for Pins */
	r->numPins = 4;
	r->pin = calloc(4, sizeof(row_*));
	ReturnNULLIf(r->pin == NULL);
	r->pin[0] = matrixFindOrAddRow(r->matrix, 'v', pNodeLeft);
	ReturnNULLIf(r->pin[0] == NULL, "%s.%i", refdes, 0);
	r->pin[1] = matrixFindOrAddRow(r->matrix, 'v', nNodeLeft);
	ReturnNULLIf(r->pin[1] == NULL, "%s.%i", refdes, 1);
	r->pin[2] = matrixFindOrAddRow(r->matrix, 'v', pNodeRight);
	ReturnNULLIf(r->pin[2] == NULL, "%s.%i", refdes, 2);
	r->pin[3] = matrixFindOrAddRow(r->matrix, 'v', nNodeRight);
	ReturnNULLIf(r->pin[3] == NULL, "%s.%i", refdes, 3);
	
	return r;
}

/*---------------------------------------------------------------------------*/

device_ * deviceNewNPins(matrix_ *matrix, control_ *control, char *refdes,
		char *nodes[], int nNodes)
{
	device_ *r = NULL;
	int i;
	
	ReturnNULLIf(refdes == NULL);
	ReturnNULLIf(nodes == NULL);
	for(i = 0; i < nNodes; i++) {
		ReturnNULLIf(nodes[i] == NULL);
	}
	
	/* Allocate Memory */
	r = calloc(1, sizeof(device_));
	ReturnNULLIf(r == NULL);
	r->refdes = malloc(strlen(refdes) + 1);
	ReturnNULLIf(r->refdes == NULL);
	strcpy(r->refdes, refdes);
	r->matrix = matrix;
	r->control = control;
	
	/* Add Row Pointers for Pins */
	r->numPins = nNodes;
	r->pin = calloc(nNodes, sizeof(row_*));
	ReturnNULLIf(r->pin == NULL);
	for(i = 0; i < nNodes; i++) {
		r->pin[i] = matrixFindOrAddRow(r->matrix, 'v', nodes[i]);
		ReturnNULLIf(r->pin[i] == NULL, "%s.%i", refdes, 0);
	}
	
	return r;
}

/*===========================================================================*/

