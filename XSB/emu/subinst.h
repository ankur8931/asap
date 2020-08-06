/* File:      subinst.h
** Author(s): Terrance Swift
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
** $Id: subinst.h,v 1.11 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#define OUTER_FIXPOINT 1
#define ITER_FIXPOINT 2
#define SCHED_ANSWERS 3
#define SIMPL_REMOVE_DE 7
#define SIMPL_REMOVE_DL 8
#define NEW_ANSWER_SIMPL_POS_UNS 11
#define NEW_ANSWER_SIMPL_POS_UNC 12
#define NEW_ANSWER_SIMPL_NEG_FAIL 13
#define NEW_ANSWER_SIMPL_NEG_SUC 14

extern Cell subinst_table[256][2];

/* --------------------------------------------------------------------- */

/* The following global variables are used for ProfileLeader, and
   represent summary statistics on the size and confomation of SCCs. 
   It can only be configured with Profile nd cannot be configured with the 
   multi-threaded engine. */

#if defined(PROFILE) && !defined(MULTI_THREAD)
extern int trapped_prolog_cps;
extern int max_subgoals;
extern int max_completed;
extern int max_consumers_in_ascc;
extern int max_compl_susps_in_ascc;

/* The following global variables are used for SpitOutGraph, which 
   prints out the entire SDG */

extern int sdg_sample_rate;
extern int sdg_check_num;

#endif



