/* File:      emuloop_aux.h
** Author(s): Luis Castro
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
** $Id: emuloop_aux.h,v 1.19 2010-08-19 15:03:36 spyrosh Exp $
** 
*/

#ifndef __EMULOOP_AUX_H__
#define __EMULOOP_AUX_H__

/*----------------------------------------------------------------------*/
/* Shared code transformed into macros in order to allow for            */
/* local operand-variables in emulator loop                             */
/* lfcastro, 10102000                                                   */
/*----------------------------------------------------------------------*/

#ifdef SLG_GC
#define SUBTRYME                                                          \
{                                                                         \
  CPtr cps_top;	                /* cps_top only needed for efficiency */  \
  CPtr old_top;                                                           \
                                                                          \
  save_find_locx(ereg);		/* sets ebreg to the top of the E-stack	*/\
  check_tcpstack_overflow;                                                \
  old_top = cps_top = top_of_cpstack;                                     \
  save_registers(cps_top, (Cell)op1, rreg);                               \
  save_choicepoint(cps_top, ereg, (byte *)op2, breg);                     \
  cp_prevtop(cps_top) = old_top;                                          \
  breg = cps_top;                                                         \
  hbreg = hreg;                                                           \
/*  XSB_Next_Instr();  */                                                     \
} 
#else
#define SUBTRYME                                                          \
{                                                                         \
  CPtr cps_top;	                /* cps_top only needed for efficiency */  \
                                                                          \
  save_find_locx(ereg);		/* sets ebreg to the top of the E-stack	*/\
  check_tcpstack_overflow;                                                \
  cps_top = top_of_cpstack;                                               \
  save_registers(cps_top, (Cell)op1, rreg);                               \
  save_choicepoint(cps_top, ereg, (byte *)op2, breg);                     \
  breg = cps_top;                                                         \
  hbreg = hreg;                                                           \
/*  XSB_Next_Instr(); */                                                      \
} 
#endif
/*----------------------------------------------------------------------*/

#if defined(PROFILE) && !defined(MULTI_THREAD)
#define RESTORE_SUB                                                       \
{                                                                         \
  CPtr tbreg;                                                             \
                                                                          \
  tbreg = breg;                                                           \
  /*  switch_envs(tbreg); */                                              \
  undo_bindings(tbreg);                                                   \
  ptcpreg = cp_ptcp(tbreg);                                               \
  delayreg = cp_pdreg(tbreg);                                             \
  restore_some_wamregs(tbreg, ereg);                                      \
  restore_registers(tbreg, (int)op1, rreg);                               \
  if (restore_type == 1) { /* trust */                                    \
    if (breg > bfreg) {                                                   \
      trapped_prolog_cps = trapped_prolog_cps                             \
	            + sizeof(struct choice_point)                         \
	            + ((int)op1 * sizeof(CPtr));}                         \
    breg = cp_prevbreg(breg);                                             \
    restore_trail_condition_registers(breg);                              \
  }                                                                       \
  XSB_Next_Instr();                                                       \
} 
#else
#define RESTORE_SUB                                                       \
{                                                                         \
  CPtr tbreg;                                                             \
                                                                          \
  tbreg = breg;                                                           \
  /*  switch_envs(tbreg); */                                              \
  undo_bindings(tbreg);                                                   \
  ptcpreg = cp_ptcp(tbreg);                                               \
  delayreg = cp_pdreg(tbreg);                                             \
  restore_some_wamregs(tbreg, ereg);                                      \
  restore_registers(tbreg, (int)op1, rreg);                               \
  if (restore_type == 1) { /* trust */                                    \
    breg = cp_prevbreg(breg);                                             \
    restore_trail_condition_registers(breg);                              \
  }                                                                       \
  XSB_Next_Instr();                                                       \
} 
#endif
/*----------------------------------------------------------------------*/

#define TABLE_RESTORE_SUB  {                                              \
  CPtr tbreg;                                                             \
  CPtr xtemp1;                                                            \
                                                                          \
  tbreg = breg;                                                           \
  switch_envs(tbreg);                                                     \
  /* This CP should be used for the dependency graph */                   \
  ptcpreg = tcp_subgoal_ptr(tbreg);                                       \
  delayreg = NULL;                                                        \
  restore_some_wamregs(tbreg, ereg);                                      \
  table_restore_registers(tbreg, (int)op1, rreg);                         \
  if (restore_type == 1) {                                                \
    xtemp1 = tcp_prevbreg(breg);                                          \
    restore_trail_condition_registers(xtemp1);                            \
  }                                                                       \
  XSB_Next_Instr();                                                       \
} 

#endif
