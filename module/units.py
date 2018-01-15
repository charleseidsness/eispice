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
This module provides functions that can be used to turn a string with
units into a floating point number. Strings can consist of a floating
number followed by a multiplier (listed below) and then followed with
any characters, e.g. '10nF', '1e-2pCRUD', '100mOhms'. If passed anything
other than a string the functions in this module behaive similar to the
builtin float function.

Multipliers:
T -- 1e12
G -- 1e9
M -- 1e6
k -- 1e3
m -- 1e-3
u -- 1e-6
n -- 1e-9
p -- 1e-12
f -- 1e-15

Functions:
float -- returns a float value from a string
floatList1D -- takes a 1D list and returns numpy array of floats
floatList2D -- takes a 2D list and returns numpy array of floats

"""

from numpy import array, ndarray

import re

_pattern = re.compile(
    "\s*(?P<valueN>[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)"
    "(?P<unitsN>\w*)\s*"
    "(?:[/]\s*(?P<valueD>[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)"
    "(?P<unitsD>\w*))?"
    )

T = 1e12
G = 1e9
M = 1e6
k = 1e3
m = 1e-3
u = 1e-6
n = 1e-9
p = 1e-12
f = 1e-15

def _float(value, units):
    if units is ('' or None):
        return builtinFloat(value)
    elif units.find('T') is 0:
        return builtinFloat(value) * T
    elif units.find('G') is 0:
        return builtinFloat(value) * G
    elif units.find('M') is 0:
        return builtinFloat(value) * M
    elif units.find('k') is 0:
        return builtinFloat(value) * k
    elif units.find('m') is 0:
        return builtinFloat(value) * m
    elif units.find('u') is 0:
        return builtinFloat(value) * u
    elif units.find('n') is 0:
        return builtinFloat(value) * n
    elif units.find('p') is 0:
        return builtinFloat(value) * p
    elif units.find('f') is 0:
        return builtinFloat(value) * f
    else:
        return builtinFloat(value)

builtinFloat = float

def float(value):
    """
    If passed a string returns a float of the string value multiplied by
    the repsective multiplier. e.g. '10nF' returns 1e-8, '1e-2pCRUD'
    returns 1e-11, '100mOhms' returns 0.1.

    If passed anything other than a string this function calls the bultin
    float function and returns the result.

    This function can also operate on fractions, e.g. '10nF/1ns'.

    Multipliers:
    T -- 1e12
    G -- 1e9
    M -- 1e6
    k -- 1e3
    m -- 1e-3
    u -- 1e-6
    n -- 1e-9
    p -- 1e-12
    f -- 1e-15

    Arguments:
    value -- either a string with a multiplier or any other value

    Returns:
    A floating point value.

    Examples:
    >>> print(float(10.0)==10.0)
    True
    >>> print(float(10)==10.0)
    True
    >>> print(float('10nF')==1e-08)
    True
    >>> print(float('12nF/2nH')==6.0)
    True
    """

    if not isinstance(value, str):
        return builtinFloat(value)

    match = _pattern.match(value)

    if match is None:
        return None

    if match.group('valueD') is not ('' or None):
        numerator = _float(match.group('valueN'), match.group('unitsN'))
        denominator = _float(match.group('valueD'), match.group('unitsD'))
        return  (numerator / denominator)
    else:
        return _float(match.group('valueN'), match.group('unitsN'))

def floatList2D(data):
    """
    Steps through a 2D list, i.e. [[0,1],[3,5]] and converts each value
    using the units.float function.

    Arguments:
    data -- 2D list

    Returns:
    A numpy array of floating point values.

    Example:
    >>> print(floatList2D([['0.3m',1.2],['3n','5u']]))
    [[  3.00000000e-04   1.20000000e+00]
     [  3.00000000e-09   5.00000000e-06]]
    """

    if not isinstance(data, ndarray):
        fdata = []
        for i in range(0,len(data)):
            fdata.append([])
            for j in range(0,len(data[i])):
                fdata[-1].append(float(data[i][j]))
        return array(fdata)

    return data

def floatList1D(data):
    """
    Steps through a 1D list, i.e. [0,1,3,5] and converts each value
    using the units.float function.

    Arguments:
    data -- 1D list

    Returns:
    A numpy array of floating point values.

    Example:
    >>> print(floatList1D(['0.3m',1.2,'3n','5u']))
    [  3.00000000e-04   1.20000000e+00   3.00000000e-09   5.00000000e-06]
    """

    if not isinstance(data, ndarray):
        fdata = []
        for i in range(0,len(data)):
            fdata.append(float(data[i]))
        return array(fdata)

    return data

if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print('Testing Complete')
