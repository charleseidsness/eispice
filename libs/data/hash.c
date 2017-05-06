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
    char *key;
    void *record;
} record_;

struct _hash {
    record_ *records;
    unsigned int records_count;
    unsigned int size_index;
};

static int hashGrow(hash_ *h)
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
            ReturnErrIf(hashAdd(h, old_recs[i].key, old_recs[i].record));
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

int hashFreeKeyAndRecord(char *key, void *record)
{
	if(key != NULL) {
		free(key);
	}
	if(record != NULL) {
		free(record);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/

int hashAddPointer(hash_ *h, char *key, void *record, void **recordPtr)
{
    record_ *recs;
    unsigned int off, ind, size, code;
	
	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);
	ReturnErrIf(*key == '\0');
	
    if (h->records_count > (sizes[h->size_index] * load_factor)) {
        ReturnErrIf(hashGrow(h));
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
    recs[ind].record = record;
	
    h->records_count++;
	
	*recordPtr = &recs[ind].record;
	
    return 0;
}

int hashAdd(hash_ *h, char *key, void *record)
{
    void *recordPtr;
	ReturnErrIf(hashAddPointer(h, key, record, &recordPtr));
	return 0;
}

int hashFind(hash_ *h, char *key, void **record)
{
    record_ *recs;
    unsigned int off, ind, size;
    unsigned int code;
	
	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);
	ReturnErrIf(record == NULL);
	
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
				*record = recs[ind].record;
				return 0;
			}
		}
        ind = (code + (int)pow(++off,2)) % size;
    }
	
    	/* Couldn't find the key */
	*record = NULL;
    return 0;
}

int hashFindPointer(hash_ *h, char *key, void ***record)
{
    record_ *recs;
    unsigned int off, ind, size;
    unsigned int code;
	
	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);
	ReturnErrIf(record == NULL);
	
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
				*record = &recs[ind].record;
				return 0;
			}
		}
        ind = (code + (int)pow(++off,2)) % size;
    }
	
    	/* Couldn't find the key */
	*record = NULL;
    return 0;
}

int hashRemove(hash_ *h, char *key, void **record)
{
    unsigned int code;
    record_ *recs;
    unsigned int off, ind, size;
	
	ReturnErrIf(h == NULL);
	ReturnErrIf(key == NULL);
	ReturnErrIf(record == NULL);
	
	code = strhash(key);
	
    recs = h->records;
    size = sizes[h->size_index];
    ind = code % size;
    off = 0;
	
    while (recs[ind].hash) {
        if ((code == recs[ind].hash) && (recs[ind].key != NULL)) {
			if(!strcmp(key, recs[ind].key)) {
				/* Do not erase hash, so probes for collisions succeed */
				*record = recs[ind].record;
				recs[ind].key = NULL;
				recs[ind].record = NULL;
				h->records_count--;
				return 0;
			}
        }
        ind = (code + (int)pow(++off, 2)) % size;
    }
	
	/* Couldn't find the key */
	*record = NULL;
    return 0;
}

int hashLength(hash_ *h, unsigned int *length)
{
    ReturnErrIf(h == NULL);
	ReturnErrIf(length == NULL);
	*length = h->records_count;
	return 0;
}

int hashExecute(hash_ *h, hashExecute_ f, void *private)
{
	int i;
	ReturnErrIf(h == NULL);
	
	if(f != NULL) {
		for(i = 0; i < sizes[h->size_index]; i++) {
			if(h->records[i].key != NULL) {
				ReturnErrIf(f(h->records[i].key, h->records[i].record, 
						private));
			}
		}
	}
	
	return 0;
}

int hashDestroy(hash_ **h, hashDestroy_ f)
{
	int i;
	
	ReturnErrIf(h == NULL);
	ReturnErrIf((*h) == NULL);
	Debug("Destroying Hash %p", *h);
	
	if(f != NULL) {
		for(i = 0; i < sizes[(*h)->size_index]; i++) {
			if((*h)->records[i].key != NULL) {
				ReturnErrIf(f((*h)->records[i].key, (*h)->records[i].record));
			}
		}
	}
	
	if((*h)->records != NULL) {
		free((*h)->records);
	}
	free(*h);
	
	*h = NULL;
	
	return 0;
}

hash_ * hashNew(hash_ *h, unsigned int capacity)
{
    int i, sind = 0;
	
    capacity /= load_factor;
	
    for (i = 0; i < sizes_count; i++) {
        if (sizes[i] > capacity) {
			sind = i;
			break;
		}
	}
	
	h = calloc(sizeof(hash_), 1);
	ReturnNULLIf(h == NULL);
	
	h->records = calloc(sizes[sind], sizeof(record_));
	ReturnNULLIf(h->records == NULL);
    
	Debug("Creating Hash %p", h);
    h->records_count = 0;
    h->size_index = sind;
	
    return h;
}

