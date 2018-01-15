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
This module provides a class that can be used as a base class to
create esipice sub-circuits.

Classes:
Subckt -- Base calss used to define a sub-circuit.
"""

_subcktCnt = 0

class Subckt(list):
    """
    Is used as a base class that is inherited by User Defined a
    sub-circuit. Similar to a subckt in Berkely Spice.

    Example:
    >>> import eispice
    >>> class Subckt1(eispice.Subckt):
    ...    def __init__(self, pNode, nNode, Rx, Ry):
    ...        self.Rx = eispice.R(pNode, self.node('node'), Rx)
    ...        self.Ry = eispice.R(nNode, self.node('node'), Ry)
    >>> class Subckt2(eispice.Subckt):
    ...    def __init__(self, pNode, nNode, Rx, Ry):
    ...        self.Rx = eispice.R(pNode, self.node('node'), Rx)
    ...        self.Xy = Subckt1(nNode, self.node('node'), Rx, Ry)
    >>> cct = eispice.Circuit("Subckt Test")
    >>> cct.Vx = eispice.V(1, 0, 10)
    >>> cct.Xx = Subckt2(1, 0, 100, 100)
    >>> cct.op()
    >>> cct.check_i('Vx', -10.0 / 300.0)
    True
    """

    def __new__(self, *args, **argsk):
        # NOTE: This is not necissailly the best way to do this
        self.subcktCnt = globals()['_subcktCnt']
        globals()['_subcktCnt'] += 1
        return list.__new__(self, args, argsk)

    def node(self, name):
        """Identifys a local node, that doesn't exit the Subckt"""
        return "%s@%s" % (self.subcktCnt, name)

    def device(self, name):
        """Identifys a local device, that doesn't exit the Subckt"""
        return "%s#%s" % (self.subcktCnt, name)

    def __setattr__(self, name, value):
        """
        Adds a device that can be accessed using its name, and creates a
        flat version by adding a prefix based on the instantiations id to
        uniquly label the device instances. This is done because the base
        simulator doesn't support hiearchy and every device needs a unique
        identifier. If the device being attached is a Subckt it pulls in
        its flat circuit dict.
        """
        self.__dict__[name] = value

        try:
            for (subName, subValue) in value:
                self.append((subName, subValue))
        except TypeError:
            self.append((self.device(name),value))

# --------------------------------------------------------------------------- #
#                                  Test                                       #
# --------------------------------------------------------------------------- #

if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print('Testing Complete')
