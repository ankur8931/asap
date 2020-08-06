/* File:      biassert_defs.h
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

#define MARK_CPSTACK_RETRACTALL                 0
#define UNMARK_CPSTACK_RETRACT                  1
#define DB_RETRACTALL0                          2
#define INIT_DBCLAUSE_CGC_BLOCKS                3
#define INIT_STANDARD_CGC_BLOCKS                4
#define INTERNED_TRIE_CPS_CHECK                 5
#define INIT_CALL_CLEANUP_BLOCKS                6
#define GET_TRIE_INFO                           7
#define SHAS_TRIE_INTERN                        8
#define SHAS_TRIE_INTERNED                      9
#define LOCK_TRIE_MUTEX                        10
#define UNLOCK_TRIE_MUTEX                      11
#define SHAS_TRIE_UNINTERN                     12
#define FIRST_TRIE_PROPERTY                    14
#define NEXT_TRIE_PROPERTY                     15
#define TRIE_DROP                              16
#define CONVERT_TO_DYNA                        17
#define GET_DYNAMIC_PRED_EP                    18
#define DYNAMIC_PRED_HAS_CLAUSES               19

#define DONT_EXPAND_HASHES                      1
#define EXPAND_HASHES                           0

#define NO_CPS_CHECK                            0
#define CPS_CHECK                               1


