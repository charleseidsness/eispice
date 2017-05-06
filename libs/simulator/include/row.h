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

#ifndef ROW_H
#define ROW_H

#include <data.h>

typedef struct _row row_;

typedef struct {
	char type;
	char *name;
} rowName_;

extern row_ gndRow;

listFindReturn_ rowCompare(row_ *r, rowName_ *name);

char * rowGetName(row_ *r);
int rowGetIndex(row_ *r);
double rowGetSolution(row_ *r);
double ** rowGetSolutionPtr(row_ *r);
double ** rowGetRHSPtr(row_ *r);

int rowRHSPlus(row_ *r, double plus);
int rowSetRHSPtr(row_ *r, double *rhs);
int rowSetSolutionPtr(row_ *r, double *solution);

int rowDestroy(row_ *r);
row_ * rowNew(rowName_ name, int index);

#endif

