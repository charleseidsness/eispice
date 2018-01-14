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
This module contains the fucntions used to caculate IBIS Driver
IV multipliers.

Functions:
calcDriverKDoubleWave -- Calculates a TA Table using 2 Waveforms
calcDriverKSingleWave -- Calculates a TA Table using 1 Waveform
calcDriverKRamp -- Calculates a TA Table using Ramp Data

"""

from ibis_const import *

import warnings

import waveform
import circuit
import device

def calcDriverKDoubleWave(model, direction, speed, tstepDivisor=10):
    """
    Calculates a Time/Multiplier (TA) Table using 2 Waveforms

    Arguments:
    model -- IBIS Model Class
    direction -- either Rising or Falling (constants defined above)
    speed -- either Typical, Maximum, or Minimum (constants defined above)
    tstepDivisor -- Number of simulation points between defined waveform
        points, a larger number may yeild a more accurate result but will
        take longer to process.

    Returns:
    (ku, kd) -- two 2D Arrays, the first column in each is time and the
    second is either the up (ku) or down (kd) VI Multiplier

    To create an IBIS Defined Driver Model that changes state the Driver's
    two VI Tables (Pull-Up and Pull-Down) are modified during the simulation
    by multiplying them    at each time step with values from the TA Tables.

    This method of calculating a Driver TA Table is the most accuarate.
    It uses two provided Waveforms driven into two different loads. This
    function uses the eispice simulation engine to simulate two instanciations
    of the follow circuit:

            VCC                             VCC
             +                               +
             |                               |  Power Clamp IV
            .-. Pull-Up IV                  .-. (Xcu0/1)
            | | (Xpu0/1)                    | |
            | |                             | |       Vfixture
   .-----.  '-'       _ Current Probe       '-'          +
   |  G  |   |       / \ (Vmeas)             |     ___   |
   |_-_-_|----------(_/_)--------o-----------o----|___|--o
   |     |   |       \_/         |           |  Rfixture
   '-----'  .-.                  |          .-.
  Waveform  | | Pull-Down IV    --- C_comp  | | Ground Clamp IV
  (Xwv0/1)  | | (Xpd0/1)        --- (Cc0/1) | | (Xcd0/1)
            '-'                  |          '-'
             |                   |           |
            ===                 ===         ===
            GND                 GND         GND

    The value of the two multipliers (ku and ku) are calculated at
    each time step using a two B-Elements, based on the folowing relationships:

    Imeas0 = ku*Ipu0 - kd*Ipd0 (1)
    Imeas1 = ku*Ipu1 - kd*Ipd1 (2)

    Solving (1) for ku:

         Imeas0 - kd*Ipd0
    ku = ----------------- (3)
               Ipu0

    Substitute into (2) and solving results in:

         Imeas0*Ipu1 - Imeas1*Ipu0
    kd = ------------------------- (4)
           Ipd1*Ipu0 - Ipd0*Ipu1

    To limit calculation error the paired releationship is used to
    calculate ku, not (3):

         Imeas0*Ipd1 - Imeas1*Ipd0
    ku = ------------------------- (5)
           Ipd1*Ipu0 - Ipd0*Ipu1

    """

    if direction == Rising:
        wave = model.rising_waveform
    elif direction == Falling:
        wave = model.falling_waveform
    else:
        raise RuntimeError('Direction must be Rising or Falling.')

    # Based on the schematic defined above
    cct = circuit.Circuit("Double Waveform")

    # ----------------------- Waveform 0 -----------------------------------
    cct.Vcc = device.V('vcc', 0, model.voltage_range[speed])
    cct.Xwv0 = device.V('wv0', 0, 0, waveform.PWL(wave[0].data[speed]))
    cct.Xpu0 = device.VI('vcc', 'wv0', waveform.PWL(model.pullup[speed]))
    cct.Xpd0 = device.VI('wv0', 0, waveform.PWL(model.pulldown[speed]))
    cct.Vmeas0 = device.V('wv0', 'ts0', 0)
    cct.Cc0 = device.C('ts0', 0, model.c_comp[speed])
    cct.Rfix0 = device.R('ts0', 'fix0', wave[0].r_fixture)
    if speed == Typical:
        cct.Vfix0 = device.V('fix0', 0, wave[0].v_fixture)
    elif speed == Maximum:
        try:
            cct.Vfix0 = device.V('fix0', 0, wave[0].v_fixture_min)
        except AttributeError:
            cct.Vfix0 = device.V('fix0', 0, wave[0].v_fixture)
    elif speed == Minimum:
        try:
            cct.Vfix0 = device.V('fix0', 0, wave[0].v_fixture_max)
        except AttributeError:
            cct.Vfix0 = device.V('fix0', 0, wave[0].v_fixture)
    # May not have GND and/or Power Clamps
    if hasattr(model, 'power_clamp'):
        cct.Xcu0 = device.VI('vcc', 'ts0', waveform.PWL(model.power_clamp[speed]))
    if hasattr(model, 'gnd_clamp'):
        cct.Xcd0 = device.VI('ts0', 0, waveform.PWL(model.gnd_clamp[speed]))


    # ----------------------- Waveform 1 -----------------------------------
    cct.Xwv1 = device.V('wv1', 0, 0, waveform.PWL(wave[1].data[speed]))
    cct.Xpu1 = device.VI('vcc', 'wv1', waveform.PWL(model.pullup[speed]))
    cct.Xpd1 = device.VI('wv1', 0, waveform.PWL(model.pulldown[speed]))
    cct.Vmeas1 = device.V('wv1', 'ts1', 0)
    cct.Cc1 = device.C('ts1', 0, model.c_comp[speed])
    cct.Rfix1 = device.R('ts1', 'fix1', wave[1].r_fixture)
    if speed == Typical:
        cct.Vfix1 = device.V('fix1', 0, wave[1].v_fixture)
    elif speed == Maximum:
        try:
            cct.Vfix1 = device.V('fix1', 0, wave[1].v_fixture_min)
        except AttributeError:
            cct.Vfix1 = device.V('fix1', 0, wave[1].v_fixture)
    elif speed == Minimum:
        try:
            cct.Vfix1 = device.V('fix1', 0, wave[1].v_fixture_max)
        except AttributeError:
            cct.Vfix1 = device.V('fix1', 0, wave[1].v_fixture)
    # May not have GND and/or Power Clamps
    if hasattr(model, 'power_clamp'):
        cct.Xcu1 = device.VI('vcc', 'ts1', waveform.PWL(model.power_clamp[speed]))
    if hasattr(model, 'gnd_clamp'):
        cct.Xcd1 = device.VI('ts1', 0, waveform.PWL(model.gnd_clamp[speed]))

    # ------------------ Multiplier Calculator -----------------------------
    # Equation (4)
    cct.Bkdn = device.B('kd', 0, 'v',
        "(i(Xpu0)*i(Vmeas1) - i(Xpu1)*i(Vmeas0))"
        "/ (i(Xpd0)*i(Xpu1) - i(Xpd1)*i(Xpu0))")
    # Equation (5)
    cct.Bkun = device.B('ku',    0    ,'v',
        "(i(Xpd0)*i(Vmeas1) - i(Xpd1)*i(Vmeas0))"
        "/ (i(Xpd0)*i(Xpu1) - i(Xpd1)*i(Xpu0))")

    # The simulation length is based on the last time point from the longest
    # defined waverform.
    tstop0 = wave[0].data[speed][wave[0].data[speed].shape[0] - 1][0]
    tstop1 = wave[1].data[speed][wave[1].data[speed].shape[0] - 1][0]
    tstop = max(tstop0, tstop1)

    # The simulation step length is based on the shortest step between points
    # 0 and 1.
    tstep0 = wave[0].data[speed][1][0] - wave[0].data[speed][0][0]
    tstep1 = wave[1].data[speed][1][0] - wave[1].data[speed][0][0]
    tstep = min(tstep0, tstep1)

    # Run the simulations
    cct.tran(tstep/tstepDivisor, tstop)

    # Only need the results for ku and kd
    # TODO: Could filter out the time points that aren't on
    # the orginal waveform list to save some memory.
    ku = cct.voltage_array('ku')[1::]    # remove the first point
    kd = cct.voltage_array('kd')[1::]    # remove the first point

    return (ku, kd)

def calcDriverKSingleWave(model, direction, speed, tstepDivisor=10):
    """
    Calculates a Time/Multiplier (TA) Table using 1 Waveform

    Arguments:
    model -- IBIS Model Class
    direction -- either Rising or Falling (constants defined above)
    speed -- either Typ, Max, or Min (constants defined above)
    tstepDivisor -- Number of simulation points between defined waveform
        points, a larger number may yeild a more accurate result but will
        take longer to process.

    Returns:
    (ku, kd) -- two 2D Arrays, the first column in each is time and the
    second is either the up (ku) or down (kd) VI Multiplier

    Refer to calcDriverKDoubleWave for a general description of the TA Table
    creation process. This function uses only one defined waveform and is
    less accurate as a result but unfortunatally there aren't always 2
    waveformes provided in an IBIS Model.

    Since there is only a single defined waveform equations (4) and (5) have
    to be modified:

    Imeas = ku*Ipu - kd*Ipd + Ifix(1)
    ku - kd = 1 (2)

    Solving (1) for ku:

    ku = 1 + kd (3)

    Substitute into (2) and solving results in:

         Imeas - Ipu - Ifix
    kd = ------------------ (4)
              Ipu - Ipd

    To limit calculation error the paired releationship is used to
    calculate ku, not (3):

         Imeas - Ipd - Ifx
    ku = ------------------ (5)
              Ipu - Ipd

    """

    # Would be nice to make this work properly someday, but until then
    # just default to the ramp.
    return calcDriverKRamp(model, direction, speed, tstepDivisor)

    #~ warnings.warn(
        #~ 'IBIS Driver Model based on Single Waveform, will have poor accuracy')

    #~ if direction == Rising:
        #~ wave = model.rising_waveform
    #~ elif direction == Falling:
        #~ wave = model.falling_waveform
    #~ else:
        #~ raise RuntimeError, 'Direction must be Rising or Falling.'

    #~ # Based on the schematic defined above
    #~ cct = circuit.Circuit("Single Waveform")

    #~ # ----------------------- Waveform -------------------------------------
    #~ cct.Vcc = device.V('vcc', 0, model.voltage_range[speed])
    #~ cct.Xwv = device.V('wv', 0, 0, waveform.PWL(wave[0].data[speed]))
    #~ cct.Xpu = device.VI('vcc', 'wv', waveform.PWL(model.pullup[speed]))
    #~ cct.Xpd = device.VI('wv', 0, waveform.PWL(model.pulldown[speed]))
    #~ cct.Vmeas = device.V('wv', 'ts', 0)
    #~ cct.Cc = device.C('ts', 0, model.c_comp[speed])
    #~ cct.Rfix = device.R('ts', 'fix', wave[0].r_fixture)
    #~ if speed == Typical:
        #~ cct.Vfix = device.V('fix', 0, wave[0].v_fixture)
    #~ elif speed == Maximum:
        #~ try:
            #~ cct.Vfix = device.V('fix', 0, wave[0].v_fixture_min)
        #~ except AttributeError:
            #~ cct.Vfix = device.V('fix', 0, wave[0].v_fixture)
    #~ elif speed == Minimum:
        #~ try:
            #~ cct.Vfix = device.V('fix', 0, wave[0].v_fixture_max)
        #~ except AttributeError:
            #~ cct.Vfix = device.V('fix', 0, wave[0].v_fixture)
    #~ # May not have GND and/or Power Clamps
    #~ if hasattr(model, 'power_clamp'):
        #~ cct.Xcu = device.VI('vcc', 'ts', waveform.PWL(model.power_clamp[speed]))
    #~ if hasattr(model, 'gnd_clamp'):
        #~ cct.Xcd = device.VI('ts', 0, waveform.PWL(model.gnd_clamp[speed]))

    #~ # ------------------ Multiplier Calculator -----------------------------
    #~ # Equation (4)
    #~ cct.Bkdn = device.B('kd', 0, 'v',
        #~ "(i(Vmeas) - i(Xpu) - i(Vfix))"
        #~ "/ (i(Xpd) - i(Xpu))")
    #~ # Equation (5)
    #~ cct.Bkun = device.B('ku',    0    ,'v',
        #~ "(i(Vmeas) - i(Xpd) - i(Vfix))"
        #~ "/ (i(Xpd) - i(Xpu))")

    #~ # The simulation length is based on the last time point of the waveform
    #~ tstop = wave[0].data[speed][wave[0].data[speed].shape[0] - 1][0]

    #~ # The simulation step length is based on the step between points 0 and 1.
    #~ tstep = wave[0].data[speed][1][0] - wave[0].data[speed][0][0]

    #~ # Run the simulations
    #~ cct.tran(tstep/tstepDivisor, tstop)

    #~ # Only need the results for ku and kd
    #~ # TODO: Could filter out the time points that aren't on
    #~ # the orginal waveform list to save some memory.
    #~ ku = cct.voltage('ku')[1::]    # remove the first point which tends to be bad
    #~ kd = cct.voltage('kd')[1::]    # remove the first point which tends to be bad

    #~ return (ku, kd)

def calcDriverKRamp(model, direction, speed, tstepDivisor=10):
    """
    Calculates a Time/Multiplier (TA) Table using 1 Waveform

    Arguments:
    model -- IBIS Model Class
    direction -- either Rising or Falling (constants defined above)
    speed -- either Typ, Max, or Min (constants defined above)
    tstepDivisor -- Number of simulation points between defined waveform
        points, a larger number may yeild a more accurate result but will
        take longer to process.

    Returns:
    (ku, kd) -- two 2D Arrays, the first column in each is time and the
    second is either the up (ku) or down (kd) VI Multiplier

    Refer to calcDriverKDoubleWave for a general description of the TA Table
    creation process. This function uses the ramp data and interpolates
    a gaussina rising edge from it to use as a wavefrom example. It is
    less accurate as a result but unfortunatally there aren't always 2
    waveformes provided in an IBIS Model.

    Since there is only a single defined waveform equations (4) and (5) have
    to be modified:

    Imeas = ku*Ipu - kd*Ipd + Ifix(1)
    ku - kd = 1 (2)

    Solving (1) for ku:

    ku = 1 + kd (3)

    Substitute into (2) and solving results in:

         Imeas - Ipu - Ifix
    kd = ------------------ (4)
              Ipu - Ipd

    To limit calculation error the paired releationship is used to
    calculate ku, not (3):

         Imeas - Ipd - Ifx
    ku = ------------------ (5)
              Ipu - Ipd

    """

    warnings.warn(
        'IBIS Driver Model based on Ramp Specs only, will have poor accuracy')

    #~ vol = model.voltage_range[speed] - model.ramp.dv_r[speed]/0.6
    vol = 0.0
    #~ voh = model.ramp.dv_r[speed]/0.6
    voh = model.voltage_range[speed]

    if direction == Rising:
        ramp = model.ramp.dt_r[speed]
        wave = waveform.Gauss(vol, voh, 0, ramp)
    elif direction == Falling:
        ramp = model.ramp.dt_f[speed]
        wave = waveform.Gauss(voh, vol, 0, ramp)
    else:
        raise RuntimeError('Direction must be Rising or Falling.')

    # Based on the schematic defined above
    cct = circuit.Circuit("Ramp")

    # ----------------------- Waveform -------------------------------------
    cct.Vcc = device.V('vcc', 0, model.voltage_range[speed])
    cct.Xwv = device.V('wv', 0, 0, wave)
    cct.Xpu = device.VI('vcc', 'wv', waveform.PWL(model.pullup[speed]))
    cct.Xpd = device.VI('wv', 0, waveform.PWL(model.pulldown[speed]))
    cct.Vmeas = device.V('wv', 'ts', 0)
    cct.Cc = device.C('ts', 0, model.c_comp[speed])
    cct.Rfix = device.R('ts', 'fix', model.ramp.r_load)
    if direction == Rising:
        cct.Vfix = device.V('fix', 0, 0.0)
    elif direction == Falling:
        cct.Vfix = device.V('fix', 0, model.voltage_range[speed])
    # May not have GND and/or Power Clamps
    if hasattr(model, 'power_clamp'):
        cct.Xcu = device.VI('vcc', 'ts', waveform.PWL(model.power_clamp[speed]))
    if hasattr(model, 'gnd_clamp'):
        cct.Xcd = device.VI('ts', 0, waveform.PWL(model.gnd_clamp[speed]))

    # ------------------ Multiplier Calculator -----------------------------
    # Equation (4)
    cct.Bkdn = device.B('kd', 0, 'v',
        "(i(Vmeas) - i(Xpu) - i(Vfix))"
        "/ (i(Xpd) - i(Xpu))")
    # Equation (5)
    cct.Bkun = device.B('ku', 0, 'v',
        "(i(Vmeas) - i(Xpd) - i(Vfix))"
        "/ (i(Xpd) - i(Xpu))")

    # The simulation length is based on the last time point of the waveform
    tstop = ramp*4

    # The simulation step length is based on the step between points 0 and 1.
    tstep = ramp*0.5

    # Run the simulations
    cct.tran(tstep/tstepDivisor, tstop)

    # Only need the results for ku and kd
    # TODO: Could filter out the time points that aren't on
    # the orginal waveform list to save some memory.
    ku = cct.voltage_array('ku')[1::]    # remove the first point
    kd = cct.voltage_array('kd')[1::]    # remove the first point

    #~ if direction == Rising and speed == Typical:
        #~ import plot
        #~ plot.plot(cct)

    return (ku, kd)
