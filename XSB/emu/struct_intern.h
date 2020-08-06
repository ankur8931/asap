/* File:      hashcons.h
** Author(s): Warren
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
** Copyright (C) ECRC, Germany, 1990
** 
** XSB is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XSB is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: struct_intern.h,v 1.2 2013-05-06 21:10:25 dwarren Exp $
** 
*/


/* Initial data structures for hash consing: linear, add hash tables
later.

hc_block[256] pointers to chains of blocks containing the records for
terms of arity i.  (0 is used for the list constructor ./2) The first
word of a block chains multiple blocks for records of the given arity
i.  That is followed by hc_num_in_block records to contain the
structure records; the first word in the record is a link to the next
structure record, the 2nd through arity+2 words contain the structure
record, i.e., the psc ptr followed by the fields (arity of them) of
the structure record.

*/

struct intterm_rec {
  struct intterm_rec *next;
  Cell intterm_psc;
};

struct intterm_block {
  struct intterm_block *nextblock;
  struct intterm_rec recs;
};
  

struct hc_block_rec {
  struct intterm_block *base; /* base of chain of blocks for arity i records */
  struct intterm_rec **hashtab;  /* address of hash table for thse blocks */
  Integer hashtab_size;
  struct intterm_rec *freechain; /* base of chain of free arity i structure records (filled by gc) */
  struct intterm_rec *freedisp; /* address of first free record in (first) block */
};

// make bigger?
#define hc_num_in_block 1000

struct term_subterm {
  Cell term;
  Cell newterm;
  int subterm_index;
  int ground;
};


int isinternstr_really(prolog_term term);
prolog_term intern_rec(CTXTdeclc prolog_term term);
prolog_term intern_term(CTXTdeclc prolog_term term);
