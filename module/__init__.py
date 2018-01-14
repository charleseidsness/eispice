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
eispice is a clone of the Berkley Spice 3 Simulation Engine. It was
originally targeted toward PCB level Signal Integrity Simulation; simulating
IBIS model defined devices, transmission lines, and passive termination but
the scope of the tool has been slowly expanding to include more general
purpose circuit simulation features.

For more information refer to the eispice webpage:
<www.thedigitalmachine.net/eispice>

Please report all bugs to:
<charles@thedigitalmachine.net>

Classes:
Circuit -- an eispice circuit

Functions:
about -- prints info on eispice and it's libraries
logFile -- records the eispice log to the specified file instead of stdout
errFile -- records the eispice error log to the specified file instead of stderr

Modules:
device -- basic device library
ibis -- support for ibis defined models
plot -- simple, default plot utility
subckt -- provides sub-circuit support
waveform -- provides wavefroms that can be used in sources
test -- the eispice test framework

"""

from simulator_ import about, logFile, errorFile

from circuit import *
from device import *
from ibis import *
from plot import *
from subckt import *
from waveform import *
from test import *
import calc
