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

#ifndef complex_H
#define complex_H

typedef struct {
	double r;
	double i;
} complex_;

static const complex_ COMPLEX0 = {0.0, 0.0};
static const complex_ COMPLEX1R = {1.0, 0.0};
static const complex_ COMPLEXn1R = {-1.0, 0.0};
static const complex_ COMPLEX1I = {0.0, 1.0};
static const complex_ COMPLEXn1I = {0.0, -1.0};

complex_ complexSqrt(complex_ z);
complex_ complexExp(complex_ z);
complex_ complexMult(complex_ a, complex_ b);
complex_ complexAdd(complex_ a, complex_ b);
complex_ complexSub(complex_ a, complex_ b);
complex_ complexConj(complex_ a);
complex_ complexDiv(complex_ a, complex_ b);
complex_ complexDivInt(complex_ a, int b);

#endif
