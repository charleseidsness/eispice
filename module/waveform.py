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
This module provides all of basic eispice waveforms for use with
speific device, e.g. V, I, VI.

Basic Waveform Classes:
-- These classes are wrappers around waveforms defined in the
simulator library (written in C)
PWL -- Piece-Wise Linear
PWC -- Piece-Wise Cubic-Spline
SFFM -- Single Frequency FM
Exp -- Exponential Rise and/or Fall
Pulse -- Pulse Train
Gauss -- Pulse Train with Gaussian Edges
Sin    -- Sine Wave
SFFM

"""

from numpy import array, double

import units
import simulator_

class PWL(simulator_.PWL_):
    """
    Piece-Wise Linear Waveform
    -- A 2D curve, points between defined data points are calculated
    via linear interpolation.

    Example:
    >>> import eispice
    >>> wave = eispice.PWL([['2n', 4],['12n', 3],['50n', 20],['75n', -20], \
            ['95n', -22]])
    >>> cct = eispice.Circuit('PWL Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, 8.815789474e+00, '25n')
    True
    >>> cct.check_v(1, -2.000000000e+01, '75n')
    True
    """
    def __init__(self, data):
        """
        Arguments:
        data -- 2D Array Representing the PWL Curve
        """
        data = units.floatList2D(data)
        data = data[data[:,0].argsort(),] # sort by first column for simulator
        simulator_.PWL_.__init__(self, data)

class PWC(simulator_.PWC_):
    """
    Piece-Wise Cubic-Spline Waveform
    -- A 2D curve, points between defined data points are calculated
    as cubic splines.

    Example:
    >>> import eispice
    >>> wave = eispice.PWC([['2n', 4],['12n', 3],['50n', 20],['75n', -20], \
            ['95n', -22]])
    >>> cct = eispice.Circuit('PWC Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, 1.148836888e+01, '25n')
    True
    >>> cct.check_v(1, -2.000000000e+01, '75n')
    True
    """
    def __init__(self, data):
        """
        Arguments:
        data -- 2D Array Representing the PWC Curve
        """
        data = units.floatList2D(data)
        data = data[data[:,0].argsort(),] # sort by first column for simulator
        simulator_.PWC_.__init__(self, data)

class SFFM(simulator_.SFFM_):
    """Single Frequency FM Waveform

    Example:
    >>> import eispice
    >>> wave = eispice.SFFM(1, 4, '100M', 2, '10M')
    >>> cct = eispice.Circuit('SFFM Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, -2.630296891e+00, '25n')
    True
    >>> cct.check_v(1, 4.631842886e+00, '75n')
    True
    """
    def __init__(self, *args):
        """
        Arguments:
        Vo -- Offset
        Va -- Amplitude
        Fc -- (optional) Carrier Frequency, default = 1/tstop
        MDI -- (optional) Modulation Index, default = 0.0
        Fs -- (optional) Signal Frequency, default = 1/tstop
        """
        simulator_.SFFM_.__init__(self,*units.floatList1D(args))

class Exp(simulator_.Exp_):
    """Exponential Rise and/or Fall Waveform

    Example:
    >>> import eispice
    >>> wave = eispice.Exp(0, 4, '5n', '2n', '25n', '5n')
    >>> cct = eispice.Circuit('Exp Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, 3.999818400e+00, '25n')
    True
    >>> cct.check_v(1, 1.818267660e-04, '75n')
    True
    """
    def __init__(self, *args):
        """
        Arguments:
        V1 -- Initial Value
        V2 -- Pulsed Value
        Td1 -- (optional) Rise Delay Time, default = 0.0
        Tau1 -- (optional) Rise Time Constant, default = tstep
        Td2 -- (optional) Fall Delay Time, default = td1 + tstep
        Tau2 -- (optional) Fall Time Constant, default = tstep
        """
        simulator_.Exp_.__init__(self,*units.floatList1D(args))

class Pulse(simulator_.Pulse_):
    """Pulse Train Waveform

    Example:
    >>> import eispice
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct = eispice.Circuit('Pulse Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, 4, '25n')
    True
    >>> cct.check_v(1, 8, '75n')
    True
    """
    def __init__(self, *args):
        """
        Arguments:
        V1 -- Initial Value
        V2 -- Pulsed Value
        Td -- (optional) Delay Time, default = 0.0
        Tr -- (optional) Rise Time, default = tstep
        Tf -- (optional) Fall Time, default = tstep
        PW -- (optional) Pulse Width, default = tstop
        Per -- (optional) Period, default = tstop
        """
        simulator_.Pulse_.__init__(self,*units.floatList1D(args))

class Gauss(simulator_.Gauss_):
    """Pulse Train Waveform with Gaussian Edges

    Example:
    >>> import eispice
    >>> wave = eispice.Gauss(0, 3.3, '0n', '2n', '5n', '10n', '50n')
    >>> cct = eispice.Circuit('Gauss Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, 1.517639357e-01, '25n')
    True
    >>> cct.check_v(1, 3.148220583e+00, '65n')
    True
    """
    def __init__(self, *args):
        """
        Arguments:
        V1 -- Initial Value
        V2 -- Pulsed Value
        Td -- (optional) Delay Time, default = 0.0
        Tr -- (optional) Rise Time (20% to 80%), default = tstep
        Tf -- (optional) Fall Time (20% to 80%), default = tstep
        PW -- (optional) Pulse Width, default = tstop
        Per -- (optional) Period, default = tstop
        """
        simulator_.Gauss_.__init__(self,*units.floatList1D(args))

class Sin(simulator_.Sin_):
    """Sine Wave Waveform

    Example:
    >>> import eispice
    >>> wave = wave = eispice.Sin(0, 4, '50M', '5n', '10M')
    >>> cct = eispice.Circuit('Sin Test')
    >>> cct.Rv = eispice.R(1, eispice.GND, 1)
    >>> cct.Vx = eispice.V(1, 0, eispice.GND, wave)
    >>> cct.tran('0.5n','100n')
    >>> cct.check_v(1, -6.561833244e-04, '25n')
    True
    >>> cct.check_v(1, 1.910792387e-04, '75n')
    True
    """
    def __init__(self, *args):
        """
        Arguments:
        Vo --> Offset
        Va --> Amplitude
        Fc --> (optional) Frequency, default = 1/tstop
        Td --> (optional) Delay, default = 0.0
        DF --> (optional) Damping Factor, default = 0.0
        """
        simulator_.Sin_.__init__(self,*units.floatList1D(args))

if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print('Testing Complete')
