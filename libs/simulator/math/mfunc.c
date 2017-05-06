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
#include <float.h>
#define ML 4
#include <log.h>

#include "complex.h"
#include "netlib.h"

/*===========================================================================*/

int mfuncSqrt(complex_ *A, int N, complex_ *work, int M)
{
	/* This function is based on:
		[2] Nicholas J. Higham. A new sqrtm for MATLAB. Numerical Analysis
		Report No. 336, Manchester Centre for Computational Mathematics,
		Manchester, England, January 1999.
	*/
	
	int i, j, k;
	double *rwork;
	complex_ *eigval, *schur, *R, s, *H, *X;
	
	ReturnErrIf(A == NULL);
	ReturnErrIf(work == NULL);
	ReturnErrIf(N <= 0);
	ReturnErrIf(M < (4*N+4*N*N));
	
	/* Divide up the workspace */
	rwork = (double*)(&work[2*N]);
	eigval = &work[3*N];
	schur = &work[4*N];
	R = &work[4*N + N*N];
	H = &work[4*N + 2*N*N];
	X = &work[4*N + 3*N*N];
	
	/* Calculate the Schur Unity Matrx and and Eiganvalues */
	ReturnErrIf(netlibZGEES('V', 'N', NULL, N, A, N, 0, eigval, schur, N, work,
			2*N, rwork, NULL));
		
	/* Setup the diagonal mtrx R */
	for(j = 0; j < N; j++) {
		for(i = 0; i < N; i++) {
			if (i == j) {
				R[j*N+i] = complexSqrt(A[j*N+i]);
			} else {
				R[j*N+i].r = 0.0; R[j*N+i].i = 0.0;
			}
		}
	}
	
	/* The Higham Algorithum [2] Section 5 */
	for(j = 0; j < N; j++) {
		for(i = j-1; i >= 0; i--) {
			s = A[j*N + i];
			for (k = i+1; k < (i + j + 1); k++) {
				s = complexSub(s, complexMult(R[k*N+i], R[j*N+k]));
			}
			/* R(i,j) = s/(R(i,i)+R(j,j)) */
			R[j*N + i] = complexDiv(s, complexAdd(R[i*N + i], R[j*N + j]));
		
	   	}
    }
	
	/* Calculate the Hermitian */
	for(j = 0; j < N; j++) {
		for(i = 0; i < N; i++) {
			H[j*N + i] = complexConj(schur[i*N + j]);
	   	}
    }
	
	/*U * R * U.hermitian */
	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEX1R, schur, N, R, N, 
			COMPLEX0, X, N));
	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEX1R, X, N, H, N, 
			COMPLEX0, A, N));
	
	return 0;
}

/*---------------------------------------------------------------------------*/

int mfuncExp(complex_ *A, int N, complex_ *work, int M)
{
	/* This function is based on the sqrtm function above. It should 
		probably be replaced with the Pade Approximation version of an 
		expm algorithim like the one that can be found in Octave.
	*/
	
	int i, j, k;
	double *rwork;
	complex_ *eigval, *schur, *R, s, *H, *X;
	
	ReturnErrIf(A == NULL);
	ReturnErrIf(work == NULL);
	ReturnErrIf(N <= 0);
	ReturnErrIf(M < (4*N+4*N*N));
	
	
	/* Divide up the workspace */
	rwork = (double*)(&work[2*N]);
	eigval = &work[3*N];
	schur = 	&work[4*N];
	R = 	&work[4*N + N*N];
	H = 	&work[4*N + 2*N*N];
	X = 	&work[4*N + 3*N*N];
	
	/* Calculate the Schur Unity Matrix and and Eiganvalues */
	ReturnErrIf(netlibZGEES('V', 'N', NULL, N, A, N, 0, eigval, schur, N, work,
			2*N, rwork, NULL));
	
	/* Setup the diagonal mtrx R */
	for(j = 0; j < N; j++) {
		for(i = 0; i < N; i++) {
			if (i == j) {
				R[j*N+i] = complexExp(A[j*N+i]);
			} else {
				R[j*N+i].r = 0.0; R[j*N+i].i = 0.0;
			}
		}
	}
	
	/* The Higham Algorithum [2] Section 5 */
	for(j = 0; j < N; j++) {
		for(i = j-1; i >= 0; i--) {
			s = A[j*N + i];
			for (k = i+1; k < (i + j + 1); k++) {
				s = complexSub(s, complexMult(R[k*N+i], R[j*N+k]));
			}
			/* R(i,j) = s/(R(i,i)+R(j,j)) */
			R[j*N + i] = complexDiv(s, complexAdd(R[i*N + i], R[j*N + j]));
		
	   	}
    }
	
	/* Calculate the Hermitian */
	for(j = 0; j < N; j++) {
		for(i = 0; i < N; i++) {
			H[j*N + i] = complexConj(schur[i*N + j]);
	   	}
    }
	
	/*U * R * U.hermitian */
	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEX1R, schur, N, R, N, 
			COMPLEX0, X, N));
	ReturnErrIf(netlibZGEMM('N', 'N', N, N, N, COMPLEX1R, X, N, H, N, 
			COMPLEX0, A, N));
	
	return 0;
}

/*===========================================================================*/

