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

#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>

#include "log.h"
LogMaster;
#include "calc.h"

#define MAX_VALUES	256

static double value[MAX_VALUES];
static double *valueP[MAX_VALUES];
static int numValues = -1;

typedef enum {
	SOLVE_NORMAL,
	SOLVE_DIFF,
	SOLVE_EVAL,
} solveType;

static double ** getVariable(char *name, void *private)
{
	ReturnNULLIf(numValues >= MAX_VALUES, "Can have a maximum of %i variables",
		MAX_VALUES);
	
	printf("Please enter value: %s = ", name);
	
	fflush(stdout);
	numValues++;
	scanf("%lg", &value[numValues]);
	valueP[numValues] = &value[numValues];
	
	return &valueP[numValues];
}

void help()
{
	Info("calculon %i.%i", CALC_MAJOR_VERSION, CALC_MINOR_VERSION);
    Info("Usage: calculon <options> <formula>");
	Info("Options:");
	Info("\t-v, --version : display version info");
	Info("\t-h, --help : display help info");
	Info("\t-e <file>, --error <file> : file to print errors to (default is stderr)");
	Info("\t-l <file>, --log <file>	 : file to print log to (default is stdout)");
	Info("\t-d <var>, --diff <var> : calculate derivative of function w.r.t variable");
	Info("\t-E , --evaluate : evaluate an binary operation");
	Info("\t-m, --mindiv <value> : set minimum value for denominator (default 1E-12)");
	Info("Supported Operators:");
	Info("NOTE: <f(x)> represents a function including variable x");
	Info("and <f(...)> represents a function containg any number of variables");
	Info("\tabs(<f(...)>) -- absolute value");
	Info("\tacosh(<f(...)>) -- hyperbolic arc cosine");
	Info("\tacos(<f(...)>) -- arc cosine");
	Info("\tasinh(<f(...)>) -- hyperbolic arc sine");
	Info("\tasin(<f(...)>) -- arc sine");
	Info("\tatanh(<f(...)>) -- hyperbolic arc tangent");
	Info("\tatan(<f(...)>) -- arc tangent");
	Info("\tcosh(<f(...)>) -- hyperbolic cosine");
	Info("\tcos(<f(...)>)	 -- cosine");
	Info("\texp(<f(...)>) -- expoponential, i.e. e^x");
	Info("\tln(<f(...)>) -- natural logrithum, i.e. log_e(x)");
	Info("\tlog(<f(...)>) -- logrithum");
	Info("\tsinh(<f(...)>) -- hyperbolic sine");
	Info("\tsin(<f(...)>) -- sine");
	Info("\tsqrt(<f(...)>) -- square-root");
	Info("\ttan(<f(...)>) -- tangent");
	Info("\turamp(<f(...)>) -- ramp function, x = 0 for x < 0, and x = x for x > 0");
	Info("\tu(<f(...)>)	-- step function, x = 0 for x < 0, and x = 1 for x > 0");
	Info("\t<f(...)> + <f(...)> -- add");
	Info("\t<f(...)> - <f(...)> -- subtract");
	Info("\t<f(...)> * <f(...)> -- multipy");
	Info("\t<f(...)> / <f(...)> -- divide");
	Info("\t<f(...)> ^ <f(...)> -- power, i.e. x^y");
	Info("\tnumber -- constant (floating point)");
	Info("\tsequence of characters -- variable (user will be queried for value");
	Info("\t\tduring solve block");
	Info("Example:");
	Info("\tcalculon \"g*10 + sin(y) - cos(y) + diff(x^2 - y)\"");
}

int main(int argc, char *argv[])
{
	int opt;
	struct option longopts[] = { 
			{"iterations", 0, NULL, 'n'},
			{"help", 0, NULL, 'h'},
			{"version", 0, NULL, 'v'},
			{"error", 1, NULL, 'e'},
			{"log", 1, NULL, 'l'},
			{"diff", 1, NULL, 'd'},
			{"zero", 1, NULL, 'z'},
			{"mindiv", 1, NULL, 'm'},
			{0, 0, 0, 0}
    };
	calc_ *calc = NULL;
	char *formula = NULL, *diff = NULL;
	double solution = 0, minDiv = 1e-12;
	int result;
	solveType type = SOLVE_NORMAL;
    
	/* Process the command line options */
	while((opt = getopt_long(argc, argv, "d:hve:l:d:zm:E", longopts, NULL)) != -1) {
		switch(opt) {
		case 'm': minDiv = atof(optarg); break;
		case 'v': calcInfo(); ExitSuccess;
		case 'e': OpenErrorFile(optarg); break;
		case 'l': OpenLogFile(optarg); break;
		case 'h': help(); ExitSuccess;
		case 'd': type = SOLVE_DIFF; diff = optarg; break;
		case 'E': type = SOLVE_EVAL; break;
		case '?': ExitFailure("Unkown option");
        case ':': ExitFailure("Option needs a value");
		default:  help(); ExitFailure("Invalid option");
		}
	}
	formula = argv[argc-1];
	Debug("Formula: %s", formula);
	calc = calcNew(calc, formula, getVariable, NULL, &minDiv);
	ExitFailureIf(calc == NULL, "Failed to create calculon");
	
	switch(type) {
		case SOLVE_NORMAL:
			ExitFailureIf(calcSolve(calc, &solution));
			Info("Solution: %e", solution);
			break;
		case SOLVE_DIFF:
			ExitFailureIf(calcDiff(calc, diff, &solution));
			Info("Solution: %e", solution);
			break;
		case SOLVE_EVAL:
			ExitFailureIf(calcEvaluate(calc, &result));
			Info("Result: %i", result);
			break;
	}
	
	calcDestroy(&calc);
	CloseErrorFile;
	CloseLogFile;
	ExitSuccess;
}
