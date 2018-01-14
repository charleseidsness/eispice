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
This module provides classes and functions that can be used to calculate
the value and derivative of an equation, using standard Python notation.
It is used by the PyB Device.

Classes:
Variable -- Defines a variable used in an equation.
Result -- Result of an equation based on Variables

Functions:
These functions can be used in an equation with Variables (in addition to
+,-,*,/,**): acos(x), cos(x), asin(x), sin(x), atan(x), tan(x), exp(x),
ln(x) , log(x), sqrt(x).

"""

import math

def _ln(x):
	"""Returns the natural logrithum of |x|, but if x is 0 returns 0"""
	if x is not 0.0:
		return math.log(abs(x))
	else:
		return 0.0

def _divide(x, y, divmin = 1e-12):
	"""Returns x/y, but if y < divmin returns x/divmin"""
	if y < divmin:
		return x / divmin
	else:
		return x / y

def _sign(x):
	"""Returns the 0 ix x is 0, -1 i x is <0 and +1 if x >0"""
	if x == 0:
		return 0
	else:
		return x / abs(x)

# --------------------------------------------------------------------------- #
#                             Variable Class                                  #
# --------------------------------------------------------------------------- #

class Variable:
	"""
	A variable, which when used in an equation returns the value of
	the equation and the value of the derivative of the equation wrt the
	variable.

	Example:

	>>> from eispice import calc

	Define some variables:
	>>> x = calc.Variable(2)
	>>> y = calc.Variable(3)

	Calculate z and it's derivatives
	>>> z = x**x + cos(20*y + x) - (x / (y+50)) # z is of type _Result

	Results:
	>>> print round(z,6) # z's value
	-21.826493
	>>> print round(z[x],6) # dz/dx
	20.761769
	>>> print round(z[y],6) # dz/dx
	14.283614

	Change the value of x:
	>>> x(10)

	Re-calculate z and it's derivatives:
	>>> z = x**x + cos(20*y + x) - (x / (y+50))

	Results:
	>>> print round(z,6) # z's value
	9999999995.33
	>>> print round(z[x],6) # dz/dx
	33025850929.7
	>>> print round(z[y],6) # dz/dx
	-15.577814
	"""

	def __init__(self, value=0.0):
		"""Creates the varaible and sets its value."""
		self.value = float(value)
		self.deriv = {}
		self.deriv[hash(id(self))] = 1.0

	def __call__(self, value=0.0):
		"""Changes the value of the Variable."""
		self.value = float(value)
		self.deriv = {}
		self[self] = 1.0

	def __getitem__(self, k):
		"""Returns the derivative wrt the key variable."""
		k = hash(k)
		return self.deriv.get(k, 0.0)

	def __contains__(self, elt):
		"""Returns true if the derivative wrt the key variable exists"""
		elt = hash(elt)
		return elt in self.deriv

	def __setitem__(self, k, value):
		"""Sets the derivative wrt the key variable, not adviable to use."""
		k = hash(k)
		self.deriv[k] = value

	def __iter__(self):
		"""Returns an iterator of derivative keys."""
		return self.deriv.iterkeys()

	def __add__(self, other):
		return _Result(lambda x, y: x + y, lambda x, y, dx, dy: dx + dy,
				self, other)

	def __radd__(self, other):
		return self.__add__(other)

	def __sub__(self, other):
		return _Result(lambda x, y: x - y, lambda x, y, dx, dy: dx - dy,
				self, other)

	def __rsub__(self, other):
		return self.__sub__(other)

	def __mul__(self, other):
		return _Result(lambda x, y: x * y, lambda x, y, dx, dy: dx*y + dy*x,
				self, other)

	def __rmul__(self, other):
		return self.__mul__(other)

	def __divide__(self, other):
		return _Result(lambda x, y: _divide(x,y),
				lambda x, y, dx, dy: (dx*y - dy*x)/(y*y),
				self, other)

	def __rdiv__(self, other):
		return self.__divide__(other)

	def __pow__(self, other, modulo=None):
		if modulo is not None:
			return NotImplemented
		return _Result(lambda x, y: x**y,
				lambda x, y, dx, dy: (x**y)*(dx*_divide(y,x) + dy*_ln(x)),
				self, other)

	def __rpow__(self, other):
		return self.__pow__(other)

	def __neg__(self):
		return _Result(lambda x: -x, lambda x, dx: -dx, self)

	def __pos__(self):
		return _Result(lambda x: x, lambda x, dx: dx, self)

	def __abs__(self):
		return _Result(lambda x: abs(x), lambda x, dx: _sign(x) * dx, self)

	def __int__(self):
		"""Returns an integer which represents the value of the variable."""
		return int(self.value)

	def __long__(self):
		"""Returns an long which represents the value of the variable."""
		return long(self.value)

	def __float__(self):
		"""Returns an float which represents the value of the variable."""
		return self.value

	def __str__(self):
		"""Returns an string which represents the value of the variable."""
		return str(self.value)

	def __repr__(self):
		return "id(%s)" % id(self)

	def __hash__(self):
		return hash(id(self))

	def __lt__(a, b):
		return float(a) < float(b)

	def __le__(a, b):
		return float(a) <= float(b)

	def __eq__(a, b):
		return float(a) == float(b)

	def __ne__(a, b):
		return float(a) != float(b)

	def __ge__(a, b):
		return float(a) >= float(b)

	def __gt__(a, b):
		return float(a) > float(b)

	def acos(x):
		return _Result(lambda x: math.acos(x),
				lambda x, dx: _divide(-1.0 , math.sqrt(1.0-x*x)) * dx, x)

	def cos(x):
		return _Result(lambda x: math.cos(x),
				lambda x, dx: -1.0*math.sin(x) * dx, x)

	def asin(x):
		return _Result(lambda x: math.asin(x),
				lambda x, dx: _divide(1.0, math.sqrt(1.0-x*x)) * dx, x)

	def sin(x):
		return _Result(lambda x: math.sin(x),
				lambda x, dx: math.cos(x) * dx, x)

	def atan(x):
		return _Result(lambda x: math.atan(x),
				lambda x, dx: _divide(1.0, math.sqrt(1.0+x*x)) * dx, x)

	def tan(x):
		return _Result(lambda x: math.tan(x),
				lambda x, dx: (_divide(1.0, math.cos(x))**2) * dx, x)

	def exp(x):
		return _Result(lambda x: math.exp(x),
				lambda x, dx: math.exp(x) * dx, x)

	def ln(x):
		return _Result(lambda x:_ln(x),
				lambda x, dx: _divide(1.0, x) * dx)

	def log(x):
		return _Result(lambda x: math.log10(x),
				lambda x, dx: _divide(1.0, x*math.log(10.0)) * dx)

	def sqrt(x):
		return _Result(lambda x: math.sqrt(x),
				lambda x, dx: _divide(1.0, 2.0*math.sqrt(x)) * dx)

class _Result(Variable):
	"""The reslult of an equation containing Variables."""

	def __init__(self, f, d, x, y=None):
		"""
		Arguments:
		f -- A function used to solve for the value
		d -- A function used to solve for the derrivative
		x -- The value of the input (either a constant or a Variable)
		y -- Optional second value for fucntions that involve two
		variables / constants, i.e. cos(x) vs. (x + y)
		"""
		Variable.__init__(self)

		if y is None:
			self.value = f(float(x))
			if hasattr(x, 'deriv'):
				for (key, value) in x.deriv.iteritems():
					self.deriv[key] = d(x.value, value)

		else:
			self.value = f(float(x), float(y))
			if hasattr(x, 'deriv') and hasattr(y, 'deriv'):
				for (key, value) in x.deriv.iteritems():
					self.deriv[key] = d(x.value, y.value,
							value, y.deriv.get(key, 0.0))
				for (key, value) in y.deriv.iteritems():
					self.deriv[key] = d(x.value, y.value,
							x.deriv.get(key, 0.0), value)
			elif hasattr(x, 'deriv'):
				for (key, value) in x.deriv.iteritems():
					self.deriv[key] = d(x.value, float(y), value, 0.0)
			elif hasattr(y, 'deriv'):
				for (key, value) in y.deriv.iteritems():
					self.deriv[key] = d(float(x), y.value, 0.0, value)

# --------------------------------------------------------------------------- #
#                           Standard Functions                                #
# --------------------------------------------------------------------------- #

def acos(x):
	"""Returns the arc-cosine of a Variable or value."""
	try:
		return x.acos()
	except AttributeError:
		return math.acos(x)

def cos(x):
	"""Returns the cosine of a Variable or value."""
	try:
		return x.cos()
	except AttributeError:
		return math.cos(x)

def asin(x):
	"""Returns the arc-sine of a Variable or value."""
	try:
		return x.asin()
	except AttributeError:
		return math.asin(x)

def sin(x):
	"""Returns the sine of a Variable or value."""
	try:
		return x.sin()
	except AttributeError:
		return math.sin(x)

def atan(x):
	"""Returns the arc-tangent of a Variable or value."""
	try:
		return x.atan()
	except AttributeError:
		return math.atan(x)

def tan(x):
	"""Returns the tangent of a Variable or value."""
	try:
		return x.tan()
	except AttributeError:
		return math.tan(x)

def exp(x):
	"""Returns the exponent of a Variable or value."""
	try:
		return x.exp()
	except AttributeError:
		return math.exp(x)

def ln(x):
	"""Returns the natural logrithum of a Variable or value."""
	try:
		return x.ln()
	except AttributeError:
		return _ln(x)

def log(x):
	"""Returns the logrithum base 10 of a Variable or value."""
	try:
		return x.log()
	except AttributeError:
		return math.log10(x)

def sqrt(x):
	"""Returns the square-root of a Variable or value."""
	try:
		return x.sqrt()
	except AttributeError:
		return math.sqrt(x)

# --------------------------------------------------------------------------- #
#                                  Test                                       #
# --------------------------------------------------------------------------- #

if __name__ == '__main__':

	import doctest
	doctest.testmod(verbose=False)
	print 'Testing Complete'
