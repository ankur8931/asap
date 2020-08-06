/* File:      inst_xsb.c
** Author(s): Warren, Swift, Xu, Sagonas, Freire, Johnson
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
** $Id: inst_xsb.c,v 1.23 2011-05-18 19:21:40 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "inst_xsb.h"
#include "subinst.h"

extern void init_builtin_table(void);

/* ------------------------------------------------------------------- */
#ifdef PROFILE

/* Initialize this stuff for profiling.  In profiling, the number of
   occurrences of each instruction can be counted, as well as
   operations that happen within instructions, such as switch_envs,
   and others.  Thus the instruction tables have an added field, to
   keep the count.  In addition, statistics on SCCs can be derived and
   the SDGs themselves printed out, if you like. */

Cell inst_table[BUILTIN_TBL_SZ][6];

UInteger num_switch_envs;
UInteger num_switch_envs_iter;

Cell subinst_table[BUILTIN_TBL_SZ][2];

#define XSB_INST(inum, inst, label, op1type, op2type, op3type, op4type) \
        inst_table[inst][0] = (Cell)( #inst ); \
	inst_table[inst][1] = op1type; \
	inst_table[inst][2] = op2type; \
	inst_table[inst][3] = op3type; \
	inst_table[inst][4] = op4type; \
	inst_table[inst][5] = 0

#define profile_inst(inst) \
        inst_table[inst][5] = inst_table[inst][5] + 1

#define set_subinst_table(inst,instr) \
        subinst_table[inst][0] = (Cell)(instr); \
	subinst_table[inst][1] = 0

/* ProfileLeader */
int max_subgoals = 0;
int max_completed = 0;
int max_consumers_in_ascc = 0;
int max_compl_susps_in_ascc = 0;
int trapped_prolog_cps = 0;

/* SpitOutGraph */
int sdg_sample_rate = 2000;
int sdg_check_num = 0;

#else
Cell inst_table[BUILTIN_TBL_SZ][5];

#define XSB_INST(inum, inst, label, op1type, op2type, op3type, op4type) \
        inst_table[inst][0] = (Cell)( #inst ); \
	inst_table[inst][1] = op1type; \
	inst_table[inst][2] = op2type; \
	inst_table[inst][3] = op3type; \
	inst_table[inst][4] = op4type
#endif /* PROFILE */

/*----------------------------------------------------------------------*/

int sizeof_inst(byte inst)
{ /* returns the size of an instruction in bytes */
  int i, size;

  size = 1;
  for (i=2; i <= 4 && inst_table[inst][i] != X; i++) {
    size++;
  }
  size *= sizeof(byte *); /* works for both 32 & 64 bit machines */
  return size;
}

void init_instructions() 
{
#include "xsb_inst_list.h"
#undef XSB_INST
}

#ifdef PROFILE /* for profiling only */
void init_subinst_table(void)
{
  set_subinst_table(OUTER_FIXPOINT,"outer_fixpoint");
  set_subinst_table(ITER_FIXPOINT,"inner_fixpoint");
  set_subinst_table(SCHED_ANSWERS,"sched_answers");
  set_subinst_table(SIMPL_REMOVE_DL,"simpl_remove_dl");
  set_subinst_table(SIMPL_REMOVE_DE,"simpl_remove_de");
  set_subinst_table(NEW_ANSWER_SIMPL_POS_UNS,"new_answer_simpl_pos_uns");
  set_subinst_table(NEW_ANSWER_SIMPL_POS_UNC,"new_answer_simpl_pos_unc");
  set_subinst_table(NEW_ANSWER_SIMPL_NEG_FAIL,"new_answer_simpl_neg_fail");
  set_subinst_table(NEW_ANSWER_SIMPL_NEG_SUC,"new_answer_simpl_neg_succ");
  num_switch_envs=0;
  num_switch_envs_iter=0;
}
#endif

/*----------------------------------------------------------------------*/

void init_inst_table(void)
{
    init_instructions();
    init_builtin_table();
#ifdef PROFILE
    init_subinst_table();
#endif
}

/*----------------------------------------------------------------------*/
