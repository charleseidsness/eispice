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
#include <superlu.h>

#include "matrix.h"
#include "history.h"

typedef int (*matrixLibraryFunction)(matrix_ *r);
typedef struct _matrixLibrary matrixLibrary_;
struct _matrix {
	list_ *nodes;
	list_ *rows;
	list_ *history;
	/* Data */
	double *A;
	int *aRow;
	int *aColStart;
	double *X;
	double *B;
	int lenA;
	int lenXB;
	int index;
	/* LU Library */
	char *name;
	matrixLibraryFunction unconfig;
	matrixLibraryFunction solve;
	matrixLibraryFunction solveAgain;
	matrixLibrary_ *library;
};

/*===========================================================================
 |                       SuperLU Library Interface                           |
  ===========================================================================*/

static char *matrixNameSuperLU = "SuperLU";

struct _matrixLibrary {
	SuperMatrix A;
	SuperMatrix B;
	SuperMatrix X;
	SuperMatrix L;
	SuperMatrix U;
	int *perm_r;
    int *perm_c;
	int *etree;
	char equed;
	int info;
	double *R;
	double *C;
	double rpg;
	double rcond;
	double *ferr;
	double *berr;
	mem_usage_t mem_usage;
	superlu_options_t control;
    SuperLUStat_t stat;
	int firstPass;
};

/*---------------------------------------------------------------------------*/

static int matrixSolveSuperLU(matrix_ *r)
{
	matrixLibrary_ *p;
	
	ReturnErrIf(r == NULL);
	p = r->library;
	ReturnErrIf(p == NULL);
	
	/* Set the control back to full factorization (if it was changed) */
	p->control.Fact = DOFACT;
	
	if(p->firstPass) {
		p->firstPass = 0;
	} else {
		/* Free the LU Matrices so we can use them */
		Destroy_SuperNode_Matrix(&p->L);
		Destroy_CompCol_Matrix(&p->U);
	}
	
	/* Check out the SuperLU Header files and the SuperLU User's manual
	 * to make sense of this mess...
	 */
	dgssvx(&p->control, &p->A, p->perm_c, p->perm_r, p->etree, &p->equed, 
			p->R, p->C, &p->L, &p->U, NULL, 0, &p->B, &p->X, &p->rpg, 
			&p->rcond, p->ferr, p->berr, &p->mem_usage, &p->stat, &p->info);
	
	ReturnErrIf(p->info < 0, "SuperLU: %ith arg had illegal value", -p->info);
	if(p->info > 0) {
		ReturnErrIf((p->info <= p->A.ncol), "SuperLU: U is singular");
		ReturnErrIf((p->info == (p->A.ncol + 1)), 
				"SuperLU: RCOND is singular to working precision");
		ReturnErrIf((p->info > (p->A.ncol + 1)), 
				"SuperLU: memory allocation error");
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int matrixSolveAgainSuperLU(matrix_ *r)
{
	matrixLibrary_ *p;
	
	ReturnErrIf(r == NULL);
	p = r->library;
	ReturnErrIf(p == NULL);
	
	/* Set the SuperLU control to use the same pattern and row permitations */
	p->control.Fact = SamePattern_SameRowPerm;
	
	/* Check out the SuperLU Header files and the SuperLU User's manual
	 * to make sense of this mess...
	 */
	dgssvx(&p->control, &p->A, p->perm_c, p->perm_r, p->etree, &p->equed, 
			p->R, p->C, &p->L, &p->U, NULL, 0, &p->B, &p->X, &p->rpg, 
			&p->rcond, p->ferr, p->berr, &p->mem_usage, &p->stat, &p->info);
	
	ReturnErrIf(p->info < 0, "SuperLU: %ith arg had illegal value", -p->info);
	if(p->info > 0) {
		ReturnErrIf((p->info <= p->A.ncol), "SuperLU: U is singular");
		ReturnErrIf((p->info == (p->A.ncol + 1)), 
				"SuperLU: RCOND is singular to wokring precision");
		ReturnErrIf((p->info > (p->A.ncol + 1)), 
				"SuperLU: memory allocation error");
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int matrixUnconfigSuperLU(matrix_ *r)
{
	matrixLibrary_ *p;
	
	ReturnErrIf(r == NULL);
	p = r->library;
	ReturnErrIf(p == NULL);
	
	Debug("Unconfiguring Solution %p", r);
	
	SUPERLU_FREE(p->perm_r);
	SUPERLU_FREE(p->perm_c);
	SUPERLU_FREE(p->etree);
	SUPERLU_FREE(p->R);
    SUPERLU_FREE(p->C);
	SUPERLU_FREE(p->ferr);
    SUPERLU_FREE(p->berr);
	Destroy_CompCol_Matrix(&p->A);
	/* Setting these to NULL keeps the matrix object from trying to free
	 * them, as they were already freed by SuperLU.
	 */
	r->A = NULL;
	r->aRow = NULL;
	r->aColStart = NULL;
	Destroy_SuperMatrix_Store(&p->B);
	r->B = NULL;
	Destroy_SuperMatrix_Store(&p->X);
	r->X = NULL;
	StatFree(&p->stat);
	if(!p->firstPass) {
		Destroy_SuperNode_Matrix(&p->L);
		Destroy_CompCol_Matrix(&p->U);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

static int matrixInitializeSuperLU(matrix_ *r, control_ *control)
{
	matrixLibrary_ *p;
	
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->library != NULL);
	
	Debug("Configuring Solution %p", r);
	
	r->name = matrixNameSuperLU;
	r->unconfig = matrixUnconfigSuperLU;
	r->solve = matrixSolveSuperLU;
	r->solveAgain = matrixSolveAgainSuperLU;
	
	r->library = calloc(1, sizeof(matrixLibrary_));
	ReturnErrIf(r->library == NULL);
	
	p = r->library;
	
	/* Create solution A in the format expected by SuperLU. */
	dCreate_CompCol_Matrix(&p->A, r->lenXB, r->lenXB, r->lenA, r->A,
			r->aRow, r->aColStart, SLU_NC, SLU_D, SLU_GE);
	
	/* Create right-hand solution matrix X. */
	dCreate_Dense_Matrix(&p->B, r->lenXB, 1, r->B, r->lenXB,
			SLU_DN, SLU_D, SLU_GE);
	
	dCreate_Dense_Matrix(&p->X, r->lenXB, 1, r->X, r->lenXB,
			SLU_DN, SLU_D, SLU_GE);
	
	/* Create permitation matrices. */
	p->perm_r = intMalloc(r->lenXB);
	ReturnErrIf(p->perm_r == NULL);
	p->perm_c = intMalloc(r->lenXB);
	ReturnErrIf(p->perm_r == NULL);
	p->etree = intMalloc(r->lenXB);
	ReturnErrIf(p->etree == NULL);
	p->R = (double *) SUPERLU_MALLOC(p->A.nrow * sizeof(double));
	ReturnErrIf(p->R == NULL);
	p->C = (double *) SUPERLU_MALLOC(p->A.ncol * sizeof(double));
	ReturnErrIf(p->C == NULL);
	p->ferr = (double *) SUPERLU_MALLOC(1 * sizeof(double));
	ReturnErrIf(p->ferr == NULL);
	p->berr = (double *) SUPERLU_MALLOC(1 * sizeof(double));
	ReturnErrIf(p->berr == NULL);
	
	/* Set the default input control. */
	/* TODO: Make these configurable with an control command */
	p->control.Fact = DOFACT;
	p->control.Equil = NO;
	p->control.ColPerm = COLAMD;
	p->control.Trans = NOTRANS;
	p->control.IterRefine = NOREFINE;
	p->control.PrintStat = NO;
	p->control.SymmetricMode = NO;
	p->control.DiagPivotThresh = 1.0;
	p->control.PivotGrowth = NO;
	p->control.ConditionNumber = NO;
	
	/* Initialize the statistics variables. */
    StatInit(&p->stat);
	
	/* Ready for first pass (used to know when to free the LU Matrices) */
	p->firstPass = -1;
	
	return 0;
}

/*===========================================================================
 |                            Solve Ab=x for b                               |
  ===========================================================================*/

int matrixSolve(matrix_ *r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->solve == NULL);
	ReturnErrIf(r->solve(r));
	return 0;
}

/*---------------------------------------------------------------------------*/

int matrixSolveAgain(matrix_ *r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->solveAgain == NULL);
	if(r->solveAgain != NULL) {
		ReturnErrIf(r->solveAgain(r));
	} else if(r->solve != NULL) {
		ReturnErrIf(r->solve(r));
	} else {
		ReturnErr("No solve function defined");
	}
	return 0;
}

/*===========================================================================
 |                             Build Matrix A                                |
  ===========================================================================*/

node_ * matrixFindOrAddNode(matrix_ *r, row_ *row, row_ *col)
{
	nodeIndex_ index;
	node_ *node;
	ReturnNULLIf(r == NULL);
	ReturnNULLIf(row == NULL);
	ReturnNULLIf(col == NULL);
	
	if((row == &gndRow) || (col == &gndRow)) {
		return &gndNode;
	}
	
	index.row = rowGetIndex(row);
	ReturnNULLIf(index.row < 0);
	index.col = rowGetIndex(col);
	ReturnNULLIf(index.col < 0);
	
	ReturnNULLIf(listFind(r->nodes, (void*)&index, (listFind_)nodeCompare, 
			(void*)&node));
	
	/* If can't find the node then create it */
	if(node == NULL) {
		node = nodeNew(index);
		ReturnNULLIf(listAdd(r->nodes, node, (listAdd_)nodeAdd));
	}
	
	return node;
}

/*---------------------------------------------------------------------------*/

row_ * matrixFindOrAddRow(matrix_ *r, char rowType, char *rowName)
{
	rowName_ name;
	row_ *row;
	int index;
	ReturnNULLIf(r == NULL);
	ReturnNULLIf(rowName == NULL);
	
	name.type = rowType;
	name.name = rowName;
	
	ReturnNULLIf(listFind(r->rows, (void*)&name, (listFind_)rowCompare, 
			(void*)&row));
	
	/* If can't find the row then create it */
	if(row == NULL) {
		index = listLength(r->rows);
		ReturnNULLIf(index < 0);
		row = rowNew(name, index);
		ReturnNULLIf(listAdd(r->rows, row, NULL));
	}
	
	return row;
}

/*===========================================================================
 |                          Store Data Control                               |
  ===========================================================================*/

int matrixClear(matrix_ *r)
{
	ReturnErrIf(r == NULL);
	
	Debug("Clearing Matrix");
	
	memset(r->A, 0x0, sizeof(double)*r->lenA);
	memset(r->X, 0x0, sizeof(double)*r->lenXB);
	memset(r->B, 0x0, sizeof(double)*r->lenXB);
	
	/* Clear History List */
	if(r->history != NULL) {
		ReturnErrIf(listClear(r->history, (listDestroy_)historyDestroy));
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int matrixRecord(matrix_ *r, double time, unsigned int flag)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(listAdd(r->history, 
			historyNew(time, r->X, r->lenXB, flag), NULL));
	if(flag & HISTORY_FLAG_END) {
		Debug("Last matrix record");
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

int matrixRecall(matrix_ *r)
{
	history_ *history;
	ReturnErrIf(r == NULL);
	ReturnErrIf(listGetLast(r->history, (void*)&history));
	ReturnErrIf(historyRecall(history, r->X, r->lenXB));
	return 0;
}

/*---------------------------------------------------------------------------*/

list_ * matrixGetHistory(matrix_ *r)
{
	ReturnNULLIf(r == NULL);
	return r->history;
}

/*---------------------------------------------------------------------------*/

static int matrixFillData(history_ *history, double *data[])
{
	int length;
	
	length = historyGetLength(history) + 1;
	ReturnErrIf(length < 1);
	ReturnErrIf(historyGetAllData(history, *data, length));
	(*data) += length;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int matrixGetVariables(row_ *row, char **variables[])
{
	if(row != &gndRow) {
		**variables = rowGetName(row);
		ReturnErrIf(**variables == NULL);
		(*variables)++;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

int matrixGetSolution(matrix_ *r, double *data[], char **variables[], 
		int *numPoints, int *numVariables)
{
	double *dataPtr;
	char **variablePtr;
	
	/* TODO: This seems like a waste to processing time, especially
		if there are a lot of variables and data points but the
		data is a linked list, and has to be passed back to the
		caller as an array. (though maybe that can be looked at).
		Another option would be to guess what the number of points
		will be and store into an array instead of a linked list.
	*/
	
	ReturnErrIf(r == NULL);
	
	*numVariables = listLength(r->rows);
	ReturnErrIf(numVariables < 0);
	
	*numPoints = listLength(r->history);
	ReturnErrIf(numPoints < 0);
	
	*data = malloc((*numPoints) * (*numVariables) * sizeof(double));
	ReturnErrIf(*data == NULL);
	
	dataPtr = *data;
	ReturnErrIf(listExecute(r->history, (listExecute_)matrixFillData, 
					(void*)&dataPtr));
	
	*variables = malloc((*numVariables + 1) * sizeof(char*));
	ReturnErrIf(*variables == NULL);
	
	variablePtr = *variables;
	*variablePtr = "time";
	variablePtr++;
	ReturnErrIf(listExecute(r->rows, (listExecute_)matrixGetVariables, 
					(void*)&variablePtr));
	*variablePtr = NULL;
	
	return 0;
}

/*===========================================================================
 |                               Initialize                                  |
  ===========================================================================*/

static int matrixInitializeRows(row_ *row, matrix_ *r)
{
	ReturnErrIf(row == NULL);
	
	/* Trying to set the gnd row will raise a flag and we can exit */
	if(rowSetRHSPtr(row, &r->B[r->index]))
		return 0;
	ReturnErrIf(rowSetSolutionPtr(row, &r->X[r->index]));
	r->index++;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

static int matrixInitializeNodes(node_ *node, matrix_ *r)
{
	int row, col;
	ReturnErrIf(node == NULL);
	
	/* Trying to set the gnd node will raise a flag and we can exit */
	if(nodeSetDataPtr(node, &r->A[r->index]))
		return 0;
	row = nodeGetRow(node);
	ReturnErrIf(row < 0);
	col = nodeGetCol(node);
	ReturnErrIf(col < 0);
	r->aRow[r->index] = row - 1; /* Minus 1 becuase ignore gnd row */
	r->aColStart[col] = r->index + 1; /* plus one because fist start is 0 */
	r->index++;
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int matrixInitialize(matrix_ *r, control_ *control)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->A != NULL);
	
	r->lenA = listLength(r->nodes) - 1; /* Minus the ground node */
	ReturnErrIf(r->lenA < 0);
	r->lenXB = listLength(r->rows) - 1; /* Minus the ground row */
	ReturnErrIf(r->lenXB < 0);
	
	r->A = calloc(r->lenA, sizeof(double));
	ReturnErrIf(r->A == NULL);
	
	r->aRow = calloc(r->lenA, sizeof(int));
	ReturnErrIf(r->aRow == NULL);
	
	r->aColStart = calloc((r->lenXB+1), sizeof(int));
	ReturnErrIf(r->aColStart == NULL);
	
	r->X = calloc(r->lenXB, sizeof(double));
	ReturnErrIf(r->X == NULL);
	
	r->B = calloc(r->lenXB, sizeof(double));
	ReturnErrIf(r->B == NULL);
	
	r->index = 0;
	ReturnErrIf(listExecute(r->nodes, (listExecute_)matrixInitializeNodes, r));
	r->index = 0;
	ReturnErrIf(listExecute(r->rows, (listExecute_)matrixInitializeRows, r));
	
	if(control->luLibrary == CONTROL_LU_SUPERLU) {
		ReturnErrIf(matrixInitializeSuperLU(r, control));
	} else {
		ReturnErr("Unsupported matrix library");
	}
	
	return 0;
}

/*===========================================================================
 |                          Constructor / Destructor                         |
  ===========================================================================*/

int matrixDestroy(matrix_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	
	Debug("Destroying Matrix %p", (*r));
	
	if((*r)->rows != NULL) {
		if(listDestroy(&(*r)->rows, (listDestroy_)rowDestroy)) {
			Warn("Error destroying row list");
		}
	}
	
	if((*r)->nodes != NULL) {
		if(listDestroy(&(*r)->nodes, (listDestroy_)nodeDestroy)) {
			Warn("Error destroying node list");
		}
	}
	
	if((*r)->history != NULL) {
		if(listDestroy(&(*r)->history, (listDestroy_)historyDestroy)) {
			Warn("Error destroying history list");
		}
	}
	
	if((*r)->unconfig != NULL) {
		ReturnErrIf((*r)->unconfig(*r));
	}
	
	if((*r)->A != NULL)
		free((*r)->A);
	if((*r)->aRow != NULL)
		free((*r)->aRow);
	if((*r)->aColStart != NULL)
		free((*r)->aColStart);
	if((*r)->X != NULL)
		free((*r)->X);
	if((*r)->B != NULL)
		free((*r)->B);
	
	if((*r)->library != NULL)
		free((*r)->library);
	
	free(*r);
	*r = NULL;
	return 0;
}

/*---------------------------------------------------------------------------*/

static char * defaultName = "None";

matrix_ * matrixNew(matrix_ *r)
{
	ReturnNULLIf(r != NULL);
	r = calloc(1, sizeof(matrix_));
	ReturnNULLIf(r == NULL);
	
	Debug("Creating Matrix %p", r);
	
	/* Create Row List */
	r->rows = listNew(r->rows);
	GotoFailedIf(r->rows == NULL);
	/* Add the ground row */
	GotoFailedIf(listAdd(r->rows, &gndRow, NULL));
	
	/* Create Node List */
	r->nodes = listNew(r->nodes);
	GotoFailedIf(r->nodes == NULL);
	/* Add the ground node */
	GotoFailedIf(listAdd(r->nodes, &gndNode, NULL));
	
	/* Create Results List */
	r->history = listNew(r->history);
	GotoFailedIf(r->history == NULL);
	
	r->name = defaultName;
	
	return r;

failed:
	matrixDestroy(&r);
	return NULL;
}

/*===========================================================================*/

