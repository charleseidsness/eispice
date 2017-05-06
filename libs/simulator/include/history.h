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

#ifndef HISTORY_H
#define HISTORY_H

#include <data.h>

typedef struct _history history_;

listSearchReturn_ historySearch(history_ *r, history_ *next, double *time);
listSearchReturn_ historySearchBreak(history_ *r, history_ *next, 
		double *time);

int historyRecall(history_ *r, double *data, int length);
int historyGetTime(history_ *r, double *time);
int historyGetData(history_ *r, double *data, int index);
int historyGetAllData(history_ *r, double *data, int length);
int historyGetLength(history_ *r);

int historyDestroy(history_ *r);

#define HISTORY_FLAG_BRKPOINT	(1<<0)
#define HISTORY_FLAG_END		(1<<1)
history_ * historyNew(double time, double *data,int length, 
		unsigned int flag);

#endif
