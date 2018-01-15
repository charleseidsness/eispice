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
This module provides a very basic Tk based plotting tool. It is intended
to be a basic default plotter for eispice, if nothing else is avalible but
I recomend using something more advanced, like matplotlib or the plotter
built into eide if it's ever finished.

Classes:
Plot -- A simple Tk based plotter Widget

Functions:
plot -- Creates a new plot based on an eispice circuit's output.
plot_voltage -- Plot voltages at specific nodes
plot_current -- Plot currents through specific devices
"""

import tkinter as tk
import tkinter.font as tkFont
import numpy

from datetime import datetime

# --------------------------------------------------------------------------- #
#                                    Plotter                                  #
#                     (A very simple Tk based plotting tool)                  #
# --------------------------------------------------------------------------- #

def callback_hide(event):
    item = event.widget.find_withtag('current')
    tag = event.widget.gettags(item)[0]
    if event.widget.itemconfig(item, 'fill')[4] == 'grey':
        event.widget.itemconfig(tag, state=tk.NORMAL)
        event.widget.itemconfig(item, fill='black')
    else:
        event.widget.itemconfig(tag, state=tk.HIDDEN)
        event.widget.itemconfig(item, state=tk.NORMAL)
        event.widget.itemconfig(item, fill='grey')

def callback_marker(event):
    if ((event.x > event.widget.lMargin) and
            (event.x < (event.widget.width - event.widget.rMargin)) and
        (event.y > event.widget.tMargin) and
            (event.y < (event.widget.height - event.widget.bMargin))):

        if event.widget.markerIndex == 2:
            event.widget.delete('m0')
            event.widget.delete('m1')
            event.widget.delete('marker')
            event.widget.markerIndex = 0
            return

        marker = 'm' + str(event.widget.markerIndex)
        fill = event.widget.markerColour[event.widget.markerIndex]

        event.widget.delete(marker)
        event.widget.delete('marker')

        event.widget.create_line(event.widget.lMargin, event.y,
                event.widget.width - event.widget.rMargin, event.y,
                width=1, tags=marker, fill=fill)
        event.widget.create_line(event.x, event.widget.tMargin,
                event.x, event.widget.height - event.widget.bMargin,
                width=1, tags=marker, fill=fill)

        event.widget.create_text(event.widget.width-2,
                (2+(event.widget.axisFont.cget('size')+5)*
                    (event.widget.markerIndex+1)),
                text='(%.4g, %.4g)' % event.widget.pixel2axis((event.x, event.y)),
                font=event.widget.axisFont,
                anchor=tk.NE, tags=marker, fill=fill)

        event.widget.marker[event.widget.markerIndex] = event.widget.pixel2axis((event.x, event.y))

        delta = (event.widget.marker[1][0] - event.widget.marker[0][0],
            event.widget.marker[1][1] - event.widget.marker[0][1])

        event.widget.create_text(event.widget.width-2,2,
                text='D(%.4g, %.4g)' % delta,
                font=event.widget.axisFont,
                anchor=tk.NE, tags='marker')

        event.widget.markerIndex = (event.widget.markerIndex+1)%3

class Plot(tk.Canvas):
    """
    A very simple Tk based plotter. This class is specifically
    designed for use by the plot function.

    Usage Example:

    root = tk.Tk()

    plot = Plot(root, "Test Plot")

    plot.title = 'Title'
    plot.subTitle = 'Sub Title'
    plot.xAxis = 'X-Axis'
    plot.yAxis = 'Y-Axis'

    plot.data.append([(1,1),(2,2),(3,3)])
    plot.legend.append('Test 1')

    plot.data.append([(1,5),(3,12),(3.2,7)])
    plot.legend.append('Test 2')

    plot.plot()

    root.mainloop()
    """

    def __init__(self, master=None, title='', subTitle='', xAxis = '',
            yAxis = '', bg = 'white', width = 550, height = 300,
            lMargin = 75, tMargin = 55, rMargin = 25, bMargin = 55,
            lineWidth = 2, xTicks = 5, yTicks = 10,
            legend = None, data = None):
        """
        Arguments:
        master -- Tk root
        title -- plot title
        """

        tk.Canvas.__init__(self, master, width=width, height=height, bg = bg)

        self.bind("<Button-1>", callback_marker)

        master.title(title)
        self.master = master

        self.title=title
        self.subTitle=subTitle
        self.xAxis = xAxis
        self.yAxis = yAxis

        self.bg = bg

        self.width = width
        self.height = height
        self.lMargin = lMargin
        self.tMargin = tMargin
        self.rMargin = rMargin
        self.bMargin = bMargin

        self.lineWidth = lineWidth

        self.xTicks = xTicks
        self.yTicks = yTicks

        self.colours = ['red', 'navy', 'green', 'orange', 'purple',
                'cyan', 'magenta',  'yellow', 'darkred']

        if legend == None:
            self.legend = []
        else:
            self.legend = legend

        if data == None:
            self.data = []
        else:
            self.data = data

        self.xRange = [9e999999999999999999, -9e999999999999999999]
        self.yRange = [9e999999999999999999, -9e999999999999999999]

        self.markerIndex = 0
        self.marker = [(0,0),(0,0)]
        self.markerColour = ['red', 'blue']

        self.titleFont = tkFont.Font(family='Helvetica', size=12, weight='bold')
        self.axisFont = tkFont.Font(family='Helvetica', size=9)


    def _drawLabels(self):

        self.create_text(self.width/2, self.tMargin/2.5,
                text=self.title, anchor=tk.CENTER, font=self.titleFont)
        self.create_text(self.width/2,
                (self.tMargin/2.5+2+self.titleFont.cget('size')),
                text=self.subTitle, anchor=tk.CENTER, font=self.axisFont)

        self.create_text(self.lMargin,
                self.tMargin - 2 -self.titleFont.cget('size'),
                text=self.yAxis, anchor=tk.CENTER, font=self.axisFont)
        self.create_text(self.width/2, self.height - self.bMargin/2.5,
                text=self.xAxis, anchor=tk.CENTER, font=self.axisFont)

    def _drawGrid(self):

        rEdge = self.width - self.rMargin
        bEdge = self.height - self.bMargin
        width = self.width - self.lMargin - self.rMargin
        height = self.height - self.bMargin - self.tMargin

        self.create_line(self.lMargin, bEdge, rEdge, bEdge, width=2)
        self.create_line(self.lMargin, bEdge, self.lMargin,
                self.tMargin, width=2)

        # Create the x-axis
        for i in range(self.xTicks + 1):
            x = self.lMargin + i * width / self.xTicks
            self.create_line(x, bEdge, x, self.tMargin, width=1,
                    stipple='gray50')
            value = (i*(self.xRange[1] - self.xRange[0]) /
                    self.xTicks + self.xRange[0])
            self.create_text(x, bEdge + self.axisFont.cget('size')/1.5,
                    text='%.4g'% value, anchor=tk.N, font=self.axisFont)

        # Create the y-axis
        for i in range(self.yTicks + 1):
            y = bEdge - i * height / self.yTicks
            self.create_line(self.lMargin, y, rEdge, y, width=1,
                    stipple='gray50')
            value = (i*(self.yRange[1] - self.yRange[0]) /
                    self.yTicks + self.yRange[0])
            self.create_text(self.lMargin -
                    self.axisFont.cget('size')/1.5, y,
                    text='%.4g'% value, anchor=tk.E, font=self.axisFont)

    def _drawPlots(self):

        scaled = []
        for i in range(len(self.data)):
            colour = self.colours[i%len(self.colours)]
            scaled = []
            tag = 'tag' + str(i)
            tagTxt = 'txt' + str(i)
            for data in self.data[i]:
                scaled.append(self.axis2pixel(data))

            self.create_line(scaled, fill=colour, smooth=1,
                    width=self.lineWidth, tags=tag)

            if len(self.legend) > i:
                yLocation = self.tMargin + i*(self.axisFont.cget('size') + 5)
                self.create_line([((self.width - self.rMargin) + 10, yLocation),
                        ((self.width - self.rMargin) + 20, yLocation)],
                        fill=colour, width=self.lineWidth, tags=tag)
                self.create_text((self.width - self.rMargin) + 25, yLocation,
                        text=self.legend[i], anchor=tk.W, font=self.axisFont,
                        activefill='grey', tags=(tag,tagTxt))
                self.tag_bind(tagTxt, '<Button-1>', callback_hide)

    def _autoAxis(self):

        scaled = []
        for data in self.data:
            for x,y in data:
                self.xRange[0] = min(self.xRange[0], x)
                self.xRange[1] = max(self.xRange[1], x)
                self.yRange[0] = min(self.yRange[0], y)
                self.yRange[1] = max(self.yRange[1], y)

        # add some extra space and rounding to get a nice grid
        # this is kind of convoluted, maybe someone else can think
        # up someting better and submit it, I'd be happy to change this
        if self.yRange[1]-self.yRange[0] != 0.0:
            decimals = numpy.log10(abs(self.yRange[1]-self.yRange[0]))
        else:
            decimals = 1
        if decimals < 0:
            decimals = int(numpy.floor(decimals)-1)
        else:
            decimals = int(numpy.ceil(decimals)+1)
        space = 0.1*abs(self.yRange[1]-self.yRange[0])
        self.yRange[0] = self.yRange[0] - space
        self.yRange[1] = self.yRange[1] + space
        self.yRange[0] = numpy.round(self.yRange[0],decimals=abs(decimals))
        self.yRange[1] = numpy.round(self.yRange[1],decimals=abs(decimals))

        decimals = numpy.log10(abs(self.xRange[1]-self.xRange[0]))
        if decimals < 0:
            decimals = int(numpy.floor(decimals))-1
        else:
            decimals = int(numpy.ceil(decimals))+1
        self.xRange[0] = numpy.round(self.xRange[0],decimals=abs(decimals))
        self.xRange[1] = numpy.round(self.xRange[1],decimals=abs(decimals))

        # make room for the legend
        maxText = 0
        for legend in self.legend:
            maxText = max(maxText, len(legend))

        self.rMargin = self.rMargin + maxText*self.axisFont.cget('size') + 25

    def pixel2axis(self, pixel):

        rEdge = self.width - self.rMargin
        bEdge = self.height - self.bMargin

        width = self.width - self.lMargin - self.rMargin
        height = self.height - self.bMargin - self.tMargin

        xScale = width/float((self.xRange[1] - self.xRange[0]))
        yScale = height/float((self.yRange[1] - self.yRange[0]))

        return ((pixel[0] - self.lMargin)/xScale + self.xRange[0],
                (pixel[1] - bEdge)/-yScale + self.yRange[0])

    def axis2pixel(self, axis):

        rEdge = self.width - self.rMargin
        bEdge = self.height - self.bMargin

        width = self.width - self.lMargin - self.rMargin
        height = self.height - self.bMargin - self.tMargin

        if (self.xRange[1] - self.xRange[0]) != 0:
            xScale = width/float((self.xRange[1] - self.xRange[0]))
        else:
            xScale = 1
        if (self.yRange[1] - self.yRange[0]) != 0:
            yScale = height/float((self.yRange[1] - self.yRange[0]))
        else:
            yScale = 1

        return (self.lMargin + (axis[0] - self.xRange[0]) * xScale,
                bEdge - (axis[1] - self.yRange[0]) * yScale)

    def plot(self):

        if len(self.data) > 0:
            self._autoAxis()
            self._drawLabels()
            self._drawGrid()
            self._drawPlots()
            self.pack()


# --------------------------------------------------------------------------- #
#                      Functions for Plotting Circuits
# --------------------------------------------------------------------------- #

def plot(*lst):
    """
    eispice Basic Plotter

    This function will plot the voltage and current results from an
    eispice simulation or simulations.

    Arguments:
    -- any number of eispice Circuits

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Plotter Test")
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct.Vx = eispice.V(1, eispice.GND, 4, wave)
    >>> cct.Cx = eispice.C(1, eispice.GND, '10n')
    >>> cct.tran('0.5n', '100n')
    >>> eispice.plot(cct)
    """

    root = tk.Tk()

    for circuit in lst:

        if circuit.results == None:
            raise RuntimeError("Circuit %s has no results." % circuit.title)

        try:
            vPlot.title += " ," + circuit.title
        except NameError:
            vPlot = Plot(root, circuit.title, subTitle=datetime.now(),
                    xAxis="Time (s)", yAxis="Voltage (V)")

        for n in range(0,(circuit.results.shape[1])):
            if ((circuit.variables[n][0] == 'v') and
                    (circuit.variables[n].find('#') == -1) and
                    (circuit.variables[n].find('@') == -1)):
                data = numpy.take(circuit.results, [0, n], 1)
                vPlot.data.append(data)
                vPlot.legend.append(circuit.variables[n])

        try:
            iPlot.title += "," + circuit.title
        except NameError:
            iPlot = Plot(root, circuit.title, subTitle=datetime.now(),
                    xAxis="Time (s)", yAxis="Current (A)")

        for n in range(0,(circuit.results.shape[1])):
            if ((circuit.variables[n][0] == 'i') and
                    (circuit.variables[n].find('#') == -1)):
                data = numpy.take(circuit.results, [0, n], 1)
                iPlot.data.append(data)
                iPlot.legend.append(circuit.variables[n])

    vPlot.plot()
    iPlot.plot()
    root.mainloop()

def plot_voltage(cct, *nodename):
    """
    eispice Voltage Plotter

    This function will plot the voltage at a node or at a list of nodes
    of an eispice simulation.

    Arguments:
    cct -- an eispice circuit
    nodename -- name of the node to plot as a string or list of nodes
    to plot

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Voltage Plotter Test")
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct.Vx = eispice.V(1, eispice.GND, 4, wave)
    >>> cct.Cx = eispice.C(1, eispice.GND, '10n')
    >>> cct.tran('0.5n', '100n')
    >>> eispice.plot_voltage(cct, 1)
    """
    root = tk.Tk()
    plot = Plot(root, cct.title, subTitle=datetime.now(),
            xAxis="Time (s)", yAxis="Voltage (V)")

    if isinstance(nodename, str):
        plot.data.append(cct.voltage_array(nodename))
    else:
        for node in nodename:
            plot.data.append(cct.voltage_array(node))
            plot.legend.append('v('+str(node)+')')

    plot.plot()
    root.mainloop()

def plot_current(cct, *devicename):
    """
    eispice Current Plotter

    This function will plot the current through a device or list of
    devices of an eispice simulation.

    Arguments:
    cct -- an eispice circuit
    devicename -- name of the device to plot as a string or list of
    devices to plot

    Example:
    >>> import eispice
    >>> cct = eispice.Circuit("Current Plotter Test")
    >>> wave = eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n')
    >>> cct.Vx = eispice.V(1, eispice.GND, 4, wave)
    >>> cct.Cx = eispice.C(1, eispice.GND, '10n')
    >>> cct.tran('0.5n', '100n')
    >>> eispice.plot_current(cct, 'Vx')
    """
    root = tk.Tk()
    plot = Plot(root, cct.title, subTitle=datetime.now(),
            xAxis="Time (s)", yAxis="Current (A)")

    if isinstance(devicename, str):
        plot.data.append(cct.current_array(device))
    else:
        for device in devicename:
            plot.data.append(cct.current_array(device))
            plot.legend.append('i('+str(device)+')')

    plot.plot()
    root.mainloop()

if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)
    print('Testing Complete')
