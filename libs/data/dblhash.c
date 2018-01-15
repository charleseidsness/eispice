/* Copyright 2006 David Crawshaw, released under the new BSD license.
 * Version 2, from http://www.zentus.com/c/hash.html */

/* Modified to better fit into the data library by Charles Eidsness */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <log.h>

#include "data.h"

/* Table is sized by primes to minimise clustering.
   See: http://planetmath.org/encyclopedia/GoodHashTablePrimes.html */
static unsigned int sizes[] = {
    13, 37, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457, 1610612741
};
static int sizes_count = sizeof(sizes) / sizeof(sizes[0]);
static float load_factor = 0.65;

typedef struct {
    unsigned int hash;
	char freeMem;
    char *key;
    double value;
} record_;

struct _dblhash {
    record_ *records;
    unsigned int records_count;
    unsigned int size_index;
};

static int dblhashGrow(dblhash_ *h)
{
	int i;
    record_ *old_recs, *new_recs;
    unsigned int old_recs_length;

	Debug("Growing Hash %p", h);

	ReturnErrIf(h->size_index == (sizes_count - 1));

    old_recs_length = sizes[h->size_index];
    old_recs = h->records;

	new_recs = calloc(sizes[++h->size_index], sizeof(record_));
	ReturnErrIf(new_recs == NULL);
	h->records = new_recs;

    h->records_count = 0;

    /* rehash table */
    for (i=0; i < old_recs_length; i++) {
        if (old_recs[i].hash && old_recs[i].key) {
            ReturnErrIf(dblhashAdd(h, old_recs[i].key, old_recs[i].freeMem,
					old_recs[i].value));
		}
	}

    free(old_recs);

    return 0;
}

/* algorithm djb2 */
static unsigned int strhash(char *str)
{
    int c = 1;
    int hash = 5381;
	while (c) {
		c = *str++;
        hash = hash * 33 + c;
	}
    return hash == 0 ? 1 : hash;
}

/*===========================================================================*/

int dblhashFindPtr(dblhash_ *h, char *key, double **value)
{
    record_ *recs;
    unsigned int off, ind, size;
    unsigned int code;

	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);
	ReturnErrIf(value == NULL);

	code = strhash(key);

    recs = h->records;
    size = sizes[h->size_index];
    ind = code % size;
    off = 0;

    /* search on hash which remains even if a record has been removed,
     * so hash_remove() does not need to move any collision records
	 */
    while (recs[ind].hash) {
        if ((code == recs[ind].hash) && (recs[ind].key != NULL)) {
			if(!strcmp(key, recs[ind].key)) {
				*value = &recs[ind].value;
				return 0;
			}
		}
        ind = (code + (int)pow(++off,2)) % size;
    }

    	/* Couldn't find the key */
	*value = NULL;
    return 0;
}

int dblhashFind(dblhash_ *h, char *key, double *value)
{
    double *valuePtr;
	ReturnErrIf(dblhashFindPtr(h, key, &valuePtr));
	if(valuePtr != NULL) {
		*value = *valuePtr;
	} else {
		*value = HUGE_VAL;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

int dblhashAddPtr(dblhash_ *h, char *key, char freeMem, double value,
		double **valuePtr)
{
    record_ *recs;
    unsigned int off, ind, size, code;

	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);
	ReturnErrIf(*key == '\0');
	ReturnErrIf(valuePtr == NULL);

    if (h->records_count > (sizes[h->size_index] * load_factor)) {
        ReturnErrIf(dblhashGrow(h));
    }

    code = strhash(key);
    recs = h->records;
    size = sizes[h->size_index];

    ind = code % size;
    off = 0;

    while (recs[ind].key) {
        ind = (code + (int)pow(++off,2)) % size;
	}

    recs[ind].hash = code;
    recs[ind].key = key;
	recs[ind].freeMem = freeMem;
    recs[ind].value = value;
	*valuePtr = &recs[ind].value;

    h->records_count++;

    return 0;
}

int dblhashAdd(dblhash_ *h, char *key, char freeMem, double value)
{
    double *valuePtr;
	ReturnErrIf(dblhashAddPtr(h, key, freeMem, value, &valuePtr));
	return 0;
}

/*---------------------------------------------------------------------------*/

int dblhashRemove(dblhash_ *h, char *key)
{
    unsigned int code;
    record_ *recs;
    unsigned int off, ind, size;

	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);

	code = strhash(key);

    recs = h->records;
    size = sizes[h->size_index];
    ind = code % size;
    off = 0;

    while (recs[ind].hash) {
        if ((code == recs[ind].hash) && (recs[ind].key != NULL)) {
			if(!strcmp(key, recs[ind].key)) {
				/* Do not erase hash, so probes for collisions succeed */
				if((recs[ind].freeMem != 'n') && (recs[ind].key != NULL)) {
					free(recs[ind].key);
				}
				recs[ind].key = NULL;
				recs[ind].value = HUGE_VAL;
				h->records_count--;
				return 0;
			}
        }
        ind = (code + (int)pow(++off, 2)) % size;
    }

	ReturnErr("Couldn't find %s", key);
}

/*---------------------------------------------------------------------------*/

int dblhashLength(dblhash_ *h, unsigned int *length)
{
    ReturnErrIf(h == NULL);
	ReturnErrIf(length == NULL);
	*length = h->records_count;
	return 0;
}

/*---------------------------------------------------------------------------*/

int dblhashDestroy(dblhash_ **h)
{
	int i;

	ReturnErrIf(h == NULL);
	ReturnErrIf((*h) == NULL);
	Debug("Destroying Hash %p", *h);

	if((*h)->records != NULL) {
		for(i = 0; i < sizes[(*h)->size_index]; i++) {
			if(((*h)->records[i].freeMem != 'n') &&
					((*h)->records[i].key != NULL)) {
				free((*h)->records[i].key);
			}
		}
		free((*h)->records);
	}

	free(*h);
	*h = NULL;

	return 0;
}

/*---------------------------------------------------------------------------*/

dblhash_ * dblhashNew(dblhash_ *h, unsigned int capacity)
{
    int i, sind = 0;

    capacity /= load_factor;

    for (i = 0; i < sizes_count; i++) {
        if (sizes[i] > capacity) {
			sind = i;
			break;
		}
	}

	h = calloc(sizeof(dblhash_), 1);
	ReturnNULLIf(h == NULL);

	h->records = calloc(sizes[sind], sizeof(record_));
	ReturnNULLIf(h->records == NULL);

	Debug("Creating Hash %p", h);
    h->records_count = 0;
    h->size_index = sind;

    return h;
}

/*===========================================================================*/

