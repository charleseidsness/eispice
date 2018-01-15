#
# Copyright (C) 2006-2007 Cooper Street Innovations Inc.
# Charles Eidsness    <charles@cooper-street.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

"""
This module provides the basic eispice devices.

Basic Devices:
L -- Inductor
C -- Capacitor
R -- Resistor
B -- Behaivioral
I -- Current Source
V -- Voltage Source
VI -- Voltage/Current Curve
G -- Voltage-Controlled Current Source
E -- Voltage-Controlled Voltage Source
F -- Current-Controlled Current Source
H -- Current-Controlled Voltage Source
T -- Simple Transmission Line
W -- Multi-Conductor Frequency Dependent T-Line
D -- Diode Model
PyB -- A Python Base Behaivorial Model
D -- Diode Model
Q -- BJT Model

Composite Models:
RealC -- Capacitor Model with ESR and ESL
"""

import warnings

from numpy import array

import units
import simulator_
import calc
import subckt

Current = 'i'
Voltage = 'v'
Capacitor = 'c'
Time = 'time'
GND = '0'

#-----------------------------------------------------------------------------#
#                                Passives                                     #
#-----------------------------------------------------------------------------#

class L(simulator_.Inductor_):
    """Inductor Model

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Inductor Test")
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct.Ix = eispice.I(1, eispice.GND, 4, wave)
    >>> cct.Lx = eispice.L(1, eispice.GND, '10n')
    >>> cct.tran('0.5n', '100n')
    >>> cct.check_v(1, 1.333333e+01, '1.85e-8')
    True
    >>> cct.check_v(1, -2.000000e+01, '1.05e-8')
    True
    """
    def __init__(self, pNode, nNode, L):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        L -- inductance in Henrys
        """
        simulator_.Inductor_.__init__(self, str(pNode), str(nNode),
                units.float(L))

class C(simulator_.Capacitor_):
    """Capacitor Model

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Capacitor Test")
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct.Vx = eispice.V(1, eispice.GND, 4, wave)
    >>> cct.Cx = eispice.C(1, eispice.GND, '10n')
    >>> cct.tran('0.5n', '100n')
    >>> cct.check_i('Vx', 1.333333e+01, '1.85e-8')
    True
    >>> cct.check_i('Vx', -2.000000e+01, '1.05e-8')
    True
    """
    def __init__(self, pNode, nNode, C):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        C -- capacitance in Farads
        """
        simulator_.Capacitor_.__init__(self, str(pNode), str(nNode),
                units.float(C))

class R(simulator_.Resistor_):
    """Resistor Model

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Resistor Test")
    >>> cct.Rv = eispice.R(1, eispice.GND, 324)
    >>> cct.Vx = eispice.V(1, eispice.GND, 12.6)
    >>> cct.op()
    >>> cct.check_v(1, 1.26e+01)
    True
    >>> cct.check_i('Vx', -3.88889e-02)
    True
    """
    def __init__(self, pNode, nNode, R):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        R -- resistance in Ohms
        """
        simulator_.Resistor_.__init__(self, str(pNode), str(nNode),
                units.float(R))

class RealC(subckt.Subckt):
    """Capacitor Model that includes ESL and ESR.

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Real Capacitor Test")
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct.Vx = eispice.V(1, eispice.GND, 4, wave)
    >>> cct.Cx = eispice.RealC(1, eispice.GND, '10n', '1n', '0.1')
    >>> cct.tran('0.5n', '100n')
    >>> cct.check_i('Vx', -4.832985, '1.85e-8')
    True
    >>> cct.check_i('Vx', -0.2574886, '1.05e-8')
    True
    """

    def __init__(self, pNode, nNode, cap, ESL, ESR):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        C -- capacitance in Farads
        ESL -- Effective Series Inductance in Henrys
        ESR -- Effective Series Resistance in Ohms
        """
        self.C = C(pNode, self.node('n0'), cap)
        self.R = R(self.node('n0'), self.node('n1'), ESR)
        self.L = L(self.node('n1'), nNode, ESL)

#-----------------------------------------------------------------------------#
#                              Behaivioral                                    #
#-----------------------------------------------------------------------------#

class B(simulator_.Behavioral_):
    """
    Behaivioral Model

    This is the most versatile device (and the easiest to abuse). This
    device is similar to the B element in Spice3, i.e. it should be
    possible to cut and paste B element equations in from spice3.

    The equation string can consist of any of the following functions and
    operators: abs(), acosh(), acos(), asinh(), asin(), atanh(), atan(),
    cosh(), cos(), exp(), ln(), log(), sinh(), sin(), sqrt(), tan(), uramp(),
    u(), +, -, *, /, ^.

    The function "u" is the unit step function, with a value of one for
    arguments greater than one and a value of zero for arguments less than
    zero. The function "uramp" is the integral of the unit step: for an input
    x, the value is zero if x is less than zero, or if x is greater than zero\
    the value is x.

    It is also possible to add a time variable to the B element equations using
    the keyword time, i.e.

    device.B(2, 0, device.Voltage, 'sin(2*3.14159*100e6*time)')

    Will result in a sinewave input (though it will run slower than the V
    element with a sin waveform input).

    Examples:

    1. Nonlinear Current Source
    >>> import eispice
    >>> cct = eispice.Circuit("Nonlinear Current Test")
    >>> cct.Rv = eispice.R(1, eispice.GND, 10)
    >>> cct.Vx = eispice.V(1, eispice.GND, 7)
    >>> cct.Rb = eispice.R(2, eispice.GND, 3)
    >>> cct.Bx = eispice.B(2, eispice.GND, eispice.Current, \
            "4.739057e-04 * (uramp( v(2,0) --5.060000e+00)) " \
            "+ sqrt(v(1)) / sinh(i(Vx))")
    >>> cct.op()
    >>> cct.check_v(1, 7)
    True
    >>> cct.check_v(2, 10.44121563)
    True
    >>> cct.check_i('Vx', -0.7)
    True

    2. Nonlinear Voltage Source
    >>> import eispice
    >>> cct = eispice.Circuit("Nonlinear Time Test")
    >>> cct.Rb = eispice.R(2, eispice.GND, 3)
    >>> cct.Bx = eispice.B(2, eispice.GND, eispice.Voltage, \
            'sin(2*3.14159*100e6*time)')
    >>> cct.tran('0.5n', '15n')
    >>> cct.check_v(2, 9.486671194e-01, '3n')
    True
    >>> cct.check_v(2, 3.088685702e-01, '10.5n')
    True

    3. Nonlinear Capacitor
    >>> import eispice
    >>> cct = eispice.Circuit("Non-Linear Capacitor Test")
    >>> wave = eispice.Pulse('1u', '10u', '10n', '5n', '3n', '5n', '50n')
    >>> cct.Vc = eispice.V(3, 0, '1n', wave)
    >>> cct.Vx = eispice.V(1, 0, 1, \
            eispice.Pulse('1m', '10m', '10n', '2n', '3n', '5n', '20n'))
    >>> cct.Cx = eispice.B(1, 0, eispice.Capacitor, 'v(3)*10')
    >>> cct.tran('0.5n', '100n')
    >>> cct.check_i('Vx', -1.810333469e+02, '12n')
    True
    >>> cct.check_i('Vx', -3.770187057e2, '71n')
    True
    """

    def __init__(self, pNode, nNode, type, equation):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        type -- either Voltage to create a voltage source, Current to create
        a current source or Capacitor to create a non-linear Capacitor
        equation -- string containing B Element equation
        """
        simulator_.Behavioral_.__init__(self, str(pNode), str(nNode),
                type, str(equation))

class PyB(simulator_.CallBack_):
    """
    Python Based Behavioural Model (Call-Back Model)

    This device is intended as an alternate to the spice3-like B-Element.
    It provides an inheritable class that can be used to define Python based
    Behavioural models. The constructor takes as arguments, the positive node,
    the negative node, the type (Voltage or Current), and a list of node
    voltages, source currents, or 'Time' to be passed to the callback method,
    model and should redefined as part of the new device class.

    Examples:

    1. Nonlinear Current Source
    >>> import eispice
    >>> class MyDevice(eispice.PyB):
    ...     def __init__(self, pNode, nNode):
    ...         eispice.PyB.__init__(self, pNode, nNode, eispice.Current, \
                    self.v(pNode))
    ...     def model(self, vP):
    ...         return 2*vP
    >>> cct = eispice.Circuit("Call-Back Current Test")
    >>> cct.Vx = eispice.V(1, 0, 4)
    >>> cct.PyBx = MyDevice(1, 0)
    >>> cct.op()
    >>> cct.check_i('Vx', -8.0)
    True

    2. Nonlinear Voltage Source
    >>> import eispice
    >>> class MyDevice(eispice.PyB):
    ...     def __init__(self, pNode, nNode):
    ...         eispice.PyB.__init__(self, pNode, nNode, eispice.Current,
    ...                 self.v(pNode), self.v(nNode), eispice.Time)
    ...     def model(self, vP, vN, time):
    ...         if time > 10e-9:
    ...             return 2 * (vP - vN)
    ...         else:
    ...             return 0.0
    >>> cct = eispice.Circuit("Call-Back Time Test")
    >>> cct.Vx = eispice.V(1, 0, 4)
    >>> cct.PyBx = MyDevice(1, 0)
    >>> cct.tran('1n', '25n')
    >>> cct.check_i('Vx', 0, '2n')
    True
    >>> cct.check_i('Vx', -8, '20n')
    True

    3. Simple CMOS Model
    >>> import eispice
    >>> cct = eispice.Circuit('PyB Defined Behavioral MOS Divider Test')
    >>> class PMOS(eispice.PyB):
    ...     def __init__(self, d, g, s, k=2.0e-6, w=2, l=1, power=2.0):
    ...         eispice.PyB.__init__(self, d, s, eispice.Current, self.v(g), \
                    self.v(s))
    ...         self.Vt = 0.7
    ...         self.beta = k * (w/l)
    ...         self.power = power
    ...     def model(self, Vg, Vs):
    ...         if ((Vs - Vg) > self.Vt):
    ...             return -0.5* self.beta * (Vs - Vg - self.Vt)**self.power
    ...         else :
    ...             return 0.0
    >>> class NMOS(eispice.PyB):
    ...     def __init__(self, d, g, s, k=5.0e-6, w=2, l=1, power=2.0):
    ...         eispice.PyB.__init__(self, d, s, eispice.Current, self.v(g), \
                    self.v(s))
    ...         self.Vt = 0.7
    ...         self.beta = k * (w/l)
    ...         self.power = power
    ...     def model(self, Vg, Vs):
    ...         if ((Vg - Vs) > self.Vt):
    ...             return 0.5* self.beta * (Vg - Vs - self.Vt)**self.power
    ...         else :
    ...             return 0.0
    >>> cct.Vcc = eispice.V('vcc', eispice.GND, 3.3)
    >>> cct.Mx = PMOS('vg', 'vg', 'vcc', k=2.0e-6)
    >>> cct.My = NMOS('vg', 'vg', eispice.GND, k=5.0e-6)
    >>> cct.Rl = eispice.R('vg', eispice.GND, '10G')
    >>> cct.op()
    >>> cct.check_v('vg', 1.436097)
    True
    """

    def __init__(self, pNode, nNode, type, *variables):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        type -- either Voltage to create a voltage source or Current
        to create a current source
        *variables -- remaining arguments are a list of all of the
        voltage nodes and current probes that will be passed back
        to the model method (in the same order)
        """
        simulator_.CallBack_.__init__(self, str(pNode), str(nNode),
                type, variables, self.callBack)

        self.args = []
        for varaiable in variables:
            self.args.append(calc.Variable())

        self.range = range(len(variables))

    def callBack(self, data, derivs):
        """Wrapper around the low-level PyB call-back."""

        for i in self.range:
            self.args[i](data[i])

        result = self.model(*self.args)

        if not isinstance(result, calc.Variable):
            for i in self.range:
                derivs[i] = 0.0
        else:
            for i in self.range:
                derivs[i] = result[self.args[i]]

        return float(result)

    def model(self, *args):
        """This should be redefined when this class is inherited."""
        warnings.warn('PyB model has not been defined.')
        return 0.0

    def v(self, node):
        """Returns the name of a node used within the simulator."""
        return 'v(%s)' % str(node)

    def i(self, source):
        """Returns the name of a current probe within the simulator."""
        return 'i(%s)' % str(source)

#-----------------------------------------------------------------------------#
#                                 Sources                                     #
#-----------------------------------------------------------------------------#

class I(simulator_.CurrentSource_):
    """Current Source Model

    Example (refer to the waveforms module for more examples):
    >>> import eispice
    >>> cct = eispice.Circuit("Current Pulse Test")
    >>> cct.Ix = eispice.I(1, eispice.GND, 4, \
            eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n'))
    >>> cct.Vx = eispice.V(2, 1, 0)
    >>> cct.Rx = eispice.R(2, eispice.GND, 10)
    >>> cct.tran('1n', '20n')
    >>> cct.check_i('Vx', 4, '5n')
    True
    >>> cct.check_i('Vx', 8, '15n')
    True
    """
    def __init__(self, pNode, nNode, dcValue=0.0, wave=None):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        dcValue -- DC Value in Amps
        wave -- (optional) waveform
        """
        if wave == None:
            simulator_.CurrentSource_.__init__(self, str(pNode), str(nNode),
                    units.float(dcValue))
        else:
            simulator_.CurrentSource_.__init__(self, str(pNode), str(nNode),
                    units.float(dcValue), wave)

class V(simulator_.VoltageSource_):
    """Voltage Source Model

    Example (refer to the waveforms module for more examples):
    >>> import eispice
    >>> cct = eispice.Circuit("Voltage Pulse Test")
    >>> cct.Ix = eispice.V(1, eispice.GND, 4, \
            eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n'))
    >>> cct.Rx = eispice.R(1, eispice.GND, 10)
    >>> cct.tran('1n', '20n')
    >>> cct.check_v(1, 4, '5n')
    True
    >>> cct.check_v(1, 8, '15n')
    True
    """
    def __init__(self, pNode, nNode, dcValue=0.0, wave=None):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        dcValue -- DC Value in Volts
        wave -- (optional) waveform
        """
        if wave == None:
            simulator_.VoltageSource_.__init__(self, str(pNode), str(nNode),
                    units.float(dcValue))
        else:
            simulator_.VoltageSource_.__init__(self, str(pNode), str(nNode),
                    units.float(dcValue), wave)

class VI(simulator_.VICurve_):
    """Voltage/Current (VI) Curve Model

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("VI Curve Test")
    >>> data = array([[-10, -10],[-5, -2],[0, 1],[1, 3],[5, 8], \
            [10, 10],[12, 8]])
    >>> cct.Vx = eispice.V(1, 0, 10)
    >>> cct.VIx = eispice.VI(1, 0, eispice.PWL(data))
    >>> cct.op()
    >>> cct.check_i('VIx', 10)
    True
    >>> cct.Vx.DC = -5
    >>> cct.op()
    >>> cct.check_i('VIx', -2)
    True
    """
    def __init__(self, pNode, nNode, vi, ta=None):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        vi -- PWL or PWC waveform defining the VI curve
        ta -- (optional) PWL or PWC waveform defining the a time/multiplier
        curve, at each time point the IV curve is scaled by the repective A
        """
        if ta == None:
            simulator_.VICurve_.__init__(self, str(pNode), str(nNode), vi)
        else:
            simulator_.VICurve_.__init__(self, str(pNode), str(nNode), vi, ta)

class G(simulator_.Behavioral_):
    """Voltage-Controlled Current Source

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("VCCS Test")
    >>> cct.Vx = eispice.V(1, 0, 3.2)
    >>> cct.Iy = eispice.G(2, 0, 1, 0, 2)
    >>> cct.Ry = eispice.R(2, 3, 4.1)
    >>> cct.Vy = eispice.V(3, 0, 0)
    >>> cct.op()
    >>> cct.check_i('Vy', -6.4)
    True
    """
    def __init__(self, pNode, nNode, pControlNode, nControlNode, value):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        pNode -- positive control node name
        nNode -- negative control node name
        value -- gain (Siemens)
        """
        equation = ('v(%s,%s)*%e' % (str(pControlNode), str(nControlNode),
                units.float(value)))
        simulator_.Behavioral_.__init__(self, str(pNode), str(nNode),
                Current, equation)

class E(simulator_.Behavioral_):
    """Voltage-Controlled Voltage Source

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("VCVS Test")
    >>> cct.Vx = eispice.V(1, 0, 3.2)
    >>> cct.Vy = eispice.E(2, 0, 1, 0, 2.7)
    >>> cct.Ry = eispice.R(2, 0, 4.1)
    >>> cct.op()
    >>> cct.check_v(2, 8.64)
    True
    """
    def __init__(self, pNode, nNode, pControlNode, nControlNode, value):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        pNode -- positive control node name
        nNode -- negative control node name
        value -- gain
        """
        equation = ('v(%s,%s)*%e' % (str(pControlNode), str(nControlNode),
                units.float(value)))
        simulator_.Behavioral_.__init__(self, str(pNode), str(nNode),
                Voltage, equation)

class F(simulator_.Behavioral_):
    """Current-Controlled Current Source

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("CCCS Test")
    >>> cct.Ix = eispice.I(1, 0, 3.2)
    >>> cct.Rx = eispice.R(1, 4, 7.3)
    >>> cct.Vx = eispice.V(4, 0, 0)
    >>> cct.Iy = eispice.F(2, 0, 'Vx', 1.75)
    >>> cct.Ry = eispice.R(2, 3, 6.2)
    >>> cct.Vy = eispice.V(3, 0, 0)
    >>> cct.op()
    >>> cct.check_i('Vy', 5.6)
    True
    """
    def __init__(self, pNode, nNode, controlDevice, value):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        controlDevice -- name of control voltage source (string)
        value -- gain
        """
        equation = ('i(%s)*%e' % (str(controlDevice),
                units.float(value)))
        simulator_.Behavioral_.__init__(self, str(pNode), str(nNode),
                Current, equation)

class H(simulator_.Behavioral_):
    """Current-Controlled Voltage Source

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("CCVS Test")
    >>> cct.Ix = eispice.I(1, 0, 2.7)
    >>> cct.Rx = eispice.R(1, 4, 10.4)
    >>> cct.Vx = eispice.V(4, 0, 0)
    >>> cct.Vy = eispice.H(2, 0, 'Vx', 3.2)
    >>> cct.Ry = eispice.R(2, 0, 3.7)
    >>> cct.op()
    >>> cct.check_v(2, -8.64)
    True
    """
    def __init__(self, pNode, nNode, controlDevice, value):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        controlDevice -- name of control voltage source (string)
        value -- gain
        """
        equation = ('i(%s)*%e' % (str(controlDevice),
                units.float(value)))
        simulator_.Behavioral_.__init__(self, str(pNode), str(nNode),
                Voltage, equation)

#-----------------------------------------------------------------------------#
#                           Transmission Lines                                #
#-----------------------------------------------------------------------------#

class T(simulator_.TLine_):
    """Basic Transmission Line Model

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Simple Transmission Line Model Test")
    >>> cct.Vx = eispice.V('vs',0, 0, eispice.Pulse(0, 1, '0n','1n','1n',\
            '4n','8n'))
    >>> cct.Rt = eispice.R('vs', 'vi', 50)
    >>> cct.Tg = eispice.T('vi', 0, 'vo', 0, 50, '2n')
    >>> cct.Cx = eispice.C('vo',0,'5p')
    >>> cct.tran('0.01n', '10n')
    >>> cct.check_v('vo', 0.3726765, '2.6n')
    True
    >>> cct.check_i('Vx', -0.008381298823, '4.62n')
    True
    """
    def __init__(self, pNodeLeft, nNodeLeft, pNodeRight, nNodeRight, Z0, Td,
            loss=None):
        """
        Arguments:
        pNodeLeft -- positive node on the left side to tline
        nNodeLeft -- negative node on the left side to tline
        pNodeRight -- positive node on the right side to tline
        nNodeRight -- negative node on the right side to tline
        Z0 --> characteristic impedance in Ohms
        Td --> time delay in seconds
        x.loss --> (optional) loss factor times length, is unitless
        """
        if loss == None:
            simulator_.TLine_.__init__(self, str(pNodeLeft),
                str(nNodeLeft), str(pNodeRight), str(nNodeRight),
                units.float(Z0), units.float(Td))
        else:
            simulator_.TLine_.__init__(self, str(pNodeLeft),
                str(nNodeLeft), str(pNodeRight), str(nNodeRight),
                units.float(Z0), units.float(Td), units.float(loss))

class W(simulator_.TLineW_):
    """
    W-Element Transmission Line Model

    An RLGC matrix defined coupled, frequency dependent transmission-line
    model, should be roughly equivalent to the W-Element in HSPICE.
    """

    def __init__(self, iNodes, iRef, oNodes, oRef, length, R0, L0, C0,
            G0=None, Rs=None, Gd=None, fgd=1e100, fK=1e9, M=6):
        """
        Arguments:
        iNodes -- list/tuple or single string of input nodes
        iRef -- name of the input refrence node
        oNodes -- list/tuple or single string of output nodes
        iRef -- name of the output refrence node
        length -- length of the t-line in meters
        R0 -- DC resistance matrix or float from single T-Line (ohm/m)
        L0 -- DC inductance matrix or float from single T-Line  (H/m)
        C0 -- DC capacitance matrix or float from single T-Line  (F/m)
        G0 -- (optional) DC shunt conductance matrix (S/m)
        Rs -- (optional) Skin-effect resistance matrix (Ohm/m*sqrt(Hz))
        Gd -- (optional) Dielectric-loss conductance matrix (S/m*Hz)
        fgd -- (optional) Cut-Off for Dielectric loss (Hz)
        fK -- (optional) Cut-Off for T-Line Model (Hz)
        M -- (optional) Order of Approximation of Curve Fit (unitless)
        """

        warnings.warn("W-Element Model not complete.")

        # Makes it possible to send a single string for a non-coupled T-Line

        if isinstance(iNodes, str):
            iNodes = (iNodes, )

        if isinstance(oNodes, str):
            oNodes = (oNodes, )

        if len(iNodes) != len(oNodes):
            raise (RuntimeError,
                "Must have the same number of input and output nodes.")

        nodes = len(iNodes)

        # Do some parameter checking here that is tough to do in C, also
        # create zeroed matracies if G and/or R aren't defined.

        if nodes > 1:
            if G0 is None:
                G0 = array(zeros((nodes, nodes)))
            if Rs is None:
                Rs = array(zeros((nodes, nodes)))
            if Gd is None:
                Gd = array(zeros((nodes, nodes)))
            R0 =  array(units.floatList2D(R0))
            L0 =  array(units.floatList2D(L0))
            C0 =  array(units.floatList2D(C0))
            G0 =  array(units.floatList2D(G0))
            Rs =  array(units.floatList2D(Rs))
            Gd =  array(units.floatList2D(Gd))
            if (R0.shape[0] != nodes) or (R0.shape[1] != nodes):
                raise (RuntimeError,
                    "R0 must be a square matrix with as many rows as nodes.")
            if (L0.shape[0] != nodes) or (L0.shape[1] != nodes):
                raise (RuntimeError,
                    "L0 must be a square matrix with as many rows as nodes.")
            if (C0.shape[0] != nodes) or (C0.shape[1] != nodes):
                raise (RuntimeError,
                    "C0 must be a square matrix with as many rows as nodes.")
            if (G0.shape[0] != nodes) or (G0.shape[1] != nodes):
                raise (RuntimeError,
                    "G0 must be a square matrix with as many rows as nodes.")
            if (Rs.shape[0] != nodes) or (Rs.shape[1] != nodes):
                raise (RuntimeError,
                    "Rs must be a square matrix with as many rows as nodes.")
            if (Gd.shape[0] != nodes) or (Gd.shape[1] != nodes):
                raise (RuntimeError,
                    "Gd must be a square matrix with as many rows as nodes.")
        else:
            if G0 is None:
                G0 = 0.0
            if Rs is None:
                Rs = 0.0
            if Gd is None:
                Gd = 0.0
            R0 =  array((units.float(R0),))
            L0 =  array((units.float(L0),))
            C0 =  array((units.float(C0),))
            G0 =  array((units.float(G0),))
            Rs =  array((units.float(Rs),))
            Gd =  array((units.float(Gd),))

        nodeNames = tuple([str(i) for i in iNodes] + [str(iRef)]
             + [str(i) for i in oNodes] + [str(oRef)])

        simulator_.TLineW_.__init__(self, nodeNames, int(M),
                units.float(length), L0, C0, R0, G0, Rs, Gd, units.float(fgd),
                units.float(fK))

#-----------------------------------------------------------------------------#
#                              Semiconductors                                 #
#-----------------------------------------------------------------------------#

class D(subckt.Subckt):
    """
    Diode Model

    A Berkley spice3f5 compatible Junction Diode Model.

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Diode Test")
    >>> wave = eispice.Pulse(0, 1, '.5u', '5u', '5u', '.5u')
    >>> cct.Vx = eispice.V(1, eispice.GND, 0, wave)
    >>> cct.Dx = eispice.D(1, eispice.GND, IS='2.52n', RS=0.568, N=1.752, \
            CJO='4p', M=0.4, TT='20n')
    >>> cct.tran('0.5u', '10u')
    >>> cct.check_i('Vx', -7.326775239e-02, '4.6u')
    True
    >>> cct.check_i('Vx', -1.781444540e-01, '6.4u')
    True
    """

    def __init__(self, pNode, nNode, area=1.0,
            IS=1.0e-14, RS=0, N=1, TT=0, CJO=0, VJ=1, M=0.5, EG=1.11, XTI=3.0,
            KF=0, AF=1, FC=0.5, BV=1e100, IBV=1e-3, TNOM=27):
        """
        Arguments:
        pNode -- positive node name
        nNode -- negative node name
        area -- area factor (for spice3f5 compatibility) -- default = 1.0
        IS -- saturation current (A) -- default = 1.0e-14
        RS -- ohmic resistance (Ohms) -- default = 0
        N -- emission coefficient -- default = 1
        TT -- transit-time (sec) -- default = 0
        CJO -- zero-bias junction capacitance (F) -- default = 0
        VJ -- junction potential -- default = 0.5
        M -- grading coefficient (V) -- default = 1
        EG -- reserved for possible future use
        XTI -- reserved for possible future use
        KF -- reserved for possible future use
        AF -- reserved for possible future use
        FC -- Coefficient for Cd formula -- default = 0.5
        BV -- reverse breakdown voltage (V) -- default = 1e100
        IBV -- current at breakdown voltage (A) -- default = 1e-3
        TNOM -- parameter measurement temperature (degC) -- default = 27
        """

        pNode = str(pNode)
        nNode = str(nNode)
        area = units.float(area)
        IS = units.float(IS)
        RS = units.float(RS)
        N = units.float(N)
        TT = units.float(TT)
        CJO = units.float(CJO)
        VJ = units.float(VJ)
        M = units.float(M)
        BV = units.float(BV)
        IBV = units.float(IBV)
        TNOM = units.float(TNOM)

        # Parasitic Resistance
        if RS != 0:
            self.Rs = R(pNode, self.node('r'), area*RS)
            pNode = self.node('r')

        # Local Variables
        k = 1.3806503e-23                 # Boltzmann's Constant (1/JK)
        q = 1.60217646e-19                 # Electron Charge (C)
        Vt = ((k*(TNOM+273.15))/q)        # Thermal Voltage

        # Saturation and Breakdown Current
        Is = ('(if(v(%s,%s)>=%e)*(%e*(exp(v(%s,%s)/%e)-1)))' %
                (pNode, nNode, -BV, area*IS, pNode, nNode, N*Vt))
        Ib = ('(if(v(%s,%s)<%e)*(%e*(exp((-%e-v(%s,%s))/%e)-1)))' %
                (pNode, nNode, -BV, area*IS, BV, pNode, nNode,Vt))
        self.Isb = B(pNode, nNode, Current, Is + '+' + Ib)

        # Junction (Depletion) and Diffusion Capacitance
        Cd = '(%e*(exp(v(%s,%s)/%e)))' % (area*TT*IS/N*Vt, pNode, nNode, N*Vt)
        Cj1 = ('(if(v(%s,%s)<%e)*(%e/((1-v(%s,%s)/%e)^%e)))' %
                (pNode, nNode, FC*VJ, area*CJO, pNode, nNode, VJ, M))
        Cj2 = ('(if(v(%s,%s)>=%e)*%e*(1-%e+%e*v(%s,%s)))' %
                (pNode, nNode, FC*VJ, (area*CJO/((1-FC)**(M+1))),
                FC*(1+M), (M/VJ), pNode, nNode))
        self.Cjd = B(pNode, nNode, Capacitor, Cd + '+' + Cj1 + '+' + Cj2)


class Q(subckt.Subckt):
    """BJT Model

    A Berkley spice3f5 compatible BJT Model.
    """

    def __init__(self, cNode, bNode, eNode, sNode=GND, area=1.0,
            IS=1.0e-16, BF=100, NF=1.0, VAF=1e100, IKF=1e100, ISE=0, NE=1.5,
            BR=1, NR=1, VAR=1e100, IKR=1e100, ISC=0, NC=2, RB=0, IRB=1e100,
            RBM=None, RE=0, RC=0, CJE=0, VJE=0.75, MJE=0.33, TF=0, XTF=0,
            VTF=1e100, ITF=0, PTF=0, CJC=0, VJC=0.75, MJC=0.33, XCJC=1,
            TR=0, CJS=0, VJS=0.75, MJS=0, XTB=0, EG=1.11, XTI=3, KF=0, AF=1,
            FC=0.5, TNOM=27):
        """
        Arguments:
        cNode -- collector node name
        bNode -- base node name
        eNode -- emitter node name
        bNode -- substrate node name -- default = GND

        area -- area factor (for spice3f5 compatibility) -- default = 1.0
        """

        cNode = str(cNode)
        bNode = str(bNode)
        eNode = str(eNode)
        sNode = str(sNode)
        area = units.float(area)
        IS = units.float(area)
        BF = units.float(area)
        NF = units.float(area)
        VAF = units.float(area)
        IKF = units.float(area)
        ISE = units.float(area)
        NE = units.float(area)
        BR = units.float(area)
        NR = units.float(area)
        VAR = units.float(area)
        IKR = units.float(area)
        ISC = units.float(area)
        NC = units.float(area)
        RB = units.float(area)
        IRB = units.float(area)
        if RBM == None:
            RBM = RB
        RBM = units.float(area)
        RE = units.float(area)
        RC = units.float(area)
        CJE = units.float(area)
        VJE = units.float(area)
        MJE = units.float(area)
        TF = units.float(area)
        XTF = units.float(area)
        TR = units.float(area)
        CJS = units.float(area)
        VJS = units.float(area)
        MJS = units.float(area)
        XTB = units.float(area)
        EG = units.float(area)
        XTI = units.float(area)
        KF = units.float(area)
        AF = units.float(area)
        FC = units.float(area)
        TNOM = units.float(area)

        # Local Variables
        k = 1.3806503e-23                 # Boltzmann's Constant (1/JK)
        q = 1.60217646e-19                 # Electron Charge (C)
        Vt = ((k*(TNOM+273.15))/q)        # Thermal Voltage

        # Parasitic Resistance
        if RE != 0:
            self.Re = R(eNode, self.node('re'), area*RE)
            eNode = self.node('re')
        if RC != 0:
            self.Rc = R(eNode, self.node('rc'), area*RC)
            cNode = self.node('rc')
        #~ if RB != 0:
            #~ Rb =
            #~ self.Rb = R(eNode, self.node('rc'), area*RC)
            #~ cNode = self.node('rc')

        # Saturation and Breakdown Current
        #~ Ibe = '(%e*(exp(v(%s,%s)/%e)-1))' % (IS/BF, rNode, nNode, NF*Vt)
        #~ Ibe = '(%e*(exp(v(%s,%s)/%e)-1))' % (IS/BF, rNode, nNode, NF*Vt)
        #~ self.Isb = B(rNode, nNode, Current, Is + '+' + Ib)

        warnings.warn("BJT Model not complete.")


if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print('Testing Complete')
