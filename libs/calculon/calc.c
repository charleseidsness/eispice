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
#include <math.h>
#include <data.h>

#include "calc.h"
#include "tokenizer.h"
#include "parser.h"

struct _calc {
	hash_ *variables;
	list_ *tokens;
	void *parser;
};

int calcSolve(calc_ *r, double *solution)
{
	tokenSolveArgs_ args;

	ReturnErrIf(r == NULL);
	ReturnErrIf(solution == NULL);

	args.parser = r->parser;
	args.solution = solution;
	args.diffVariable = NULL;

	ReturnErrIf(listExecute(r->tokens, (listExecute_)tokenSolve, &args));
	ReturnErrIf(isnan(*solution), "Parser failed");

	return 0;
}

int calcEvaluate(calc_ *r, int *result)
{
	tokenSolveArgs_ args;
	double solution;

	ReturnErrIf(r == NULL);
	ReturnErrIf(result == NULL);

	args.parser = r->parser;
	args.solution = &solution;
	args.diffVariable = NULL;

	/* Put Parser in evaluate mode */
	Debug("%s", ParseTokenName(TOKEN_EVAL));
	Parse(r->parser, TOKEN_EVAL, NULL, &solution);
	ReturnErrIf(isnan(solution), "Parser failed");

	ReturnErrIf(listExecute(r->tokens, (listExecute_)tokenSolve, &args));
	ReturnErrIf(isnan(solution), "Parser failed");

	*result = (solution == 0) ? 0 : 1;

	return 0;
}

int calcDiff(calc_ *r, char *variable, double *solution)
{
	tokenSolveArgs_ args;

	ReturnErrIf(r == NULL);
	ReturnErrIf(solution == NULL);
	ReturnErrIf(variable == NULL);

	args.parser = r->parser;
	args.solution = solution;

	ReturnErrIf(hashFind(r->variables, variable, (void*)&args.diffVariable));
	ReturnErrIf(args.diffVariable == NULL,
			"Couldn't find variable %s.", variable);

	/* Put Parser in differentiation mode */
	Debug("%s", ParseTokenName(TOKEN_DIFF));
	Parse(r->parser, TOKEN_DIFF, NULL, solution);
	ReturnErrIf(isnan(*solution), "Parser failed");

	ReturnErrIf(listExecute(r->tokens, (listExecute_)tokenSolve, &args));
	ReturnErrIf(isnan(*solution), "Parser failed");

	return 0;
}

typedef struct {
	void *private;
	calc_ *r;
	calcGetVarPtr_ getVarFunc;
} getVarFuncArgs_;

static double ** localGetVarFunc(char *varName, void *private)
{
	getVarFuncArgs_ *args;
	double **data;

	args = (getVarFuncArgs_ *)private;

	ReturnNULLIf(hashFind(args->r->variables, varName, (void*)&data));
	if(data == NULL) {
		data = args->getVarFunc(varName, args->private);
		ReturnNULLIf(data == NULL);
		ReturnNULLIf(hashAdd(args->r->variables, varName, (void*)data));
	}

	return data;
}

int calcInfo(void)
{
	Info("Calculon %i.%i", CALC_MAJOR_VERSION, CALC_MINOR_VERSION);
	Info("Compiled " __DATE__ " at " __TIME__);
	Info("(c) 2006 Cooper Street Innovations Inc.");
	return ((CALC_MAJOR_VERSION << 16) + CALC_MINOR_VERSION);
}

static int calcVariableDestroy(char *name, double **value)
{
	/* The variable name is the only locally stored memory */
	if(name != NULL) {
		free(name);
	}
	return 0;
}

int calcDestroy(calc_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(*r == NULL);
	Debug("Destroying calculon %p", *r);

	if((*r)->tokens != NULL) {
		if(listDestroy(&(*r)->tokens, (listDestroy_)tokenDestroy)) {
			Warn("Failed to destroy token list");
		}
	}
	if((*r)->parser != NULL) {
		ParseFree((*r)->parser, free);
	}
	if((*r)->variables != NULL) {
		if(hashDestroy(&(*r)->variables, (hashDestroy_)calcVariableDestroy)) {
			Warn("Failed to destroy variable table");
		}
	}

	free(*r);
	*r = NULL;
	return 0;
}

calc_ * calcNew(calc_ *r, char *buffer, calcGetVarPtr_ getVarFunc,
		void *private, double *minDiv)
{
	getVarFuncArgs_ args;

	ReturnNULLIf(NULL != 0);
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(buffer == NULL);
	ReturnNULLIf(getVarFunc == NULL);
	r = calloc(1, sizeof(calc_));
	ReturnNULLIf(r == NULL, "Malloc Failed");

	Debug("Creating calculon %p", r);

	args.private = private;
	args.r = r;
	args.getVarFunc = getVarFunc;

	Debug("%p", &args);

	r->variables = hashNew(r->variables, 1);
	ReturnNULLIf(r->variables == NULL);

	r->tokens = tokenizerNew(buffer, localGetVarFunc,
			(void *)&args, minDiv);
	ReturnNULLIf(r->tokens == NULL, "Failed to parse string");

	r->parser = ParseAlloc(malloc);
	ReturnNULLIf(r->parser == NULL, "Failed to create parser");

	return r;
}
