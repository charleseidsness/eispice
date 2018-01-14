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
This module contains the IBIS file parser.

Classes:
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

"""

import re, os, sys
import units
from numpy import array

from ibis_driver import *
from ibis_device import *

# -------------------------------------------------------------------------- #

Done = 'done'

_reNamed = r"""(?x)(?i)\s*
    (?P<key>%s)\s*
    (?P<name>.*?)
    [|\r\n]"""
_reValue = r"""(?x)(?i)\s*
    (?P<key>%s)[\s=]*(?P<name>\S*?)\s*
    [|\r\n]"""
_reLabeled = r"""(?x)(?i)
    \[(?P<key>%s)\]\s*
    (?P<name>.*?)
    [|\r\n]"""
_reLabel = '(?i)\[(?P<key>%s)\]\s*'
_reRange = r"""(?x)(?i)\s*
    (?P<key>%s)\s*
    (?P<typ>\S*)\s*
    (?P<min>\S*)\s*
    (?P<max>\S*)\s*
    [|\r\n]"""
_reLabeledRange = r"""(?x)(?i)
    \[(?P<key>%s)\]\s*
    (?P<typ>\S*)\s*
    (?P<min>\S*)\s*
    (?P<max>\S*)\s*
    [|\r\n]"""
_reComment = '\s*[|\r\n\f]'
_reAny = '(?P<line>.*)'

class _Builder:
    """A customized awk-like parser"""

    def __init__(self):
        self.patterns = []
        self.fdin = None
        #  Some useful re's
        self.addRE(_reComment)

    def addRE(self, pattern, handler=None):
        self.patterns.append((re.compile(pattern), handler))

    def _process(self, line):
        for pattern, handler in self.patterns:
            match = pattern.match(line.lower())
            if match:
                if callable(handler):
                    return handler(**match.groupdict())
                else:
                    return handler
        raise RuntimeError(line)

    def process(self):
        for line in iter(self.fdin.readline, ''):
            if self._process(line) is Done:
                pos = int(self.fdin.tell()) - len(line)
                self.fdin.seek(pos, 0)
                break

    def handleName(self, key, name):
        key = key.replace(" ","_").lower()
        setattr(self, key, name.strip())

    def handleBlock(self, key, name):
        name = name.strip()
        for line in iter(self.fdin.readline, ''):
            if (line[0] != '['):
                name = name + '\n'
                name = name + line.strip()
            else:
                pos = int(self.fdin.tell()) - len(line)
                self.fdin.seek(pos, 0)
                break
        key = key.replace(" ","_").lower()
        setattr(self, key, name)

    def handleValue(self, key, name):
        key = key.replace(" ","_").lower()
        setattr(self, key, units.float(name))

    def handleMulti(self, key, name):
        key = key.replace(" ","_").lower()
        if not hasattr(self, key):
            setattr(self, key, {})
        dic = getattr(self, key)
        dic[name.strip()] = globals()['Ibis' + key.title()](self.fdin)

    def handleSingle(self, key):
        key = key.replace(" ","_").lower()
        setattr(self, key, globals()['Ibis' + key.title()](self.fdin))

    def handleSingleDict(self, key):
        key = key.replace(" ","_").lower()
        setattr(self, key, vars(globals()['Ibis' + key.title()](self.fdin)))

    def handleRange(self, key, typ, min, max):
        key = key.replace(" ","_").lower()
        if not hasattr(self, key):
            setattr(self, key, {})
        dic = getattr(self, key)
        dic['typ'] = units.float(typ)
        dic['min'] = units.float(min)
        dic['max'] = units.float(max)
        if dic['min'] == None:
            dic['min'] = dic['typ']
        if dic['max'] == None:
            dic['max'] = dic['typ']

    def handleVICurve(self, key):
        key = key.replace(" ","_").lower()
        setattr(self, key, vars(IbisVICurve(self.fdin)))

    def handleWaveform(self, key):
        key = key.replace(" ","_").lower()
        if not hasattr(self, key):
            setattr(self, key, [])
        lst = getattr(self, key)
        lst.append(IbisWaveform(self.fdin))

    def handleCommentChar(self, key, name):
        if name != '':
            raise RuntimeError("Don't support comment characters other than |. ")

    def handleUnkown(self, line):
        raise RuntimeError('Unsupported Line\n%s' % line)

# -------------------------------------------------------------------------- #

class IbisPinData(_Builder):
    """IBIS Pin Data"""
    def __init__(self, signal, model, R, L, C):
        self.signal = signal
        self.model = model
        self.R = units.float(R)
        self.L = units.float(L)
        self.C = units.float(C)

class IbisModel_Selector(_Builder):
    """IBIS Model Selector Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.models = []

        self.addRE("\[", Done)
        self.addRE(_reNamed % "\S*", self.handle)
        self.addRE(_reAny, Done)

        self.process()

        del self.fdin
        del self.patterns

    def handle(self, key, name):
        self.models.append((key, name))

class IbisVICurve(_Builder):
    """IBIS Model VI Curve Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.typ = []
        self.min = []
        self.max = []

        self.addRE("\[", Done)
        self.addRE(_reRange % "\S*", self.handle)

        self.process()

        self.typ = array(self.typ)
        self.min = array(self.min)
        self.max = array(self.max)

        del self.fdin
        del self.patterns

    def handle(self, key, typ, min, max):
        key = units.float(key)
        if key == None:
            return
        self.typ.append([key, units.float(typ)])
        if units.float(min) == None:
            self.min.append([key, units.float(typ)])
        else:
            self.min.append([key, units.float(min)])
        if units.float(max) == None:
            self.max.append([key, units.float(typ)])
        else:
            self.max.append([key, units.float(max)])


class IbisPin(_Builder):
    """IBIS Model Pin Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.addRE("\[", Done)
        self.addRE(r"""(?x)\s*?
            (?P<key>\S*)\s*    # Pin name
            (?P<signal>\S*)\s*    # Signal Name
            (?P<model>\S*)\s*?    # Model Name
            (?:                    # RLC's are optional
                (?P<R>\S*?)\s*?  # Resistance
                (?P<L>\S*?)\s*?  # Inductance
                (?P<C>\S*?)\s*?  # Capacitance
            )?[|\r\n]""", self.handle)

        self.process()

        del self.fdin
        del self.patterns

    def handle(self, key, signal, model, R=None, L=None, C=None):
        setattr(self, key, IbisPinData(signal, model, R, L, C))

class IbisPackage(_Builder):
    """IBIS Model Package Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.addRE(_reRange % "r[ _]pkg", self.handleRange)
        self.addRE(_reRange % "l[ _]pkg", self.handleRange)
        self.addRE(_reRange % "c[ _]pkg", self.handleRange)
        self.addRE(_reAny, Done)

        self.process()

        del self.fdin
        del self.patterns

class IbisWaveform(_Builder):
    """IBIS Model Waveform Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.data = {}
        self.data['typ'] = []
        self.data['min'] = []
        self.data['max'] = []

        self.addRE("\[", Done)
        self.addRE(_reValue % "r[ _]fixture", self.handleValue)
        self.addRE(_reValue % "c[ _]fixture", self.handleValue)
        self.addRE(_reValue % "l[ _]fixture", self.handleValue)
        self.addRE(_reValue % "v[ _]fixture_min", self.handleValue)
        self.addRE(_reValue % "v[ _]fixture_max", self.handleValue)
        self.addRE(_reValue % "v[ _]fixture", self.handleValue)
        self.addRE("\s*" + _reRange % "\S*", self.handle)
        self.addRE(_reAny, Done)

        self.process()

        self.data['typ'] = array(self.data['typ'])
        self.data['min'] = array(self.data['min'])
        self.data['max'] = array(self.data['max'])

        del self.fdin
        del self.patterns

    def handle(self, key, typ, min, max):
        self.data['typ'].append([units.float(key), units.float(typ)])
        if units.float(min) == None:
            self.data['min'].append([units.float(key), units.float(typ)])
        else:
            self.data['min'].append([units.float(key), units.float(min)])
        if units.float(max) == None:
            self.data['max'].append([units.float(key), units.float(typ)])
        else:
            self.data['max'].append([units.float(key), units.float(max)])


class IbisRamp(_Builder):
    """IBIS Model Ramp Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        # Default Load
        self.r_load = 50

        _reDvDtRange = r"""(?x)(?i)\s*
            (?P<key>%s)\s*
            (?P<typ_dv>\S*)\/(?P<typ_dt>\S*)\s*
            (?P<min_dv>\S*)\/(?P<min_dt>\S*)\s*
            (?P<max_dv>\S*)\/(?P<max_dt>\S*)\s*
            [|\r\n]"""

        _reDvDtNARange = r"""(?x)(?i)\s*
            (?P<key>%s)\s*
            (?P<typ_dv>\S*)\/(?P<typ_dt>\S*)\s*
            na\s*na\s*
            [|\r\n]"""

        self.addRE(_reDvDtRange % "dv/dt_r", self.handle)
        self.addRE(_reDvDtRange % "dv/dt_f", self.handle)
        self.addRE(_reDvDtNARange % "dv/dt_r", self.handleNA)
        self.addRE(_reDvDtNARange % "dv/dt_f", self.handleNA)
        self.addRE(_reValue % "r_load", self.handleValue)
        self.addRE(_reAny, Done)

        self.process()

        del self.fdin
        del self.patterns

    def handle(self, key, typ_dv, typ_dt, min_dv, min_dt, max_dv, max_dt):

        key_dv = 'dv_' + key.lower()[-1]
        key_dt = 'dt_' + key.lower()[-1]
        if not hasattr(self, key_dv):
            setattr(self, key_dv, {})
        if not hasattr(self, key_dt):
            setattr(self, key_dt, {})
        dic_dv = getattr(self, key_dv)
        dic_dt = getattr(self, key_dt)
        dic_dv['typ'] = units.float(typ_dv)
        dic_dv['min'] = units.float(min_dv)
        dic_dv['max'] = units.float(max_dv)
        dic_dt['typ'] = units.float(typ_dt)
        dic_dt['min'] = units.float(min_dt)
        dic_dt['max'] = units.float(max_dt)

    def handleNA(self, key, typ_dv, typ_dt):

        key_dv = 'dv_' + key.lower()[-1]
        key_dt = 'dt_' + key.lower()[-1]
        if not hasattr(self, key_dv):
            setattr(self, key_dv, {})
        if not hasattr(self, key_dt):
            setattr(self, key_dt, {})
        dic_dv = getattr(self, key_dv)
        dic_dt = getattr(self, key_dt)
        dic_dv['typ'] = units.float(typ_dv)
        dic_dv['min'] = units.float(typ_dv)
        dic_dv['max'] = units.float(typ_dv)
        dic_dt['typ'] = units.float(typ_dt)
        dic_dt['min'] = units.float(typ_dt)
        dic_dt['max'] = units.float(typ_dt)


class IbisModel_Spec(_Builder):
    """IBIS Model Spec Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.addRE(_reRange % "vinl", self.handleRange)
        self.addRE(_reRange % "vinh", self.handleRange)
        self.addRE(_reRange % "vinh[+]", self.handleRange)
        self.addRE(_reRange % "vinh[-]", self.handleRange)
        self.addRE(_reRange % "vinl[+]", self.handleRange)
        self.addRE(_reRange % "vinl[-]", self.handleRange)
        self.addRE(_reRange % "s[ _]overshoot[ _]high", self.handleRange)
        self.addRE(_reRange % "s[ _]overshoot[ _]low", self.handleRange)
        self.addRE(_reRange % "d[ _]overshoot[ _]high", self.handleRange)
        self.addRE(_reRange % "d[ _]overshoot[ _]low", self.handleRange)
        self.addRE(_reRange % "d[ _]overshoot[ _]time", self.handleRange)
        self.addRE(_reRange % "pulse_high", self.handleRange)
        self.addRE(_reRange % "pulse[ _]low", self.handleRange)
        self.addRE(_reRange % "pulse[ _]time", self.handleRange)
        self.addRE(_reRange % "vmeas", self.handleRange)
        self.addRE(_reRange % "vref", self.handleRange)
        self.addRE(_reRange % "cref", self.handleRange)
        self.addRE(_reRange % "rref", self.handleRange)
        self.addRE(_reRange % "cref[ _]rising", self.handleRange)
        self.addRE(_reRange % "cref[ _]falling", self.handleRange)
        self.addRE(_reRange % "rref[ _]rising", self.handleRange)
        self.addRE(_reRange % "rref[ _]falling", self.handleRange)
        self.addRE(_reRange % "vref[ _]rising", self.handleRange)
        self.addRE(_reRange % "vref[ _]falling", self.handleRange)
        self.addRE(_reRange % "vmeas[ _]rising", self.handleRange)
        self.addRE(_reRange % "vmeas[ _]falling", self.handleRange)
        self.addRE(_reRange % "rref[ _]diff", self.handleRange)
        self.addRE(_reRange % "cref[ _]diff", self.handleRange)
        self.addRE(_reAny, Done)

        self.process()

        del self.fdin
        del self.patterns

class IbisModel(_Builder):
    """IBIS Model Model Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.addRE(_reNamed % "model[ _]type", self.handleName)
        self.addRE(_reNamed % "polarity", self.handleName)
        self.addRE(_reNamed % "enable", self.handleName)
        self.addRE(_reValue % "vinl", self.handleValue)
        self.addRE(_reValue % "vinh", self.handleValue)
        self.addRE(_reValue % "vmeas", self.handleValue)
        self.addRE(_reValue % "cref", self.handleValue)
        self.addRE(_reValue % "rref", self.handleValue)
        self.addRE(_reValue % "vref", self.handleValue)
        self.addRE(_reRange % "c_comp", self.handleRange)
        self.addRE(_reLabeledRange % "temperature[ _]range", self.handleRange)
        self.addRE(_reLabeledRange % "voltage[ _]range", self.handleRange)
        self.addRE(_reLabeledRange % "gnd[ _]clamp[ _]reference", self.handleRange)
        self.addRE(_reLabeledRange % "power[ _]clamp[ _]reference", self.handleRange)
        self.addRE(_reLabeledRange % "pulldown[ _]reference", self.handleRange)
        self.addRE(_reLabeledRange % "pullup[ _]reference", self.handleRange)
        self.addRE(_reLabel % "gnd[ _]clamp", self.handleVICurve)
        self.addRE(_reLabel % "power[ _]clamp", self.handleVICurve)
        self.addRE(_reLabel % "pulldown", self.handleVICurve)
        self.addRE(_reLabel % "pullup", self.handleVICurve)
        self.addRE(_reLabel % "ramp", self.handleSingle)
        self.addRE(_reLabel % "model[ _]spec", self.handleSingle)
        self.addRE(_reLabel % "rising[ _]waveform", self.handleWaveform)
        self.addRE(_reLabel % "falling[ _]waveform", self.handleWaveform)
        self.addRE(_reAny, Done)

        self.process()

        del self.fdin
        del self.patterns

        # --------- Calculate the Driver TA Table (if it is a driver) -------

        if ((self.model_type.lower() == "3-state")
                or (self.model_type.lower() == "output")
                or (self.model_type.lower() == "i/o")):

            self.pullup_k = {}
            self.pulldown_k = {}

            self.calcDriverK = {}
            # Determine which function to use to calculate ku and kd
            # For both falling an rising Waveforms
            try:
                waveform = self.rising_waveform[1]
                self.calcDriverK[Rising] = calcDriverKDoubleWave
            except IndexError:
                self.calcDriverK[Rising] = calcDriverKSingleWave
            except AttributeError:
                self.calcDriverK[Rising] = calcDriverKRamp

            try:
                waveform = self.falling_waveform[1]
                self.calcDriverK[Falling] = calcDriverKDoubleWave
            except IndexError:
                self.calcDriverK[Falling] = calcDriverKSingleWave
            except AttributeError:
                self.calcDriverK[Falling] = calcDriverKRamp


            for direction in [Rising, Falling]:

                self.pullup_k[direction] = {}
                self.pulldown_k[direction] = {}

                # Calculate ku and kd at Typ, Min, and Max
                for speed in [Typical, Minimum, Maximum]:
                    (ku, kd) = self.calcDriverK[direction](self,
                            direction, speed)
                    self.pullup_k[direction][speed] = ku
                    self.pulldown_k[direction][speed] = kd

class IbisComponent(_Builder):
    """IBIS Model Component Data"""
    def __init__(self, fdin):
        _Builder.__init__(self)
        self.fdin = fdin

        self.addRE(_reNamed % "si[ _]location", self.handleName)
        self.addRE(_reNamed % "timing[ _]location", self.handleName)
        self.addRE(_reLabeled % "manufacturer", self.handleName)
        self.addRE(_reLabel % "package", self.handleSingle)
        self.addRE(_reLabel % "pin", self.handleSingleDict)
        self.addRE(_reAny, Done)

        self.process()

        del self.fdin
        del self.patterns

class Ibis_Parser(_Builder):
    """IBIS Model"""
    def __init__(self, filename, device=None):
        """
        Arguments:
        filename -- IBIS Model to import
        device -- (optional) name of default device, default = first device
        """

        _Builder.__init__(self)
        if filename == 'test':
            import ibis_test
            import io
            self.fdin = io.StringIO(ibis_test.ibis_test)
        else:
            filename = os.path.join(sys.path[0], filename)
            self.fdin = open(filename, 'r')

        self.addRE(_reLabeled % "ibis[ _]ver", self.handleName)
        # Todo: Make this comment char thing work
        self.addRE(_reLabeled % "comment[ _]char", self.handleCommentChar)
        self.addRE(_reLabeled % "file[ _]name", self.handleName)
        self.addRE(_reLabeled % "file[ _]rev", self.handleName)
        self.addRE(_reLabeled % "date", self.handleName)
        self.addRE(_reLabeled % "source", self.handleBlock)
        self.addRE(_reLabeled % "notes", self.handleBlock)
        self.addRE(_reLabeled % "disclaimer", self.handleBlock)
        self.addRE(_reLabeled % "copyright", self.handleBlock)
        self.addRE(_reLabeled % "component", self.handleMulti)
        self.addRE(_reLabeled % "model[ _]selector", self.handleMulti)
        self.addRE(_reLabeled % "model", self.handleMulti)
        self.addRE(_reLabel % "end", Done)
        self.addRE(_reAny, self.handleUnkown)

        self.process()

        self.fdin.close()

        del self.fdin
        del self.patterns

        # Set the default component (called device)
        if device == None:
            self.device = next(iter(self.component.keys()))
        else:
            if device in self.component:
                self.device = device
            else:
                raise RuntimeError("Can not find %s in the IBIS File" % device)
