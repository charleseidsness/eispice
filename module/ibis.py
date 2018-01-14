#
# Copyright (C) 2005-2007 Cooper Street Innovations Inc.
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
This module provides support for ibis model defined devices.

Device Classes:
Receiver -- IBIS Receiver Device Model
Driver -- IBIS Driver Device Model
Package -- IBIS Package Device Model
Pin -- IBIS Pin Model (Buffer model plus PCackage Model)

IBIS Classes:
IbisPinData -- IBIS Pin Data
IbisModel_Selector -- IBIS Model Selector Data
IbisVICurve -- IBIS VI Curve Data
IbisPin -- IBIS Pin Data
IbisPackage -- IBIS Package Data
IbisWaveform -- IBIS Waveform Data
IbisRamp -- IBIS Ramp Data
IbisModel_Spec -- IBIS Spec Data
IbisModel -- IBIS Model Data
IbisComponent -- IBIS Component Data
Ibis -- Top Level IBIS Data

Functions:
calcDriverKDoubleWave -- Calculates a TA Table using 2 Waveforms
calcDriverKSingleWave -- Calculates a TA Table using 1 Waveform
calcDriverKRamp -- Calculates a TA Table using Ramp Data

"""

from ibis_parser import *

class Ibis(Ibis_Parser):
    """IBIS Model

    Examples:

    >>> import eispice
    >>> ibs = eispice.Ibis('test')

    1. Double Waveform Defined Model
    >>> cct = eispice.Circuit('IBIS Double Waveform Test')
    >>> cct.Driver = ibs['2']('vsx')
    >>> cct.Vmeas = eispice.V('vsx', 'vs', 0)
    >>> cct.Rt = eispice.R('vs', 'vi', '33.2')
    >>> cct.Tg = eispice.T('vi', 0, 'vo', 0, 50, '2n')
    >>> cct.Receiver = ibs['1']('vo')
    >>> cct.tran('0.01n', '15n')
    >>> cct.check_v('vo', 2.942970279e-01, '4.7n')
    True
    >>> cct.check_i('Vmeas', 2.200965422e-02, '7.1n')
    True

    2. Ramp Waveform Defined Model
    >>> cct = eispice.Circuit('IBIS Ramp Test')
    >>> cct.Driver = ibs['4']('vsx')
    >>> cct.Vmeas = eispice.V('vsx', 'vs', 0)
    >>> cct.Rt = eispice.R('vs', 'vi', '33.2')
    >>> cct.Tg = eispice.T('vi', 0, 'vo', 0, 50, '2n')
    >>> cct.Receiver = ibs['1']('vo')
    >>> cct.tran('0.01n', '15n')
    >>> cct.check_v('vo', -2.416134420e-01, '1.7n')
    True
    >>> cct.check_i('Vmeas', 1.521229286e-02, '4.9n')
    True

    """
    def __init__(self, filename, device=None):
        """
        Arguments:
        filename -- IBIS Model to import
        device -- (optional) name of default device, default = first device
        """

        Ibis_Parser.__init__(self, filename, device)

    def __getitem__(self, pin):
        """Returns a device model for the specified pin."""

        class PinBuilder:
            def __init__(self, pin, ibs):
                self.pin = pin
                self.ibs = ibs
            def __call__(self, node, speed=Typical, direction=Rising, io=Output,
                    modelName=None):
                return Pin(self.ibs, self.pin, node, speed, direction, io, modelName)

        return PinBuilder(pin.lower(), self)

if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print 'Testing Complete'
