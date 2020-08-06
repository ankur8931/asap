/* Copyright (C) 2004 Christopher Clark <firstname.lastname@cl.cam.ac.uk> */

#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "hashtable.h"
#include "hashtable_private.h"

#include "auxlry.h"
#include "memory_xsb.h"

//#define HASHTABLE_DEBUG
#ifdef HASHTABLE_DEBUG
#define hashtable_debug(X) printf X
#else 
#define hashtable_debug(X) 
#endif
/*
Credit for primes table: Aaron Krowne
 http://br.endernet.org/~akrowne/
 http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
*/
static const unsigned int primes[] = {
2, 17, 53, 97, 193, 389,
769, 1543, 3079, 6151,
12289, 24593, 49157, 98317,
196613, 393241, 786433, 1572869,
3145739, 6291469, 12582917, 25165843,
50331653, 100663319, 201326611, 402653189,
805306457, 1610612741
};
const unsigned int prime_table_length = sizeof(primes)/sizeof(primes[0]);
const double max_load_factor = 0.65;

/* maintain chain of all hashtables in incr, for use when deleting all tables */
static struct hashtable *hashtable_chain = NULL;

/*****************************************************************************/
struct hashtable *
create_hashtable1(unsigned int minsize,
                 unsigned int (*hashf) (void*),
                 int (*eqf) (void*,void*))
{
    struct hashtable *h;
    unsigned int pindex, size = primes[0];
  hashtable_debug(("create_hashtable1 hashtable_chain %p\n",hashtable_chain));
    /* Check requested hashtable isn't too large */
    if (minsize > (1u << 30)) return NULL;
    /* Enforce size as prime */
    for (pindex=0; pindex < prime_table_length; pindex++) {
        if (primes[pindex] >= minsize) { size = primes[pindex]; break; }
    }
    h = (struct hashtable *)mem_alloc(sizeof(struct hashtable),INCR_TABLE_SPACE);
    if (NULL == h) return NULL; /*oom*/
    h->table = (struct entry **)mem_alloc(sizeof(struct entry*) * size,INCR_TABLE_SPACE);
    if (NULL == h->table) { mem_dealloc(h,sizeof(struct hashtable),INCR_TABLE_SPACE); return NULL; } /*oom*/
    memset(h->table, 0, size * sizeof(struct entry *));
    h->tablelength  = size;
    h->primeindex   = pindex;
    h->entrycount   = 0;
    h->hashfn       = hashf;
    h->eqfn         = eqf;
    h->loadlimit    = (unsigned int) ceil(size * max_load_factor);
    h->prev         = NULL;
    h->next         = hashtable_chain;
    hashtable_chain = h;
    if (h->next) (h->next)->prev = h;
    hashtable_debug(("create_hashtable2 h %p hashtable_chain %p next %p\n",h,hashtable_chain,h->next));
    return h;
}

/*****************************************************************************/
unsigned int
hash1(struct hashtable *h, void *k)
{
    /* Aim to protect against poor hash functions by adding logic here
     * - logic taken from java 1.4 hashtable source */
    unsigned int i = h->hashfn(k);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return i;
}

/*****************************************************************************/
static int
hashtable1_expand(struct hashtable *h)
{
    /* Double the size of the table to accomodate more entries */
    struct entry **newtable;
    struct entry *e;
    struct entry **pE;
    unsigned int size, newsize, i, index;
    /* Check we're not hitting max capacity */
    if (h->primeindex == (prime_table_length - 1)) return 0;
    size = primes[h->primeindex];
    newsize = primes[++(h->primeindex)];

    newtable = (struct entry **)mem_alloc(sizeof(struct entry*) * newsize,INCR_TABLE_SPACE);
    if (NULL != newtable)
    {
        memset(newtable, 0, newsize * sizeof(struct entry *));
        /* This algorithm is not 'stable'. ie. it reverses the list
         * when it transfers entries between the tables */
        for (i = 0; i < h->tablelength; i++) {
            while (NULL != (e = h->table[i])) {
                h->table[i] = e->next;
                index = indexFor(newsize,e->h);
                e->next = newtable[index];
                newtable[index] = e;
            }
        }
        mem_dealloc(h->table,sizeof(struct entry*) * size,INCR_TABLE_SPACE);
        h->table = newtable;
    }
    /* Plan B: realloc instead */
    else 
    {
        newtable = (struct entry **)
	  mem_realloc(h->table, size, newsize * sizeof(struct entry *),INCR_TABLE_SPACE);
        if (NULL == newtable) { (h->primeindex)--; return 0; }
        h->table = newtable;
        memset(newtable[h->tablelength], 0, newsize - h->tablelength);
        for (i = 0; i < h->tablelength; i++) {
            for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
                index = indexFor(newsize,e->h);
                if (index == i)
                {
                    pE = &(e->next);
                }
                else
                {
                    *pE = e->next;
                    e->next = newtable[index];
                    newtable[index] = e;
                }
            }
        }
    }
    h->tablelength = newsize;
    h->loadlimit   = (unsigned int) ceil(newsize * max_load_factor);
    return -1;
}

/*****************************************************************************/
unsigned int
hashtable1_count(struct hashtable *h)
{
    return h->entrycount;
}

/*****************************************************************************/
int
hashtable1_insert(struct hashtable *h, void *k, void *v)
{
    /* This method allows duplicate keys - but they shouldn't be used */
    unsigned int index;
    struct entry *e;
    if (++(h->entrycount) > h->loadlimit)
    {
        /* Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.*/
        hashtable1_expand(h);
    }
    e = (struct entry *)mem_alloc(sizeof(struct entry),INCR_TABLE_SPACE);
    if (NULL == e) { --(h->entrycount); return 0; } /*oom*/
    e->h = hash1(h,k);
    index = indexFor(h->tablelength,e->h);
    e->k = k;
    e->v = v;
    e->next = h->table[index];
    h->table[index] = e;
    return -1;
}

/*****************************************************************************/
void * /* returns value associated with key */
hashtable1_search(struct hashtable *h, void *k)
{
    struct entry *e;
    unsigned int hashvalue, index;
    hashvalue = hash1(h,k);
    index = indexFor(h->tablelength,hashvalue);
    e = h->table[index];
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->h) && (h->eqfn(k, e->k))) return e->v;
        e = e->next;
    }
    return NULL;
}

/*****************************************************************************/
void * /* returns value associated with key */
hashtable1_remove(struct hashtable *h, void *k)
{
    /* TODO: consider compacting the table when the load factor drops enough,
     *       or provide a 'compact' method. */

    struct entry *e;
    struct entry **pE;
    void *v;
    unsigned int hashvalue, index;

    hashvalue = hash1(h,k);
    index = indexFor(h->tablelength,hash1(h,k));
    pE = &(h->table[index]);
    e = *pE;
    while (NULL != e)
    {
        /* Check hash value to short circuit heavier comparison */
        if ((hashvalue == e->h) && (h->eqfn(k, e->k)))
        {
            *pE = e->next;
            h->entrycount--;
            v = e->v;
            //freekey(e->k);
            mem_dealloc(e,sizeof(struct entry),INCR_TABLE_SPACE);
            return v;
        }
        pE = &(e->next);
        e = e->next;
    }
    return NULL;
}

/*****************************************************************************/
/* destroy */
void
hashtable1_destroy(struct hashtable *h, int free_values)
{
    unsigned int i;
    struct entry *e, *f;
    struct entry **table = h->table;
  hashtable_debug(("hashtable destroy %p %p\n",h,h->table));
    if (free_values) /* not used in incr... */
    {
        for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
	      { f = e; e = e->next; 
		freekey(f->k); 
		free(f->v); 
		free(f); 
	      }
        }
    }
    else
    {
        for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
	      { f = e; e = e->next;
		// freekey(f->k); 
		mem_dealloc(f,sizeof(struct entry),INCR_TABLE_SPACE); 
	      }
        }
    }
    hashtable_debug(("here0 prev %p\n",h->prev));
    if (h->prev != NULL) (h->prev)->next = h->next; else hashtable_chain = h->next;
    hashtable_debug(("here1 %p\n",(h->next)));
    if (h->next != NULL) (h->next)->prev = h->prev;
    hashtable_debug(("here2 %p %p\n",h->table,h->table +h->tablelength*sizeof(struct entry *)));
    mem_dealloc(h->table,h->tablelength*sizeof(struct entry *),INCR_TABLE_SPACE);
    mem_dealloc(h,sizeof(struct hashtable),INCR_TABLE_SPACE);
}

void
hashtable1_destroy_all(int free_values) {
  while (hashtable_chain != NULL) {
    hashtable1_destroy(hashtable_chain,free_values);
  }
}

/*
 * Copyright (c) 2002, Christopher Clark
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
