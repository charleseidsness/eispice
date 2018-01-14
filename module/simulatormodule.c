/*
 * Copyright (C) 2006-2007 Cooper Street Innovations Inc.
 *    Charles Eidsness    <charles@cooper-street.com>
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

#include <Python.h>
#include <structmember.h>
#include <numpy/arrayobject.h>

/* This is to support some earlier versions of numpy */
#ifndef NPY_OWNDATA
#define NPY_OWNDATA OWNDATA
#endif

#include <log.h>
LogMaster;
#include <simulator.h>

/*===========================================================================
 |                                 Waveforms                                 |
  ===========================================================================*/

/*===========================================================================
 |                                Piece-Wise                                 |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    char type;
    PyArrayObject *pw;
} pw_;

/*---------------------------------------------------------------------------*/

static PyMemberDef pwMembers[] = {
    {"type", T_CHAR, offsetof(pw_, type), READONLY,
            "Type (l = linear, c = cspline)"},
    {"pw", T_OBJECT, offsetof(pw_, pw), READONLY, "Data (2D Array)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void pwDestroy(pw_ *r)
{
    Py_XDECREF(r->pw);
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int pwInit(pw_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"pw", NULL};

    PyObject *pw;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "O:PW", kwlist, &pw));

    r->pw = (PyArrayObject *)PyArray_ContiguousFromObject(pw,
            PyArray_DOUBLE, 2, 2);

    ReturnErrIf(r->pw == NULL);

    Py_INCREF(r->pw);

    return 0;
}

/*---------------------------------------------------------------------------*/

static int pwcInit(pw_ *r, PyObject *args, PyObject *kwds)
{
    ReturnErrIf(pwInit(r, args, kwds));
    r->type = 'c';
    return 0;
}

/*---------------------------------------------------------------------------*/

static int pwlInit(pw_ *r, PyObject *args, PyObject *kwds)
{
    ReturnErrIf(pwInit(r, args, kwds));
    r->type = 'l';
    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject pwlType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.pw",
    .tp_basicsize = sizeof(pw_),
    .tp_dealloc = (destructor)pwDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "pw",
    .tp_members = pwMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)pwlInit,
};

/*---------------------------------------------------------------------------*/

static PyTypeObject pwcType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.pw",
    .tp_basicsize = sizeof(pw_),
    .tp_dealloc = (destructor)pwDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "pw",
    .tp_members = pwMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)pwcInit,
};

/*===========================================================================
 |                                    SFFM                                   |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    double vo;
    double va;
    double fc;
    double mdi;
    double fs;
} sffm_;

/*---------------------------------------------------------------------------*/

static PyMemberDef sffmMembers[] = {
    {"Vo", T_DOUBLE, offsetof(sffm_, vo), 0, "Offset Value (V or A)"},
    {"Va", T_DOUBLE, offsetof(sffm_, va), 0, "Amplitude (V or A)"},
    {"Fc", T_DOUBLE, offsetof(sffm_, fc), 0, "Frequency (Hz)"},
    {"MDI", T_DOUBLE, offsetof(sffm_, mdi), 0, "Modulation Index"},
    {"Fs", T_DOUBLE, offsetof(sffm_, fs), 0, "Signal Frequency"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void sffmDestroy(sffm_ *r)
{
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int sffmInit(sffm_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"Vo", "Va", "Fc", "MDI", "Fs", NULL};

    r->fc = HUGE_VAL;
    r->mdi = HUGE_VAL;
    r->fs = HUGE_VAL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "dd|ddd:sffm", kwlist,
            &r->vo, &r->va, &r->fc, &r->mdi, &r->fs));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject sffmType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.sffm",
    .tp_basicsize = sizeof(sffm_),
    .tp_dealloc = (destructor)sffmDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "sffm",
    .tp_members = sffmMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)sffmInit,
};

/*===========================================================================
 |                                Exponential                                |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    double v1;
    double v2;
    double td1;
    double tau1;
    double td2;
    double tau2;
} exp_;

/*---------------------------------------------------------------------------*/

static PyMemberDef expMembers[] = {
    {"V1", T_DOUBLE, offsetof(exp_, v1), 0, "Offset Value (V or A)"},
    {"V2", T_DOUBLE, offsetof(exp_, v2), 0, "Amplitude (V or A)"},
    {"Td1", T_DOUBLE, offsetof(exp_, td1), 0, "Frequency (Hz)"},
    {"Tau1", T_DOUBLE, offsetof(exp_, tau1), 0, "Time Delay (seconds)"},
    {"Td2", T_DOUBLE, offsetof(exp_, td2), 0, "Damping Factor"},
    {"Tau2", T_DOUBLE, offsetof(exp_, tau2), 0, "Damping Factor"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void expDestroy(exp_ *r)
{
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int expInit(exp_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"V1", "V2", "Td1", "Tau1", "Td2", "Tau2", NULL};

    r->td1 = HUGE_VAL;
    r->tau1 = HUGE_VAL;
    r->td2 = HUGE_VAL;
    r->tau2 = HUGE_VAL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "dd|dddd:exp", kwlist,
            &r->v1, &r->v2, &r->td1, &r->tau1, &r->td2, &r->tau2));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject expType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.exp",
    .tp_basicsize = sizeof(exp_),
    .tp_dealloc = (destructor)expDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "exp",
    .tp_members = expMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)expInit,
};

/*===========================================================================
 |                                   Pulse                                   |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    double v1;
    double v2;
    double td;
    double tr;
    double tf;
    double pw;
    double per;
} pulse_;

/*---------------------------------------------------------------------------*/

static PyMemberDef pulseMembers[] = {
    {"V1", T_DOUBLE, offsetof(pulse_, v1), 0, "Initial Value (V or A)"},
    {"V2", T_DOUBLE, offsetof(pulse_, v2), 0, "Pulsed Value (V or A)"},
    {"Td", T_DOUBLE, offsetof(pulse_, td), 0, "Delay Time (seconds)"},
    {"Tr", T_DOUBLE, offsetof(pulse_, tr), 0, "Rise Time (seconds)"},
    {"Tf", T_DOUBLE, offsetof(pulse_, tf), 0, "Fall Time (seconds)"},
    {"PW", T_DOUBLE, offsetof(pulse_, pw), 0, "Pulse Width (seconds)"},
    {"Per", T_DOUBLE, offsetof(pulse_, per), 0, "Period (seconds)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void pulseDestroy(pulse_ *r)
{
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int pulseInit(pulse_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"V1", "V2", "Td", "Tr", "Tf", "PW", "Per", NULL};

    r->td = HUGE_VAL;
    r->tr = HUGE_VAL;
    r->tf = HUGE_VAL;
    r->pw = HUGE_VAL;
    r->per = HUGE_VAL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "dd|ddddd:pulse",
            kwlist, &r->v1, &r->v2, &r->td, &r->tr, &r->tf, &r->pw, &r->per));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject pulseType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.pulse",
    .tp_basicsize = sizeof(pulse_),
    .tp_dealloc = (destructor)pulseDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "pulse",
    .tp_members = pulseMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)pulseInit,
};

/*===========================================================================
 |                                   Gauss                                   |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    double v1;
    double v2;
    double td;
    double tr;
    double tf;
    double pw;
    double per;
} gauss_;

/*---------------------------------------------------------------------------*/

static PyMemberDef gaussMembers[] = {
    {"V1", T_DOUBLE, offsetof(gauss_, v1), 0, "Initial Value (V or A)"},
    {"V2", T_DOUBLE, offsetof(gauss_, v2), 0, "Pulsed Value (V or A)"},
    {"Td", T_DOUBLE, offsetof(gauss_, td), 0, "Delay Time (seconds)"},
    {"Tr", T_DOUBLE, offsetof(gauss_, tr), 0, "Rise Time (20% to 80%) (seconds)"},
    {"Tf", T_DOUBLE, offsetof(gauss_, tf), 0, "Fall Time (20% to 80%) (seconds)"},
    {"PW", T_DOUBLE, offsetof(gauss_, pw), 0, "Pulse Width (seconds)"},
    {"Per", T_DOUBLE, offsetof(gauss_, per), 0, "Period (seconds)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void gaussDestroy(gauss_ *r)
{
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int gaussInit(gauss_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"V1", "V2", "Td", "Tr", "Tf", "PW", "Per", NULL};

    r->td = HUGE_VAL;
    r->tr = HUGE_VAL;
    r->tf = HUGE_VAL;
    r->pw = HUGE_VAL;
    r->per = HUGE_VAL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "dd|ddddd:gauss",
            kwlist, &r->v1, &r->v2, &r->td, &r->tr, &r->tf, &r->pw, &r->per));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject gaussType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.gauss",
    .tp_basicsize = sizeof(gauss_),
    .tp_dealloc = (destructor)gaussDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "gauss",
    .tp_members = gaussMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)gaussInit,
};

/*===========================================================================
 |                                 Sinunsodial                               |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    double vo;
    double va;
    double fc;
    double td;
    double df;
} sin_;

/*---------------------------------------------------------------------------*/

static PyMemberDef sinMembers[] = {
    {"Vo", T_DOUBLE, offsetof(sin_, vo), 0, "Offset Value (V or A)"},
    {"Va", T_DOUBLE, offsetof(sin_, va), 0, "Amplitude (V or A)"},
    {"Fc", T_DOUBLE, offsetof(sin_, fc), 0, "Frequency (Hz)"},
    {"Td", T_DOUBLE, offsetof(sin_, td), 0, "Delay Time (seconds)"},
    {"DF", T_DOUBLE, offsetof(sin_, df), 0, "Damping Factor"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void sinDestroy(sin_ *r)
{
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int sinInit(sin_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"Vo", "Va", "Fc", "Td", "DF", NULL};

    r->fc = HUGE_VAL;
    r->td = HUGE_VAL;
    r->df = HUGE_VAL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "dd|ddd:sin", kwlist,
            &r->vo, &r->va, &r->fc, &r->td, &r->df));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject sinType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.sin",
    .tp_basicsize = sizeof(sin_),
    .tp_dealloc = (destructor)sinDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "sin",
    .tp_members = sinMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)sinInit,
};

/*===========================================================================
 |                                 Devices                                   |
  ===========================================================================*/

/*===========================================================================
 |                             Device Base Class                             |
  ===========================================================================*/

#define DeviceDealloc(device) deviceDestroy(&device)

#define DeviceInit(device, args) deviceInit(&device, args, NULL)

typedef struct {
    PyObject_HEAD
    PyObject *node;
} device_;

/*---------------------------------------------------------------------------*/

static PyMemberDef deviceMembers[] = {
    {"node", T_OBJECT, offsetof(device_, node), READONLY, "Node List"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void deviceDestroy(device_ *r)
{
    Debug("Destroying Device");
    Py_XDECREF(r->node);
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int deviceInit(device_ *r, PyObject *args, PyObject *kwds)
{
    Debug("Initializing Device");

    r->node = args;
    ReturnErrIf(r->node == NULL);
    Py_INCREF(r->node);

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject deviceType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.Device",
    .tp_basicsize = sizeof(device_),
    .tp_dealloc = (destructor)deviceDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Device Base Class",
    .tp_members = deviceMembers,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)deviceInit,
};

/*===========================================================================
 |                                 VI Curve                                  |
  ===========================================================================*/

typedef struct {
    device_ device;
    pw_ *vi;
    double *viData;
    int viLength;
    char viType;
    pw_ *ta;
    double *taData;
    int taLength;
    char taType;
} vicurve_;

/*---------------------------------------------------------------------------*/

static void vicurveDestroy(vicurve_ *r)
{
    Py_XDECREF(r->vi);
    Py_XDECREF(r->ta);
    DeviceDealloc(r->device);
}

/*---------------------------------------------------------------------------*/

static PyMemberDef vicurveMembers[] = {
    {"VI", T_OBJECT, offsetof(vicurve_, vi), 0, "VI Piece-Wise"},
    {"TA", T_OBJECT, offsetof(vicurve_, ta), 0, "TA Piece-Wise"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int vicurveSetAttr(vicurve_ *r, PyObject *name, PyObject *stimulus)
{
    if(!strcmp(PyString_AsString(name), "VI")) {
        ReturnErrIf(!(PyObject_TypeCheck(stimulus, &pwlType)
                || PyObject_TypeCheck(stimulus, &pwcType)),
                "Must be a PW object");
        ReturnErrIf(r->viType != ((pw_*)stimulus)->type,
                "Can't change type of pre-existing PW");
        Py_XDECREF(r->vi);
        r->vi = (pw_*)stimulus;
        r->viData = (double *)&((pw_*)stimulus)->pw->data;
        r->viLength = ((pw_*)stimulus)->pw->dimensions[0];
    } else if(!strcmp(PyString_AsString(name), "TA")) {
        ReturnErrIf(r->ta == NULL, "Can't add a multiplier to a pre-existing "
            "VI-Curve with no multiplier.");
        ReturnErrIf(!(PyObject_TypeCheck(stimulus, &pwlType)
                || PyObject_TypeCheck(stimulus, &pwcType)),
                "Must be a PW object");
        ReturnErrIf(r->taType != ((pw_*)stimulus)->type,
                "Can't change type of pre-existing PW");
        Py_XDECREF(r->ta);
        r->ta = (pw_*)stimulus;
        r->taData = (double *)&((pw_*)stimulus)->pw->data;
        r->taLength = ((pw_*)stimulus)->pw->dimensions[0];
    } else {
        return PyObject_GenericSetAttr((PyObject *)r, name, stimulus);
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static int vicurveInit(vicurve_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    static char *kwlist[] = {"pNode", "nNode", "VI", "TA", NULL};

    r->ta = NULL;
    r->taData = NULL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|O:VICurve",
            kwlist, &pNode, &nNode, &r->vi, &r->ta));

    ReturnErrIf(!(PyObject_TypeCheck(r->vi, &pwlType)
                || PyObject_TypeCheck(r->vi, &pwcType)),
                "Must be a PW object");
    if(r->ta != NULL) {
        ReturnErrIf(!(PyObject_TypeCheck(r->ta, &pwlType)
                || PyObject_TypeCheck(r->ta, &pwcType)),
                "Must be a PW object");
    }

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    r->viData = (double *)r->vi->pw->data;
    r->viLength = r->vi->pw->dimensions[0];
    r->viType = r->vi->type;
    if(r->ta != NULL) {
        r->taData = (double *)r->ta->pw->data;
        r->taLength = r->ta->pw->dimensions[0];
        r->taType = r->ta->type;
    }

    Py_INCREF(r->vi);
    Py_XINCREF(r->ta);

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject vicurveType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.VICurve",
    .tp_basicsize = sizeof(vicurve_),
    .tp_dealloc = (destructor)vicurveDestroy,
    .tp_setattro = (setattrofunc)vicurveSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "VICurve",
    .tp_members = vicurveMembers,
    .tp_init = (initproc)vicurveInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int vicurveAdd(vicurve_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddVICurve(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            &r->viData, &r->viLength, r->viType, &r->taData, &r->taLength,
            r->taType));
    return 0;
}

/*===========================================================================
 |                                   T-Line                                  |
  ===========================================================================*/

typedef struct {
    device_ device;
    double    Z0;
    double    Td;
    double    loss;
} tline_;

/*---------------------------------------------------------------------------*/

static PyMemberDef tlineMembers[] = {
    {"Z0", T_DOUBLE, offsetof(tline_, Z0), 0, "Characteristic Impedance (Ohms)"},
    {"Td", T_DOUBLE, offsetof(tline_, Td), 0, "Propigation Delay (seconds)"},
    {"loss", T_DOUBLE, offsetof(tline_, loss), 0, "Loss (loss factor times length)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int tlineInit(tline_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNodeLeft, *nNodeLeft, *pNodeRight, *nNodeRight;

    static char *kwlist[] = {"pNodeLeft", "nNodeLeft", "pNodeRight",
            "nNodeRight", "Z0", "Td", "loss", NULL};

    r->loss = HUGE_VAL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOOOdd|d:T", kwlist,
            &pNodeLeft, &nNodeLeft, &pNodeRight, &nNodeRight,
            &r->Z0, &r->Td, &r->loss));

    DeviceInit(r->device, Py_BuildValue("OOOO", pNodeLeft, nNodeLeft,
            pNodeRight, nNodeRight));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject tlineType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.TLine",
    .tp_basicsize = sizeof(tline_),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "TLine",
    .tp_members = tlineMembers,
    .tp_init = (initproc)tlineInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int tlineAdd(tline_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddTLine(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 2)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 3)),
            &r->Z0, &r->Td, &r->loss));
    return 0;
}

/*===========================================================================
 |                             T-Line (W-Element)                            |
  ===========================================================================*/

typedef struct {
    device_ device;
    int M;                /* order of approixmation */
    double len;         /* Length of the T-Line in inches */
    PyArrayObject *L0;    /* DC inductance matrix per unit length */
    PyArrayObject *C0;    /* DC capacitance matrix per unit length */
    PyArrayObject *R0;    /* DC resistance matrix per unit length */
    PyArrayObject *G0;    /* DC shunt conductance matrix per unit length */
    PyArrayObject *Rs;    /* Skin-effect resistance matrix per unit length */
    PyArrayObject *Gd;    /* Dielectric-loss conductance matrix per unit length */
    double fgd;            /* Cut-off for the Dielectric Loss */
    double fK;            /* Cut-off frequency */
} tlineW_;

/*---------------------------------------------------------------------------*/

static PyMemberDef tlineWMembers[] = {
    {"M"    , T_INT, offsetof(tlineW_, M), 0,
            "order of approixmation"},
    {"len", T_DOUBLE, offsetof(tlineW_, len), 0,
            "Length of the T-Line in inches"},
    {"L0", T_OBJECT, offsetof(tlineW_, L0), 0,
            "DC inductance matrix per unit length"},
    {"C0", T_OBJECT, offsetof(tlineW_, C0), 0,
            "DC capacitance matrix per unit length"},
    {"R0", T_OBJECT, offsetof(tlineW_, R0), 0,
            "DC resistance matrix per unit length"},
    {"G0", T_OBJECT, offsetof(tlineW_, G0), 0,
            "DC shunt conductance matrix per unit length"},
    {"Rs", T_OBJECT, offsetof(tlineW_, Rs), 0,
            "Skin-effect resistance matrix per unit length"},
    {"Gd", T_OBJECT, offsetof(tlineW_, Gd), 0,
            "Dielectric-loss conductance matrix per unit length"},
    {"fgd", T_DOUBLE, offsetof(tlineW_, fgd), 0,
            "Cut-off for the Dielectric Loss"},
    {"fK    ", T_DOUBLE, offsetof(tlineW_, fK), 0, "Cut-off frequency"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static void tlineWDestroy(tlineW_ *r)
{
    Py_XDECREF(r->L0);
    Py_XDECREF(r->C0);
    Py_XDECREF(r->R0);
    Py_XDECREF(r->G0);
    Py_XDECREF(r->Rs);
    Py_XDECREF(r->Gd);
    DeviceDealloc(r->device);
}


/*---------------------------------------------------------------------------*/

static int tlineWInit(tlineW_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *nodes;

    static char *kwlist[] = {"nodes", "M", "len", "L0", "C0", "R0", "G0",
            "Rs", "Gd", "fgd", "fK", NULL};

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OidOOOOOOdd:W", kwlist,
            &nodes, &r->M, &r->len, &r->L0, &r->C0, &r->R0, &r->G0,
            &r->Rs, &r->Gd, &r->fgd, &r->fK));

    Py_INCREF(r->L0);
    Py_INCREF(r->C0);
    Py_INCREF(r->R0);
    Py_INCREF(r->G0);
    Py_INCREF(r->Rs);
    Py_INCREF(r->Gd);

    DeviceInit(r->device, nodes);

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject tlineWType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.TLineW",
    .tp_basicsize = sizeof(tlineW_),
    .tp_dealloc = (destructor)tlineWDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "TLine W-Elemnet",
    .tp_members = tlineWMembers,
    .tp_init = (initproc)tlineWInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int tlineWAdd(tlineW_ *r, simulator_ *simulator, PyObject *name)
{
    int i, numNodes;
    char **nodes;

    /* Covert the Tuple of PyStrings into an array of C Stings */
    numNodes = PyTuple_Size(((device_*)r)->node);
    nodes = malloc(sizeof(char *)*numNodes);
    for(i = 0; i < numNodes; i++) {
        nodes[i] = PyString_AsString(PyTuple_GetItem(((device_*)r)->node, i));
    }

    ReturnErrIf(simulatorAddTLineW(simulator,
            PyString_AsString(name),
            nodes, numNodes,
            &r->M,
            &r->len,
            (double**)&r->L0->data,
            (double**)&r->C0->data,
            (double**)&r->R0->data,
            (double**)&r->G0->data,
            (double**)&r->Rs->data,
            (double**)&r->Gd->data,
            &r->fgd,
            &r->fK));

    free(nodes);

    return 0;
}

/*===========================================================================
 |                                  Sources                                  |
  ===========================================================================*/

typedef struct {
    device_ device;
    char type;
    double dc;
    PyObject *stimulus;
    char stimulusType;
    double *args[7];
} source_;

/*---------------------------------------------------------------------------*/

static void sourceDestroy(source_ *r)
{
    Py_XDECREF(r->stimulus);
    DeviceDealloc(r->device);
}

/*---------------------------------------------------------------------------*/

static PyMemberDef sourceMembers[] = {
    {"type", T_CHAR, offsetof(source_, type), READONLY, "type"},
    {"DC", T_DOUBLE, offsetof(source_, dc), 0, "DC Value (Amps or Volts)"},
    {"wave", T_OBJECT, offsetof(source_, stimulus), 0, "Waveform"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int sourceSetStimulus(source_ *r, PyObject *stimulus)
{
    if(stimulus == NULL) {
        r->stimulusType = 0x0;
    } else if(PyObject_TypeCheck(stimulus, &sinType)) {
        r->stimulus = stimulus;
        r->stimulusType = 's';
        r->args[0] = &((sin_*)stimulus)->vo;
        r->args[1] = &((sin_*)stimulus)->va;
        r->args[2] = &((sin_*)stimulus)->fc;
        r->args[3] = &((sin_*)stimulus)->td;
        r->args[4] = &((sin_*)stimulus)->df;
        r->args[5] = NULL;
        r->args[6] = NULL;
    } else if(PyObject_TypeCheck(stimulus, &pulseType)) {
        r->stimulus = stimulus;
        r->stimulusType = 'p';
        r->args[0] = &((pulse_*)stimulus)->v1;
        r->args[1] = &((pulse_*)stimulus)->v2;
        r->args[2] = &((pulse_*)stimulus)->td;
        r->args[3] = &((pulse_*)stimulus)->tr;
        r->args[4] = &((pulse_*)stimulus)->tf;
        r->args[5] = &((pulse_*)stimulus)->pw;
        r->args[6] = &((pulse_*)stimulus)->per;
    } else if(PyObject_TypeCheck(stimulus, &gaussType)) {
        r->stimulus = stimulus;
        r->stimulusType = 'g';
        r->args[0] = &((gauss_*)stimulus)->v1;
        r->args[1] = &((gauss_*)stimulus)->v2;
        r->args[2] = &((gauss_*)stimulus)->td;
        r->args[3] = &((gauss_*)stimulus)->tr;
        r->args[4] = &((gauss_*)stimulus)->tf;
        r->args[5] = &((gauss_*)stimulus)->pw;
        r->args[6] = &((gauss_*)stimulus)->per;
    } else if(PyObject_TypeCheck(stimulus, &expType)) {
        r->stimulus = stimulus;
        r->stimulusType = 'e';
        r->args[0] = &((exp_*)stimulus)->v1;
        r->args[1] = &((exp_*)stimulus)->v2;
        r->args[2] = &((exp_*)stimulus)->td1;
        r->args[3] = &((exp_*)stimulus)->tau1;
        r->args[4] = &((exp_*)stimulus)->td2;
        r->args[5] = &((exp_*)stimulus)->tau2;
        r->args[6] = NULL;
    } else if(PyObject_TypeCheck(stimulus, &sffmType)) {
        r->stimulus = stimulus;
        r->stimulusType = 'f';
        r->args[0] = &((sffm_*)stimulus)->vo;
        r->args[1] = &((sffm_*)stimulus)->va;
        r->args[2] = &((sffm_*)stimulus)->fc;
        r->args[3] = &((sffm_*)stimulus)->mdi;
        r->args[4] = &((sffm_*)stimulus)->fs;
        r->args[5] = NULL;
        r->args[6] = NULL;
    } else if(PyObject_TypeCheck(stimulus, &pwlType)
                || PyObject_TypeCheck(stimulus, &pwcType)) {
        r->stimulus = stimulus;
        r->stimulusType = ((pw_*)stimulus)->type;
        r->args[0] = (double *)&((pw_*)stimulus)->pw->data;
        r->args[1] = (double *)&((pw_*)stimulus)->pw->dimensions[0];
        r->args[2] = NULL;
        r->args[3] = NULL;
        r->args[4] = NULL;
        r->args[5] = NULL;
        r->args[6] = NULL;
    } else {
        ReturnErr("Stimulus must be a sin, pulse, exp, sffm, or pw object");
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static int sourceSetAttr(source_ *r, PyObject *name, PyObject *stimulus)
{
    if(!strcmp(PyString_AsString(name), "wave")) {
        ReturnErrIf(stimulus->ob_type != r->stimulus->ob_type,
                "Can't change stimulus type");
        ReturnErrIf(sourceSetStimulus(r, stimulus));
    } else {
        return PyObject_GenericSetAttr((PyObject *)r, name, stimulus);
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static int sourceInit(source_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    static char *kwlist[] = {"pNode", "nNode", "DC", "wave", NULL};

    r->stimulus = NULL;

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOd|O:source", kwlist,
            &pNode, &nNode, &r->dc, &r->stimulus));

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    ReturnErrIf(sourceSetStimulus(r, r->stimulus));

    Py_XINCREF(r->stimulus);

    return 0;
}

/*---------------------------------------------------------------------------*/

static int iSourceInit(source_ *r, PyObject *args, PyObject *kwds)
{
    ReturnErrIf(sourceInit(r, args, kwds));
    r->type = 'i';
    return 0;
}

/*---------------------------------------------------------------------------*/

static int vSourceInit(source_ *r, PyObject *args, PyObject *kwds)
{
    ReturnErrIf(sourceInit(r, args, kwds));
    r->type = 'v';
    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject iSourceType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.source",
    .tp_basicsize = sizeof(source_),
    .tp_dealloc = (destructor)sourceDestroy,
    .tp_setattro = (setattrofunc)sourceSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Current Source",
    .tp_members = sourceMembers,
    .tp_init = (initproc)iSourceInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static PyTypeObject vSourceType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.Source",
    .tp_basicsize = sizeof(source_),
    .tp_dealloc = (destructor)sourceDestroy,
    .tp_setattro = (setattrofunc)sourceSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Voltage Source",
    .tp_members = sourceMembers,
    .tp_init = (initproc)vSourceInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int sourceAdd(source_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddSource(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            r->type, &r->dc,r->stimulusType, r->args));
    return 0;
}

/*===========================================================================
 |                             Non-Linear Source                             |
  ===========================================================================*/

typedef struct {
    device_ device;
    char type;
    PyObject *equation;
} nlSource_;

/*---------------------------------------------------------------------------*/

static void nlSourceDestroy(nlSource_ *r)
{
    Py_XDECREF(r->equation);
    DeviceDealloc(r->device);
}

/*---------------------------------------------------------------------------*/

static PyMemberDef nlSourceMembers[] = {
    {"type", T_CHAR, offsetof(nlSource_, type), READONLY, "type"},
    {"equation", T_OBJECT, offsetof(nlSource_, equation), READONLY, "equation"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int nlSourceInit(nlSource_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    char *type;
    static char *kwlist[] = {"pNode", "nNode", "type", "equation", NULL};

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOsS:B", kwlist,
            &pNode, &nNode, &type, &r->equation));

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    r->type = type[0];
    Py_INCREF(r->equation);

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject nlSourceType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.NLSource",
    .tp_basicsize = sizeof(nlSource_),
    .tp_dealloc = (destructor)nlSourceDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Non-Linear Source",
    .tp_members = nlSourceMembers,
    .tp_init = (initproc)nlSourceInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int nlSourceAdd(nlSource_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddNonlinearSource(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            r->type,
            PyString_AsString(r->equation)));
    return 0;
}

/*===========================================================================
 |                              Call-Back Source                             |
  ===========================================================================*/

typedef struct {
    device_ device;
    char type;
    PyObject *variables;
    PyObject *callback;
    PyArrayObject *values;
    PyArrayObject *derivs;
    PyObject *arglist;
} cbSource_;

/*---------------------------------------------------------------------------*/

static void cbSourceDestroy(cbSource_ *r)
{
    Py_XDECREF(r->variables);
    Py_XDECREF(r->callback);
    Py_XDECREF(r->values);
    Py_XDECREF(r->derivs);
    Py_XDECREF(r->arglist);
    DeviceDealloc(r->device);
}

/*---------------------------------------------------------------------------*/

static PyMemberDef cbSourceMembers[] = {
    {"type", T_CHAR, offsetof(cbSource_, type), READONLY, "type"},
    {"variables", T_OBJECT, offsetof(cbSource_, variables), READONLY, "variables"},
    {"callback", T_OBJECT, offsetof(cbSource_, callback), READONLY, "callback"},
    {"values", T_OBJECT, offsetof(cbSource_, values), 0, "values"},
    {"derivs", T_OBJECT, offsetof(cbSource_, derivs), 0, "derivatives"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int cbSourceInit(cbSource_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    char *type;
    int length;

    static char *kwlist[] = {"pNode", "nNode", "type", "variables",
            "callback", NULL};

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOsOO:CB", kwlist,
            &pNode, &nNode, &type, &r->variables, &r->callback));

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    r->type = type[0];
    ReturnErrIf(!PyCallable_Check(r->callback));
    Py_XINCREF(r->callback);

    ReturnErrIf(!PyTuple_Check(r->variables));
    Py_XINCREF(r->variables);

    length = PyTuple_Size(r->variables);
    ReturnErrIf(length <= 0);

    r->derivs = (PyArrayObject*)PyArray_FromDims(1, &length, PyArray_DOUBLE);
    ReturnErrIf(r->derivs == NULL);
    Py_INCREF(r->derivs);

    r->values = (PyArrayObject*)PyArray_FromDims(1, &length, PyArray_DOUBLE);
    ReturnErrIf(r->values == NULL);
    Py_INCREF(r->values);

    r->arglist = Py_BuildValue("(OO)", r->values, r->derivs);
    Py_INCREF(r->arglist);

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject cbSourceType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.CBSource",
    .tp_basicsize = sizeof(cbSource_),
    .tp_dealloc = (destructor)cbSourceDestroy,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Call-Back Source",
    .tp_members = cbSourceMembers,
    .tp_init = (initproc)cbSourceInit,
    .tp_base = &deviceType,
    .tp_new = (newfunc)PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

int cbSourceCallback(double *xN, void *private)
{
    PyObject *result;

    /* Call the callback */
    result = PyEval_CallObject(((cbSource_*)private)->callback,
            ((cbSource_*)private)->arglist);

    ReturnErrIf(PyErr_Occurred() != NULL, "Python raised an exception");

    if(result != Py_None) {
        *xN = PyFloat_AsDouble(result);
    } else {
        *xN = 0.0;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static int cbSourceAdd(cbSource_ *r, simulator_ *simulator, PyObject *name)
{
    int numVars, i;
    char **vars;

    numVars = PyTuple_Size(r->variables);

    vars = malloc(sizeof(char*)*numVars);
    ReturnErrIf(vars == NULL);

    for(i = 0; i < numVars; i++) {
        vars[i] = PyString_AsString(PyTuple_GetItem(r->variables, i));
    }

    ReturnErrIf(simulatorAddCallbackSource(simulator,
                PyString_AsString(name),
                PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
                PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
                r->type, vars, (double*)r->values->data,
                (double*)r->derivs->data, numVars, cbSourceCallback,
                (void*)r));

    free(vars);

    return 0;
}

/*===========================================================================
 |                                 Inductor                                  |
  ===========================================================================*/

typedef struct {
    device_ device;
    double    L;
} inductor_;

/*---------------------------------------------------------------------------*/

static PyMemberDef inductorMembers[] = {
    {"L", T_DOUBLE, offsetof(inductor_, L), 0, "inductance (Henrys)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int inductorInit(inductor_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    static char *kwlist[] = {"pNode", "nNode", "L", NULL};

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOd:L", kwlist,
            &pNode, &nNode, &r->L));

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject inductorType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.Inductor",
    .tp_basicsize = sizeof(inductor_),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Inductor",
    .tp_members = inductorMembers,
    .tp_init = (initproc)inductorInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int inductorAdd(inductor_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddInductor(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            &r->L));
    return 0;
}

/*===========================================================================
 |                                 Capacitor                                 |
  ===========================================================================*/

typedef struct {
    device_ device;
    double    C;
} capacitor_;

/*---------------------------------------------------------------------------*/

static PyMemberDef capacitorMembers[] = {
    {"C", T_DOUBLE, offsetof(capacitor_, C), 0, "capacitance (Farads)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int capacitorInit(capacitor_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    static char *kwlist[] = {"pNode", "nNode", "C", NULL};

    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOd:C", kwlist,
            &pNode, &nNode, &r->C));

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject capacitorType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.Capacitor",
    .tp_basicsize = sizeof(capacitor_),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Capacitor",
    .tp_members = capacitorMembers,
    .tp_init = (initproc)capacitorInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int capacitorAdd(capacitor_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddCapacitor(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            &r->C));
    return 0;
}

/*===========================================================================
 |                                 Resistor                                  |
  ===========================================================================*/

typedef struct {
    device_ device;
    double    R;
} resistor_;

/*---------------------------------------------------------------------------*/

static PyMemberDef resistorMembers[] = {
    {"R", T_DOUBLE, offsetof(resistor_, R), 0, "resistance (Ohms)"},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static int resistorInit(resistor_ *r, PyObject *args, PyObject *kwds)
{
    PyObject *pNode, *nNode;
    static char *kwlist[] = {"pNode", "nNode", "R", NULL};

    Debug("Initializing Resistor");
    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "OOd:R", kwlist,
            &pNode, &nNode, &r->R));

    DeviceInit(r->device, Py_BuildValue("OO", pNode, nNode));

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject resistorType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.Resistor",
    .tp_basicsize = sizeof(resistor_),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Resistor",
    .tp_members = resistorMembers,
    .tp_init = (initproc)resistorInit,
    .tp_base = &deviceType,
    .tp_new = PyType_GenericNew,
};

/*---------------------------------------------------------------------------*/

static int resistorAdd(resistor_ *r, simulator_ *simulator, PyObject *name)
{
    ReturnErrIf(simulatorAddResistor(simulator,
            PyString_AsString(name),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 0)),
            PyString_AsString(PyTuple_GetItem(((device_*)r)->node, 1)),
            &r->R));
    return 0;
}

/*===========================================================================
 |                               Circuit                                     |
  ===========================================================================*/

typedef struct {
    PyObject_HEAD
    PyObject *title;
    PyObject *devices;
    simulator_ *simulator;
    PyArrayObject *results;
    PyObject *variables;
} circuit_;

/*---------------------------------------------------------------------------*/

static void circuitDestroy(circuit_ *r)
{
    Py_XDECREF(r->title);
    Py_XDECREF(r->devices);
    Py_XDECREF(r->variables);
    Py_XDECREF(r->results);
    if(simulatorDestroy(&r->simulator)) {
        Warn("Failed to destroy simulator");
    }
    r->ob_type->tp_free((PyObject*)r);
}

/*---------------------------------------------------------------------------*/

static int circuitBuildResults(circuit_ *r, double *data, int dims[2])
{
    Py_XDECREF(r->results);
    r->results = (PyArrayObject*)PyArray_FromDimsAndData(2, dims,
            PyArray_DOUBLE, (char*)data);
    ReturnErrIf(r->results == NULL);
    /* We own the data, should free it when we're done with it. */
    /* TODO: Make sure this does what we want and doesn't create
        a memory leak. */
    r->results->flags ^= NPY_OWNDATA;
    Py_INCREF(r->results);
    return 0;
}

/*---------------------------------------------------------------------------*/

static int circuitBuildNames(circuit_ *r, char **vars, int dims[2])
{
    long int i;
    Py_XDECREF(r->variables);
    r->variables = PyList_New(dims[1]);
    for(i = 0; i < dims[1]; i++) {
        ReturnErrIf(PyList_SetItem(r->variables, i,
                PyString_FromString(vars[i])));
    }
    free(vars);
    return 0;
}

/*----------------------------- Operating Point -----------------------------*/

static PyObject * circuitOP(circuit_ *r, PyObject *args)
{
    double *data;
    int dims[2];
    char **vars;

    ReturnNULLIf(!PyArg_ParseTuple(args, ":op"));

    ReturnNULLIf(simulatorRunOperatingPoint(r->simulator, &data, &vars,
            &dims[0], &dims[1]));

    ReturnNULLIf(circuitBuildResults(r, data, dims));
    ReturnNULLIf(circuitBuildNames(r, vars, dims));

    Py_RETURN_NONE;
}

/*---------------------------- Transient Analysis ---------------------------*/

static PyObject * circuitTran(circuit_ *r, PyObject *args)
{
    double tstep, tstop, tmax = 0.0;
    int restart;
    double *data;
    int dims[2];
    char **vars;


    ReturnNULLIf(!PyArg_ParseTuple(args, "dd|di:tran", &tstep, &tstop, &tmax,
            &restart));

    ReturnNULLIf(simulatorRunTransient(r->simulator, tstep, tstop, tmax,
            restart, &data, &vars, &dims[0], &dims[1]));

    ReturnNULLIf(circuitBuildResults(r, data, dims));
    ReturnNULLIf(circuitBuildNames(r, vars, dims));

    Py_RETURN_NONE;
}

/*------------------------------- Print Circuit -----------------------------*/

static PyObject * circuitPrintDevices(circuit_ *r, PyObject *args)
{
    ReturnNULLIf(simulatorPrintDevices(r->simulator));
    Py_RETURN_NONE;
}

/*---------------------------------------------------------------------------*/

static PyMethodDef circuitMethods[] = {
    {"op_", (PyCFunction)circuitOP, METH_VARARGS,
            PyDoc_STR("Operating Point Analysis")},
    {"tran_", (PyCFunction)circuitTran, METH_VARARGS,
            PyDoc_STR("Transient Analysis")},
    {"devices_", (PyCFunction)circuitPrintDevices, METH_VARARGS,
            PyDoc_STR("Print Circuit")},
    {NULL, NULL}        /* sentinel */
};

/*---------------------------------------------------------------------------*/

static PyMemberDef circuitMembers[] = {
    {"title", T_OBJECT, offsetof(circuit_, title), 0, "Circuit Title"},
    {"results", T_OBJECT, offsetof(circuit_, results), READONLY,
            "Results of the last simulation."},
    {"variables", T_OBJECT, offsetof(circuit_, variables), READONLY,
            "List that contains the column headers for the results array."},
    {NULL}  /* Sentinel */
};

/*---------------------------------------------------------------------------*/

static PyObject * circuitGetAttr(circuit_ *r, PyObject *name)
{
    PyObject *device;
    device = PyDict_GetItem(r->devices, name);
    if(device != NULL) {
        Py_INCREF(device);
        return device;
    }
    return PyObject_GenericGetAttr((PyObject *)r, name);
}

/*---------------------------------------------------------------------------*/
static PyTypeObject circuitType;
static int circuitSetAttr(circuit_ *r, PyObject *name, PyObject *device)
{
    ReturnErrIf(device == NULL, "Device removal not supported.");

    ReturnErrIf(PyDict_GetItem(r->devices, name) != NULL,
            "Device %s already exists.", PyString_AsString(name));

    if(PyObject_TypeCheck(device, &inductorType)) {
        /* Inductor */
        ReturnErrIf(inductorAdd((inductor_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &capacitorType)) {
        /* Capacitor */
        ReturnErrIf(capacitorAdd((capacitor_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &resistorType)) {
        /* Resistor */
        ReturnErrIf(resistorAdd((resistor_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &nlSourceType)) {
        /* Non-Linear Source */
        ReturnErrIf(nlSourceAdd((nlSource_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &cbSourceType)) {
        /* Call-Back Source */
        ReturnErrIf(cbSourceAdd((cbSource_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &iSourceType) ||
            PyObject_TypeCheck(device, &vSourceType)) {
        /* Current or Voltage Source */
        ReturnErrIf(sourceAdd((source_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &tlineType)) {
        /* Transmission Line */
        ReturnErrIf(tlineAdd((tline_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &tlineWType)) {
        /* Transmission Line W-Element */
        ReturnErrIf(tlineWAdd((tlineW_*)device, r->simulator, name));
    } else if(PyObject_TypeCheck(device, &vicurveType)) {
        /* VI Curve */
        ReturnErrIf(vicurveAdd((vicurve_*)device, r->simulator, name));
    } else {
        /* Default */
        return PyObject_GenericSetAttr((PyObject *)r, name, device);
    }

    ReturnErrIf(PyDict_SetItem(r->devices, name, device));
    return 0;
}

/*---------------------------------------------------------------------------*/

static int circuitInit(circuit_ *r, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"title", NULL};

    r->title = NULL;
    ReturnErrIf(!PyArg_ParseTupleAndKeywords(args, kwds, "|S:circuit", kwlist,
            &r->title));
    Py_XINCREF(r->title);

    r->devices = PyDict_New();
    ReturnErrIf(r->devices == NULL);
    Py_XINCREF(r->devices);

    r->simulator = simulatorNew(r->simulator);
    ReturnErrIf(r->simulator == NULL);

    r->results = NULL;
    r->variables = NULL;

    return 0;
}

/*---------------------------------------------------------------------------*/

static PyTypeObject circuitType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "simulator.CircuitBase",
    .tp_basicsize = sizeof(circuit_),
    .tp_dealloc = (destructor)circuitDestroy,
    .tp_getattro = (getattrofunc)circuitGetAttr,
    .tp_setattro = (setattrofunc)circuitSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "simulator Circuit Base Class",
    .tp_members = circuitMembers,
    .tp_methods = circuitMethods,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)circuitInit,
};


/*===========================================================================
 |                                 Utilities                                 |
  ===========================================================================*/

static PyObject * logFile(PyObject *self, PyObject *args)
{
    const char *filename;

    ReturnNULLIf(!PyArg_ParseTuple(args, "s:logFile", &filename));

    CloseLogFile;
    OpenLogFile(filename);

    Py_RETURN_NONE;
}

/*---------------------------------------------------------------------------*/

static PyObject * errorFile(PyObject *self, PyObject *args)
{
    const char *filename;

    ReturnNULLIf(!PyArg_ParseTuple(args, "s:errorFile", &filename));

    CloseLogFile;
    OpenErrorFile(filename);

    Py_RETURN_NONE;
}

/*---------------------------------------------------------------------------*/

static PyObject * about(PyObject *self, PyObject *args)
{
    ReturnNULLIf(!PyArg_ParseTuple(args, ":about"));

    Info(" ");
    Info("The eispice simulator module contains the following libraries:");
    Info(" ");
    simulatorInfo();

    Py_RETURN_NONE;
}

/*===========================================================================
 |                                  esipice                                  |
  ===========================================================================*/

static PyMethodDef simulatorMethods[] = {
    {"logFile",  logFile, METH_VARARGS, "Set the log file name."},
    {"errorFile",  errorFile, METH_VARARGS, "Set the error file name."},
    {"about",  about, METH_VARARGS, "Print some library info."},
    {NULL, NULL}        /* sentinel */
};

/*---------------------------------------------------------------------------*/

/* Called when python exits */
void simulatorCleanup(void)
{
    CloseErrorFile;
    CloseLogFile;
}

/*---------------------------------------------------------------------------*/

PyDoc_STRVAR(simulatorDoc, "Circuit Simulator");

PyMODINIT_FUNC initsimulator_(void)
{
    PyObject *m;

    if (PyType_Ready(&circuitType) < 0)        return;
    if (PyType_Ready(&inductorType) < 0)    return;
    if (PyType_Ready(&capacitorType) < 0)    return;
    if (PyType_Ready(&resistorType) < 0)    return;
    if (PyType_Ready(&nlSourceType) < 0)    return;
    if (PyType_Ready(&cbSourceType) < 0)    return;
    if (PyType_Ready(&iSourceType) < 0)        return;
    if (PyType_Ready(&vSourceType) < 0)        return;
    if (PyType_Ready(&sinType) < 0)            return;
    if (PyType_Ready(&gaussType) < 0)        return;
    if (PyType_Ready(&pulseType) < 0)        return;
    if (PyType_Ready(&expType) < 0)            return;
    if (PyType_Ready(&sffmType) < 0)        return;
    if (PyType_Ready(&pwlType) < 0)            return;
    if (PyType_Ready(&pwcType) < 0)            return;
    if (PyType_Ready(&tlineType) < 0)        return;
    if (PyType_Ready(&tlineWType) < 0)        return;
    if (PyType_Ready(&vicurveType) < 0)        return;
    if (PyType_Ready(&deviceType) < 0)        return;

    /* Import the array object */
    import_array();

    /* Create the module and add the functions */
    m = Py_InitModule3("simulator_", simulatorMethods, simulatorDoc);
    if (m == NULL)
        return;

    Py_INCREF(&circuitType);
    PyModule_AddObject(m, "Circuit_", (PyObject *)&circuitType);
    PyModule_AddObject(m, "Inductor_", (PyObject *)&inductorType);
    PyModule_AddObject(m, "Capacitor_", (PyObject *)&capacitorType);
    PyModule_AddObject(m, "Resistor_", (PyObject *)&resistorType);
    PyModule_AddObject(m, "Behavioral_", (PyObject *)&nlSourceType);
    PyModule_AddObject(m, "CallBack_", (PyObject *)&cbSourceType);
    PyModule_AddObject(m, "CurrentSource_", (PyObject *)&iSourceType);
    PyModule_AddObject(m, "VoltageSource_", (PyObject *)&vSourceType);
    PyModule_AddObject(m, "TLine_", (PyObject *)&tlineType);
    PyModule_AddObject(m, "TLineW_", (PyObject *)&tlineWType);
    PyModule_AddObject(m, "VICurve_", (PyObject *)&vicurveType);

    PyModule_AddObject(m, "Sin_", (PyObject *)&sinType);
    PyModule_AddObject(m, "Pulse_", (PyObject *)&pulseType);
    PyModule_AddObject(m, "Gauss_", (PyObject *)&gaussType);
    PyModule_AddObject(m, "Exp_", (PyObject *)&expType);
    PyModule_AddObject(m, "SFFM_", (PyObject *)&sffmType);
    PyModule_AddObject(m, "PWL_", (PyObject *)&pwlType);
    PyModule_AddObject(m, "PWC_", (PyObject *)&pwcType);

    PyModule_AddObject(m, "Device_", (PyObject *)&deviceType);

    Py_AtExit(simulatorCleanup);

}

/*===========================================================================*/

