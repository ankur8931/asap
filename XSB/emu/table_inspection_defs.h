/* File:      table_inspection_defs.h
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
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License
** for more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** 
*/

#define FIND_COMPONENTS                          0
#define FIND_FORWARD_DEPENDENCIES                1
#define FIND_BACKWARD_DEPENDENCIES               2
#define FIND_ANSWERS                             3
#define CALL_SUBS_SLG_NOT                        4
#define GET_PRED_CALLTRIE_PTR                    5
#define GET_CALLSTO_NUMBER                       6
#define GET_ANSWER_NUMBER                        7
#define EARLY_COMPLETE_ON_NTH                    8
#define EARLY_COMPLETE                           9
#define START_FOREST_VIEW                        10
#define STOP_FOREST_VIEW                         11
//#define FLUSH_FOREST_VIEW                        34   -- placed below
#define TNOT_SETUP                               12
#define GET_CURRENT_SCC                          13
#define PRINT_COMPLETION_STACK                   14
#define PRINT_CYCLIC_TERM                        15
#define MARK_TERM_CYCLIC_1                       16
#define GET_VISITORS_NUMBER                      17
#define GET_SCC_DUMPFILE                         18
#define CHECK_VARIANT                            19
#define IS_CONDITIONAL_ANSWER                    20
#define SET_TIF_PROPERTY                         21 
#define GET_TIF_PROPERTY                         22
#define IMMED_ANS_DEPENDS_PTRLIST                23
#define GET_RESIDUAL_SCCS                        24
#define TEMP_FUNCTION                            25
#define TEMP_FUNCTION_2                          26
#define ABOLISH_NONINCREMENTAL_TABLES            27
#define SET_FOREST_LOGGING_FOR_PRED              28
#define INCOMPLETE_SUBGOAL_INFO                  29
#define GET_CALL_FROM_SF                         30
#define GET_POS_AFFECTS                          31
#define GET_NEG_AFFECTS                          32
#define PSC_IMMUTABLE                            33
#define FLUSH_FOREST_VIEW                        34

/* added by Bart Demoen & Kostis Sagonas for debugging and convenience */
#define PRINT_LS                240
#define PRINT_TR                241
#define PRINT_HEAP              242
#define PRINT_CP                243
#define PRINT_REGS              244
#define PRINT_ALL_STACKS        245


// For delete return
#define ANSWER_SUBSUMPTION   0
#define USER_DELETE          1

// For xxx_tif_property
#define SUBGOAL_DEPTH  0
#define ANSWER_DEPTH   1
#define INTERNING_GROUND 2

#define IS_SUBGOAL_FRAME 0
#define IS_ASI           1

#define ERROR_ON_INCOMPLETE 0
#define SKIP_ON_INCOMPLETE  1

// Used in abolish_table_call_single
#define DONT_INVALIDATE   0
#define DO_INVALIDATE     1

#define SHOULDNT_COND_WARN     0
#define SHOULD_COND_WARN       1
