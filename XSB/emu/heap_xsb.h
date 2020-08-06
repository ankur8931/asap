/* File:      heap_xsb.h
** Author(s): David S. Warren, Jiyang Xu, Kostis Sagonas
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
** $Id: heap_xsb.h,v 1.22 2011-05-18 19:21:40 dwarren Exp $
** 
*/


/*---- The following are macros for setting heap values ----------------*/

#define new_heap_free(sh_reg) bld_free(sh_reg); sh_reg++
/* make a free variable on the top of the heap */

#define new_heap_string(sh_reg, str) bld_string(sh_reg, str); sh_reg++
/* make a new string node on the heap, with a string"str" */

#define new_heap_nil(sh_reg) bld_nil(sh_reg); sh_reg++

#define new_heap_num(sh_reg, val) bld_int_tagged(sh_reg, val); sh_reg++
/* make a new num node on the heap, with value val */

#define new_heap_int(sh_reg, val) bld_int_tagged(sh_reg, makeint(val)); sh_reg++
/* tag and make a new int on the heap, with int_val val */

#define new_heap_float(sh_reg, val) bld_float_tagged(sh_reg, val); sh_reg++
/* make a new float node on the heap, with value val in tagged format.*/

#define new_heap_functor(sh_reg, psc) bld_functor(sh_reg++, psc)
/* make a new functor node in heap */

#define new_heap_node(sh_reg, x) bld_copy(sh_reg++, x)
/* make a new heap node with value x (one word type) */

/*----------------------------------------------------------------------*/

#include "heap_defs_xsb.h"
#include "context.h"
#include "sys/types.h"

/*----- The following functions are used in other parts of the system --*/

extern int  gc_heap(CTXTdeclc int, int);
extern size_t  mark_heap(CTXTdeclc int,size_t *);
extern void glstack_ensure_space(CTXTdeclc size_t,int);
extern xsbBool glstack_realloc(CTXTdeclc size_t,int);
extern char *string_find_safe(char *str);
extern void initialize_glstack(CPtr, CPtr);

extern void print_cp(CTXTdeclc int);
extern void print_tr(CTXTdeclc int);
extern void print_ls(CTXTdeclc int);
extern void print_all_stacks(CTXTdeclc int);
extern void print_regs(CTXTdeclc int,int);
extern void print_heap(CTXTdeclc int,int,int);
extern void print_gc_statistics(CTXTdecl);
/*----------------------------------------------------------------------*/
