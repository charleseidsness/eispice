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

#ifndef NETLIB_H
#define NETLIB_H

#include "complex.h"

int netlibZGEMM(char transa, char transb, int m, int n, int k,
		complex_ alpha, complex_ *A, int lda, complex_ *B, int ldb,
		complex_ beta, complex_ *C, int ldc);

int netlibDGEMM(char transa, char transb, int m, int n, int k,
		double alpha, double *A, int lda, double *B, int ldb,
		double beta, double *C, int ldc);

int netlibZGESV(int n, int nrhs, complex_ *A, int lda, int *pivot, complex_ *b,
		int ldb);

int netlibDGESV(int n, int nrhs, double *A, int lda, int *pivot, double *b,
		int ldb);

int netlibZGEES(char jobvs, char sort, complex_ *select, int n, complex_ *A,
		int lda, int sdim, complex_ *W, complex_ *VS, int lvds,
		complex_ *WORK, int lwork, double *RWORK, int *BWORK);

int netlibRPOLY(double *op, int *degree, double *zeror, double *zeroi);

int netlibDGETRF(int m, int n, double *A, int lda, int *ipiv);

int netlibDGETRI(int n, double *A, int lda, int *ipiv, double *work, int lwork);

int netlibERF(double x, double *y);

#endif
