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
This module contains the eispice test framework.
"""

import unittest
import doctest
import warnings
import eispice

def test_ibis():
    """Extended eispice.Ibis Model Testing
    NOTE: Will only work on my development tree, these models are copyrighted
    and can not be distributed with eispice. CE
    """

    # 29le010.ibs
    ibs = eispice.Ibis("../development/29le010.ibs")
    cct = eispice.Circuit('test_29le010')
    cct.Driver = ibs['17']('vs')
    cct.Tg = eispice.T('vs', 0, 'vo', 0, 75, '10n')
    cct.Receiver = ibs['2']('vo')
    cct.tran('0.1n', '5n')
    eispice.plot(cct)

    # 39wf800a.ibs
    ibs = eispice.Ibis("../development/39wf800a.ibs")
    cct = eispice.Circuit('test_39wf800a')
    cct.Driver = ibs['e2']('vs')
    cct.Tg = eispice.T('vs', 0, 'vo', 0, 100, '10n')
    cct.Receiver = ibs['a1']('vo')
    cct.tran('0.1n', '5n')
    eispice.plot(cct)

    # ad9222bcp.ibs
    ibs = eispice.Ibis("../development/ad9222bcp.ibs")
    cct = eispice.Circuit('test_ad9222bcp')
    cct.Driver = ibs['39']('vs')
    cct.Tg = eispice.T('vs', 0, 'vo', 0, 50, '10n')
    cct.Receiver = ibs['38']('vs')
    cct.tran('0.1n', '200n')
    eispice.plot(cct)

    # ad9289bbc.ibs
    ibs = eispice.Ibis("../development/ad9289bbc.ibs")
    cct = eispice.Circuit('test_ad9289bbc')
    cct.DriverP = ibs['b1']('vsp')
    cct.DriverN = ibs['b1']('vsn', direction=eispice.Falling)
    cct.Rl = eispice.R('vsp',0,50)
    cct.R2 = eispice.R('vsn',0,50)
    cct.tran('0.1n', '10n')
    eispice.plot(cct)

    # ahc1g14.ibs
    ibs = eispice.Ibis("../development/ahc1g14.ibs")
    cct = eispice.Circuit('test_ahc1g14')
    cct.Driver = ibs['4']('vx', modelName='ahc_xgouti_50')
    cct.Rl = eispice.R('vx','vs',33.2)
    cct.Tg = eispice.T('vs', 0, 'vo', 0, 50, '10n')
    cct.Receiver = ibs['2']('vs', modelName='ahc_xg14in_50')
    cct.tran('0.1n', '50n')
    eispice.plot(cct)

    # alvc00m_300.ibs
    ibs = eispice.Ibis("../development/alvc00m_300.ibs")
    cct = eispice.Circuit('test_alvc00m_300')
    cct.Driver = ibs['3']('vx')
    cct.Rl = eispice.R('vx','vs',33.2)
    cct.Tg = eispice.T('vs', 0, 'vo', 0, 50, '10n')
    cct.Receiver = ibs['1']('vs')
    cct.tran('0.1n', '50n')
    eispice.plot(cct)

    # au1000.ibs
    ibs = eispice.Ibis("../development/au1000.ibs")
    cct = eispice.Circuit('test_au1000')
    cct.Driver = ibs['A1']('vx')
    cct.Rl = eispice.R('vx','vs',33.2)
    cct.Tg = eispice.T('vs', 0, 'vo', 0, 50, '10n')
    cct.Receiver = ibs['A10']('vs')
    cct.tran('0.1n', '50n')
    eispice.plot(cct)

def test_bugs():
    """Test bugfixes."""

    cct = eispice.Circuit('Break-Point Stall Bug')
    cct.Vx = eispice.V('vs',0, 0,
            eispice.Pulse(0, 1, '0n','1n','1n','4n','8n'))
    cct.Rt = eispice.R('vs', 'vi', 50)
    cct.Tg = eispice.T('vi', 0, 'vo', 0, 50, '2n')
    cct.Cx = eispice.C('vo',0,'5p')
    cct.tran('0.01n', '18n')
    eispice.plot(cct)

    cct = eispice.Circuit("LC Tline Bug")
    cct.Vs = eispice.V('vs', 0, 0,
            eispice.Pulse(0, 1, '15n', '1n', '1n', '5n', '50n'))
    cct.Rs = eispice.R('vs', 1, 50)
    for i in range(1,100):
        setattr(cct,"L%i"%i,eispice.L(i,i+1,"2n"))
        setattr(cct,"C%i"%i,eispice.C(i+1,0,"1p"))
    cct.Rl = eispice.R(i+1, 0, 50)
    cct.tran('0.1n', '200n')
    eispice.plot_voltage(cct, 1 ,'%i' % (i+1))

    cct = eispice.Circuit("SuperLU Hangup Bug")
    cct.Vx = eispice.V('1', '0', 4,
            eispice.Pulse(4, 8, '10n', '2n', '3n', '5n', '20n'))
    cct.Vy = eispice.V('1', '0', 4,
            eispice.Pulse(8, 4, '10n', '2n', '3n', '5n', '20n'))
    cct.tran('0.5n', '100n')
    eispice.plot(cct)

def test():
    """Runs all of the eispice test scripts."""
    warnings.simplefilter('ignore', UserWarning)
    suite = unittest.TestSuite()
    suite.addTest(doctest.DocTestSuite('calc'))
    suite.addTest(doctest.DocTestSuite('circuit'))
    suite.addTest(doctest.DocTestSuite('device'))
    suite.addTest(doctest.DocTestSuite('ibis'))
    suite.addTest(doctest.DocTestSuite('subckt'))
    suite.addTest(doctest.DocTestSuite('units'))
    suite.addTest(doctest.DocTestSuite('waveform'))
    runner = unittest.TextTestRunner()
    runner.run(suite)

if __name__ == '__main__':

    test()
    #~ test_ibis()
    #~ test_bugs()
