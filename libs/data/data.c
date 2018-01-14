/*
 * Copyright (C) 2006 Cooper Street Innovations Inc.
 *	Charles Eidsness    <charles@cooper-street.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <log.h>

#include "data.h"

int dataInfo(void)
{
	Info("Data Library %i.%i", DATA_MAJOR_VERSION, DATA_MINOR_VERSION);
	Info("Compiled " __DATE__ " at " __TIME__);
	Info("(c) 2006 Cooper Street Innovations Inc.");
	return ((DATA_MAJOR_VERSION << 16) + DATA_MINOR_VERSION);
}
