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
#include <log.h>

#include "piecewise.h"
#include "control.h"
#include "waveform.h"
#include "netlib.h"

struct _waveform {
	char type;	/* Type of Source
				 * pulse --> 'p'
				 * sin   --> 's'
				 * exp   --> 'e'
				 * pwl   --> 'l'
				 * pwc   --> 'c'
				 * sffm  --> 'f'
				 * gauss  --> 'g'
				 */
	control_ *control;
	union {
		struct {	/* Pulse Parameters */
			double *v1;	/* Initial Value (Volt or Amp) */
			double *v2;	/* Pulsed Value (Volt or Amp) */
			double *td;	/* Delay Time (seconds) */
			double *tr;	/* Rise Time (seconds) */
			double *tf;	/* Fall Time (seconds) */
			double *pw;	/* Pulse Width (seconds) */
			double *per;	/* Period (seconds) */
		} pulse;
		struct {	/* Gauss Parameters */
			double *v1;	/* Initial Value (Volt or Amp) */
			double *v2;	/* Pulsed Value (Volt or Amp) */
			double *td;	/* Delay Time (seconds) */
			double *tr;	/* Rise Time (20% to 80%) (seconds) */
			double *tf;	/* Fall Time (20% to 80%) (seconds) */
			double *pw;	/* Pulse Width (seconds) */
			double *per;	/* Period (seconds) */
		} gauss;
		struct {	/* Sin Parameters */
			double *vo;	/* Offset (Volt or Amp) */
			double *va;	/* Amplitude (Volt or Amp) */
			double *fc;	/* Frequency (Hz) */
			double *td;	/* Delay Time (seconds) */
			double *df;	/* Damping Factor (Hz) */
		} sin;
		struct {	/* Exponential Parameters */
			double *v1;	/* Initial Value (Volt or Amp) */
			double *v2;	/* Pulsed Value (Volt or Amp) */
			double *td1;	/* Rise Delay Time (seconds) */
			double *tu1;	/* Rise Time Constant (seconds) */
			double *td2;	/* Fall Delay Time (seconds) */
			double *tu2;	/* Fall Time Constant (seconds) */
		} exp;
		struct { /* Piece-Wise Parameters */
			piecewise_ *pw;
			int index;
			double dc;
		} pw;
		struct {	/* Single-Frequency FM Parameters */
			double *vo;	/* Offset (Volt or Amp) */
			double *va;	/* Amplitude (Volt or Amp) */
			double *fc;	/* Carrier Frequency (Hz) */
			double *mdi;	/* Modulation Index (unit-less) */
			double *fs;	/* Signal Frequency (Hz) */
		} sffm;
	} d;
	/* Default Settings */
	double zero;
	double tstep;
	double tstop;
	double fmin;
	double td2;
};

/*===========================================================================*/

static int waveformParseArgs(waveform_ *r, double *args[7], double **dcPtr)
{
	int i, argsUsed = 0;
	switch(r->type) {
	case 'p': /* pulse */
		argsUsed = 7;
		r->d.pulse.v1 = args[0];
		r->d.pulse.v2 = args[1];
		r->d.pulse.td = args[2];
		r->d.pulse.tr = args[3];
		r->d.pulse.tf = args[4];
		r->d.pulse.pw = args[5];
		r->d.pulse.per = args[6];
		*dcPtr = r->d.pulse.v1;
		break;
	case 'g': /* gauss */
		argsUsed = 7;
		r->d.pulse.v1 = args[0];
		r->d.pulse.v2 = args[1];
		r->d.pulse.td = args[2];
		r->d.pulse.tr = args[3];
		r->d.pulse.tf = args[4];
		r->d.pulse.pw = args[5];
		r->d.pulse.per = args[6];
		*dcPtr = r->d.pulse.v1;
		break;
	case 's': /* sin */
		argsUsed = 5;
		r->d.sin.vo = args[0];
		r->d.sin.va = args[1];
		r->d.sin.fc = args[2];
		r->d.sin.td = args[3];
		r->d.sin.df = args[4];
		*dcPtr = r->d.sin.vo;
		break;
	case 'e': /* exp */
		argsUsed = 6;
		r->d.exp.v1 = args[0];
		r->d.exp.v2 = args[1];
		r->d.exp.td1 = args[2];
		r->d.exp.tu1 = args[3];
		r->d.exp.td2 = args[4];
		r->d.exp.tu2 = args[5];
		*dcPtr = r->d.exp.v1;
		break;
	case 'l': /* pwl */
	case 'c': /* pwc */
		argsUsed = 2;
		r->d.pw.pw = piecewiseNew(r->d.pw.pw, (double**)args[0],
				(int*)args[1], r->type);
		ReturnErrIf(r->d.pw.pw == NULL);
		*dcPtr = &r->d.pw.dc;
		break;
	case 'f': /* sffm */
		argsUsed = 5;
		r->d.sffm.vo = args[0];
		r->d.sffm.va = args[1];
		r->d.sffm.fc = args[2];
		r->d.sffm.mdi = args[3];
		r->d.sffm.fs = args[4];
		*dcPtr = r->d.sffm.vo;
		break;
	default:
		ReturnErr("Unsupported waveform type %c", r->type);
	}

	for(i = 0; i < argsUsed; i++) {
		ReturnErrIf(args[i] == NULL, "Argument %i is NULL", i);
	}

	return 0;
}

/*===========================================================================*/

int waveformNextStep(waveform_ *r, double *nextStep)
{
	double tp, time;
	ReturnErrIf(r == NULL);
	ReturnErrIf(nextStep == NULL);

	time = r->control->time;

	switch(r->type) {
	case 'p': /* pulse */
		tp = fmod(time,  *r->d.pulse.per);
		*nextStep = *r->d.pulse.td - tp;
		if(*nextStep > 0) break;
		*nextStep += *r->d.pulse.tr;
		if(*nextStep > 0) break;
		*nextStep += *r->d.pulse.pw;
		if(*nextStep > 0) break;
		*nextStep += *r->d.pulse.tf;
		if(*nextStep > 0) break;
		*nextStep = *r->d.pulse.per - tp;
		break;
	case 'g': /* gauss */
		/* Should be a smooth waveform so don't expect break-points */
		break;
	case 's': /* sin */
		if(time < *r->d.sin.td) {
			*nextStep = *r->d.sin.td;
		}
		break;
	case 'e': /* exp */
		if(time < *r->d.exp.td1) {
			*nextStep = *r->d.exp.td1 - time;
		} else if(time < *r->d.exp.td2) {
			*nextStep = *r->d.exp.td2 - time;
		}
		break;
	case 'l': /* pwl */
	case 'c': /* pwc */
		ReturnErrIf(piecewiseGetNextX(r->d.pw.pw, &r->d.pw.index, time, &tp));
		if(tp != HUGE_VAL) {
			*nextStep = tp - time;
		}
		break;
	default:
		break;
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int waveformCalcValue(waveform_ *r, double *value)
{
	double time;
	double v1, v2, tr, td, tf, pw, per, tn;
	double vo, va, freq, theta, td1, td2;
	double tau1, tau2;
	double fc, mdi, fs;
	double dvdt;

	time = r->control->time;
	*value = 0.0;

	switch(r->type) {
	case 'p': /* pulse */
		v1 = *r->d.pulse.v1;
		v2 = *r->d.pulse.v2;
		td = *r->d.pulse.td;
		tr = *r->d.pulse.tr;
		tf = *r->d.pulse.tf;
		pw = *r->d.pulse.pw;
		per = *r->d.pulse.per;
		tn = fmod(time, per);
		if(tn <= td) {
			*value = v1;
		} else if(tn <= (tr + td)) {
			*value = v1 + ((v2 - v1)/tr)*(tn - td);
		} else if(tn <= (tr + td + pw)) {
			*value = v2;
		} else if(tn <= (tr + td + pw + tf)) {
			*value = v2 - ((v2 - v1)/(tf))*(tn - (tr + td + pw));
		} else {
			*value = v1;
		}
		break;
	case 'g': /* gauss */
		/* This is based on the equations in "High-Speed Digital Design" by
			Johnson and Graham, Appendix B, scaling was done using table B.1 */
		v1 = *r->d.pulse.v1;
		v2 = *r->d.pulse.v2;
		td = *r->d.pulse.td;
		tr = *r->d.pulse.tr;
		tf = *r->d.pulse.tf;
		pw = *r->d.pulse.pw;
		per = *r->d.pulse.per;
		tn = fmod(time, per);

		/* Calculate 't3' using the relationships in table B.1, remember that
			the provided values (td and tf from the user) are the 20% to 80%
			rise and fall times */

		tr = (tr/0.672)*0.281*2;
		tf = (tf/0.672)*0.281*2;

		/* Calculate the values to send to the error function */
		tr = (tn - (tr/0.281)*0.672-td)/tr;
		tf = (tn - (tf/0.281)*0.672-td-pw)/tf;

		/* Calculate the error functions */
		ReturnErrIf(netlibERF(tr, &vo));
		ReturnErrIf(netlibERF(tf, &va));

		/* Calculate the voltage value */
		*value = v1;
		*value += 0.5*(v2-v1)*(1+vo);
		*value += 0.5*(v1-v2)*(1+va);

		break;
	case 's': /* sin */
		vo = *r->d.sin.vo;
		va = *r->d.sin.va;
		freq = *r->d.sin.fc;
		td = *r->d.sin.td;
		theta = *r->d.sin.df;
		if(time <= td) {
			*value = vo;
		} else {
			*value = vo + va*exp(-(time-td)*theta)*sin(2*M_PI*freq*(time-td));
		}
		break;
	case 'e': /* exp */
		v1 = *r->d.exp.v1;
		v2 = *r->d.exp.v2;
		td1 = *r->d.exp.td1;
		tau1 = *r->d.exp.tu1;
		td2 = *r->d.exp.td2;
		tau2 = *r->d.exp.tu2;
		if(time <= td1) {
			*value = v1;
		} else if(time <= td2) {
			*value = v1 + (v2-v1)*(1-exp(-(time-td1)/tau1));
		} else {
			*value = v1 + (v2-v1)*(1-exp(-(time-td1)/tau1)) +
					(v1-v2)*(1-exp(-(time-td2)/tau2));
		}
		break;
	case 'l': /* pwl */
	case 'c': /* pwc */
		ReturnErrIf(piecewiseCalcValue(r->d.pw.pw, &r->d.pw.index, time,
				value, &dvdt));
		break;
	case 'f': /* sffm */
		vo = *r->d.sffm.vo;
		va = *r->d.sffm.va;
		fc = *r->d.sffm.fc;
		mdi = *r->d.sffm.mdi;
		fs = *r->d.sffm.fs;
		*value = vo + va*sin(2*M_PI*fc*time + mdi*sin(2*M_PI*fs*time));
		break;
	default:
		ReturnErr("Unsupported waveform type %c", r->type);
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

int waveformInitialize(waveform_ *r)
{
	double dvdt;

	r->zero = 0.0;
	r->tstep = r->control->tstep;
	ReturnErrIf(r->tstep == HUGE_VAL);
	/* Want to be just longer than the end */
	r->tstop = r->control->tstop+r->tstep;
	ReturnErrIf(r->tstop == HUGE_VAL);
	r->fmin = 1 / r->tstop;

	switch(r->type) {
	case 'p': /* pulse */
		if(*r->d.pulse.td == HUGE_VAL)
			r->d.pulse.td = &r->zero;
		if(*r->d.pulse.tf == HUGE_VAL)
			r->d.pulse.tf = &r->tstep;
		if(*r->d.pulse.pw == HUGE_VAL)
			r->d.pulse.pw = &r->tstop;
		if(*r->d.pulse.per == HUGE_VAL)
			r->d.pulse.per = &r->tstop;
		break;
	case 'g': /* gauss */
		if(*r->d.pulse.td == HUGE_VAL)
			r->d.pulse.td = &r->zero;
		if(*r->d.pulse.tf == HUGE_VAL)
			r->d.pulse.tf = &r->tstep;
		if(*r->d.pulse.pw == HUGE_VAL)
			r->d.pulse.pw = &r->tstop;
		if(*r->d.pulse.per == HUGE_VAL)
			r->d.pulse.per = &r->tstop;
		break;
	case 's': /* sin */
		if(*r->d.sin.fc == HUGE_VAL)
			r->d.sin.fc = &r->fmin;
		if(*r->d.sin.td == HUGE_VAL)
			r->d.sin.td = &r->zero;
		if(*r->d.sin.df == HUGE_VAL)
			r->d.sin.df = &r->zero;
		break;
	case 'e': /* exp */
		if(*r->d.exp.td1 == HUGE_VAL)
			r->d.exp.td1 = &r->zero;
		if(*r->d.exp.tu1 == HUGE_VAL)
			r->d.exp.tu1 = &r->tstep;
		if(*r->d.exp.td2 == HUGE_VAL) {
			r->td2 = r->tstep + (*r->d.exp.td1);
			r->d.exp.td2 = &r->td2;
		}
		if(*r->d.exp.tu2 == HUGE_VAL)
			r->d.exp.tu2 = &r->tstep;
		break;
	case 'l': /* pwl */
	case 'c': /* pwc */
		ReturnErrIf(piecewiseInitialize(r->d.pw.pw));
		ReturnErrIf(piecewiseCalcValue(r->d.pw.pw, &r->d.pw.index, 0.0,
				&r->d.pw.dc, &dvdt));
		break;
	case 'f': /* sffm */
		if(*r->d.sffm.fc == HUGE_VAL)
			r->d.sffm.fc = &r->fmin;
		if(*r->d.sffm.mdi == HUGE_VAL)
			r->d.sffm.mdi = &r->zero;
		if(*r->d.sffm.fs == HUGE_VAL)
			r->d.sffm.fs = &r->fmin;
		break;
	default:
		break;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

int waveformDestroy(waveform_ **r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf((*r) == NULL);
	Debug("Destroying Waveform %p", *r);

	if((((*r)->type == 'l') || ((*r)->type == 'c')) &&
			((*r)->d.pw.pw != NULL)) {
		if(piecewiseDestroy(&(*r)->d.pw.pw)) {
			Warn("Error destroying piecewise");
		}
	}

	free(*r);
	*r = NULL;

	return 0;
}

/*---------------------------------------------------------------------------*/

waveform_ * waveformNew(waveform_ *r, control_ *control, char type,
		double *args[7], double **dcPtr)
{
	ReturnNULLIf(r != NULL);
	ReturnNULLIf(control == NULL);
	ReturnNULLIf((type != 'p') && (type != 'g') && (type != 's') &&
			(type != 'e') && (type != 'l') && (type != 'c') && (type != 'f'));
	ReturnNULLIf(args == NULL);

	r = calloc(1, sizeof(waveform_));
	ReturnNULLIf(r == NULL, "Malloc Failed");

	Debug("Creating Waveform %p", r);

	r->type = type;
	r->control = control;
	ReturnNULLIf(waveformParseArgs(r, args, dcPtr));

	return r;
}

/*===========================================================================*/

