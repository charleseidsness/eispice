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

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "control.h"

typedef struct _waveform waveform_;

int waveformNextStep(waveform_ *r, double *nextStep);
int waveformCalcValue(waveform_ *r, double *value);

int waveformInitialize(waveform_ *r);

int waveformDestroy(waveform_ **r);
waveform_ * waveformNew(waveform_ *r, control_ *control, char type, 
		double *args[7], double **dcPtr);

#endif
