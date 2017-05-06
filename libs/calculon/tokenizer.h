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

        
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <data.h>

typedef struct _token token_;
struct _token {
	int type;			/* Token type, refer to parser.h for types. */
	double **variable;	/* Pointer to a pointer of variabel data, only 
							relavent for TOKEN_VARIABLE types. */
	double constant;		/* holds the value of a constant, only relavent for 
							TOKEN_CONSTANT types. */
	double *minDiv;		/*	Minimum denominator value, protects against 
						 *  divide by zero */
};

typedef struct {
	void *parser;
	double *solution;
	double **diffVariable;
} tokenSolveArgs_;

int tokenSolve(token_ *r, tokenSolveArgs_ *args);
int tokenDiff(token_ *r, tokenSolveArgs_ *args);
int tokenZeroXing(token_ *r, tokenSolveArgs_ *args);

int tokenDestroy(token_ *r);

typedef double ** (*tokenizerGetVarPtr)(char *varName, void *private);
list_ * tokenizerNew(char *buffer, tokenizerGetVarPtr getVarFunc,
		void *private, double *minDiv);

#endif
