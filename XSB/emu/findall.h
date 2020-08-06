/* File:      findall.h
** Author(s): Bart Demoen, (mod dsw oct 99)
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
** $Id: findall.h,v 1.11 2013-05-06 21:10:24 dwarren Exp $
** 
*/

#ifndef __FINDALL_H__

#define __FINDALL_H__

/* Findall copies its templates to a findall-heap.  This heap is allocated in
   chunks of FINDALL_CHUNCK_SIZE (Cell)entries.  Since more than one findall
   can be active at the same time, each findall gets a number, determined by
   the global variables nextfree; this happens in findall_init. The maximal
   number of active findalls is MAX_FINDALLS.

   A solution list of findall is represented by its size and a pointer to the
   beginning of the solution list and a pointer to the tail of this solution
   list.  The size is important for copying back to the heap, to ensure that
   there is enough space, before we start copying.  The tail is a free
   variable.

   One solution list or template can be in more than one chunk. Chuncks are
   linked together by the first field in the chunk; this field is only needed
   for the deallocation of the chunks, not for the copying itself.

   Trailing of variables (to ensure proper sharing) is done on a special
   purpose trail, which consists also of chuncks, linked together.

   Everything is allocated dynamically, and freed asap.

   findall_clean should be called at the start of every toplevel.  */

#define FINDALL_CHUNCK_SIZE 4000 /* anything > MAX_ARITY+2 is good */

/* one invocation of findall is associated with one entry in the
   findall_solutions array: we then call this entry active; the type of the
   entry is findall_solution_list */

#define F_TR_NUM 250 /* the number of trail entries in a chunck of the trail
			it must be a multiple of 2
		     */

typedef struct f_tr_chunk {
  struct f_tr_chunk *previous ;
  CPtr tr[F_TR_NUM] ;
} f_tr_chunk ;

typedef struct {
  CPtr	first_chunk ;	 /* chunk in which solution list starts
			    the solution list starts at offset +1 */
  CPtr   tail ;		 /* tail of solution list - always a free var */
  CPtr   current_chunk ; /* most recent alloc chunk for this solution list */
  CPtr   top_of_chunk ;	 /* where next template can be copied to 
			    points inside current_chunk */
  CPtr   termptr ;       /* pointer to root of term stored */
  int size ;		 /* size is the size of the solution list - init = 1
			    when entry not active, size = the next free entry 
			    the last free entry, has size = -1, for overflow
			    checking */ 
} findall_solution_list ;

#ifndef MULTI_THREAD
extern findall_solution_list *findall_solutions, *current_findall ;

extern CPtr gl_bot, gl_top ;
#endif

#endif /* __FINDALL_H__ */
