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
This module provides the main Circuit class for the eispice simulator. The
Circuit class holds the netlist, device models, and is used to run sims
on a specific circuit instance.

Classes:
Circuit -- an eispice circuit
"""

from scipy import interpolate

import units
from simulator_ import Circuit_
import subckt

Current = 'i'
Voltage = 'v'
Time = 'time'
GND = '0'

def sign(value):
    """Returns the signe of a value."""
    if value >= 0:
        return 1
    else:
        return -1

class _interp1d(interpolate.interp1d):
    """
    Local implimentation of the scipy intep1d class so that
        the x and y objects can be renamed.
    """
    def __init__(self, *args):
        try:
            interpolate.interp1d.__init__(self, *args)
        except ValueError:
            # only single time point (operating point only)
            self.time = None
            self.all = args[1]
            return
        self.time = self.x
        self.all = self.y
        self.value = None
    def __call__(self, time):
        if self.time == None:
            return self.all
        return float(interpolate.interp1d.__call__(self, units.float(time)))

class _dict(dict):
    """
    Local implimentation of a dictionary so that node names can be
        passed as keys, expressed as either integers of strings.
    """
    def __getitem__(self, key):
        return dict.__getitem__(self, str(key))

class Circuit(Circuit_):
    """
    *italics*This is the primary class in eispice, it holds the netlist, device
    models, and is used to run sims on a specific circuit instance.
    In our average simulation, devices are added to the circuit, a
    simulation is run, and then the results are plotted or manipulated to
    determine some sort of result, e.g. under-shoot, noise levels, etc.

    Once a simulation has been run new devices can't be added, and devices
    can never be removed, but parameter values can be changed and new
    simulations run on the same circuit.

    To add a device to a circuit create a new method that is devined as a
    specific device, i.e.:
    circuit.R2 = eispice.R('2', '0', '20k')

    To run an simulation on a circuit call the simulation method, e.g.:
    circuit.tran('0.1n', '100n')

    The results of the last simulation can be accesses using the i, v,
    and t dictionaries, the voltage_array and current_array methods or
    directly using the results and variables arrays.

    Example (remove the >>> 's when putting commands into a batch-mode file):

    Import the eispice module into Python:
    >>> import eispice

    Create a new instance of a circuit:
    >>> cct = eispice.Circuit("Circuit Test")

    Create a couple of devices:
    >>> cct.Vx = eispice.V(1, eispice.GND, 10)
    >>> cct.Rx = eispice.R(1, eispice.GND, '100')

    Run a transient simulation:
    >>> cct.tran('1n', '2n')

    Check the result at 1 nano-second:
    >>> cct.check_v(1,10,'1n')
    True
    """

    def __init__(self, title='eispice'):
        """
        Circuit constructor.

        Arguments:
        title -- String containg the circuit's name.
        """
        Circuit_.__init__(self, title)

    def __setattr__(self, name, value):
        """
        Adds a device to the circuit, if it's a list of devices
        like a Subckt it adds all of the devices in the Subckt.
        """
        try:
            for (subName, subValue) in value:
                Circuit_.__setattr__(self, subName, subValue)
        except TypeError:
            Circuit_.__setattr__(self, name, value)

    def _results(self):
        """Creates dictionaries of results for easy access."""
        Circuit_.__setattr__(self, 't', self.results.transpose()[0])
        Circuit_.__setattr__(self, 'i', {})
        Circuit_.__setattr__(self, 'v', _dict())
        for n in range(0,(self.results.shape[1])):
            if (self.variables[n][0] == 'i'):
                self.i[self.variables[n][2:-1]] = _interp1d(self.t,
                        self.results.transpose()[n])
            if (self.variables[n][0] == 'v'):
                self.v[self.variables[n][2:-1]] = _interp1d(self.t,
                        self.results.transpose()[n])

    # For documentation purposes only
    def i():
        """
        Dictionary of current results, i.e. circuit.i['Vx']('1n') returns
        the current through Voltage Source Vx at 1 nansecond.
        """

    # For documentation purposes only
    def v():
        """
        Dictionary of voltage results, i.e. circuit.v[1]('1n') returns
        the voltage at node 1 at 1 nansecond.
        """

    def current_array(self, *variables):
        """
        Returns an array where the first column is time, and the subsiquent
        columns are the value of the current through each listed device.
        NOTE: Like spice3f5 currents are only calculated through voltage
        sources and inductors (plus a few other special devices). To measure
        current add a voltage source with 0V DC to your circuit as a probe.
        """
        index = []        # list of indexes in the results array
        index.append(self.variables.index('time'))    # first variable is time
        for variable in variables:
            index.append(self.variables.index('i(%s)' % str(variable)))
        return self.results.take(index, 1)

    def voltage_array(self, *variables):
        """
        Returns a results array where the first column is time, and the
        subsiquent columns are the value of the voltage at each listed node.
        """
        index = []        # list of indexes in the results array
        index.append(self.variables.index('time'))    # first variable is time
        for variable in variables:
            index.append(self.variables.index('v(%s)' % str(variable)))
        return self.results.take(index, 1)

    def tran(self, tstep, tstop, tmax=0.0, restart=False):
        """
        Runs a Transient analysis and sets the value of the circuit's
        results array accordingly. It is equivalent to the Spice3 tran
        command.

        Arguments:
        tstep -- desired plotting increment, the actual time-steps will
            vary from step to step, this is simply a suggestion
        tstop -- the last time point to simulate
        tmax -- maximum step size, it is set to tstep if not defined or 0.0
        restart -- start a new simulation (False forces the simulator to
            continue from the last transient simulation) -- default = False

        Example:
        >>> import eispice
        >>> cct = eispice.Circuit("Circuit Tran Test")
        >>> cct.Vx = eispice.V(1, eispice.GND, 1)
        >>> cct.Rx = eispice.R(1, eispice.GND, '1')
        >>> cct.tran('0.1n','1n', '0.5n')
        >>> cct.check_v(1, '1','0.5n')
        True
        """
        self.tran_(units.float(tstep), units.float(tstop), units.float(tmax),
                restart)
        self._results()


    def op(self):
        """
        Runs an Operating Point analysis and sets the value of the circuit's
        results array accordingly. It is equivalent to the Spice3 op command.

        Example:
        >>> import eispice
        >>> cct = eispice.Circuit("Circuit Tran Test")
        >>> cct.Vx = eispice.V(1, eispice.GND, 1)
        >>> cct.Rx = eispice.R(1, eispice.GND, '1')
        >>> cct.op()
        >>> cct.check_v(1, '1')
        True
        """
        self.op_()
        self._results()

    def devices(self):
        """Prints a list of the devices in the circuit."""
        self.devices_()

    def _check(self, name, value, time, simValue):
        """
        Raises an exception if the value of the variable at time (seconds)
        does not equal the value supplied +/-0.01%.

        Arguments:
        name -- name of the node or device
        value -- value to check against
        time -- time point to check at (seconds) -- default = 0.0
        type -- either Voltage or Current -- default = Voltage
        """

        if (simValue < value*(1.0 - sign(value)*1e-4)):
            print("%s FAIL: %.9e != %.9e" % (name, value, simValue))
            return False
        if (simValue > value*(1.0 + sign(value)*1e-4)):
            print("%s FAIL: %.9e != %.9e" % (name, value, simValue))
            return False

        return True

    def check_v(self, name, value, time=0.0):
        """
        Raises an exception if the value of the voltage at time (seconds)
        does not equal the value supplied +/-0.01%.

        Arguments:
        name -- name of the node
        value -- value to check against
        time -- time point to check at (seconds) -- default = 0.0
        """

        value = units.float(value)
        time = units.float(time)
        name = str(name)

        return self._check(name, value, time, self.v[name](time))

    def check_i(self, name, value, time=0.0):
        """
        Raises an exception if the value of the current at time (seconds)
        does not equal the value supplied +/-0.01%.

        Arguments:
        name -- name of the device
        value -- value to check against
        time -- time point to check at (seconds) -- default = 0.0
        type -- either Voltage or Current -- default = Voltage
        """

        value = units.float(value)
        time = units.float(time)
        name = str(name)

        return self._check(name, value, time, self.i[name](time))


if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print('Testing Complete')
