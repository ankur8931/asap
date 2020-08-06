/* File:      debug_xsb.h
** Author(s): Luís Castro
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
** $Id: debug_xsb.h,v 1.39 2013-05-01 17:54:56 tswift Exp $
** 
*/

#ifndef __DEBUG_XSB_H__
#define __DEBUG_XSB_H__

#define STRIDESIZE     30

#define EOS	"--------------------BOTTOM_OF_STACK--------------------\n"
#define EOFR	"--------------------------------------------\n"
#define EOSUBG	"\n------------------------------------------------------------\n"

/* extern int cur_log_level; */
#define cur_log_level pflags[VERBOSENESS_LEVEL]
typedef struct subgoal_frame *VariantSF;
/* Verboseness levels */
#define LOG_QUIET 0
#define LOG_SHY 1
#define LOG_VERBOSE 2
#define LOG_LOUD 3

/* The following is a first approximation at which message categories
   fit into each verboseness level */
#define LOG_DEBUG              LOG_SHY
#define LOG_ASSERT             LOG_VERBOSE
#define LOG_RETRACT            LOG_LOUD
#define LOG_RETRACT_GC         LOG_LOUD
#define LOG_DELAY              LOG_LOUD
#define LOG_DELAYVAR           LOG_LOUD
#define LOG_TRIE_STACK         LOG_LOUD
#define LOG_ATTV               LOG_LOUD
#define LOG_GC                 LOG_VERBOSE
#define LOG_HASHTABLE          LOG_LOUD
#define LOG_REALLOC            LOG_VERBOSE
#define LOG_SCHED              LOG_LOUD
#define LOG_STORAGE            LOG_LOUD
#define LOG_STRUCT_MANAGER     LOG_LOUD
#define LOG_TRIE_INSTR         LOG_LOUD
#define LOG_INTERN             LOG_LOUD
#define LOG_TRIE               LOG_LOUD
#define LOG_BD                 LOG_LOUD
#define LOG_COMPLETION         LOG_LOUD

typedef struct forest_log_buffer {
  int fl_size;
  char * fl_buffer;
} forest_log_buffer_struct;
typedef forest_log_buffer_struct *forestLogBuffer;

#ifndef MULTI_THREAD
extern forest_log_buffer_struct fl_buffer_1;
extern forest_log_buffer_struct fl_buffer_2;
extern forest_log_buffer_struct fl_buffer_3;

extern  forestLogBuffer forest_log_buffer_1;
extern  forestLogBuffer forest_log_buffer_2;
extern  forestLogBuffer forest_log_buffer_3;
#endif

#define maybe_realloc_buffers(BUFFER,SIZE) {				\
    if (SIZE > (BUFFER->fl_size)/2) {					\
      /*      printf("reallocing buffer to %d\n",(BUFFER->fl_size)*2);*/ \
      BUFFER->fl_buffer							\
	= (char *)mem_realloc((BUFFER->fl_buffer),(BUFFER->fl_size),	\
			      2*(BUFFER->fl_size),BUFF_SPACE);		\
      /*      printf("buffer so far: %s\n\n",BUFFER->fl_buffer);*/	\
      gdb_dummy();							\
      BUFFER->fl_size = (BUFFER->fl_size)*2;				\
    }									\
  }

#ifndef MULTI_THREAD
extern void print_delay_list(FILE *, CPtr);
extern void print_delay_element(FILE *, Cell);
extern void print_registers(FILE *,Psc,long);
extern int  sprint_subgoal(forestLogBuffer, int, VariantSF );
extern void sprintAnswerTemplate(forestLogBuffer, CPtr, int);
extern void sprintNonCyclicRegisters(forestLogBuffer,Psc);
extern void sprintCyclicRegisters(forestLogBuffer,Psc);
extern void sprintCyclicTerm(forestLogBuffer, Cell);
extern void print_completion_stack(FILE*);
extern void printCyclicTerm(Cell);
extern void mark_cyclic(Cell);
extern int  sprint_delay_list(forestLogBuffer, CPtr);
void print_local_stack_nonintr(char *);
extern void print_callnode(FILE *, callnodeptr);
extern void print_subgoal(FILE *, VariantSF);
#else
extern void print_delay_list(struct th_context * ,FILE *, CPtr);
extern void print_registers(struct th_context * ,FILE *,Psc,long);
extern int sprint_subgoal(struct th_context *, forestLogBuffer,int,  VariantSF );
extern void sprintAnswerTemplate(struct th_context *, forestLogBuffer, CPtr, int);
extern void sprintCyclicTerm(struct th_context *, forestLogBuffer, Cell);
extern void sprintCyclicRegisters(struct th_context *,forestLogBuffer,Psc);
extern void sprintNonCyclicRegisters(struct th_context *,forestLogBuffer,Psc);
extern void print_completion_stack(struct th_context *,FILE*);
extern void printCyclicTerm(struct th_context *,Cell);
extern void mark_cyclic(struct th_context *,Cell);
extern int sprint_delay_list(struct th_context *, forestLogBuffer, CPtr);
void print_local_stack_nonintr(struct th_context *,char *);
extern void print_callnode(struct th_context *, FILE *, callnodeptr);
extern void print_subgoal(struct th_context *,FILE *, VariantSF);
#endif

extern int sprint_quotedname(char *, int,char *);
extern int sprintTerm(forestLogBuffer, Cell);
extern int ctrace_ctr;
extern void printterm(FILE *, Cell, long);

/* dbg_* macros */
#ifdef DEBUG_VERBOSE
/* in error_xsb.c */
#define xsb_dbgmsg(a)                               \
      xsb_dbgmsg1 a
/* in debug_xsb.c */
#define dbg_print_subgoal(LOG_LEVEL,FP,SUBG)          \
   if (LOG_LEVEL <= cur_log_level)                    \
      print_subgoal(FP,SUBG)
#define dbg_printterm(LOG_LEVEL,FP,TERM,DEPTH)        \
   if (LOG_LEVEL <= cur_log_level)                    \
      printterm(FP,TERM,DEPTH)
#define dbg_print_completion_stack(LOG_LEVEL)         \
   if (LOG_LEVEL <= cur_log_level)                    \
      print_completion_stack()
#define dbg_print_delay_list(LOG_LEVEL,FP,DLIST)      \
   if (LOG_LEVEL <= cur_log_level)                    \
      print_delay_list(FP,DLIST)

/* in dynamic_stack.c */
#define dbg_dsPrint(LOG_LEVEL,DS,COMMENT)             \
   if (LOG_LEVEL <= cur_log_level)                    \
      dsPrint(DS,COMMENT)

#define dbg_smPrint(LOG_LEVEL,SM_RECORD,STRING)       \
   if (LOG_LEVEL <= cur_log_level)                    \
      smPrint(SM_RECORD,STRING)

/* in tst_utils.c */
#define dbg_printTriePathType(LOG_LEVEL,FP,TYPE,LEAF) \
   if (LOG_LEVEL <= cur_log_level)                    \
      printTriePathType(FP,TYPE,LEAF)
#define dbg_printTrieNode(LOG_LEVEL,FP,PTN)           \
   if (LOG_LEVEL <= cur_log_level)                    \
      printTrieNode(FP,PTN)
#define dbg_printAnswerTemplate(LOG_LEVEL,FP,PAT,S)   \
   if (LOG_LEVEL <= cur_log_level)                    \
      printAnswerTemplate(FP,PAT,S)
extern void print_completion_stack(FILE *);
#define xsb_dbgmsg(a) xsb_dbgmsg1 a
#else
#define xsb_dbgmsg(a)

#define dbg_print_subgoal(L,F,S)
#define dbg_printterm(L,F,T,D)
#define dbg_print_completion_stack(L)

#define dbg_print_delay_list(L,F,D)
#define dbg_dsPrint(L,D,C)
#define dbg_smPrint(L,SR,ST)
#define dbg_printTriePathType(L,F,T,LF)
#define dbg_printTrieNode(L,F,P)
#define dbg_printAnswerTemplate(L,F,P,S)

#endif
#endif /* __DEBUG_XSB_H__ */
