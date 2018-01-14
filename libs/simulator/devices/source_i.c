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

#include "checkbreak.h"
#include "waveform.h"
#include "device_internal.h"

/* Pin Designations */
#define K			0
#define J			1
#define NP			2

/*===========================================================================
 |                            Private Structure                              |
  ===========================================================================*/

struct _devicePrivate {
	double dc;			/* DC Value (Amps) */
	double *dcParam;
	waveform_ *waveform;
	checkbreak_ *checkbreak;
	row_ *rowK;
	row_ *rowJ;
};

/*===========================================================================
 |                             Class Functions                               |
  ===========================================================================*/

static int deviceClassNextStep(device_ *r, double *nextStep)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->waveform != NULL) {

		Debug("Next Breaking %s %s %p", r->class->type, r->refdes, r);

		ReturnErrIf(waveformNextStep(p->waveform, nextStep));
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassStep(device_ *r, int *breakPoint)
{
	devicePrivate_ *p;
	double dc;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	if(p->waveform != NULL) {

		Debug("Stepping %s %s %p", r->class->type, r->refdes, r);

		ReturnErrIf(waveformCalcValue(p->waveform, &dc));

		ReturnErrIf(rowRHSPlus(p->rowK, -(dc - p->dc)));
		ReturnErrIf(rowRHSPlus(p->rowJ, dc - p->dc));
		p->dc = dc;

		/* Check to see if we changed the voltage enough to warrant a break */
		*breakPoint = checkbreakIsBreak(p->checkbreak, dc);
		ReturnErrIf(*breakPoint < 0);
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassLoad(device_ *r)
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Loading %s %s %p", r->class->type, r->refdes, r);

	/* Initialise / Reset State Data */
	ReturnErrIf(checkbreakInitialize(p->checkbreak, 0.0));
	if(p->waveform != NULL) {
		ReturnErrIf(waveformInitialize(p->waveform));
	}
	ReturnErrIf(p->dcParam == NULL);
	p->dc = *p->dcParam;

	/* Modified Nodal Analysis Stamp
	 *	                  	+  __  -
	 *	  |_Vk_Vj_|_rhs_|	__/  \__
	 *	k | -- -- | -Ir |	k \__/ j
	 *	j | -- -- |  Ir |	------->
	 *	                  	   Ir
	 */

	ReturnErrIf(rowRHSPlus(p->rowK, -p->dc));
	ReturnErrIf(rowRHSPlus(p->rowJ, p->dc));

	return 0;
}

/*--------------------------------------------------------------------------*/

static int deviceClassUnconfig(device_ *r)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Unconfiging %s %s %p", r->class->type, r->refdes, r);

	if(p->checkbreak != NULL) {
		if(checkbreakDestroy(&p->checkbreak)) {
			Warn("Error destroying break check");
		}
	}

	if(p->waveform != NULL) {
		if(waveformDestroy(&p->waveform)) {
			Warn("Error destroying waveform");
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

static int deviceClassPrint(device_ *r)
{
	devicePrivate_ *p;
	ReturnErrIf(r == NULL);
	p = r->private;
	ReturnErrIf(p == NULL);

	Debug("Printing %s %s %p", r->class->type, r->refdes, r);

	Info("%s -- %s %s -> %s; DC = %gA", r->class->type,
			r->refdes, rowGetName(r->pin[K]), rowGetName(r->pin[J]),
			*p->dcParam);

	return 0;
}

/*===========================================================================
 |                                  Class                                    |
  ===========================================================================*/

deviceClass_ deviceCurrentSource = {
	.type = "Current Source",
	.unconfig = deviceClassUnconfig,
	.load = deviceClassLoad,
	.linearize = NULL,
	.initStep = NULL,
	.step = deviceClassStep,
	.minStep = NULL,
	.nextStep = deviceClassNextStep,
	.integrate = NULL,
	.print = deviceClassPrint,
};


/*===========================================================================
 |                              Configuration                                |
  ===========================================================================*/

int deviceCurrentSourceConfig(device_ *r, double *dc, char type,
		double *args[7])
{
	devicePrivate_ *p;

	ReturnErrIf(r == NULL);
	ReturnErrIf(r->private != NULL);
	ReturnErrIf(r->numPins != NP);

	/* Copy in class pointer */
	r->class = &deviceCurrentSource;

	Debug("Configuring %s %s %p", r->class->type, r->refdes, r);

	/* allocate space for private data */
	r->private = calloc(1, sizeof(devicePrivate_));
	ReturnErrIf(r->private == NULL);
	p = r->private;

	/* Copy in parameter pointers */
	if(type != 0x0) {
		p->waveform = waveformNew(p->waveform, r->control, type, args,
				&p->dcParam);
		ReturnErrIf(p->waveform == NULL);
		ReturnErrIf(p->dcParam == NULL);
	} else {
		p->waveform = NULL;
		p->dcParam = dc;
	}

	/* Setup the break checking object */
	p->checkbreak = checkbreakNew(p->checkbreak, r->control, 'A');
	ReturnErrIf(p->checkbreak == NULL);

	/* Create required nodes and rows (see MNA stamp above) */
	p->rowK = r->pin[K];
	ReturnErrIf(p->rowK == NULL);
	p->rowJ = r->pin[J];
	ReturnErrIf(p->rowJ == NULL);

	return 0;
}

/*===========================================================================*/

