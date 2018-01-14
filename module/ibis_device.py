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
This module contains the IBIS device models

Device Classes:
Receiver -- IBIS Receiver Device Model
Driver -- IBIS Driver Device Model
Package -- IBIS Package Device Model
Pin -- IBIS Pin Model (Buffer model plus PCackage Model)

"""

from ibis_const import *
import warnings

import subckt
import device
import waveform

class Receiver(subckt.Subckt):
    """IBIS Reciver Buffer Device Model"""
    def __init__(self, iNode, pwrNode, gndNode, model, speed = Typical):
        """
        Arguments:
        iNode -- Input Node
        pwrNode -- Vcc Node
        gndNode -- Ground Node
        model -- IbisModel
        speed -- (optional) Maximum, Minimum, Typical, default = Typical
        """

        self.__dict__['model'] = model
        self.__dict__['speed'] = speed

        if hasattr(model, 'c_comp'):
            self.C_comp = device.C(iNode, gndNode, model.c_comp[speed])
        if hasattr(model, 'gnd_clamp'):
            self.VI_gnd = device.VI(iNode, gndNode,
                    waveform.PWL(model.gnd_clamp[speed]))
        if hasattr(model, 'power_clamp'):
            self.VI_pwr = device.VI(pwrNode, iNode,
                    waveform.PWL(model.power_clamp[speed]))

    def __setattr__(self, name, value):

        if ((name == 'model') or (name == 'speed')):
            if hasattr(self.model, 'c_comp'):
                if hasattr(self, 'C_comp'):
                    self.C_comp.C = self.model.c_comp[self.speed]
                else:
                    raise RuntimeError("Can't add C_comp")
            if hasattr(self.model, 'gnd_clamp'):
                if hasattr(self, 'VI_gnd'):
                    self.VI_gnd.VI = waveform.PWL(self.model.gnd_clamp[self.speed])
                else:
                    raise RuntimeError("Can't add VI_gnd")
            if hasattr(self.model, 'power_clamp'):
                if hasattr(self, 'VI_pwr'):
                    self.VI_pwr.VI = waveform.PWL(self.model.power_clamp[self.speed])
                else:
                    raise RuntimeError("Can't add VI_pwr")
        else:
            subckt.Subckt.__setattr__(self, name, value)

class Driver(subckt.Subckt):
    """IBIS Driver Buffer Device Model"""
    def __init__(self, oNode, pwrNode, gndNode, model, speed = Typical,
            direction = Rising):
        """
        Arguments:
        oNode -- Output Node
        pwrNode -- Vcc Node
        gndNode -- Ground Node
        model -- IbisModel
        speed -- (optional) Maximum, Minimum, or Typical, default = Typical
        direction -- (optional) Rising or Falling, default = Rising
        """

        self.__dict__['model'] = model
        self.__dict__['speed'] = speed
        self.__dict__['direction'] = direction

        if hasattr(model, 'c_comp'):
            self.C_comp = device.C(oNode, gndNode, model.c_comp[speed])
        if hasattr(model, 'gnd_clamp'):
            self.VI_gnd = device.VI(oNode, gndNode,
                    waveform.PWL(model.gnd_clamp[speed]))
        if hasattr(model, 'power_clamp'):
            self.VI_pwr = device.VI(pwrNode, oNode,
                    waveform.PWL(model.power_clamp[speed]))
        if hasattr(model, 'pullup'):
            self.VI_pu = device.VI(pwrNode, oNode,
                    waveform.PWL(model.pullup[speed]),
                    waveform.PWL(model.pullup_k[direction][speed]))
        if hasattr(model, 'pulldown'):
            self.VI_pd = device.VI(oNode, gndNode,
                    waveform.PWL(model.pulldown[speed]),
                    waveform.PWL(model.pulldown_k[direction][speed]))

    def __setattr__(self, name, value):

        if ((name == 'model') or (name == 'speed') or (name == 'direction')):
            if hasattr(self.model, 'C_comp'):
                if hasattr(self, 'C_comp'):
                    self.C_comp.C = self.model.c_comp[self.speed]
                else:
                    raise RuntimeError("Can't add C_comp")
            if hasattr(self.model, 'gnd_clamp'):
                if hasattr(self, 'VI_gnd'):
                    self.VI_gnd.VI = waveform.PWL(self.model.gnd_clamp[self.speed])
                else:
                    raise RuntimeError("Can't add VI_gnd")
            if hasattr(self.model, 'power_clamp'):
                if hasattr(self, 'VI_pwr'):
                    self.VI_pwr.VI = waveform.PWL(self.model.power_clamp[self.speed])
                else:
                    raise RuntimeError("Can't add VI_pwr")
            if hasattr(self.model, 'pullup'):
                if hasattr(self, 'VI_pu'):
                    self.VI_pu.VI = waveform.PWL(self.model.pullup[self.speed])
                    self.VI_pu.TA = waveform.PWL(self.model.pullup_k[self.direction][self.speed])
                else:
                    raise RuntimeError("Can't add VI_pu")
            if hasattr(self.model, 'pulldown'):
                if hasattr(self, 'VI_pd'):
                    self.VI_pd.VI = waveform.PWL(self.model.pulldown[self.speed])
                    self.VI_pd.TA = waveform.PWL(self.model.pulldown_k[self.direction][self.speed])
                else:
                    raise RuntimeError("Can't add VI_pd")
        else:
            subckt.Subckt.__setattr__(self, name, value)

class Package(subckt.Subckt):
    """IBIS Package Device Model (Package Paracitics)"""
    def __init__(self, pinNode, dieNode, gndNode, pin, speed=Typical):
        """
        Arguments:
        pinNode -- Pin Node (PCB Side of the model)
        dieNode -- Die Node (Die Side of the model)
        gndNode -- Ground Node
        pin -- IbisPin
        speed -- (optional) Maximum, Minimum, or Typical, default = Typical
        """

        self.__dict__['pin'] = pin

        self.Cpkg = device.C(pinNode, gndNode, pin.C)
        self.Rpkg = device.R(self.node('node'), pinNode, pin.R)
        self.Lpkg = device.L(dieNode, self.node('node'), pin.L)

    def __setattr__(self, name, value):

        if (name == 'pin'):
            self.Cpkg.C = self.pin.C
            self.Lpkg.L = self.pin.L
            self.Rpkg.R = self.pin.R
        else:
            subckt.Subckt.__setattr__(self, name, value)

class Pin(subckt.Subckt):
    """IBIS Pin Device Model (Package Paracitics plus Buffer)"""
    def __init__(self, ibs, pinName, node, speed=Typical,
            direction=Rising, io=Output, modelName=None):
        """
        Arguments:
        ibs -- Ibis Class
        pinName -- Pin Name (string)
        node -- Node to connect pin to
        speed -- (optional) Maximum, Minimum, or Typical, default = Typical
        direction -- (optional) Rising or Falling, default = Rising
        --- Only applies when the pin is a driver
        modelName -- (optional) If the driver uses the Model Selector keyword
            this argument selects the model to use
        io -- (optional) Output or Input, default = Output
        --- Only applies when the pin is an I/O Buffer
        """

        pin = ibs.component[ibs.device].pin[str(pinName)]
        try:
            model = ibs.model[pin.model]
        except KeyError:
            if modelName == None:
                warn = '\nPin ' + pinName + ':\n'
                warn += 'No model selected but driver has a model selector keyword.\n'
                warn += 'Avaliable Models are (set the modelName argument to the model you want to use):\n'
                for name, description in ibs.model_selector[pin.model].models:
                    if modelName == None:
                        modelName = name
                    warn += name
                    warn += ': ' + description + '\n'
                warn += 'Using ' + modelName
                warnings.warn(warn)
            model = ibs.model[modelName]


        vcc = self.node('vcc')
        die = self.node('die')

        self.Vcc = device.V(vcc, 0, model.voltage_range[speed])

        if ((model.model_type == 'input') or
                ((model.model_type == 'i/o') and io==Input)):

            self.Buffer = Receiver(die, vcc, 0, model, speed)

        elif ((model.model_type == 'output') or
                (model.model_type == '3-state') or
                (model.model_type == 'i/o')):

            self.Buffer = Driver(die, vcc, 0, model, speed, direction)

        else:

            raise RuntimeError("Unsupported Model Type %s" % model.model_type)

        R = pin.R
        if (pin.R == None):
            pin.R = ibs.component[ibs.device].package.r_pkg[speed]

        C = pin.C
        if (pin.C == None):
            pin.C = ibs.component[ibs.device].package.c_pkg[speed]

        L = pin.L
        if (pin.L == None):
            pin.L = ibs.component[ibs.device].package.l_pkg[speed]

        self.Package = Package(node, die, 0, pin, speed)

        pin.R = R
        pin.C = C
        pin.L = L
