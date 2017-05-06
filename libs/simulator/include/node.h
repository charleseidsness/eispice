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

#ifndef NODE_H
#define NODE_H

typedef struct _node node_;

typedef struct {
	int row;
	int col;
} nodeIndex_;

extern node_ gndNode;

listFindReturn_ nodeCompare(node_ *r, nodeIndex_ *index);
listAddReturn_ nodeAdd(node_ *r, node_ *newNode);

int nodeGetCol(node_ *r);
int nodeGetRow(node_ *r);

int nodeDataPlus(node_ *r, double plus);
int nodeDataSet(node_ *r, double value);
int nodeDataClear(node_ *r);
int nodeSetDataPtr(node_ *r, double *data);

int nodeDestroy(node_ *r);
node_ * nodeNew(nodeIndex_ index);

#endif
