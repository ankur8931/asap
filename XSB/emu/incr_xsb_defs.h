/* File:      incr_xsb_defs.h  -- support for incremental evaluation.
** Author(s): Diptikalyan Saha, C.R. Ramakrishnan
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2001
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
** $Id: incr_xsb_defs.h,v 1.15 2012-10-30 23:16:01 tswift Exp $
** 
*/
//#define GET_AFFECTED_CALLS         1
//#define CONSUME_AFFECTED_CALLS     2
//#define GET_CALL_GRAPH           3
#define INVALIDATE_CALLNODE        4
//#define PRINT_CALL               5
//#define GET_CHANGED_CALLS          6
#define PSC_SET_INCR               7 
#define GET_CALLNODEPTR_INCR       8
#define INVALIDATE_SF              9
#define IMMED_DEPENDS_LIST        10
#define IMMED_AFFECTS_LIST        11
#define IS_AFFECTED               12
#define PSC_GET_INCR              13
#define INVALIDATE_CALLNODE_TRIE  14
#define RETURN_LAZY_CALL_LIST     15
#define CALL_IF_AFFECTED          16
#define CHECK_INCREMENTAL         17
#define IMMED_AFFECTS_PTRLIST     18
#define GET_SUBGOAL_FRAME         19
#define GET_INCR_SCCS             20
#define IMMED_DEPENDS_PTRLIST     21
#define PSC_GET_INTERN            22
#define PSC_SET_INTERN            23

// for psc hacking
#define NONINCREMENTAL 0
#define INCREMENTAL 1
#define OPAQUE 2

/* callnode-> recomputable values */
#define COMPUTE_DEPENDENCIES_FIRST 0
#define COMPUTE_DIRECTLY           1

/* Used for lazy call_list */
#define CALL_LIST_EVAL             0
#define CALL_LIST_INSPECT          1
#define CALL_LIST_CREATE_EVAL      2
  
/* dfs_inedges */
#define CANNOT_UPDATE              1

#define NOT_ABOLISHING             0
#define ABOLISHING                 1
