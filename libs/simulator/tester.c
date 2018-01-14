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
#define ML 4
#include <log.h>
LogMaster;
#include "simulator.h"

void help()
{
	Info("tester %i.%i", SIMULATOR_MAJOR_VERSION, SIMULATOR_MINOR_VERSION);
    Info("Usage: tester <options> <filename>");
	Info("Options:");
	Info("\t-v, --version : display version info");
	Info("\t-h, --help    : display help info");
	Info("\t-e, --error <filename> : error log (default is stderr)");
	Info("\t-l, --log <filename>   : message log (default is stdout)");
	Info("\t-<x>, --test<x> : test x");
}

void version()
{
	Info("------------------------------------------------------------------");
	simulatorInfo();
	Info("------------------------------------------------------------------");
}

void print(FILE *outFile, double *data, char **variables, int numVariables)
{
	int i;
	for(i = 1; i < numVariables; i++) {
	 fprintf(outFile, "%s = %g\n", variables[i], data[i]);
	}
}

void plot(FILE *outFile, double *data, char **variables, int numPoints,
		int numVariables)
{
	int i, j, comma;

	fprintf(outFile, "#!gnuplot\n");
	fprintf(outFile, "set data style line\n");
	fprintf(outFile, "set grid\n");

	fprintf(outFile, "set xlabel \"Time (s)\"\n");
	fprintf(outFile, "set term x11 0 font clean persist\n");
	fprintf(outFile, "plot ");

	comma = 0;
	for(i = 1; i < numVariables; i++) {
		if(comma) {
			fprintf(outFile, ",");
		} else {
	 		comma = 1;
		}
		fprintf(outFile, "'-' title \"%s\" lw 2\\\n", variables[i]);
	}
	fprintf(outFile, "\n");

	for(i = 1; i < numVariables; i++) {
		for(j = 0; j < numPoints*numVariables; j += numVariables) {
			fprintf(outFile, "%e\t%e\n", data[j], data[j + i]);
		}
		fprintf(outFile, "e\n");
	}

}

int main(int argc, char *argv[])
{
	int opt;
	struct option longopts[] = {
			{"help", 0, NULL, 'h'},
			{"version", 0, NULL, 'v'},
			{"about", 0, NULL, 'a'},
			{"error", 1, NULL, 'e'},
			{"log", 1, NULL, 'l'},
			{"output", 1, NULL, 'o'},
			{"test0", 1, NULL, '0'},
			{"test1", 1, NULL, '1'},
			{"test2", 1, NULL, '2'},
			{"test3", 1, NULL, '3'},
			{"test4", 1, NULL, '4'},
			{0, 0, 0, 0}
    };
	simulator_ *simulator = NULL;
	char *outFileName = NULL;
	FILE *outFile = stdout;
	double R, dc, Z0, Td, loss;
	double *data;
	char **variables;
	int numPoints, numVariables, i;
	double pulseD[7] = { 0, 10, 10e-9, 2e-9, 3e-9, 5e-9, 20e-9 };
	double *pulse[7] = { &pulseD[0], &pulseD[1], &pulseD[2], &pulseD[3],
			&pulseD[4], &pulseD[5], &pulseD[6] };
	double gaussD[7] = { 0, 3.3, 2e-9, 1e-9, 0.5e-9, 5e-9, 12e-9 };
	double *gauss[7] = { &gaussD[0], &gaussD[1], &gaussD[2], &gaussD[3],
			&gaussD[4], &gaussD[5], &gaussD[6] };

	/* Process the command line options */
	while((opt = getopt_long(argc,argv,"hvae:l:01234o:",longopts,NULL)) != -1) {
		switch(opt) {
		case '0':
			/* Create a new simulator object */
			simulator = simulatorNew(simulator);
			ExitFailureIf(simulator == NULL);

			R = 10;
			dc = 10;
			ExitFailureIf(simulatorAddResistor(simulator, "R1", "n1", "0", &R));
			ExitFailureIf(simulatorAddSource(simulator, "V1", "n1", "0",'v',
					&dc, 0x0, NULL));
			ExitFailureIf(simulatorRunOperatingPoint(simulator,
					&data, &variables, &numPoints, &numVariables));

			print(outFile, data, variables, numVariables);

			/* Destroy the Scripter Object */
			if(simulatorDestroy(&simulator)) {
				Warn("Failed to close simulator");
			}
			free(data);
			free(variables);
			break;
		case '1':
			/* Create a new simulator object */
			simulator = simulatorNew(simulator);
			ExitFailureIf(simulator == NULL);

			R = 10; Z0 = 50; Td = 15e-9; loss = 0.2;
			ExitFailureIf(simulatorAddResistor(simulator, "R1", "n1", "n2", &R));
			ExitFailureIf(simulatorAddSource(simulator, "V1", "n1", "0",'v',
					NULL, 'p', pulse));
			ExitFailureIf(simulatorAddTLine(simulator, "T2", "n2", "0", "n3",
					"0", &Z0, &Td, &loss));
			ExitFailureIf(simulatorRunTransient(simulator,
					0.1e-9, 50e-9, 0.0, 0,
					&data, &variables, &numPoints, &numVariables));

			plot(outFile, data, variables, numPoints, numVariables);

			/* Destroy the Scripter Object */
			if(simulatorDestroy(&simulator)) {
				Warn("Failed to close simulator");
			}
			free(data);
			free(variables);
			break;
		case '2':
			/* Create a new simulator object */
			simulator = simulatorNew(simulator);
			ExitFailureIf(simulator == NULL);

			R = 10; Z0 = 50; Td = 15e-9; loss = 0.2;
			ExitFailureIf(simulatorAddResistor(simulator, "R1", "n1", "n2", &R));
			ExitFailureIf(simulatorAddSource(simulator, "V1", "n1", "0",'v',
					NULL, 'p', pulse));
			ExitFailureIf(simulatorAddTLine(simulator, "T2", "n2", "0", "n3",
					"0", &Z0, &Td, &loss));
			for(i = 0; i < 100; i++) {
				ExitFailureIf(simulatorRunTransient(simulator,
					0.1e-9, 50e-9, 0.0, 0,
					&data, &variables, &numPoints, &numVariables));
			}

			//plot(outFile, data, variables, numPoints, numVariables);

			/* Destroy the Scripter Object */
			if(simulatorDestroy(&simulator)) {
				Warn("Failed to close simulator");
			}
			free(data);
			free(variables);
			break;
		case '3':
			/* Create a new simulator object */
			simulator = simulatorNew(simulator);
			ExitFailureIf(simulator == NULL);

			double L0[4] = {231.832e-9, 38.1483e-9, 38.1483e-9, 231.819e-9};
			double *L0p = L0;
			double C0[4] = {156.163e-12, -8.60102e-12,-8.60102e-12, 156.193e-12};
			double *C0p = C0;
			double R0[4] = {0.861113, 0, 0, 0.861113};
			double *R0p = R0;
			double G0[4] = {0,0,0,0};
			double *G0p = G0;
			double Rs[4] = {0.368757e-3, 0,0, 0.368757e-3};
			double *Rsp = Rs;
			double Gd[4] = {0,0,0,0};
			double *Gdp = Gd;
			char *nodes[6] = {"1", "3", "0", "2", "4", "0"};
			int M = 9;
			double len = 0.0265;
			double fgd = 1e100;
			double fK = 1e9;

			ExitFailureIf(simulatorAddResistor(simulator, "R1", "n1", "n2", &R));
			ExitFailureIf(simulatorAddSource(simulator, "V1", "n1", "0",'v',
					NULL, 'p', pulse));
			ExitFailureIf(simulatorAddTLineW(simulator,
					"T1", nodes, 6, &M, &len, &L0p, &C0p, &R0p, &G0p, &Rsp,
					&Gdp, &fgd, &fK));

			/* Destroy the Scripter Object */
			if(simulatorDestroy(&simulator)) {
				Warn("Failed to close simulator");
			}
			//free(data);
			//free(variables);
			break;
		case '4':
			/* Create a new simulator object */
			simulator = simulatorNew(simulator);
			ExitFailureIf(simulator == NULL);

			ExitFailureIf(simulatorAddSource(simulator, "V1", "n1", "0",'v',
					NULL, 'g', gauss));
			ExitFailureIf(simulatorRunTransient(simulator,
					0.01e-9, 50e-9, 0.0, 0,
					&data, &variables, &numPoints, &numVariables));

			plot(outFile, data, variables, numPoints, numVariables);

			/* Destroy the Scripter Object */
			if(simulatorDestroy(&simulator)) {
				Warn("Failed to close simulator");
			}
			free(data);
			free(variables);
			break;
		case 'a':
		case 'v': version(); ExitSuccess;
		case 'o':
			outFileName = optarg;
			outFile = fopen(outFileName, "wb");
			ExitFailureIf(outFile == NULL, "Failed to open %s for output",
					outFileName);
			break;
		case 'e': OpenErrorFile(optarg); break;
		case 'l': OpenLogFile(optarg); break;
		case 'h': help(); ExitSuccess;
		case '?': ExitFailure("Unkown option");
		case ':': ExitFailure("Option needs a value");
		default:  help(); ExitFailure("Invalid option");
		}
	}


	if(outFileName != NULL) {
		fclose(outFile);
	}

	CloseErrorFile;
	CloseLogFile;
	ExitSuccess;
}
