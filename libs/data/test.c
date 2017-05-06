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
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>

#include <log.h>
LogMaster;
#include "data.h"

typedef struct {
	int number;
	double value;
	char *ptr;
} data_;

static int exeFunction(data_ *data, data_ *private)
{
	ReturnErrIf(data == NULL);
	ReturnErrIf(data->ptr == NULL);
	
	Info("--> %s", data->ptr);
	
	return 0;
}

static listFindReturn_ findFunction(data_ *data, data_ *key)
{
	ReturnErrIf(data == NULL);
	ReturnErrIf(key == NULL);
	
	if(data->number == key->number) {
		return LIST_FIND_MATCH;
	}
	
	return LIST_FIND_NOTAMATCH;
}

static listAddReturn_ attachFunction(data_ *data, data_ *new)
{
	ReturnErrIf(data == NULL);
	ReturnErrIf(new == NULL);
	
	if(data->number == new->number) {
		return LIST_ADD_NOWHERE;
	}
	
	if(data->number < new->number) {
		return LIST_ADD_BEFORE;
	}
	
	return LIST_ADD_NOTHERE;
}

static int destroyFunction(data_ *r) 
{
	ReturnErrIf(r == NULL);
	/*Debug("Destroying Data %p", (*r));*/
	free(r);
	return 0;
}

data_ * dataNew(data_ *r, int number, char *ptr, double value)
{
	ReturnNULLIf(r != NULL);
	r = calloc(1, sizeof(data_));
	ReturnNULLIf(r == NULL);
	/*Debug("Creating Data %p", r);*/
	r->number = number;
	r->ptr = ptr;
	r->value = value;
	return r;
}

listSearchReturn_ searchFunction(data_ *data, data_ *next, double *value)
{
	ReturnErrIf(data == NULL);
	ReturnErrIf(next == NULL);
	ReturnErrIf(value == NULL);
	
	if((data->value >= *value) && (next->value < *value)) {
		return LIST_SEARCH_MATCH;
	}
	
	if(data->value > *value) {
		return LIST_SEARCH_NEXT;
	} else if(data->value < *value) {
		return LIST_SEARCH_PREVIOUS;
	}
	
	return LIST_SEARCH_ERR;
}

#define LEN 256
int main(int argc, char *argv[])
{
	int opt;
	struct option longopts[] = { 
			{"hash", 0, NULL, 'H'},
			{"list", 0, NULL, 'L'},
			{"version", 0, NULL, 'v'},
			{"error", 1, NULL, 'e'},
			{"log", 1, NULL, 'l'},
			{0, 0, 0, 0}
    };
	list_ *list = NULL;
	data_ *data[LEN];
	data_ *tmp;
	char *strings[LEN];
	int i;
	dblhash_ *dblhash = NULL;
	unsigned int length;
	listNode_ *node = NULL;
	int hashTest = 0, listTest = 0;
	double searchValue, value;
	
	/* Process the command line options */
	while((opt = getopt_long(argc, argv, "ve:l:HL", longopts, NULL)) != -1) {
		switch(opt) {
		case 'H': hashTest = 1; break;
		case 'L': listTest = 1; break;
		case 'v': dataInfo(); ExitSuccess;
		case 'e': OpenErrorFile(optarg); break;
		case 'l': OpenLogFile(optarg); break;
		case '?': ExitFailure("Unkown option");
        case ':': ExitFailure("Option needs a value");
		default:  ExitFailure("Invalid option");
		}
	}
	
	for(i = 0; i < LEN; i++) {
		data[i] = NULL;
		strings[i] = malloc(16);
		ExitFailureIf(strings[i] == NULL);
		sprintf(strings[i], "%i string", i);
	}
	
	if(listTest) {
		
		list = listNew(list);
		ExitFailureIf(list == NULL);
		
		for(i = 0; i < 8; i++) {
			data[i] = dataNew(data[i], i, strings[i], (double)i);
			ExitFailureIf(data[i] == NULL);
			ExitFailureIf(listAdd(list, data[i], (listAdd_)attachFunction));
		}
		
		ExitFailureIf(listFind(list, data[4], (listFind_)findFunction, 
				(void*)(&tmp)));
		ExitFailureIf(tmp == NULL);
		Info("find: four = %s", tmp->ptr);
		
		ExitFailureIf(listExecute(list, (listExecute_)exeFunction, NULL));
		
		Info("length: 8 = %i", listLength(list));
		
		searchValue = 5.0;
		ExitFailureIf(listSearch(list, &searchValue, 
				(listSearch_)searchFunction, &node, (void*)(&tmp)));
		ExitFailureIf(tmp == NULL);
		Info("search: five = %s", tmp->ptr);
		
		searchValue = 2.7;
		ExitFailureIf(listSearch(list, &searchValue, 
				(listSearch_)searchFunction, &node, (void*)(&tmp)));
		ExitFailureIf(tmp == NULL);
		Info("search: three = %s", tmp->ptr);
		
		searchValue = 6.9;
		ExitFailureIf(listSearch(list, &searchValue, 
				(listSearch_)searchFunction, &node, (void*)(&tmp)));
		ExitFailureIf(tmp == NULL);
		Info("search: seven = %s", tmp->ptr);
		
		ExitFailureIf(listDestroy(&list, (listDestroy_)destroyFunction));
	}
	
	if(hashTest) {
		
		dblhash = dblhashNew(dblhash, 6);
		ExitFailureIf(dblhash == NULL);
		
		for(i = 0; i < LEN; i++) {
			ExitFailureIf(dblhashAdd(dblhash, strings[i], 'y', i/2));
		}
		
		ExitFailureIf(dblhashLength(dblhash, &length));
		Info("Hash Length %i", length);
		
		ExitFailureIf(dblhashRemove(dblhash, strings[24]));
		ExitFailureIf(dblhashLength(dblhash, &length));
		Info("Hash Length %i", length);
		
		ExitFailureIf(dblhashFind(dblhash, strings[145], &value));
		Info("Retreved %s %e", strings[145], value);
		
		ExitFailureIf(dblhashDestroy(&dblhash));
	
	}
	
	CloseErrorFile;
	CloseLogFile;
	ExitSuccess;
}
