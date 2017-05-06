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

#include <data.h>
#include <log.h>
#include "node.h"

struct _node {
	int row;
	int col;
	double *data;
};

/*===========================================================================*/

static double gndNodeData = 0.0;
node_ gndNode = {
	.row = 0,
	.col = 0,
	.data = &gndNodeData
};

/*===========================================================================*/

listFindReturn_ nodeCompare(node_ *r, nodeIndex_ *index)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(index == NULL);
	
	if((r->col == index->col) && (r->row == index->row)) {
		return LIST_FIND_MATCH;
	}
	return LIST_FIND_NOTAMATCH;
}

/*---------------------------------------------------------------------------*/

listAddReturn_ nodeAdd(node_ *r, node_ *newNode)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(newNode == NULL);
	
	if((r->col > newNode->col) || 
			((r->col == newNode->col) && (r->row > newNode->row))) {
		return LIST_ADD_BEFORE;
	}
	return LIST_ADD_NOTHERE;
}

/*---------------------------------------------------------------------------*/

int nodeGetCol(node_ *r)
{
	ReturnErrIf(r == NULL);
	return r->col;
}

/*---------------------------------------------------------------------------*/

int nodeGetRow(node_ *r)
{
	ReturnErrIf(r == NULL);
	return r->row;
}

/*---------------------------------------------------------------------------*/

int nodeDataPlus(node_ *r, double plus)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->data == NULL);
	if(r == &gndNode)
		return 0;
	*r->data += plus;
	return 0;
}

/*---------------------------------------------------------------------------*/

int nodeDataSet(node_ *r, double value)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->data == NULL);
	if(r == &gndNode)
		return 0;
	*r->data = value;
	return 0;
}

/*---------------------------------------------------------------------------*/

int nodeDataClear(node_ *r)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(r->data == NULL);
	if(r == &gndNode)
		return 0;
	*r->data = 0.0;
	return 0;
}

/*---------------------------------------------------------------------------*/

int nodeSetDataPtr(node_ *r, double *data)
{
	ReturnErrIf(r == NULL);
	ReturnErrIf(data == NULL);
	if(r == &gndNode)
		return 1;
	r->data = data;
	return 0;
}

/*===========================================================================*/

int nodeDestroy(node_ *r)
{
	ReturnErrIf(r == NULL);
	
	if(r != &gndNode) {
		
		Debug("Destroying Node %i:%i %p", r->row, r->col, r);
		
		free(r);
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/

node_ * nodeNew(nodeIndex_ index)
{
	node_ *r;
	
	ReturnNULLIf(index.row < 0);
	ReturnNULLIf(index.col < 0);
	
	r = calloc(1, sizeof(node_));
	ReturnNULLIf(r == NULL);
	
	Debug("Creating Node %i:%i %p", index.row, index.col, r);
	
	r->row = index.row;
	r->col = index.col;
	
	return r;
}

/*===========================================================================*/

