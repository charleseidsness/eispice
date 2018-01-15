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
#include "row.h"

struct _row {
	char *name;
	int index;
	double *rhs;
	double *solution;
};

/*===========================================================================*/

static double gndRowRHS = 0.0;
static double gndRowSol = 0.0;
row_ gndRow = {
	.name = "v(0)",
	.index = 0,
	.rhs = &gndRowRHS,
	.solution = &gndRowSol
};

/*===========================================================================*/

listFindReturn_ rowCompare(row_ *r, rowName_ *name)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(name == NULL);

	if(name->type == 0x0) {
		if(!strcmp(r->name, name->name)) {
			return LIST_FIND_MATCH;
		}
	} else {
		if((r->name[0] == name->type) &&
			strlen(name->name) == (strlen(r->name) - 3)) {
				if(!strncmp(&r->name[2], name->name, strlen(name->name))) {
					return LIST_FIND_MATCH;
			}
		}
	}

	return LIST_FIND_NOTAMATCH;
}

/*---------------------------------------------------------------------------*/

char * rowGetName(row_ *r)
{
	ReturnNULLIf(r == NULL);
	return r->name;
}

/*---------------------------------------------------------------------------*/

int rowGetIndex(row_ *r)
{
	ReturnErrIf(r == NULL);
	return r->index;
}

/*---------------------------------------------------------------------------*/

double rowGetSolution(row_ *r)
{
	ReturnNaNIf(r == NULL);
	ReturnNaNIf(r->solution == NULL);
	return *r->solution;
}

/*---------------------------------------------------------------------------*/

double ** rowGetSolutionPtr(row_ *r)
{
	ReturnNULLIf(r == NULL);
	return &r->solution;
}

/*---------------------------------------------------------------------------*/

double ** rowGetRHSPtr(row_ *r)
{
	ReturnNULLIf(r == NULL);
	return &r->rhs;
}

/*---------------------------------------------------------------------------*/

int rowRHSPlus(row_ *r, double plus)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->rhs == NULL);
	*r->rhs += plus;
	return 0;
}

/*---------------------------------------------------------------------------*/

int rowSetRHSPtr(row_ *r, double *rhs)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(rhs == NULL);
	if(r == &gndRow)
		return 1;
	r->rhs = rhs;
	return 0;
}

/*---------------------------------------------------------------------------*/

int rowSetSolutionPtr(row_ *r, double *solution)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(solution == NULL);
	if(r == &gndRow)
		return 1;
	r->solution = solution;
	return 0;
}

/*---------------------------------------------------------------------------*/

int rowDestroy(row_ *r)
{
	ReturnErrIf(r == NULL);

	if(r != &gndRow) {

		Debug("Destroying Row %s:%i %p", r->name, r->index, r);

		if(r->name != NULL) {
			free(r->name);
		}
		free(r);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

row_ * rowNew(rowName_ name, int index)
{
	row_ *r;

	ReturnNULLIf(index < 0);
	ReturnNULLIf(name.name == NULL);

	r = calloc(1, sizeof(row_));
	ReturnNULLIf(r == NULL);

	Debug("Creating Row %c %s %p ", name.type, name.name, r);

	if(name.type == 0x0) {
		/* If type == 0 the name is in the format i(xxx) or v(xxx) */
		ReturnNULLIf(strlen(name.name) < 4);
		ReturnNULLIf((name.name[0] != 'i') && (name.name[0] != 'v') &&
				(name.name[0] != 'I') && (name.name[0] != 'V'));
		ReturnNULLIf((name.name[1] != '(') &&
				(name.name[strlen(name.name)] != ')'));
		r->name = malloc(strlen(name.name)+1);
		ReturnNULLIf(r->name == NULL);
		strcpy(r->name, name.name);
		if(r->name[0] == 'V') {
			r->name[0] = 'v';
		} else if(r->name[0] == 'I') {
			r->name[0] = 'i';
		}
	} else {
		ReturnNULLIf((name.type != 'i') && (name.type != 'v'));
		r->name = malloc(strlen(name.name)+ 4);
		ReturnNULLIf(r->name == NULL);
		strcpy(&r->name[2], name.name);
		r->name[0] = name.type;
		r->name[1] = '(';
		r->name[strlen(name.name)+2] = ')';
		r->name[strlen(name.name)+3] = 0x0;
	}

	r->index = index;

	return r;
}

/*===========================================================================*/

