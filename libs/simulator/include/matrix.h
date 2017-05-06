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

#ifndef MATRIX_H
#define MATRIX_H

#include <data.h>
#include "row.h"
#include "node.h"
#include "control.h"

typedef struct _matrix matrix_;

int matrixSolve(matrix_ *r);
int matrixSolveAgain(matrix_ *r);

node_ * matrixFindOrAddNode(matrix_ *r, row_ *row, row_ *col);
row_ * matrixFindOrAddRow(matrix_ *r, char rowType, char *rowName);

int matrixWriteRawfile(matrix_ *r, control_ *control);

int matrixClear(matrix_ *r);
int matrixRecall(matrix_ *r);
int matrixRecord(matrix_ *r, double time, unsigned int flag);
list_ * matrixGetHistory(matrix_ *r);
int matrixGetSolution(matrix_ *r, double *data[], char **variables[], 
		int *numPoints, int *numVariables);

int matrixInitialize(matrix_ *r, control_ *control);

int matrixDestroy(matrix_ **r);
matrix_ * matrixNew(matrix_ *r);


#endif
