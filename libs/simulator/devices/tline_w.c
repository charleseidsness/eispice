/*
 * W-Element Companion Model
 *
 *	This is an implimentation of the Companion Model of a Frequency-Dependent
 *	Coupled Transmission Line model described in [1] and [2]. (I found
 *	[2] to be more useful). The W-Element in HSPICE was originally based on
 *	this work, by Dmitri Kuzentsov, but I'm guessing they've done more work on
 *	it since then, i.e. this model is probably not equivelent to the W-Element.
 *
 * Refrences:
 *	[1] "Optimal Transient Simulation of Transmission Lines",
 *		by Kuzentsov / Schutte-Aine, IEEE February 1996
 *		- may be able to find it here: "http://ntrs.nasa.gov/archive/nasa/
 *		casi.ntrs.nasa.gov/19960016735_1996037962.pdf"
 *		- or buy it from the IEEE: "http://ieeexplore.ieee.org/Xplore/
 *		login.jsp?url=/iel4/81/10399/00486433.pdf?arnumber=486433"
 *	[2] "Efficient Circuit Simulation of Lossy Coupled Transmission
 *		Lines with Frequency-Dependent Parameters", Kuzentsov / Schutte-Aine
 *		- may be able to find it here: "http://ntrs.nasa.gov/archive/nasa/
 *		casi.ntrs.nasa.gov/19960016735_1996037962.pdf"
 *		(at the end of the package)
 */

/*
 * Copyright (C) 2007 Cooper Street Innovations Inc.
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

#define ML 4
#include <log.h>
#include <math.h>

#include "device_internal.h"
#include "mfunc.h"
#include "complex.h"
#include "netlib.h"

/*------------------------ Some useful debug macros -------------------------*/

#define DebugM1(name, matrix, rows)\
{\
	int iii;\
	for(iii = 0; iii < rows; iii++) {\
		Info("%s[%i] = %e", name, iii, (matrix)[iii]);\
	}\
}

#define DebugM2(name, matrix, rows, cols)\
{\
	int iii, jjj;\
	for(jjj = 0; jjj < cols; jjj++) {\
		for(iii = 0; iii < rows; iii++) {\
			Info("%s[%i,%i] = %e", name, iii, jjj, (matrix)[jjj*rows+iii]);\
		}\
	}\
}

#define DebugM2Complex(name, matrix, rows, cols)\
{\
	int iii, jjj;\
	for(jjj = 0; jjj < cols; jjj++) {\
		for(iii = 0; iii < rows; iii++) {\
			Info("%s[%i,%i] = %e + j%e", name, iii, jjj, \
					(matrix)[jjj*rows+iii].r, (matrix)[jjj*rows+iii].i);\
		}\
	}\
}

#define DebugM3(name, matrix, depth, rows, cols) \
{\
	int kkk, iii, jjj; \
	for(kkk = 0; kkk < depth; kkk++) { \
		for(jjj = 0; jjj < cols; jjj++) { \
			for(iii = 0; iii < rows; iii++) { \
				Info("%s[%i,%i,%i] = %e", name, kkk, iii, jjj, \
						(matrix)[kkk*depth*rows+jjj*rows+iii]); \
			} \
		} \
	}\
}

/*---------------------------------------------------------------------------*/

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

struct _devicePrivate {

	/* -- Charateristics of the TLine -- */
	int *M;		/* order of the approixmation */
	double *len; 	/* Length of the T-Line in inches */
	double *fgd;	/* Cut-off for the Dielectric Loss */
	double *fK;	/* Cut-off frequency */

	/* Row indexed matracies */
	double **L0;	/* DC inductance matrix per unit length */
	double **C0;	/* DC capacitance matrix per unit length */
	double **R0;	/* DC resistance matrix per unit length */
	double **G0;	/* DC shunt conductance matrix per unit length */
	double **Rs;	/* Skin-effect resistance matrix per unit length */
	double **Gd;	/* Dielectric-loss conductance matrix per unit length */
	/* ------------------------------- */

	int N; 			/* Number of tlines */
	int K;			/* number of sample points used for the fit */
	double *fk;		/* Frequency points (Hz) */

	/* Column indexed matracies */
	complex_ *Yc;	/* Characteristic Addmitance (Seimens) */
	complex_ *W;	/* Propigation Function */
	complex_ *Td;	/* Fixed time delay */

	/* Coeffeicents of the fit */
	double *aYc;	/* Coefficients of the the fit for Yc */
	double *aW;		/* Coefficients of the the fit for delayless W */
	double *fc;		/* Forced poles of the fit functions */

	/* Pointers to MNA Matrix Elements */
	row_ **rows;	/* Row Pointers */
	node_ **nodes;	/* Node Pointers */

};


/*===========================================================================
 |                             Local Functions                               |
  ===========================================================================*/

/*
	This function uses the Interpoltaion-Based Complex Rational Approximation
	Method for Frequency-Domain Difference Approximation ([1] III.C)
	to calculate the coefficeints for the interpolated Freq. Domain
	versions of the T-Line Characteristics.
*/
static int tLineWInitCoefs(devicePrivate_ *p)
{
	/* Declare some local names to save typing */
	int N = p->N, K = p->K, M = *(p->M);
	double *fk = p->fk;
	complex_ *Yc = p->Yc, *W = p->W;
	double *aYc = p->aYc, *aW = p->aW, *fc = p->fc;

	int P = (K*2 -1);
	int j, i, k, lwork;
	double *A, *b, *work, *At, *X;
	double *a, *rootsImag, *rootsReal, *fc2, *fk2;
	int *ipiv;

	/* Alocate Workspace Memory */
	lwork = K+N*N;		/* Minimum workspace for matrix operations */
	work = malloc((lwork+K*K+P+5*M+1+K+2*P*P)*sizeof(double));
	ReturnErrIf(work == NULL);
	/* TODO: This could probably be divided up differently, reusing
		some memory to save space */
	A = &work[lwork];			/* size K*K */
	b = &A[K*K];				/* size P */
	a = &b[P];					/* size M+1 */
	fc2 = &a[M+1];				/* size M */
	fk2 = &fc2[M];				/* size K */
	At = &fk2[K];				/* size P*P */
	ipiv = (int*)&At[P*P];		/* size M (ints) */
	X = (double*)&ipiv[M];		/* size P*P */
	rootsReal = &X[P*P];		/* size M */
	rootsImag = &rootsReal[M];	/* size M */

	/* 	Step 1: Setup the system of linear equations that represent an
		approximation of the real part of the Characteric Impedance.(see [1]).
	*/

	for(k = 0; k < K; k++) {
		b[k] = 0.0;
		for(j = 0; j < N; j++) {
			for(i = 0; i < N; i++) {
				b[k] += Yc[k*N*N+j*N+i].r;
			}
		}
	}

	/* TODO: This could be optimised by dropping the ifs */
	for(j = 0; j < K; j++) {
		for(i = 0; i < K; i++) {
			if(j == 0) {
				A[j*K+i] = 1.0;
			} else if(i == 0) {
				A[j*K+i] = 0;
			} else if(j <= M) {
				A[j*K+i] = pow(fk[i],2*j);
			} else {
				A[j*K+i] = -1.0*pow(fk[i],(2*(j-M))) * b[i];
			}
		}
	}

	/*	Step 2: Solve for the coeficients of the real value fit */

	/* Solve x = bA and store in the vector b */
	ReturnErrIf(netlibDGESV(K, 1, A, K, (int*)work, b, K));

	/*	Step 3: Solve for the roots of the denominator of the real value fit,
		these poles will be the same as the poles in the final fit.
	*/

	/* 'a' reprsents the coeficients of the denomitor (which is a polynomial)
		refer to [1] equation (9). They are put in reverse order, which is what
		the rpoly function expects.
	*/

	a[M] = 1.0;
	for(i = 0; i < M; i++) {
		a[i] = b[K-i-1];
	}


	/* Solve for the roots */
	ReturnErrIf(netlibRPOLY(a, &M, rootsReal, rootsImag));
	ReturnErrIf(M < 2, "Not enough roots to continue");

	/*	Step 4: Remove any spurious imaginary roots and convert from roots
		to poles (by inverting sign).
	*/

	j = 0;
	for(i = 0; i < M; i++) {
		if((rootsImag[i] == 0.0) && (rootsReal[i] < 0.0)) {
			fc2[j] = -rootsReal[i];
			fc[j] = sqrt(fc2[j]);
			j++;
		}
	}

	/* Adjust the value of M to match the number of real roots */
	M = j;

	/*	Step 5: Setup the A and b matricies for the final set of linear
		equations, representing the entire approximation. [1] (11)
	*/

	/* This is a little optimisation that avoids having to calculate
		this square over and over when A is built below */
	for(k = 0; k < K; k++) {
		fk2[k] = fk[k]*fk[k];
	}

	for(k = 0; k < K; k++) {
		A[k] = 1.0;
	}
	for(k = K; k < P; k++) {
		A[k] = 0.0;
	}
	for(j = 1; j < M; j++) {
		A[j*(K*2-1)] = 1.0;
		for(k = 1; k < K; k++) {
			A[j*P + k] = 1/(1+fk2[k]/fc2[j]);
		}
		for(k = K; k < (K*2 -1); k++) {
			A[j*P + k] = -(fk[k-K+1]/fc[j])/(1+(fk2[k-K+1]/fc2[j]));
		}
	}

	/* transpose(A) x A */
	ReturnErrIf(netlibDGEMM('T', 'N', M, M, P, 1.0, A, P, A, P, 0.0, At, M));
	/* Calculate the inverse of At */
	ReturnErrIf(netlibDGETRF(M, M, At, M, ipiv));
	ReturnErrIf(netlibDGETRI(M, At, M, ipiv, work, M));
	/* Calculate At x transpose(A) */
	ReturnErrIf(netlibDGEMM('N', 'T', M, P, M, 1.0, At, M, A, P, 0.0, X, M));

	/*	Step 6: Solve for the coeficients of the fit */

	/* TODO: Re-examine this statement:
		Note, this is a divergence from [1]. The paper seems to indicate that
		you should calculate a unique set of poles for both Yc and W but I
		don't think W should have any poles (based on how it was calculated). I
		was getting some very bad fits for W when I calculated it's own poles
		and I was only getting a small number of poles in the solution.
		For convenince I decided to just reuse Yc's poles to caluclate the
		fit for W. This may not make sense and may be changed someday.
	*/


	for(i = 0; i < N; i++) {
		for(j = 0; j < N; j++) {

			for(k = 0; k < K; k++) {
				b[k] = Yc[k*N*N + i*N + j].r;
			}
			for(k = K; k < P; k++) {
				b[k] = Yc[(k-K+1)*N*N + i*N + j].i;
			}
			/* (X * b) -> aYc */
			ReturnErrIf(netlibDGEMM('N', 'N', M, 1, P, 1.0, X, M, b, P, 0.0,
					&aYc[i*N*M+j*M], M));

			for(k = 0; k < K; k++) {
				b[k] = W[k*N*N + i*N + j].r;
			}
			for(k = K; k < P; k++) {
				b[k] = W[(k-K+1)*N*N + i*N + j].i;
			}
			/* (X * b) -> aW */
			ReturnErrIf(netlibDGEMM('N', 'N', M, 1, P, 1.0, X, M, b, P, 0.0,
					&aW[i*N*M+j*M], M));

		}
	}

	/* Free workspace memory */
	free(work);

	return 0;
}

/*---------------------------------------------------------------------------*/

/*
	This function calculates the Characteristic Admitance, the
	Delayless Propigation Function, and the Delay of the T-Line based on
	the characteristics supplied using the equations in table III of [1].
*/

static int tLineWInitCharacteristics(devicePrivate_ *p)
{
	/* Declare some local names to save typing */
	int N = p->N, K = p->K;
	double *L0 = *(p->L0), *C0 = *(p->C0), *R0 = *(p->R0);
	double *G0 = *(p->G0), *Rs = *(p->Rs), *Gd = *(p->Gd);
	double len = *(p->len), *fk = p->fk, fgd = *(p->fgd), fK = *(p->fK);
	complex_ *Yc = p->Yc, *W = p->W, *Td = p->Td;

	int i, j, k;				/* Counters for working through matricies */
	register int xos, yos, zos;	/* General purpose offsets */

	complex_ *Y;				/* Addmitance per unit length */
	complex_ *Z;				/* Impedance per unit length */

	complex_ *work;			/* Workspace used to manipulate matracies */
	int lwork;

	ReturnErrIf(p == 0);

	/* Alocate Workspace Memory */
	lwork = (4*N+4*N*N);		/* Minimum workspace */
	work = malloc((lwork+2*K*N*N)*sizeof(complex_));
	ReturnErrIf(work == NULL);
	Y = &work[lwork];	/* size K*N*N */
	Z = &Y[K*N*N];		/* size K*N*N */

	/* Step 1: Calculate the Admitance and Impedance per unit length
			at k frequency points.

		where...
		Y = G(f) + 2*pi*f*C
		Z = R(f) + 2*pi*f*L

		and...
		R(f) = R0 + sqrt(f)*(1 +j)*Rs 		-> Add in Skin Effect
		G(f) = G0 + (f/sqrt(1 + (f/fgd)))*Gd 	-> Add in Dielectric-loss

	*/
	for(k = 0; k < K; k++) {

		fk[k] = fK * (1-cos((M_PI*k)/(2*(K-1)))); /* From [1] page 114 */

		for(j = 0; j < N; j++) {
			for(i = 0; i < N; i++) {
				/* Fortran in column-indexed, C is row indexed, the
					transition is made here */
				xos = k*N*N+i*N+j;
				yos = j*N+i;
				Y[xos].r = G0[yos] + (fk[k]/sqrt(1+(fk[k]/fgd)))*Gd[yos];
				Y[xos].i = 2*M_PI*fk[k]*C0[yos];
				Z[xos].r = R0[yos] + sqrt(fk[k])*Rs[yos];
				Z[xos].i = sqrt(fk[k])*Rs[yos] + 2*M_PI*fk[k]*L0[yos];
				/* Avoid a divide by zero later on */
				if(Z[xos].r == 0) {
					Z[xos].r = 1E-100;
				}
				if(Y[xos].r == 0) {
					Y[xos].r = 1E-100;
				}
			}
		}
	}

	/* Step 2: Calculate Y/Z and Y*Z.

		Eventually...
		Yc = sqrt(Y/Z) and
		W = exp(-sqrt(Y*Z)*length)*exp(Td*l*2pi*f)
		or W = exp(length*(Td*2pi*f - sqrt(Z*Y)))

		But for Now...
		W = Y*Z

	*/
	for(k = 0; k < K; k++) {
		zos = k*N*N;
		ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEX1R, &Y[zos], N,
				&Z[zos], N, COMPLEX0, &W[zos], N));
	}

	/* Step 3: Calculate sqrt(Y*Z).

		W = sqrt(Y*Z)

	*/
	for(k = 0; k < K; k++) {
		zos = k*N*N;
		ReturnErrIf(mfuncSqrt(&W[zos], N, work, lwork));
	}

	/* Step 4: Calculate sqrt(Y*Z)/Z.

		Yc = sqrt(Y*Z)/Z (or sqrt(Y/Z))

	*/
	/* The next opertaion is an inplace operation, but want final result
		to be in Yc */
	memcpy(Yc, W, K*N*N*sizeof(complex_));

	for(k = 0; k < K; k++) {
		zos = k*N*N;
		ReturnErrIf(netlibZGESV(N, N, &Z[zos], N, (int*)work, &Yc[zos], N));
	}

	/* Step 5: Calculate the fixed time delay td = sqrt(C*L)*l */

	/* All of the library functions work with complex numbers, it's just
		easier to be consistent, though it's not really required for
		calculating the fixed time delay.
	*/
	for(j = 0; j < N; j++) {
		for(i = 0; i < N; i++) {
			xos = j*N+i;
			Z[xos].r = C0[xos];
			Z[xos].i = 0.0;
			Y[xos].r = L0[xos];
			Y[xos].i = 0.0;
		}
	}

	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEX1R, Z, N, Y, N,
			COMPLEX0, Td, N));
	ReturnErrIf(mfuncSqrt(Td, N, work, lwork));

	/* Step 6: Calculate exp(length*(Td*2pi*f - sqrt(Z*Y))) */
	for(k = 0; k < K; k++) {
		zos = k*N*N;

		for(j = 0; j < N; j++) {
			for(i = 0; i < N; i++) {
				xos = k*N*N+j*N+i;
				yos = j*N+i;
				W[xos].r = len*(Td[yos].r*2*M_PI*fk[k] - W[xos].r);
				W[xos].i = len*(Td[yos].i*2*M_PI*fk[k] - W[xos].i);
			}
		}

		ReturnErrIf(mfuncExp(&W[zos], N, work, lwork));

	}

	/* Free workspace memory */
	free(work);

	return 0;
}

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

/* TODO: Take a look at adding a next step object to this class. */

/*---------------------------------------------------------------------------*/

static int deviceClassInitStep(device_ *r)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Initializing Stepping %s %s %p", r->class->type, r->refdes, r);

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassStep(device_ *r, int *breakPoint)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	/* Debug("Stepping %s %s %p", r->class->type, r->refdes, r); */

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassLoad(device_ *r)
{
	devicePrivate_ *p;
	complex_ *Yxx, *Yxy;
	int i, N, *work;
	const complex_ COMPLEXn2R = {-2.0, 0.0};
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	N = p->N;

	Debug("Loading %s %s %p", r->class->type, r->refdes, r);

	/* Refer to [1] (16) for the MNA Stamp */

	/* Calculate [-W(0)^2] */
	Yxx = malloc(N*N*sizeof(complex_));
	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEXn1R, p->W, N, p->W, N,
			COMPLEX0, Yxx, N));

	/* Calculate [-W(0)^2 + I] */
	ReturnErrIf(Yxx == NULL);
	for(i = 0; i < N; i++) {
		Yxx[i*N+i].r += 1.0;
	}

	/* Calculate -2*W(0)*Yc(0) */
	Yxy = malloc(N*N*sizeof(complex_));
	ReturnErrIf(Yxy == NULL);
	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEXn2R, p->W, N, p->Yc, N,
			COMPLEX0, Yxy, N));

	/* Calculate (-2*W(0)*Yc(0))/([I - W(0)^2]) */
	work = malloc(N*sizeof(int));
	ReturnErrIf(work == NULL);
	ReturnErrIf(netlibZGESV(N, N, Yxx, N, work, Yxy, N));
	free(work);

	DebugM2Complex("Yxy", Yxy, N, N);
	DebugM2Complex("Yxx", Yxx, N, N);

	//~ /* Copy Yc into Yxx, for inplace operation */
	//~ for(i = 0; i < N; i++) {
		//~ for(j = 0; j < N; j++) {
			//~ Yxx[i*N+j].r = p->Yc[i*N+j].r;
			//~ Yxx[i*N+j].i = p->Yc[i*N+j].i;
		//~ }
	//~ }

	//~ /* Calculate (-2W(0)*Yc(0))/([I - W(0)^2])*-W(0) + Yc(0) */
	//~ ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEXn1R, Yxy, N, p->W, N,
			//~ COMPLEX1R, Yxx, N));

	//~ DebugM2Complex("Yxx", Yxx, N, N);
	//~ DebugM2Complex("Yxy", Yxy, N, N);

	//~ for(i = 0; i < p->N; i++) {
		//~ for(j = 0; j < p->N; j++) {

			//~ yc = p->Yc[i*p->N+j].r;

			//~ /* Y11 */
			//~ row = i; col = j;
			//~ n = p->N; m = p->N;
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+col], -yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+col], yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+m], yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+m], -yc));

			//~ /* Y22 */
			//~ row = i+p->N+1; col = j+p->N+1;
			//~ n = p->N*2+1; m = p->N*2+1;
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+col], -yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+col], yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+m], yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+m], -yc));

			//~ /* Y21 */
			//~ row = i+p->N+1; col = j;
			//~ n = p->N*2+1; m = p->N;
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+col], yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+col], -yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+m], -yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+m], yc));

			//~ /* Y12 */
			//~ row = i; col = j+p->N+1;
			//~ n = p->N; m = p->N*2+1;
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+col], yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+col], -yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[row*r->numPins+m], -yc));
			//~ ReturnErrIf(nodeDataPlus(p->nodes[n*r->numPins+m], yc));


		//~ }
	//~ }

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassUnconfig(device_ *r)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Unconfiging %s %s %p", r->class->type, r->refdes, r);

	free(p->fk);
	free(p->Yc);
	free(p->W);
	free(p->Td);

	free(p->aYc);
	free(p->aW);
	free(p->fc);

	free(p->rows);
	free(p->nodes);

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassPrint(device_ *r)
{
	devicePrivate_ *p;
	int i,j;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Printing %s %s %p", r->class->type, r->refdes, r);

	Info("%s --> %s:", r->class->type, r->refdes);
	Info("order=%i, numLines=%i", *p->M, p->N);
	for(i = 0; i < p->N; i++) {
		Info("%s <---> %s", rowGetName(p->rows[i]),
				rowGetName(p->rows[i+p->N+1]));
	}
	Info("%s <ref> %s", rowGetName(p->rows[p->N]),
			rowGetName(p->rows[p->N*2+1]));
	Info("length = %e", *p->len);
	for(i = 0; i < p->N; i++) {
		for(j = 0; j < p->N; j++) {
			Info("R0[%i,%i] = %e", i, j, (*(p->R0))[i*p->N+j]);
		}
	}
	for(i = 0; i < p->N; i++) {
		for(j = 0; j < p->N; j++) {
			Info("L0[%i,%i] = %e", i, j, (*(p->L0))[i*p->N+j]);
		}
	}
	for(i = 0; i < p->N; i++) {
		for(j = 0; j < p->N; j++) {
			Info("C0[%i,%i] = %e", i, j, (*(p->C0))[i*p->N+j]);
		}
	}
	for(i = 0; i < p->N; i++) {
		for(j = 0; j < p->N; j++) {
			Info("G0[%i,%i] = %e", i, j, (*(p->G0))[i*p->N+j]);
		}
	}

	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceTLineW = {
	.type = "W-Element Transmission Line",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = NULL,
	.initStep = deviceClassInitStep,
	.step = deviceClassStep,
	.minStep = NULL,
	.nextStep = NULL,
	.integrate = NULL,
	.print = deviceClassPrint,
};

/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceTLineWConfig(device_ *r,
		int *M,			/* order of approixmation */
		double *len, 	/* Length of the T-Line in inches */
		double **L0,	/* DC inductance matrix per unit length */
		double **C0,	/* DC capacitance matrix per unit length */
		double **R0,	/* DC resistance matrix per unit length */
		double **G0,	/* DC shunt conductance matrix per unit length */
		double **Rs,	/* Skin-effect resistance matrix per unit length */
		double **Gd,	/* Dielectric-loss conductance matrix per unit length */
		double *fgd,	/* Cut-off for the Dielectric Loss */
		double *fK		/* Cut-off frequency */
)
{
	devicePrivate_ *p;
	int i, j;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->class != NULL);
	ReturnErrIf(r->numPins < 4);
	ReturnErrIf(MOD(r->numPins,2) != 0);
	ReturnErrIf(*M < 3);
	ReturnErrIf(*M > 199); /* Defined by the rpoly function, but will
							probably have data over-runs way before this
							due to the fc^(2xM)values */

	/* Copy in class pointer */
	r->class = &deviceTLineW;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* allocate space for private data */
	r->private =  calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Copy in parameter pointers */
	p->M = M;
	ReturnErrIf(p->M == NULL);
	p->len = len;
	ReturnErrIf(p->len == NULL);
	p->L0 = L0;
	ReturnErrIf((p->L0 == NULL) || (*p->L0 == NULL));
	p->C0 = C0;
	ReturnErrIf((p->C0 == NULL) || (*p->C0 == NULL));
	p->R0 = R0;
	ReturnErrIf((p->R0 == NULL) || (*p->R0 == NULL));
	p->G0 = G0;
	ReturnErrIf((p->G0 == NULL) || (*p->G0 == NULL));
	p->Rs = Rs;
	ReturnErrIf((p->Rs == NULL) || (*p->Rs == NULL));
	p->Gd = Gd;
	ReturnErrIf((p->Gd == NULL) || (*p->Gd == NULL));
	p->fgd = fgd;
	ReturnErrIf(p->fgd == NULL);
	p->fK = fK;
	ReturnErrIf(p->fK == NULL);

	/* Calculate Local Parameters */
	p->K = (*M)*2 + 1;
	p->N = r->numPins/2 - 1; /* Don't include the ground node */

	/* Allocate memory for T-Line Definition */
	p->fk = malloc(p->K*sizeof(double));
	ReturnErrIf(p->fk == NULL);
	p->Yc = malloc(p->K*p->N*p->N*sizeof(complex_));
	ReturnErrIf(p->Yc == NULL);
	p->W = malloc(p->K*p->N*p->N*sizeof(complex_));
	ReturnErrIf(p->W == NULL);
	p->Td = malloc(p->N*p->N*sizeof(complex_));
	ReturnErrIf(p->Td == NULL);
	p->aYc = malloc(*(p->M)*p->N*p->N*sizeof(double));
	ReturnErrIf(p->aYc == NULL);
	p->aW = malloc(*(p->M)*p->N*p->N*sizeof(double));
	ReturnErrIf(p->aW == NULL);
	p->fc = malloc(*(p->M)*sizeof(double));
	ReturnErrIf(p->fc == NULL);

	/* Calculate Coefficients of the Fit */
	ReturnErrIf(tLineWInitCharacteristics(p));
	ReturnErrIf(tLineWInitCoefs(p));

	/* Create required nodes and rows (see MNA stamp above) */
	p->rows = malloc(sizeof(row_*)*r->numPins);
	ReturnErrIf(p->rows == NULL);
	for(i = 0; i < r->numPins; i++) {
		p->rows[i] = r->pin[i];
		ReturnErrIf(p->rows[i] == NULL);
	}

	p->nodes = malloc(sizeof(row_*)*r->numPins*r->numPins);
	ReturnErrIf(p->nodes == NULL);
	for(i = 0; i < r->numPins; i++) {
		for(j = 0; j < r->numPins; j++) {
			p->nodes[i*r->numPins +j] = matrixFindOrAddNode(r->matrix,
					p->rows[i], p->rows[j]);
			ReturnErrIf(p->nodes[i*r->numPins+j] == NULL);
		}
	}

	return 0;
}

/*===========================================================================*/


