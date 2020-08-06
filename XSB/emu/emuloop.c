/* File:      emuloop.c
** Author(s): Warren, Swift, Xu, Sagonas, Johnson
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-2013
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
** $Id: emuloop.c,v 1.240 2013-05-06 21:10:24 dwarren Exp $
** 
*/
//#define GC_TEST

#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#ifdef FOREIGN
#ifndef SOLARIS
#ifndef FOREIGN_WIN32
#include <sys/un.h>
#endif
#endif
#endif

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "register.h"
#include "error_xsb.h"
#include "inst_xsb.h"
#include "psc_xsb.h"
#include "deref.h"
#include "memory_xsb.h"
#include "heap_xsb.h"
#include "sig_xsb.h"
#include "varstring_xsb.h"
#include "emudef.h"
#include "loader_xsb.h"
#include "binding.h"
#include "flags_xsb.h"
#include "trie_internals.h"
#include "choice.h"
#include "sw_envs.h"
#include "tab_structs.h"
#include "tables.h"
#include "subinst.h"
#include "scc_xsb.h"
#include "subp.h"
#include "function.h"
#include "tr_utils.h"
#include "cut_xsb.h"
#include "export.h"
#include "orient_xsb.h"
#include "io_builtins_xsb.h"
#include "unify_xsb.h"
#include "emuloop_aux.h"
#include "remove_unf.h"
#include "thread_xsb.h"
#include "deadlock.h"
#include "rw_lock.h"
#include "debug_xsb.h"
#include "hash_xsb.h"
#include "struct_manager.h"
#include "builtin.h"
#include "call_graph_xsb.h" /* incremental evaluation */
#include "cinterf.h"
#include "struct_intern.h"
#include "ptoc_tag_xsb_i.h"
#include "cell_xsb_i.h"
#include "tables_i.h"

#ifndef MULTI_THREAD
BTNptr NodePtr, Last_Nod_Sav;
Cell *trieinstr_unif_stk;
CPtr trieinstr_unif_stkptr;
Integer  trieinstr_unif_stk_size = DEFAULT_ARRAYSIZ;
#endif

/*
 * Variable ans_var_pos_reg is a pointer to substitution factor of an
 * answer in the heap.  It is used and set in function
 * variant_answer_search().  The name of this variable is originally
 * from VarPosReg, a variable used in variant_call_search() to save
 * the substitution factor of the call.
 */
#ifndef MULTI_THREAD
CPtr	ans_var_pos_reg;
#endif


//int instr_flag = 0;    // Used for switching on PIL_TRACE
// CPtr hreg_pos; // for debugging iso incremental tables.  Can be removed once these are stable.

//#define MULTI_THREAD_LOGGING
#ifdef MULTI_THREAD_LOGGING
/* To help debug multithreaded applications: 
Creates a log-file for each thread, and
Logs calls and executes to it.
*/
FILE **th_log_file = NULL;
int *th_log_cnt = NULL;

void open_th_log_file(int tid) {
  char fname[100];
  sprintf(fname,"temp_th_log_file_%d",tid);
  th_log_file[THREAD_ENTRY(tid)] = fopen(fname,"w");
  return;
}

void log_rec(CTXTdeclc Psc psc, char *ctype) {
    if (!th_log_file) 
    {	th_log_file = mem_calloc(max_threads_glc, sizeof(FILE *), OTHER_SPACE);
    	th_log_cnt = mem_calloc(max_threads_glc, sizeof(int), OTHER_SPACE);
    }
    if (!th_log_file[xsb_thread_id]) open_th_log_file(xsb_thread_id);
    fprintf(th_log_file[xsb_thread_entry],"inst(%d,%s,'%s',%d).\n",++th_log_cnt[xsb_thread_entry],ctype,get_name(psc),get_arity(psc));
    return;
}
#endif

int wam_initialized = FALSE ;

/*----------------------------------------------------------------------*/

#include "tr_delay.h"
#include "tr_code_xsb_i.h"

/*----------------------------------------------------------------------*/
/* indirect threading-related stuff                                     */

/* TLS junk that's been useful for debugging */

#ifdef DEBUG_VM 

int xctr = 0;

void quick_print_trail(int);
void check_stack_invariants(CTXTdecl);

/*
    printf("%d %s ereg %p ef %p topl %p > %x trreg %p trefeg %p topt %p opentabs %d \n", \
           xctr,(char *)inst_table[*t_pcreg][0],                        \
           t_ereg,efreg,(top_of_localstk),                              \
           (int)*(CPtr)0x3bebcc,trreg,trfreg,(top_of_trail),            \
           (int)(((int)COMPLSTACKBOTTOM - (int)top_of_complstk) / sizeof(struct completion_stack_frame))); \
    if (xctr % 1000 == 0)                                                           \
      quick_print_trail( (int)(top_of_trail - (CPtr *)tcpstack.low + 1));       \
  }
*/

#define debug_inst(t_pcreg, t_ereg) { \
  }

#define XSB_Debug_Instr                                    
/*  if (instr_flag) {							\
    printf("%d: %s\n",++xctr,(char *)inst_table[(int) *(lpcreg)][0]);	\
    debug_inst(CTXTc lpcreg, ereg);					\
    if (hreg < hreg_pos || hfreg < hreg_pos) printf("!1 hreg %p hfreg %p\n",hreg,hfreg); \
    if (hreg < hreg_pos) printf("!2 hreg %p %p\n",hreg,hfreg);		\
    }
*/

#else

#define XSB_Debug_Instr                                   

#endif

#ifdef PROFILE

#define XSB_Profile_Instr                                     \
    if (pflags[PROFFLAG]) {                                   \
      inst_table[(int) *(lpcreg)][sizeof(Cell)+1]             \
        = inst_table[(int) *(lpcreg)][sizeof(Cell)+1] + 1;    \
      if (pflags[PROFFLAG] > 1 && (int) *lpcreg == builtin)   \
        builtin_table[(int) *(lpcreg+3)][1] =                 \
  	  builtin_table[(int) *(lpcreg+3)][1] + 1;            \
    } 

#else

#define XSB_Profile_Instr

#endif

#define handle_xsb_profile_interrupt 				\
    if (asynint_val && (asynint_val & PROFINT_MARK)) {		\
      asynint_val &= ~PROFINT_MARK;				\
      log_prog_ctr(lpcreg);					\
    }								\

/* lfcastro: with INSN_BLOCKS, we use a block for each WAM instruction, 
   and define temporary variables locally; otherwise, temp variables are 
   global to the emuloop function.

   TLS: this experiment does not seem to have worked -- no other
   occurrences of INSN_BLOCKS in the system.*/

#ifdef INSN_BLOCKS

#define Def1op          register Cell op1;
#define Def1fop         register float fop2;
#define Def2ops         register Cell op1, op2;
#define Def2fops        register Cell op1; register float fop2;
#define Def3ops         register Cell op1,op2; register CPtr op3;
#define DefOps13        register Cell op1; register CPtr op3;

#define DefGlobOps

#else

#define Def1op
#define Def1fop
#define Def2ops
#define Def2fops
#define Def3ops
#define DefOps13

#define DefGlobOps register Cell op1,op2; register CPtr op3; float fop2;

#endif

/* lfcastro: with JUMPTABLE_EMULOOP, we use GCC's first-order labels to
   create a jumptable for the WAM instructions of emuloop(); otherwise 
   a switch statement is used. */

#ifdef JUMPTABLE_EMULOOP

static void *instr_addr_table[256];
//int print_instr = 0;
//if (print_instr) {print_inst(stddbg, lpcreg); printf(" ere %p\n",ereg); } 

#define XSB_End_Instr()                                      \
                   XSB_Debug_Instr                           \
                   XSB_Profile_Instr                         \
		   goto *instr_addr_table[(byte)*lpcreg];          \
		   }


#define XSB_Next_Instr()                                     \
                   do {                                      \
                      XSB_Debug_Instr                        \
                      XSB_Profile_Instr                      \
                      goto *instr_addr_table[(byte)*lpcreg];       \
                   } while(0)


#define XSB_Start_Instr_Chained(Instr,Label)                 \
        Label: 

#define XSB_Start_Instr(Instr,Label)                         \
        Label: {
		   


#else /* no threading */

#define XSB_Next_Instr()              goto contcase

#define XSB_End_Instr()               goto contcase; }

#define XSB_Start_Instr_Chained(Instr,Label)                 \
        case Instr:

#define XSB_Start_Instr(Instr,Label)                         \
        case Instr: { 

#endif

/*----------------------------------------------------------------------*/

#define get_axx         (lpcreg[1])
#define get_vxx         (ereg-(Cell)lpcreg[1])
#define get_rxx         (rreg+lpcreg[1])

#define get_xax         (lpcreg[2])
#define get_xvx         (ereg-(Cell)lpcreg[2])
#define get_xrx         (rreg+lpcreg[2])

#define get_xxa         (lpcreg[3])
#define get_xxv         (ereg-(Cell)lpcreg[3])
#define get_xxr         (rreg+lpcreg[3])

#define get_xxxl        (*(CPtr)(lpcreg+sizeof(Cell)))
#define get_xxxs        (*(CPtr)(lpcreg+sizeof(Cell)))
#define get_xxxc        (*(CPtr)(lpcreg+sizeof(Cell)))
#define get_xxxn        (*(CPtr)(lpcreg+sizeof(Cell)))
#define get_xxxg        (*(CPtr)(lpcreg+sizeof(Cell)))
#define get_xxxi        (*(CPtr)(lpcreg+sizeof(Cell)))
#define get_xxxf        (*(float *)(lpcreg+sizeof(Cell)))

#define get_xxxxi       (*(CPtr)(lpcreg+sizeof(Cell)*2))
#define get_xxxxl       (*(CPtr)(lpcreg+sizeof(Cell)*2))

#define Op1(Expr)       op1 = (Cell)Expr
#define Op2(Expr)       op2 = (Cell)Expr
#define Op2f(Expr)      fop2 = (float)Expr
#define Op3(Expr)       op3 = (CPtr)Expr

#define Register(Expr)  (cell(Expr))
#define Variable(Expr)  (cell(Expr))

#define size_none       0
#define size_xxx        1
#define size_xxxX       2
#define size_xxxXX      3

#define ADVANCE_PC(InstrSize)  (lpcreg += InstrSize*sizeof(Cell))

/* Be sure that flag only has the following two values.	*/

#define WRITE		1
#define READFLAG	0

/* TLS Macro does not appear to be used */
#ifdef USE_BP_LPCREG
#define POST_LPCREG_DECL asm ("bp")
#else
#define POST_LPCREG_DECL
#endif

/*----------------------------------------------------------------------*/
/* The following macros work for all CPs.  Make sure this remains	*/
/* the case...								*/
/*----------------------------------------------------------------------*/

#define Fail1 lpcreg = cp_pcreg(breg)
//#define Fail1 do {lpcreg = cp_pcreg(breg); printf("fail (h:%p,e:%p,pc:%p,b:%p,hb:%p)\n",hreg,ereg,lpcreg,breg,hbreg);}while(0)

#define restore_trail_condition_registers(breg) \
      if (*breg != (Cell) &check_complete_inst) { \
	ebreg = cp_ebreg(breg); \
	hbreg = cp_hreg(breg); \
      } 

/*----------------------------------------------------------------------*/

extern int  builtin_call(CTXTdeclc byte), unifunc_call(CTXTdeclc int, CPtr);
extern Cell builtin_table[BUILTIN_TBL_SZ][2];
extern Pair build_call(CTXTdeclc Psc);

extern int is_proper_list(Cell term);
extern int is_most_general_term(Cell term);
extern int is_number_atom(Cell term);
extern int ground(Cell term);
extern int is_ground_subgoal(VariantSF);

extern void log_prog_ctr(byte *);

#ifndef MULTI_THREAD
xsbBool neg_delay;
int  xwammode, level_num;
#endif

#ifdef DEBUG_VM
int  xctr;
#endif

int  ctrace_ctr=0;

/*----------------------------------------------------------------------*/

#include "schedrev_xsb_i.h"

#ifndef LOCAL_EVAL 
#include "wfs_xsb_i.h" 
#endif 
#include "complete_local.h"

/*----------------------------------------------------------------------*/

/* place for a meaningful message when segfault is detected */
char *xsb_default_segfault_msg =
     "\n++Memory violation occurred during evaluation.\n++Please report this problem using the XSB bug tracking system accessible from\n++\t http://sourceforge.net/projects/xsb\n++Please supply the steps necessary to reproduce the bug.\n";

extern int xsb_profiling_enabled;
extern Psc psc_from_code_addr(byte *);

Integer length_dyntry_chain(byte *codeptr) {
  Integer chainlen = 0;
  while (codeptr != NULL) {
    switch (*codeptr) {
    case dyntrymeelse:
    case dynretrymeelse:
      chainlen++;
      codeptr = (*((byte **)(codeptr)+1));
      break;
    default:
      return ++chainlen;
    }
  }
  return chainlen;
}


static inline xsbBool occurs_in_list_or_struc(Cell Var, Cell Term) {
 rec_occurs_in:
  XSB_Deref(Term);
  //  printf("enter occurs_in_list_or_struc %p %p\n",Var,Term);

  switch (cell_tag(Term)) {
  case XSB_ATTV: 
  case XSB_REF: 
  case XSB_REF1: 
  case XSB_INT:
  case XSB_STRING:
  case XSB_FLOAT:
    return FALSE;
  case XSB_LIST: {
    if (Var == (Cell)clref_val(Term) || Var == (Cell)((CPtr)clref_val(Term)+1)) return TRUE;
    if (occurs_in_list_or_struc(Var,get_list_head(Term))) return TRUE;
    Term = get_list_tail(Term);
    goto rec_occurs_in;
  }
  case XSB_STRUCT: {
    int i, arity;
    arity = get_arity(get_str_psc(Term));
    if (arity == 0) return FALSE;
    if (Var > (Cell)clref_val(Term) && Var <= (Cell)((CPtr)clref_val(Term)+arity)) return TRUE;
    for (i = 1; i < arity; i++) {
      if (occurs_in_list_or_struc(Var,get_str_arg(Term,i))) return TRUE;
    }
    Term = get_str_arg(Term,arity);
    goto rec_occurs_in;
  }
  }
  return TRUE;  
  }

#ifndef MULTI_THREAD
jmp_buf xsb_abort_fallback_environment;
#endif

char *xsb_segfault_message;

/*======================================================================*/
/* the main emulator loop.						*/
/*======================================================================*/

/*
 * The WAM instructions are aligned with word (4 bytes on 32-bit machines,
 * or 8-byte on 64-bit machines), the shortest instructions (like fail)
 * take one word, and the longest ones take three words (like
 * switchon3bound).  If an instruction takes more than one word, then the
 * 2nd (or 3rd) word always contains an operand that takes one word.  The
 * one-word operands can be (see file emu/inst_xsb.h):
 *
 * 	L - label
 * 	S - structure symbol
 * 	C - constant symbol
 * 	N - number
 * 	G - string
 * 	I - 2nd & 3rd arguments of switchonbound
 * 	F - floating point number
 *
 * The opcode of all instructions takes the first byte in the first word.
 * The rest 3 bytes contain operands that needs only one byte.  These
 * one-byte operands can be:
 *
 * 	P - pad, not used
 * 	A - one byte number
 * 	V - variable offset
 * 	R - register number
 *
 * (In 64-bit machines there are 4 bytes of extra padding space for each 
 *  instruction)
 */

int emuloop(CTXTdeclc byte *startaddr)
{
  register CPtr rreg;
  register byte *lpcreg POST_LPCREG_DECL;
  DefGlobOps
  byte flag = READFLAG;  	/* read/write mode flag */
  int  restore_type;	/* 0 for retry restore; 1 for trust restore */ 
  //FILE *logfile = fopen("XSB_LOGFILE.txt","w");
#ifdef MULTI_THREAD
    int (*fp)();
#endif
#if (defined(GC) && defined(GC_TEST))
/* Used only in the garbage collection test; does not affect emulator o/w */
#define GC_INFERENCES 66 /* make sure the garbage collection test is hard */
  static int infcounter = 0;
#endif
  //  jmp_buf xsb_eval_environment; //unused

  wam_initialized = TRUE ;

  xsb_segfault_message = xsb_default_segfault_msg;
  rreg = reg; /* for SUN (TLS ???) */

#ifdef JUMPTABLE_EMULOOP

#define XSB_INST(INum,Instr,Label,d1,d2,d3,d4) \
        instr_addr_table[INum] = && Label
#include "xsb_inst_list.h"

#endif

  if(setjmp(xsb_abort_fallback_environment)) {
    lpcreg = pcreg ;
    /*
    * Short circuit untrailing to avoid possible seg faults in
    * switch_envs.
    */
    trreg = cp_trreg(breg);
    /* Restore the default signal handling */
    signal(SIGSEGV, xsb_default_segfault_handler);
   } else 
    lpcreg = startaddr;  /* first instruction of entire engine */
#ifdef JUMPTABLE_EMULOOP
  XSB_Next_Instr();
#else
contcase:     /* the main loop */
#ifdef DEBUG_VM
  if (flags[PIL_TRACE]) debug_inst(CTXTc lpcreg, ereg);
  xctr++;
#endif
#ifdef PROFILE
  if (pflags[PROFFLAG]) {
    inst_table[(int) *(lpcreg)][sizeof(Cell)+1]
      = inst_table[(int) *(lpcreg)][sizeof(Cell)+1] + 1;
    if (pflags[PROFFLAG] > 1 && (int) *lpcreg == builtin) 
      builtin_table[(int) *(lpcreg+3)][1] = 
	builtin_table[(int) *(lpcreg+3)][1] + 1;
  }
#endif
  //fprintf(logfile,"%x\n",*lpcreg);
  switch (*lpcreg) {
#endif
    
  XSB_Start_Instr(getpvar,_getpvar)  /* PVR */
    Def2ops
    Op1(Variable(get_xvx));
    Op2(Register(get_xxr));
    ADVANCE_PC(size_xxx);
   /* trailing is needed here because this instruction can also be
       generated *after* the occurrence of the first call - kostis */
    bind_copy((CPtr)op1, op2);      /* In WAM bld_copy() */
  XSB_End_Instr()

  XSB_Start_Instr(getkpvars,_getkpvars)  /* AVR */
    CPtr vaddr, vval, raddr;
    Cell rval;
    int k;
    k = get_axx;
    vaddr = get_xvx;
    raddr = get_xxr;
    ADVANCE_PC(size_xxx);
    /* check for trailing only once */
    vval = cell((CPtr *)vaddr);
    if (conditional(vval)) {
      while (k-- > 0) {
	vval = cell((CPtr *)vaddr--);
	rval = cell(raddr++);
	// bind_copy(vval,rval);
	pushtrail0(vval,rval);
	*vval = rval;
      }
    } else {
      while (k-- > 0) {
	vval = cell((CPtr *)vaddr--);
	rval = cell(raddr++);
	// bld_copy(vval,rval);
	*vval = rval;
      }
    }
  XSB_End_Instr()

  XSB_Start_Instr(getpval,_getpval) /* PVR */
    Def2ops
    Op1(Variable(get_xvx));
    Op2(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    unify_xsb(_getpval);
 XSB_End_Instr()

  XSB_Start_Instr(getstrv,_getstrv) /* PPV-S */
    Def2ops
    Op1(Variable(get_xxv));
    Op2(get_xxxs);
    ADVANCE_PC(size_xxxX);
    nunify_with_str(op1,op2);
  XSB_End_Instr()

  XSB_Start_Instr(gettval,_gettval) /* PRR */
    Def2ops
    Op1(Register(get_xrx));
    Op2(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    unify_xsb(_gettval);
  XSB_End_Instr()

  XSB_Start_Instr(getcon,_getcon) /* PPR-C */
    Def2ops
    Op1(Register(get_xxr));
    Op2(get_xxxc);
    ADVANCE_PC(size_xxxX);
    nunify_with_con(op1,op2);
  XSB_End_Instr()

  XSB_Start_Instr(getinternstr,_getinternstr) /* PPR-C */
    Def2ops
    Op1(Register(get_xxr));
    Op2(get_xxxc);
    ADVANCE_PC(size_xxxX);
    nunify_with_internstr(op1,op2);
  XSB_End_Instr()

  XSB_Start_Instr(getnil,_getnil) /* PPR */
    Def1op
    Op1(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    nunify_with_nil(op1);
  XSB_End_Instr()	

  XSB_Start_Instr(getstr,_getstr) /* PPR-S */
    Def2ops
    Op1(Register(get_xxr));
    Op2(get_xxxs);
    ADVANCE_PC(size_xxxX);
    nunify_with_str(op1,op2);
  XSB_End_Instr()

  XSB_Start_Instr(getlist,_getlist) /* PPR */
    Def1op
    Op1(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    nunify_with_list_sym(op1);
  XSB_End_Instr()

     /* TLS: so much work for such a little function! */
  XSB_Start_Instr(xorreg,_xorreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1); 
    XSB_Deref(op2);
    if (isointeger(op1)) {
      if (isointeger(op2)) {
        Integer temp = (oint_val(op2)) ^ (oint_val(op1));
        bld_oint(op3, temp); 
      } else {
	FltInt fiop2;
	if (xsb_eval(CTXTc op2, &fiop2) && isfiint(fiop2)) {
	  Integer temp = (fiint_val(fiop2)) ^ (oint_val(op1));
	  bld_oint(op3, temp);
	} else {arithmetic_abort(CTXTc op2, "'><'", op1);}
      }
    } else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop2) ^ fiint_val(fiop1);
	bld_oint(op3,temp);
      } else { arithmetic_abort(CTXTc op2, "'><'", op1); }
    }
  XSB_End_Instr()

  XSB_Start_Instr(getattv,_getattv) /* PPR */
    Def1op
    Op1(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    nunify_with_attv(op1);
  XSB_End_Instr()

/* TLS: Need trailing here: for a full explanation, see "A Note on
   Trailing in the SLGWAM on my web page. */
  XSB_Start_Instr(unipvar,_unipvar) /* PPV */
    Def1op
    Op1(get_xxv);
    ADVANCE_PC(size_xxx);
    if (!flag) {	/* if (flag == READ) */
      /* also introduce trailing here - bmd & kostis
         was: bld_copy((CPtr)op1, *(sreg++)); */
      bind_copy((CPtr)op1, *(sreg));
      sreg++;
    } else {
      bind_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
    }
  XSB_End_Instr()

  XSB_Start_Instr(unipval,_unipval) /* PPV */
    Def2ops
    Op1(Variable(get_xxv));
    ADVANCE_PC(size_xxx);
    if (flag) { /* if (flag == WRITE) */
      nbldval(op1); 
    } 
    else {
      op2 = *(sreg++);
      unify_xsb(_unipval);
    } 
  XSB_End_Instr()

  XSB_Start_Instr(unitvar,_unitvar) /* PPR */
    Def1op
    Op1(get_xxr);
    ADVANCE_PC(size_xxx);
    if (!flag) {	/* if (flag == READ) */
      bld_copy((CPtr)op1, *(sreg++));
    }
    else {
      bld_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
    }
  XSB_End_Instr()

  XSB_Start_Instr(unitvar_getlist_uninumcon,_unitvar_getlist_uninumcon) /* RAA */
    Def1op
    int num;
    Op1(get_rxx);
    num = ((get_xax) << 8)+(get_xxa);
    ADVANCE_PC(size_xxx);
    if (!flag) {	/* if (flag == READ) */
      byte *next_instr = lpcreg;
      bld_copy((CPtr)op1, *(sreg++));
      op1 = Register((CPtr)op1);
      nunify_with_list_sym(op1);
      if (lpcreg == next_instr) { /* not failed */
	if (flag) { /* write */
	  new_heap_num(hreg, makeint(num));
	} else {
	  op1 = *(sreg++);
	  nunify_with_num(op1,num);
	}
      }
    }
    else {
      bld_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
      op1 = Register((CPtr)op1);
      bld_list((CPtr)op1, hreg);
      new_heap_num(hreg, makeint(num));
    }
  XSB_End_Instr()

    /* "avar" stands for anonymous variable */
  XSB_Start_Instr(uniavar,_uniavar) /* PPP */
    ADVANCE_PC(size_xxx);
    if (!flag) {	/* if (flag == READ) */
      sreg++;
    }
    else {
      new_heap_free(hreg);
    }
  XSB_End_Instr()

  XSB_Start_Instr(unitval,_unitval) /* PPR */
    Def2ops
    Op1(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    if (flag) { /* if (flag == WRITE) */
      nbldval(op1); 
      XSB_Next_Instr();
    }
    else {
      op2 = *(sreg++);
      unify_xsb(_unitval);
    } 
  XSB_End_Instr()

  XSB_Start_Instr(unicon,_unicon) /* PPP-C */
    Def2ops
    Op2(get_xxxc);
    ADVANCE_PC(size_xxxX);
    if (flag) {	/* if (flag == WRITE) */
      new_heap_string(hreg, (char *)op2);
    }
    else {  
      /* op2 already set */
      op1 = *(sreg++);
      nunify_with_con(op1,op2);
    }
  XSB_End_Instr()

  XSB_Start_Instr(uniinternstr,_uniinternstr) /* PPP-C */
    Def2ops
    Op2(get_xxxc);
    ADVANCE_PC(size_xxxX);
    if (flag) {	/* if (flag == WRITE) */
      new_heap_node(hreg, (Cell)op2);
    }
    else {  
      /* op2 already set */
      op1 = *(sreg++);
      nunify_with_internstr(op1,op2);
    }
  XSB_End_Instr()

  XSB_Start_Instr(uninil,_uninil) /* PPP */
    Def1op
    ADVANCE_PC(size_xxx);
    if (flag) {	/* if (flag == WRITE) */
      new_heap_nil(hreg);
    }
    else {
      op1 = *(sreg++);
      nunify_with_nil(op1);
    }
  XSB_End_Instr()

  XSB_Start_Instr(getnumcon,_getnumcon) /* PPR-B */
    Def2ops
    Op1(Register(get_xxr));
    Op2(get_xxxn);
    ADVANCE_PC(size_xxxX);
    nunify_with_num(op1,op2);
  XSB_End_Instr()

  XSB_Start_Instr(getfloat,_getfloat) /* PPR-F */
    //    printf("\nGETFLOAT ENTERED!\n");
    Def2fops
    Op1(Register(get_xxr));
    Op2f(get_xxxf);
    ADVANCE_PC(size_xxxX);
    nunify_with_float_get(op1,fop2);
    //printf("\nGETFLOAT LEFT!\n");
  XSB_End_Instr()

  XSB_Start_Instr(putnumcon,_putnumcon) /* PPR-B */
    Def2ops
    Op1(get_xxr);
/*      Op2(get_xxxn); */
    op2 = *(pw)(lpcreg+sizeof(Cell));
    ADVANCE_PC(size_xxxX);
    bld_oint((CPtr)op1, op2);
  XSB_End_Instr()

  XSB_Start_Instr(putfloat,_putfloat) /* PPR-F */
    //    printf("\nPUTFLOAT ENTERED!\n");
    Def2fops
    Op1(get_xxr);
    Op2f(get_xxxf);
    ADVANCE_PC(size_xxxX);
    //    bld_float_tagged((CPtr)op1, fop2);
    bld_boxedfloat(CTXTc (CPtr)op1, fop2);
    //printf("\nPUTFLOAT DONE!\n");
  XSB_End_Instr()

#ifdef BITS64
  XSB_Start_Instr(getdfloat,_getdfloat) /* PPR-D */
    register Cell op1;
    union {
      struct {
	Integer int1;
      } intp;
      double fltp;
    } cvtr;
    Op1(Register(get_xxr));
    cvtr.intp.int1 = *(pw)(lpcreg+sizeof(Cell));
    ADVANCE_PC(size_xxxXX);
    //    printf("getdfloat: %2.16f\n",cvtr.fltp);
    nunify_with_float_get(op1,(Float)cvtr.fltp);
  XSB_End_Instr()

  XSB_Start_Instr(putdfloat,_putdfloat) /* PPR-D */
    register Cell op1;
    union {
      struct {
	Integer int1;
      } intp;
      double fltp;
    } cvtr;
    Op1(get_xxr);
    cvtr.intp.int1 = *(pw)(lpcreg+sizeof(Cell));
    ADVANCE_PC(size_xxxXX);
    bld_boxedfloat(CTXTc (CPtr)op1, (Float)cvtr.fltp);
  XSB_End_Instr()

#else
  XSB_Start_Instr(getdfloat,_getdfloat) /* PPR-D */
    register Cell op1;
    union {
      struct {
	long int1,int2; // ???? for bits64
      } intp;
      double fltp;
    } cvtr;
    Op1(Register(get_xxr));
    cvtr.intp.int1 = (long)(*(pw)(lpcreg+sizeof(Cell)));
    cvtr.intp.int2 = (long)(*(pw)(lpcreg+2*sizeof(Cell)));
    ADVANCE_PC(size_xxxXX);
    //    printf("getdfloat: %2.16f\n",cvtr.fltp);
    nunify_with_float_get(op1,cvtr.fltp);
  XSB_End_Instr()

  XSB_Start_Instr(putdfloat,_putdfloat) /* PPR-D */
    register Cell op1;
    union {
      struct {
	long int1;
	long int2;
      } intp;
      double fltp;
    } cvtr;
    Op1(get_xxr);
    cvtr.intp.int1 = *(pw)(lpcreg+sizeof(Cell));
    cvtr.intp.int2 = *(pw)(lpcreg+2*sizeof(Cell));
    ADVANCE_PC(size_xxxXX);
    bld_boxedfloat(CTXTc (CPtr)op1, (Float)cvtr.fltp);
  XSB_End_Instr()
#endif

  XSB_Start_Instr(putpvar,_putpvar) /* PVR */
    Def2ops
    Op1(get_xvx);
    Op2(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_free((CPtr)op1);
    bld_ref((CPtr)op2, (CPtr)op1);
  XSB_End_Instr()

    /* does not dereference op1 (as opposed to putdval) */
  XSB_Start_Instr(putpval,_putpval) /* PVR */
    DefOps13
    Op1(get_xvx);
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_copy(op3, *((CPtr)op1));
  XSB_End_Instr()

  XSB_Start_Instr(puttvar,_puttvar) /* PRR */
    Def2ops
    Op1(get_xrx);
    Op2(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_ref((CPtr)op1, hreg);
    bld_ref((CPtr)op2, hreg);
    new_heap_free(hreg); 
  XSB_End_Instr()

/* TLS: Need trailing here: for a full explanation, see "A Note on
   Trailing in the SLGWAM on my web page. */
  XSB_Start_Instr(putstrv,_putstrv) /*  PPV-S */
    Def2ops
    Op1(get_xxv);
    Op2(get_xxxs);
    ADVANCE_PC(size_xxxX);
    bind_cs((CPtr)op1, (Pair)hreg);
    new_heap_functor(hreg, (Psc)op2); 
  XSB_End_Instr()

  XSB_Start_Instr(putcon,_putcon) /* PPR-C */
    Def2ops
    Op1(get_xxr);
    Op2(get_xxxc);
    ADVANCE_PC(size_xxxX);
    //printf("PUTCON entered! String is %s\n", (char *) op2);
    bld_string((CPtr)op1, (char *)op2);
  XSB_End_Instr()

  XSB_Start_Instr(putnil,_putnil) /* PPR */
    Def1op
    Op1(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_nil((CPtr)op1);
  XSB_End_Instr()

/* doc tls -- differs from putstrv since it pulls from a register.
   Thus the variable is already initialized.  */
  XSB_Start_Instr(putstr,_putstr) /* PPR-S */
    Def2ops
    Op1(get_xxr);
    Op2(get_xxxs);
    ADVANCE_PC(size_xxxX);
    bld_cs((CPtr)op1, (Pair)hreg);
    new_heap_functor(hreg, (Psc)op2); 
  XSB_End_Instr()

  XSB_Start_Instr(putlist,_putlist) /* PPR */
    Def1op
    Op1(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_list((CPtr)op1, hreg);
  XSB_End_Instr()

  XSB_Start_Instr(putattv,_putattv) /* PPR */
    Def1op
    Op1(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_attv((CPtr)op1, hreg);
    new_heap_free(hreg);
  XSB_End_Instr()

/* TLS: Need trailing here: for a full explanation, see "A Note on
   Trailing in the SLGWAM on my web page. */
  XSB_Start_Instr(bldpvar,_bldpvar) /* PPV */
    Def1op
    Op1(get_xxv);
    ADVANCE_PC(size_xxx);
    bind_ref((CPtr)op1, hreg); /* trailing is needed: if o/w see ai_tests */
    new_heap_free(hreg);
  XSB_End_Instr()

  XSB_Start_Instr(bldpval,_bldpval) /* PPV */
    Def1op
    Op1(Variable(get_xxv));
    ADVANCE_PC(size_xxx);
    nbldval(op1);
  XSB_End_Instr()

  XSB_Start_Instr(bldtvar,_bldtvar) /* PPR */
    Def1op
    Op1(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_ref((CPtr)op1, hreg);
    new_heap_free(hreg);
  XSB_End_Instr()

  XSB_Start_Instr(bldavar,_bldavar) /* PPR */
    ADVANCE_PC(size_xxx);
    new_heap_free(hreg);
  XSB_End_Instr()

  XSB_Start_Instr(bldtval,_bldtval) /* PPR */
    Def1op
    Op1(Register(get_xxr));
    ADVANCE_PC(size_xxx);
    nbldval(op1);
  XSB_End_Instr()

  XSB_Start_Instr(bldtval_putlist_bldnumcon,_bldtval_putlist_bldnumcon) /* RAA */
    Def2ops   /* get extra argument to use as temp */
    int num;
    Op1(get_rxx);
    num = ((get_xax) << 8)+(get_xxa);
    ADVANCE_PC(size_xxx);
    op2 = Register((CPtr)op1);
    nbldval(op2);
    bld_list((CPtr)op1, hreg);
    new_heap_num(hreg, (Integer)makeint(num));
  XSB_End_Instr()

  XSB_Start_Instr(bldtvar_list_numcon,_bldtvar_list_numcon) /* RAA */
    int num;
    Cell h;
    num = ((get_xax) << 8)+(get_xxa);
    ADVANCE_PC(size_xxx);
    bld_ref(&h,hreg);
    new_heap_free(hreg);
    bld_list((CPtr)h, hreg);
    new_heap_num(hreg, (Integer)makeint(num));
  XSB_End_Instr()

  XSB_Start_Instr(bldcon,_bldcon) /* PPP-C */
    Def1op
    Op1(get_xxxc);
    ADVANCE_PC(size_xxxX);
    new_heap_string(hreg, (char *)op1);
  XSB_End_Instr()

  XSB_Start_Instr(bldinternstr,_bldinternstr) /* PPP-C */
    Def1op
    Op1(get_xxxc);
    ADVANCE_PC(size_xxxX);
    new_heap_node(hreg, (Cell)op1);
  XSB_End_Instr()

  XSB_Start_Instr(bldnil,_bldnil) /* PPP */
    ADVANCE_PC(size_xxx);
    new_heap_nil(hreg);
  XSB_End_Instr()

  XSB_Start_Instr(getlist_tvar_tvar,_getlist_tvar_tvar) /* RRR */
    Def3ops
    Op1(Register(get_rxx));
    Op2(get_xrx);
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    XSB_Deref(op1);
    if (islist(op1)) {
      sreg = clref_val(op1);
      op1 = (Cell)op2;
      bld_ref((CPtr)op1, *(sreg));
      op1 = (Cell)op3;
      bld_ref((CPtr)op1, *(sreg+1));
    } else if (isref(op1)) {
      bind_list((CPtr)(op1), hreg);
      op1 = (Cell)op2;
      bld_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
      op1 = (Cell)op3;
      bld_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
     } else if (isattv(op1)) {
      attv_dbgmsg(">>>> getlist_tvar_tvar: ATTV interrupt needed\n");
      add_interrupt(CTXTc op1, makelist(hreg));
      op1 = (Cell)op2;
      bld_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
      op1 = (Cell)op3;
      bld_ref((CPtr)op1, hreg);
      new_heap_free(hreg);
    }
    else Fail1;
  XSB_End_Instr()	/* end getlist_tvar_tvar */

  XSB_Start_Instr(uninumcon,_uninumcon) /* PPP-B */
    Def2ops
    Op2(get_xxxn); /* num in op2 */
    ADVANCE_PC(size_xxxX);
    if (flag) {	/* if (flag == WRITE) */
      new_heap_num(hreg, makeint(op2));
    }
    else {  /* op2 set */
      op1 = *(sreg++);
      nunify_with_num(op1,op2);
    }
  XSB_End_Instr()

  XSB_Start_Instr(unifloat,_unifloat) /* PPPF */
    //    printf("UNIFLOAT ENTERED\n");
    Def2fops
    Op2f(get_xxxf); /* num in fop2 */
    ADVANCE_PC(size_xxxX);
    if (flag) {	/* if (flag == WRITE) */
      new_heap_float(hreg, makefloat(fop2));
    }
    else {  /* fop2 set */
      op1 = cell(sreg++);
      nunify_with_float(op1,fop2);
    }
    //printf("UNIFLOAT LEFT\n");
  XSB_End_Instr()

  XSB_Start_Instr(bldnumcon,_bldnumcon) /* PPP-B */
    Def1op
    Op1(get_xxxn);  /* num to op2 */
    ADVANCE_PC(size_xxxX);
    new_heap_num(hreg, (Integer)makeint(op1));
  XSB_End_Instr()

  XSB_Start_Instr(bldfloat,_bldfloat) /* PPP-F */
    printf("BLDFLOAT ENTERED\n");
    Def1fop
    Op2f(get_xxxf); /* num to fop2 */
    ADVANCE_PC(size_xxxX);
    new_heap_float(hreg, makefloat(fop2));
    //printf("BLDFLOAT LEFT\n");
  XSB_End_Instr()

  XSB_Start_Instr(trymeelse,_trymeelse) /* PPA-L */
    Def2ops
    Op1(get_xxa);
    Op2(get_xxxl);
    if (attv_pending_interrupts) printf("Failed Assertion trymeelse\n");
#if 0
    { 
      Psc mypsc = *(CPtr)(cpreg-4);
      if (mypsc)
	if (get_type(mypsc) == T_PRED) {
	  fprintf(stddbg,"creating_cp(trymeelse(%s/%d), %p).\n",
		  get_name(mypsc), get_arity(mypsc), breg);
	}
    }
#endif
    ADVANCE_PC(size_xxxX);
    SUBTRYME
  XSB_End_Instr()

  XSB_Start_Instr(dyntrymeelse,_dyntrymeelse) /* PPA-L */
    Def2ops
    Op1(get_xxa);
    Op2(get_xxxl);
/* In the multi-threaded system a signal can be issued be another thread
   with thread signal anytime */
#ifndef MULTI_THREAD
    if (attv_pending_interrupts) printf("Failed assertion dyntrymeelse\n");
#endif
    ADVANCE_PC(size_xxxX);
    SUBTRYME
#ifdef MULTI_THREAD
    if (i_have_dyn_mutex) {
      SYS_MUTEX_UNLOCK(MUTEX_DYNAMIC);
      i_have_dyn_mutex = 0;
    }
#endif
  XSB_End_Instr()

  XSB_Start_Instr(retrymeelse,_retrymeelse) /* PPA-L */
    Def1op
    Op1(get_xxa);
    cp_pcreg(breg) = (byte *)get_xxxl;
    restore_type = 0;
    ADVANCE_PC(size_xxxX);
    RESTORE_SUB
  XSB_End_Instr()

      /* TLS: added to distinguish dynamic from static choice points when 
	 gc-ing retracted clauses. */ 

  XSB_Start_Instr(dynretrymeelse,_dynretrymeelse) /* PPA-L */
    Def1op
    Op1(get_xxa);
    cp_pcreg(breg) = (byte *)get_xxxl;
    restore_type = 0;
    ADVANCE_PC(size_xxxX);
    RESTORE_SUB
  XSB_End_Instr()

      /* TLS: according to David.  It may be that a call to a
       *  predicate P performs a lot of shallow backtracking esp. to
       *  facts. If so, the interrupt might not be handled until the
       *  engine is not executing P any more.  Putting the handler in
       *  trusts means that any interrupt posted during the
       *  backtracking will be caught, and thus gives the profiler a
       *  better chance of accurately reflecting where the time is
       *  spent. */

  XSB_Start_Instr(trustmeelsefail,_trustmeelsefail) /* PPA */
    Def1op
    Op1(get_xxa);
    restore_type = 1;
    handle_xsb_profile_interrupt;
    ADVANCE_PC(size_xxx);
    RESTORE_SUB
  XSB_End_Instr()

  XSB_Start_Instr(try,_try) /* PPA-L */
    Def2ops
    Op1(get_xxa);
    op2 = (Cell)((Cell)lpcreg + sizeof(Cell)*2);
/* In the multi-threaded system a signal can be issued be another thread
   with thread signal anytime.
   TLS: changed so that we check this assertion only if we're in debug mode
 */
#if !defined(MULTI_THREAD) && NON_OPT_COMPILE
    if (attv_pending_interrupts) printf("Failed assertion try\n");
#endif
#if 0
    { 
      Psc mypsc = *(CPtr)(cpreg-4);
      if (mypsc)
	if (get_type(mypsc) == T_PRED) {
	  fprintf(stddbg,"creating_cp(try(%s/%d), %p).\n",
		  get_name(mypsc), get_arity(mypsc), breg);
	}
    }
#endif
    lpcreg = *(pb *)(lpcreg+sizeof(Cell)); /* = *(pointer to byte pointer) */
    SUBTRYME
  XSB_End_Instr()

  XSB_Start_Instr(retry,_retry) /* PPA-L */
    Def1op
    Op1(get_xxa);
    cp_pcreg(breg) = lpcreg+sizeof(Cell)*2;
    lpcreg = *(pb *)(lpcreg+sizeof(Cell));
    restore_type = 0;
    RESTORE_SUB
  XSB_End_Instr()

  XSB_Start_Instr(trust,_trust) /* PPA-L */
    Def1op
    Op1(get_xxa);
    handle_xsb_profile_interrupt;
    lpcreg = *(pb *)(lpcreg+sizeof(Cell));
    restore_type = 1;
    RESTORE_SUB
  XSB_End_Instr()

      /* Used for tabling: puts a pointer to the subgoal_frame in the 
	 local environment for a tabled subgoal */	 
  XSB_Start_Instr(getVn,_getVn) /* PPV */
    Def1op
    Op1(get_xxv);
    ADVANCE_PC(size_xxx);
    cell((CPtr)op1) = (Cell)tcp_subgoal_ptr(breg);
  XSB_End_Instr()

  XSB_Start_Instr(getpbreg,_getpbreg) /* PPV */
    Def1op
    Op1(get_xxv);
    ADVANCE_PC(size_xxx);
    bld_oint((CPtr)op1, ((pb)tcpstack.high - (pb)breg));
  XSB_End_Instr()

  XSB_Start_Instr(gettbreg,_gettbreg) /* PPR */
    Def1op
    Op1(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_oint((CPtr)op1, ((pb)tcpstack.high - (pb)breg));
  XSB_End_Instr()

    /* Assertion commented out in MT engine because inter-thread
       signals use interrupt stack. */
  XSB_Start_Instr(putpbreg,_putpbreg) /* PPV */
    Def1op
    Op1(Variable(get_xxv));
/* In the multi-threaded system a signal can be issued be another thread
   with thread signal anytime */
//#ifndef MULTI_THREAD
    if (attv_pending_interrupts) printf("Failed assertion putpbreg\n");
    //#endif
    ADVANCE_PC(size_xxx);
    cut_code(op1);
  XSB_End_Instr()

  XSB_Start_Instr(putpbreg_ci,_putpbreg_ci) /* VAA variable,reserved_regs,arsize */
    Def1op
    //    printf("putpbreg_ci at %p(%lx,%d)\n",lpcreg,isnonvar(cell((CPtr)glstack.low)),get_axx);
    if (attv_pending_interrupts) {
      int reserved_regs,arsize;
      reserved_regs = get_xax;
      arsize = get_xxa;
      allocate_env_and_call_check_ints(reserved_regs,arsize);
    } else {
      Op1(Variable(get_vxx));
      ADVANCE_PC(size_xxx);
      cut_code(op1);
    }
  XSB_End_Instr()

  XSB_Start_Instr(puttbreg,_puttbreg) /* PPR */
    Def1op
    Op1(Register(get_xxr));
/* In the multi-threaded system a signal can be issued be another thread
   with thread signal anytime */
#ifndef MULTI_THREAD
    if (attv_pending_interrupts) printf("Failed assertion puttbreg\n");
#endif
    ADVANCE_PC(size_xxx);
    cut_code(op1);
  XSB_End_Instr()

  XSB_Start_Instr(puttbreg_ci,_puttbreg_ci) /* RAA register,reserved_regs,arsize */
    Def1op
    if (attv_pending_interrupts) {
      int reserved_regs,arsize;
      reserved_regs = get_xax;
      arsize = get_xxa;
      allocate_env_and_call_check_ints(reserved_regs,arsize);
    } else {
      Op1(Register(get_rxx));
      ADVANCE_PC(size_xxx);
      cut_code(op1);
    }
  XSB_End_Instr()

  XSB_Start_Instr(jumptbreg,_jumptbreg) /* PPR-L */	/* ??? */
    Def1op
    Op1(get_xxr);
    bld_oint((CPtr)op1, ((pb)tcpstack.high - (pb)breg));
    lpcreg = *(byte **)(lpcreg+sizeof(Cell));
#ifdef MULTI_THREAD
    if (i_have_dyn_mutex) xsb_abort("DYNAMIC MUTEX ERROR\n");
    SYS_MUTEX_LOCK(MUTEX_DYNAMIC);
    i_have_dyn_mutex = 1;
#endif
  XSB_End_Instr()

  XSB_Start_Instr(test_heap,_test_heap) /* PPA-N */
    Def2ops
    Op1(get_xxa); /* op1 = the arity of the procedure */
    Op2(get_xxxn);
    ADVANCE_PC(size_xxxX);
#ifdef GC_TEST
    if ((infcounter++ > GC_INFERENCES) || heap_local_overflow((Integer)op2))
      {
	infcounter = 0;
        fprintf(stddbg, ".");
#else
	//    printf("t_h %d %p\n",(ereg-hreg),hreg);
	if (heap_local_overflow((Integer)op2))
      {
#endif
        if (gc_heap(CTXTc (int)op1,FALSE)) { /* garbage collection potentially modifies hreg */
	  if (heap_local_overflow((Integer)op2)) {	
	    glstack_realloc(CTXTc resize_stack(glstack.size,(op2*sizeof(Cell))),(int)op1);
	    /*
	    if (pflags[STACK_REALLOC]) {
	      if (glstack_realloc(CTXTc resize_stack(glstack.size,(op2*sizeof(Cell))),(int)op1) != 0) {
		xsb_memory_error("memory","Cannot Expand Local and Global Stacks");
		xsb_basic_abort(local_global_exception);
	      }
	    } else {
	      xsb_warn(CTXTc "Reallocation is turned OFF !");
	      xsb_memory_error("memory","Cannot Expand Local and Global Stacks");
	    }
	    */
	  }
	}	/* are there any localy cached quantities that must be reinstalled ? */
      } else if (force_string_gc) {
	  gc_heap(CTXTc (int)op1,TRUE);
	}
  XSB_End_Instr()

  XSB_Start_Instr(switchonterm,_switchonterm) /* PPR-L-L */
    Def1op
    Op1(Register(get_xxr));
    XSB_Deref(op1);
    switch (cell_tag(op1)) {
    case XSB_INT:
    case XSB_STRING:
    case XSB_FLOAT:
      lpcreg = *(pb *)(lpcreg+sizeof(Cell));	    
      break;
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      ADVANCE_PC(size_xxxXX);
      break;
    case XSB_STRUCT:
      if (isboxedfloat(op1))
      {
          lpcreg = *(pb *)(lpcreg+sizeof(Cell));
          break;
      }
      if (get_arity(get_str_psc(op1)) == 0) {
	lpcreg = *(pb *)(lpcreg+sizeof(Cell));
	break;
      }
    case XSB_LIST:	/* include structure case here */
      lpcreg = *(pb *)(lpcreg+sizeof(Cell)*2); 
      break;
    }
  XSB_End_Instr()

#define struct_hash_value(op1) \
   (isboxedinteger(op1)?boxedint_val(op1): \
    (isboxedfloat(op1)?  \
     int_val(get_str_arg(op1,1)) ^ int_val(get_str_arg(op1,2)) ^ int_val(get_str_arg(op1,3)): \
     (Cell)get_str_psc(op1)))

  XSB_Start_Instr(switchonbound,_switchonbound) /* PPR-L-L */
    Def3ops
    /* op1 is register, op2 is hash table offset, op3 is modulus */
    Integer hash_temp;
    Op1(get_xxr);
    XSB_Deref(op1);
    switch (cell_tag(op1)) {
    case XSB_STRUCT: {
      Cell top1 = struct_hash_value(op1);
      //      if (isboxedfloat(op1)) printf("dfloat struct lookup: %2.14f, %0x, size=%d\n", boxedfloat_val(op1),top1,*(CPtr *)(lpcreg+sizeof(Cell)*2));
      op1 = top1;
    }
      break;
    case XSB_STRING:	/* We should change the compiler to avoid this test */
      op1 = (Cell)(isnil(op1) ? 0 : string_val(op1));
      break;
    case XSB_INT: 
      op1 = (Cell)int_val(op1);
      break;
    case XSB_FLOAT:  /* cvt to double and use that indexing.... */
#ifndef FAST_FLOATS
      {Float tempFloat = (Float)float_val(op1);
	op1 = ((ID_BOXED_FLOAT << BOX_ID_OFFSET ) | FLOAT_HIGH_16_BITS(tempFloat) )
	  ^ FLOAT_MIDDLE_24_BITS(tempFloat) ^ FLOAT_LOW_24_BITS(tempFloat);
      }
#else 
      op1 = (Cell)int_val(op1);
#endif
	//	printf("old float indexing: %0x %f\n",op1,tempFloat);
      break;
    case XSB_LIST:
      op1 = (Cell)(list_pscPair); 
      break;
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      lpcreg += 3 * sizeof(Cell);
      XSB_Next_Instr();
    }
    op2 = (Cell)(*(byte **)(lpcreg+sizeof(Cell)));
    op3 = *(CPtr *)(lpcreg+sizeof(Cell)*2);
    /* doc tls -- op2 + (op1%size)*4 */
    hash_temp = ihash((Cell)op1, (Cell)op3);
    if (hash_temp < 0) printf("Bad Hash6");
    lpcreg = 
      *(byte **)((byte *)op2 + hash_temp * sizeof(Cell));
      //      *(byte **)((byte *)op2 + ihash((Cell)op1, (Cell)op3) * sizeof(Cell));
  XSB_End_Instr()

/*******************************************************************
There are 3 "index" bytes in the switchon3bound instruction. (That's
the 3.)  Each one indicates one argument of a joint index (or compound
index?--I never remember this terminology).  So for a joint index of
1+2, the three bytes would be 1,2,0.  (If there are fewer than 3
argument positions, the remainder are set to 0.)

Star indexing, also uses switchon3bound, and it is indicated with the
argument position + 128.  So for example *(3) would be indicated with
a switchon3bound with index bytes: 131,0,0 (Where 131=128+3.)

So note that a star index can be a component of a joint index.
E.g. 1+ *(3)+2
would have index bytes: 1,131,2
It would be used if the first and second arguments have the main
functor symbol bound and the third argument had the first 5 symbols
(in depthfirst traversal) bound.

Note that this does mean that we can only index on the first 127
argument positions.
*******************************************************************/

  XSB_Start_Instr(switchon3bound,_switchon3bound) /* RRR-L-L */
    Def3ops
    int  i = 0;
    Integer j = 0;
    int indexreg[3];
    Cell opa[3]; 
    int index_max;
    /* op1 is register contents, op2 is hash table offset, op3 is modulus */
    indexreg[0] = get_axx;
    indexreg[1] = get_xax;
    indexreg[2] = get_xxa;

    if (*lpcreg == 0) { opa[0] = 0; }
    else opa[0] = Register((rreg + (indexreg[0] & 0x7f)));
    opa[1] = Register((rreg + (indexreg[1] & 0x7f)));
    opa[2] = Register((rreg + (indexreg[2] & 0x7f)));
    op2 = (Cell)(*(byte **)(lpcreg+sizeof(Cell)));
    op3 = *(CPtr *)(lpcreg+sizeof(Cell)*2); 
    /* This is not a good way to do this, but until we put retract into C,
       or add new builtins, it will have to do. */
    for (i = 0; i <= 2; i++) {
      if (indexreg[i] != 0) {
        if (indexreg[i] > 0x80) {
          int k, depth = 0;
          Cell *stk[MAXTOINDEX];
          int argsleft[MAXTOINDEX];
          stk[0] = &opa[i];
          argsleft[0] = 1;

	  index_max = (int) flags[MAXTOINDEX_FLAG];
          for (k = index_max; k > 0; k--) {
            if (depth < 0) break;
            op1 = *stk[depth];
            argsleft[depth]--;
            if (argsleft[depth] <= 0) depth--;
            else stk[depth]++;
	    XSB_Deref(op1);
	    switch (cell_tag(op1)) {
	    case XSB_FREE:
	    case XSB_REF1:
	    case XSB_ATTV:
	      ADVANCE_PC(size_xxxXX);
	      XSB_Next_Instr();
	    case XSB_INT: 
	    case XSB_FLOAT:	/* Yes, use int_val to avoid conversion problem */
	      op1 = (Cell)int_val(op1);
	      break;
	    case XSB_LIST:
              depth++;
              argsleft[depth] = 2;
              stk[depth] = clref_val(op1);
	      op1 = (Cell)(list_pscPair); 
	      break;
	    case XSB_STRUCT:
	      if (isboxedinteger(op1)) op1 = (Cell)boxedint_val(op1);
	      else if (isboxedfloat(op1)) 
		op1 = int_val(get_str_arg(op1,1)) ^
		  int_val(get_str_arg(op1,2)) ^
		  int_val(get_str_arg(op1,3));
	      else {
		depth++;
		argsleft[depth] = get_arity(get_str_psc(op1));
		stk[depth] = clref_val(op1)+1;
		op1 = (Cell)get_str_psc(op1);
		//op1 = struct_hash_value(op1); // already handled boxes
	      }
	      break;
	    case XSB_STRING:
	      op1 = (Cell)string_val(op1);
	      break;
            }
	    j += j + ihash(op1, (Integer)op3);
          }
      } else {
	op1 = opa[i];
	XSB_Deref(op1);
	switch (cell_tag(op1)) {
	case XSB_FREE:
	case XSB_REF1:
	case XSB_ATTV:
	  ADVANCE_PC(size_xxxXX);
	  XSB_Next_Instr();
	case XSB_INT: 
	case XSB_FLOAT:	/* Yes, use int_val to avoid conversion problem */
	  op1 = (Cell)int_val(op1);
	  break;
	case XSB_LIST:
	  op1 = (Cell)(list_pscPair); 
	  break;
	case XSB_STRUCT:
	  //	  op1 = (Cell)get_str_psc(op1);
	  op1 = struct_hash_value(op1);
	  break;
	case XSB_STRING:
	  op1 = (Cell)string_val(op1);
	  break;
	default:
	  xsb_error("Illegal operand in switchon3bound");
	  break;
        }
	j += j + (int)ihash(op1, (Integer)op3);
	}
      }
    }
    if (j < 0) j = -j;
    lpcreg = *(byte **)((byte *)op2 + ((j % (Integer)op3) * sizeof(Cell)));
  XSB_End_Instr()

  XSB_Start_Instr(switchonthread,_switchonthread) /* PPP-L */
#ifdef MULTI_THREAD
    Def1op
    Op1(get_xxxl);
    if (xsb_thread_entry > *((Integer *)op1+2)) Fail1;
    //    fprintf(stderr,"switchonthread to %p\n",(pb)(*((Integer *)op1+3+(th->tid))));
    if (!(lpcreg = (pb)(*((Integer *)op1+3+(xsb_thread_entry))))) Fail1;
#else
    xsb_exit("Not configured for Multithreading");
#endif
  XSB_End_Instr()

  XSB_Start_Instr(trymeorelse,_trymeorelse) /* PPA-L */
    Def2ops
    if (attv_pending_interrupts) {
      Op1(get_xxa);
      allocate_env_and_call_check_ints(0,op1);
    } else {
      Op1(0);
      Op2(get_xxxl);
#if 0
    { 
      Psc mypsc = *(CPtr)(cpreg-4);
      if (mypsc)
	if (get_type(mypsc) == T_PRED) {
	  fprintf(stddbg,"creating_cp(trymeorelse(%s/%d), %p).\n",
		  get_name(mypsc), get_arity(mypsc), breg);
	}
    }
#endif
      ADVANCE_PC(size_xxxX);
      cpreg = lpcreg; /* Another use of cpreg for inline try's for disjunctions */
      SUBTRYME
    }
  XSB_End_Instr()

  XSB_Start_Instr(retrymeorelse,_retrymeorelse) /* PPA-L */
    Def1op
    Op1(0);
    cp_pcreg(breg) = *(byte **)(lpcreg+sizeof(Cell));
    ADVANCE_PC(size_xxxX);
    restore_type = 0;
    RESTORE_SUB
  XSB_End_Instr()

  XSB_Start_Instr(trustmeorelsefail,_trustmeorelsefail) /* PPA */
    Def1op
    Op1(0);
    handle_xsb_profile_interrupt;
    ADVANCE_PC(size_xxx);
    restore_type = 1;
    RESTORE_SUB
  XSB_End_Instr()

  XSB_Start_Instr(dyntrustmeelsefail,_dyntrustmeelsefail) /* PPA-L, second word ignored */
    Def1op
    Op1(get_xxa);
    handle_xsb_profile_interrupt;
    ADVANCE_PC(size_xxxX);
    restore_type = 1;
    RESTORE_SUB
  XSB_End_Instr()

/*----------------------------------------------------------------------*/

#include "slginsts_xsb_i.h"

#include "tc_insts_xsb_i.h"

/*----------------------------------------------------------------------*/

  XSB_Start_Instr(term_comp,_term_comp) /* RRR */
    Def3ops
    Op1(get_rxx);
    Op2(get_xrx);
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_int(op3, compare(CTXTc (void *)op1, (void *)op2));
  XSB_End_Instr()

  XSB_Start_Instr(movreg,_movreg) /* PRR */
    Def2ops
    Op1(get_xrx);
    Op2(get_xxr);
    ADVANCE_PC(size_xxx);
    bld_copy((CPtr) op2, *((CPtr)op1));
  XSB_End_Instr()

#define ARITHPROC(OP, STROP)						\
    Op1(Register(get_xrx));						\
    Op3(get_xxr);							\
    ADVANCE_PC(size_xxx);						\
    op2 = *(op3);							\
    XSB_Deref(op1);							\
    XSB_Deref(op2);							\
    if (isointeger(op1)) {						\
      if (isointeger(op2)) {						\
	Integer temp = oint_val(op2) OP oint_val(op1);			\
	bld_oint(op3, temp); }						\
      else if (isofloat(op2)) {						\
	Float temp = ofloat_val(op2) OP (Float)oint_val(op1);		\
	/*printf("arith on old float\n");*/\
	bld_boxedfloat(CTXTc op3, temp); }				\
      else {								\
	FltInt fivar;							\
	if (xsb_eval(CTXTc op2, &fivar)) {				\
	  if (isfiint(fivar)) {						\
	    Integer temp = fiint_val(fivar) OP oint_val(op1);		\
	    bld_oint(op3, temp);					\
	  } else {							\
	    Float temp = fiflt_val(fivar) OP (Float)oint_val(op1);	\
	    bld_boxedfloat(CTXTc op3, temp);				\
	  }								\
	} else { arithmetic_abort(CTXTc op2, STROP, op1); }		\
      }									\
    }									\
    else if (isofloat(op1)) {						\
      /*printf("arith on old float\n");*/\
      if (isofloat(op2)) {						\
	Float temp = ofloat_val(op2) OP ofloat_val(op1);			\
	/*printf("arith on old float\n");*/					\
	bld_boxedfloat(CTXTc op3, temp); }				\
      else if (isointeger(op2)) {					\
	Float temp = (Float)oint_val(op2) OP ofloat_val(op1);		\
	bld_boxedfloat(CTXTc op3, temp); }				\
      else {								\
	FltInt fivar;							\
	if (xsb_eval(CTXTc op2, &fivar)) {				\
	  Float temp;							\
	  if (isfiint(fivar)) {						\
	    temp = fiint_val(fivar) OP float_val(op1);			\
	  } else {							\
	    temp = fiflt_val(fivar) OP float_val(op1);			\
	  }								\
	  bld_boxedfloat(CTXTc op3, temp);				\
	} else { arithmetic_abort(CTXTc op2, STROP, op1); }		\
      }									\
    }									\
    else {								\
      FltInt fiop1,fiop2;						\
      if (xsb_eval(CTXTc op1,&fiop1) && xsb_eval(CTXTc op2,&fiop2)) {	\
	if (isfiint(fiop1)) {						\
	  if (isfiint(fiop2)) {						\
	    Integer temp = fiint_val(fiop1) OP fiint_val(fiop2);	\
	    bld_oint(op3,temp);						\
	  } else {							\
	    Float temp = fiint_val(fiop1) OP fiflt_val(fiop2);		\
	    bld_boxedfloat(CTXTc op3, temp);				\
	  }								\
	} else {							\
	  Float temp;							\
	  if (isfiint(fiop2)) {						\
	    temp = fiflt_val(fiop1) OP fiint_val(fiop2);		\
	  } else {							\
	    temp = fiflt_val(fiop1) OP fiflt_val(fiop2);		\
	  }								\
	  bld_boxedfloat(CTXTc op3,temp);				\
	}								\
      } else { arithmetic_abort(CTXTc op2, STROP, op1); }		\
    }
 

  XSB_Start_Instr(addreg,_addreg) /* PRR */
    Def3ops
    ARITHPROC(+, "+");
  XSB_End_Instr() 

  XSB_Start_Instr(addintfastasgn,_addintfastasgn) /*RRA*/
    /* takes 3 1-byte operands: 
       byte 1 is register or local variable (reg if byte3&2 is 0),
           indicates target where sum will be assigned.
       byte 2 is register or local variable (reg if byte3&1 is 0)
           indicates first argument of sum.
       byte 3 >> 2 is signed integer that is second argument of sum.
           (Lowest 2 bits indicate reg or local var of 1st 2 args.)
    */
    register Cell op1; register char op2int; register CPtr op3;
    int targvar;
    Op1(get_xax);
    op2int = (char)(get_xxa);
    Op3((Cell)get_axx);
    ADVANCE_PC(size_xxx);
    //    printf("addintfastasgn: op1=%d, op2int=%d, op3=%d\n",(Integer)op1,(Integer)op2int,(Integer)op3);
    if (op2int & 0x1) op1 = (Cell)(ereg -(Cell)op1);
    else op1 = (Cell)(rreg + (Integer)op1);
    if (op2int & 0x2) {
      targvar = TRUE;
      op3 = (CPtr)(ereg - (Cell)op3);
    }
    else {
      targvar = FALSE;
      op3 = (CPtr)(rreg + (Integer)op3);
    }
    op2int = op2int >> 2;
    XSB_Deref(op1);
    if (isointeger(op1)) {
      Integer temp = oint_val(op1) + op2int;
      if (targvar) {bind_oint(op3, temp);}
      else {bld_oint(op3, temp);}
    } else if (isofloat(op1)) {
      Float temp = ofloat_val(op1) + (Float)op2int;
      /*printf("arith on old float\n");*/
      if (targvar) {bind_boxedfloat(op3, temp);}
      else {bld_boxedfloat(CTXTc op3, temp);}
    } else {
      FltInt fiop1;
      if (xsb_eval(CTXTc op1,&fiop1)) {
	if (isfiint(fiop1)) {
	  Integer temp = fiint_val(fiop1) + op2int;
	  bld_oint(op3, temp);
	} else {
	  Float temp = fiflt_val(fiop1) + (Float)op2int;
	  bld_boxedfloat(CTXTc op3, temp);
	}
      } else { arithmetic_abort(CTXTc op1, "+", makeint(op2int)); }
    }
  XSB_End_Instr() 

  XSB_Start_Instr(addintfastuni,_addintfastuni) /*RRA*/
    /* takes 3 1-byte operands:
       byte 1 is register or local variable (reg if byte3&2 is 0),
           indicates target where the result of the binop  will be assigned.
       byte 2 is register or local variable (reg if byte3&1 is 0)
           indicates first argument of binop.
       byte 3 >> 2 is signed integer that is second argument of binop
           (Lowest 2 bits indicate reg or local var of 1st 2 args.)
    */
    register Cell op1; register char op2int; register Cell op3;
    Op1(get_xax);
    op2int = (char)(get_xxa);
    //    Op3(get_axx);
    op3 = (Cell)lpcreg[1];
    ADVANCE_PC(size_xxx);
    //    printf("addintfastuni: op1=%d, op2int=%d, op3=%d\n",(Integer)op1,(Integer)op2int,(Integer)op3);
    if (op2int & 0x1) op1 = (Cell)(ereg -(Cell)op1);
    else op1 = (Cell)(rreg + (Integer)op1);
    if (op2int & 0x2) op3 = (Cell)(ereg - (Cell)op3);
    else op3 = (Cell)(rreg + (Integer)op3);
    op2int = op2int >> 2;
    XSB_Deref(op1);
    if (isointeger(op1)) {
      Integer temp = oint_val(op1) + op2int;
      nunify_with_num(op3, temp); 
    } else if (isofloat(op1)) {
      Float temp = ofloat_val(op1) + (Float)op2int;
      /*printf("arith on old float\n");*/
      nunify_with_float_get(op3, temp); 
    } else {
      FltInt fiop1;
      if (xsb_eval(CTXTc op1,&fiop1)) {
	if (isfiint(fiop1)) {
	  Integer temp = fiint_val(fiop1) + op2int;
	  nunify_with_num(op3, temp);
	} else {
	  Float temp = fiflt_val(fiop1) + (Float)op2int;
	  nunify_with_float_get(op3, temp);
	}
      } else { 
	addintfastuni_abort(CTXTc op1, makeint(op2int)); }
    }
  XSB_End_Instr() 

  XSB_Start_Instr(subreg,_subreg) /* PRR */
    Def3ops
    ARITHPROC(-, "-");
  XSB_End_Instr() 

  XSB_Start_Instr(cmpreg,_cmpreg) /* PRR */
    Def3ops
    int res = 2; /* TLS: compiler thinks res may be used uninitialized, but I dont thinks so */
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1);
    XSB_Deref(op2);
    if (isointeger(op1)) {
      if (isointeger(op2)) {
	Integer iop1 = oint_val(op1);
	Integer iop2 = oint_val(op2);
	if (iop2 > iop1) res = 1; else if (iop2 == iop1) res = 0; else res = -1;
      }	else if (isofloat(op2)) {
	Float fop1 = (Float)oint_val(op1);
	Float fop2 = ofloat_val(op2);
	/*printf("arith on old float\n");*/
	if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
      } else {
	FltInt fivar;
	if (xsb_eval(CTXTc op2, &fivar)) {
	  if (isfiint(fivar)) {
	    Integer iop1 = oint_val(op1);
	    Integer iop2 = fiint_val(fivar);
	    if (iop2 > iop1) res = 1; else if (iop2 == iop1) res = 0; else res = -1;
	  } else {
	    Float fop1 = (Float)oint_val(op1);
	    Float fop2 = fiflt_val(fivar);
	    if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
	  }
	}
	else { arithmetic_abort(CTXTc op2, "compare-operator", op1); }
      }
    }
    else if (isofloat(op1)) {
      Float fop1 = ofloat_val(op1);
      /*printf("arith on old float\n");*/
      if (isofloat(op2)) {
	Float fop2 = ofloat_val(op2);
	/*printf("arith on old float\n");*/
	if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
      } else if (isointeger(op2)) {
	Float fop2 = (Float)oint_val(op2);
	if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
      } else {
	FltInt fivar;
	if (xsb_eval(CTXTc op2, &fivar)) {
	  Float fop2;
	  if (isfiint(fivar)) {
	    fop2 = (Float)fiint_val(fivar);
	  } else {
	    fop2 = fiflt_val(fivar);
	  }
	  if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
	} else { arithmetic_abort(CTXTc op2, "compare-operator", op1); }
      }
    }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && xsb_eval(CTXTc op2,&fiop2)) {
	if (isfiint(fiop1)) {
	  if (isfiint(fiop2)) {
	    Integer iop1 = fiint_val(fiop1);
	    Integer iop2 = fiint_val(fiop2);
	    if (iop2 > iop1) res = 1; else if (iop2 == iop1) res = 0; else res = -1;
	  } else {
	    Float fop1 = (Float)fiint_val(fiop1);
	    Float fop2 = fiflt_val(fiop2);
	    if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
	  }
	} else {
	  Float fop1 = fiflt_val(fiop1);
	  Float fop2;
	  if (isfiint(fiop2)) {
	    fop2 = (Float)fiint_val(fiop2);
	  } else {
	    fop2 = fiflt_val(fiop2);
	  }
	  if (fop2 > fop1) res = 1; else if (fop2 == fop1) res = 0; else res = -1;
	}
      } else { arithmetic_abort(CTXTc op2, "compare-operator", op1); }
    }
    //#ifdef NON_OPT_COMPILE
    //    if (res == 2) xsb_abort("uninitialized use of res in cmpreg instruction");
    //#endif
    bld_oint(op3,res);
  XSB_End_Instr() 

  XSB_Start_Instr(mulreg,_mulreg) /* PRR */
    Def3ops
    ARITHPROC(*, "*");
  XSB_End_Instr() 

   /* TLS: cant use ARITHPROC because int/int -> float */
  XSB_Start_Instr(divreg,_divreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1);
    XSB_Deref(op2);
    if (isointeger(op1)) {
      Float temp;
      if (isointeger(op2)) {
        temp = (Float)oint_val(op2)/(Float)oint_val(op1);
	bld_boxedfloat(CTXTc op3, temp); }
      else if (isofloat(op2)) {
        temp = ofloat_val(op2)/(Float)oint_val(op1);
	bld_boxedfloat(CTXTc op3, temp); }
      else { 
	FltInt fiop2;
	if (xsb_eval(CTXTc op2,&fiop2)) {
	  if (isfiint(fiop2)) {
	    temp = (Float)(fiint_val(fiop2))/(Float)oint_val(op1);
	    bld_boxedfloat(CTXTc op3, temp); 
	  } else {
	    temp = fiflt_val(fiop2)/(Float)oint_val(op1);
	    bld_boxedfloat(CTXTc op3, temp); 
	  }
	} else { arithmetic_abort(CTXTc op2, "/", op1); }
      }
    } else if (isofloat(op1)) {
      if (isofloat(op2)) {
        Float temp = ofloat_val(op2)/ofloat_val(op1);
	bld_boxedfloat(CTXTc op3, temp); }
      else if (isointeger(op2)) {
        Float temp = (Float)oint_val(op2)/ofloat_val(op1);
	bld_boxedfloat(CTXTc op3, temp); }
      else {
	FltInt fiop2;
	if (xsb_eval(CTXTc op2,&fiop2)) {
	  if (isfiint(fiop2)) {
	    Float temp = (Float)(fiint_val(fiop2))/ofloat_val(op1);
	    bld_boxedfloat(CTXTc op3, temp); 
	  } else {
	    Float temp = fiflt_val(fiop2)/ofloat_val(op1);
	    bld_boxedfloat(CTXTc op3, temp); 
	  }
	} else { arithmetic_abort(CTXTc op2, "/", op1); }
      }
    } else { 
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && xsb_eval(CTXTc op2,&fiop2)) {
	Float temp;
	if (isfiint(fiop1)) {
	  if (isfiint(fiop2)) {
	    temp = (Float)fiint_val(fiop1) / (Float)fiint_val(fiop2);
	    bld_boxedfloat(CTXTc op3,temp);
	  } else {
	    temp = (Float)fiint_val(fiop1) / fiflt_val(fiop2);
	    bld_boxedfloat(CTXTc op3, temp);
	  }
	} else {
	  if (isfiint(fiop2)) {
	    temp = fiflt_val(fiop1) / (Float)fiint_val(fiop2);
	  } else {
	    temp = fiflt_val(fiop1) / fiflt_val(fiop2);
	  }
	  bld_boxedfloat(CTXTc op3,temp);
	}
      } else { arithmetic_abort(CTXTc op2, "/", op1); }
    }
  XSB_End_Instr() 

  XSB_Start_Instr(idivreg,_idivreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1);
    XSB_Deref(op2);
      if (isointeger(op1)) {
        if (oint_val(op1) != 0) {
          if (isointeger(op2)) {
            Integer temp = oint_val(op2) / oint_val(op1);
            bld_oint(op3, temp); 
          } else { 
	    FltInt fiop2;
	    if (xsb_eval(CTXTc op2, &fiop2) && isfiint(fiop2)) {
	      Integer temp = fiint_val(fiop2) / oint_val(op1);
	      bld_oint(op3,temp);
	    } else { arithmetic_abort(CTXTc op2, "//", op1); }
	  }
        } else {
	  err_handle(CTXTc ZERO_DIVIDE, 2,
		     "arithmetic expression involving is/2 or eval/2",
		     2, "non-zero number", op1);
	  lpcreg = pcreg;
        }
      }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop1) / fiint_val(fiop2);
	bld_oint(op3,temp);
      } else { arithmetic_abort(CTXTc op2, "//", op1); }
    }
  XSB_End_Instr() 

  XSB_Start_Instr(fdivreg,_fdivreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1);
    XSB_Deref(op2);
      if (isointeger(op1)) {
        if (oint_val(op1) != 0) {
          if (isointeger(op2)) {
            Integer temp = oint_val(op2) / oint_val(op1);
            bld_oint(op3, temp); 
          } else { 
	    FltInt fiop2;
	    if (xsb_eval(CTXTc op2, &fiop2) && isfiint(fiop2)) {
	      Integer temp = (Integer)floor((Float)fiint_val(fiop2) / (Float)oint_val(op1));
	      bld_oint(op3,temp);
	    } else { arithmetic_abort(CTXTc op2, "div", op1); }
	  }
        } else {
	  err_handle(CTXTc ZERO_DIVIDE, 2,
		     "arithmetic expression involving is/2 or eval/2",
		     2, "non-zero number", op1);
	  lpcreg = pcreg;
        }
      }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop1) / fiint_val(fiop2);
	bld_oint(op3,temp);
      } else { arithmetic_abort(CTXTc op2, "//", op1); }
    }
  XSB_End_Instr() 

  XSB_Start_Instr(int_test_z,_int_test_z)   /* PPR-B-L */
    Def3ops
    Op1(Register(get_xxr));
    Op2(get_xxxn);
    Op3(get_xxxxl);
    ADVANCE_PC(size_xxxXX);
    XSB_Deref(op1); 
    if (xsb_isnumber(op1)) {
      if (int_val(op1) == (Integer)op2)
	lpcreg = (byte *)op3;
    }
    else if (isboxedinteger(op1)) {
       if (oint_val(op1) == (Integer)op2)
          lpcreg = (byte *)op3;
    }	  
    else if (isboxedfloat(op1)) {
      if (ofloat_val(op1) == (double)op2)
	lpcreg = (byte *) op3;
    }
    else {
      arithmetic_comp_abort(CTXTc op1, "=\\=", op2);
    }
  XSB_End_Instr()

  XSB_Start_Instr(int_test_nz,_int_test_nz)   /* PPR-B-L */
    Def3ops
    Op1(Register(get_xxr));
    Op2(get_xxxn);
    Op3(get_xxxxl);
    ADVANCE_PC(size_xxxXX);
    XSB_Deref(op1); 
    if (xsb_isnumber(op1)) {
      if (int_val(op1) != (Integer)op2)
	lpcreg = (byte *) op3;
    }
    else if (isboxedinteger(op1)) {
       if (oint_val(op1) != (Integer)op2)
          lpcreg = (byte *)op3;
    }	  
    else if (isboxedfloat(op1)) {
      if (ofloat_val(op1) != (double)op2)
	lpcreg = (byte *) op3;
    }
    else {
      arithmetic_comp_abort(CTXTc op1, "=:=", op2);
    }
  XSB_End_Instr()

    /* Used for the @=/2 operator */
  XSB_Start_Instr(fun_test_ne,_fun_test_ne)   /* PRR-L */
    Def3ops
    Op1(Register(get_xrx));
    Op2(Register(get_xxr));
    XSB_Deref(op1);
    XSB_Deref(op2);
    if (isconstr(op1)) {
      if (!isconstr(op2) || get_str_psc(op1) != get_str_psc(op2)) {
	Op3(get_xxxl);
        lpcreg = (byte *) op3;
      } else {
	ADVANCE_PC(size_xxxX);
      }
    } else if (islist(op1)) {
      if (!islist(op2)) {
	Op3(get_xxxl);
	lpcreg = (byte *) op3;
      }
      else ADVANCE_PC(size_xxxX);
    } else {
      if (op1 != op2) {
	Op3(get_xxxl);
	lpcreg = (byte *) op3;
      }
      else ADVANCE_PC(size_xxxX);
    }
  XSB_End_Instr()

     /* TLS: so much work for such a little function! */
  XSB_Start_Instr(powreg,_powreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));                                              
    Op3(get_xxr);                                                        
    ADVANCE_PC(size_xxx);                                                
    op2 = *(op3);                                                        
    XSB_Deref(op1);	                                                 
    XSB_Deref(op2);                                                      
    {
	double b = 0, e = 0, r;
	int e_int = 0, b_int = 0;

	if( isointeger(op2) )
	  {	b = (double)oint_val(op2);
		b_int = 1;
	}
	else if( isofloat(op2) )
	{	b = ofloat_val(op2);
	  /*printf("arith on old float\n");*/
	}
        else {
	  FltInt fiop2;
	  if (xsb_eval(CTXTc op2,&fiop2)) {
	    if (isfiint(fiop2)) {
	      b = (double)fiint_val(fiop2);
	      b_int = 1;
	    } else b = fiflt_val(fiop2);
	  } else { arithmetic_abort(CTXTc op2, "**", op1); }
	}

	if( isointeger(op1) )
	  {	e = (double)oint_val(op1);
		e_int = 1;
	}
	else if( isofloat(op1) )
	{	e = ofloat_val(op1);
	  /*printf("arith on old float\n");*/
	}
	else {
	  FltInt fiop1;
	  if (xsb_eval(CTXTc op1,&fiop1)) {
	    if (isfiint(fiop1)) {
	      e = (double)fiint_val(fiop1);
	      e_int = 1;
	    } else e = fiflt_val(fiop1);
	  } else { arithmetic_abort(CTXTc op2, "**", op1); }
	}

	if( !e_int && b < 0 )
	    pow_domain_error(op1,op2);

	r = pow(b,e);

	if( b_int && e_int ) 
	{
	  bld_oint(op3, r);
	}
	else
	  bld_boxedfloat( CTXTc op3, (Float)r );
    }
  XSB_End_Instr() 

     /* TLS: so much work for such a little function! */
  XSB_Start_Instr(minreg,_minreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);

    XSB_Deref(op1);
    XSB_Deref(op2);
    if (isointeger(op1)) {
         if (isointeger(op2)) {
              if (oint_val(op2) < oint_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          }
         if (isofloat(op2)) {
              if (ofloat_val(op2) < oint_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          }
    } 
    else if (isofloat(op1)) {
         if (isointeger(op2)) {
              if (oint_val(op2) < ofloat_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          }
         if (isofloat(op2)) {
              if (ofloat_val(op2) < ofloat_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          }
    } 
    else { arithmetic_abort(CTXTc op2, "min", op1); }  /* need to resolve to xsb_eval...*/
  XSB_End_Instr() 

     /* TLS: so much work for such a little function! */
  XSB_Start_Instr(maxreg,_maxreg) /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1);
    XSB_Deref(op2);
    if (isointeger(op1)) {
         if (isointeger(op2)) {
              if (oint_val(op2) > oint_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          } else if (isofloat(op2)) {
              if (ofloat_val(op2) > oint_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          }
    } 
    else if (isofloat(op1)) {
         if (isointeger(op2)) {
              if (oint_val(op2) > ofloat_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          } else if (isofloat(op2)) {
              if (ofloat_val(op2) > ofloat_val(op1))  bld_copy(op3,op2); else bld_copy(op3,op1);
          }
    } 
   else { arithmetic_abort(CTXTc op2, "max", op1); }  /* need to resolve to xsb_eval...*/
  XSB_End_Instr() 


    /* dereferences op1 (as opposed to putpval) */
  XSB_Start_Instr(putdval,_putdval) /* PVR */
    Def2ops
    Op1(Variable(get_xvx));
    Op2(get_xxr);
    ADVANCE_PC(size_xxx);
    XSB_Deref(op1);
    bld_copy((CPtr)op2, op1);
  XSB_End_Instr()

  XSB_Start_Instr(putuval,_putuval) /* PVR */
    Def2ops
    Op1(Variable(get_xvx));
    Op2(get_xxr);
    ADVANCE_PC(size_xxx);
    XSB_Deref(op1);
    if (isnonvar(op1) || ((CPtr)(op1) < hreg) || ((CPtr)(op1) >= ereg)) {
      bld_copy((CPtr)op2, op1);
    } else {
      bld_ref((CPtr)op2, hreg);
      bind_ref((CPtr)(op1), hreg);
      new_heap_free(hreg);
    } 
  XSB_End_Instr()

  /*
   * Instruction `check_interrupt' is used before `new_answer_dealloc' to
   * handle the pending attv interrupts.  It is similar to `call' but the
   * second argument (S) is not used currently.
   */
  XSB_Start_Instr(check_interrupt,_check_interrupt)  /* PPA-S */
    Def1op

    UNUSED(op1);
    
    Op1(get_xxxs);
    ADVANCE_PC(size_xxxX);
    if (attv_pending_interrupts) {
      cpreg = lpcreg;
      bld_cs(reg + 2, hreg);	/* see subp.c: build_call() */
      new_heap_functor(hreg, true_psc);
      bld_copy(reg + 1, build_interrupt_chain(CTXT));
      lpcreg = get_ep((Psc) pflags[MYSIG_ATTV + INT_HANDLERS_FLAGS_START]);
    }
  XSB_End_Instr()

  XSB_Start_Instr(call,_call)  /* PPA-S */
    Def1op
    Psc psc;

    Op1(get_xxxs); /* the first arg is used later by alloc */
    ADVANCE_PC(size_xxxX);
    cpreg = lpcreg;
    psc = (Psc)op1;
    //    if (flags[PIL_TRACE]) fprintf(logfile,"%ld:call %s/%d (h:%p,e:%p,pc:%p,b:%p,hs:%lx)\n",(Integer)(cpu_time() * 1000),get_name(psc),get_arity(psc),hreg,ereg,lpcreg,breg,top_of_localstk-hreg);
#ifdef CP_DEBUG
    pscreg = psc;
#endif
#ifdef MULTI_THREAD_LOGGING
    log_rec(CTXTc psc, "call");
#endif
    call_sub(psc);
  XSB_End_Instr()

    /* If using the multi-threaded engine, call the function with the
       single argument, CTXT; otherwise call a parameterless
       funcion.  */
  XSB_Start_Instr(call_forn,_call_forn) {
    Def1op
    Op1(get_xxxl);
    ADVANCE_PC(size_xxxX);
#ifdef MULTI_THREAD
    fp = (int(*)())op1;
    if (fp(CTXT))  /* call foreign function */
      lpcreg = cpreg;
    else Fail1;
#else
    if (((PFI)op1)())  /* call foreign function */
      lpcreg = cpreg;
    else Fail1;
#endif
  }
  XSB_End_Instr()

  XSB_Start_Instr(load_pred,_load_pred) /* PPP-S */
    /* Executing this instruction causes itself to be changed to 
       jump or load_forn by the predicate loading process. */
    Def1op
    Psc psc;
    
    Op1(get_xxxs);
    psc = (Psc)op1; /* before getting lock, since may be changed by other loader in MT */
#ifdef MULTI_THREAD
    SYS_MUTEX_LOCK(MUTEX_LOAD_UNDEF);
    if (*lpcreg == load_pred) { /* not loaded, so call interrupt routine to load it */
#endif

    bld_cs(reg+1, build_call(CTXTc psc));   /* put call-term in r1 */
    /* get psc of undef handler */
    psc = (Psc)pflags[MYSIG_UNDEF+INT_HANDLERS_FLAGS_START];
    bld_int(reg+2, MYSIG_UNDEF);      /* undef-pred code */
#ifdef MULTI_THREAD
   } else { /* someone else loaded it, just release lock and go to it */
      SYS_MUTEX_UNLOCK(MUTEX_LOAD_UNDEF);
   }
#endif
    lpcreg = get_ep(psc);             /* ep of undef handler */
  XSB_End_Instr()


  XSB_Start_Instr(allocate_gc,_allocate_gc) /* PAA */
    Def3ops
    Op2(get_xax);
    Op3((CPtr) (Cell)get_xxa);
    ADVANCE_PC(size_xxx);
    if (efreg_on_top(ereg))
      op1 = (Cell)(efreg-1);
    else {
      if (ereg_on_top(ereg)) op1 = (Cell)(ereg - *(cpreg-(2*sizeof(Cell)-3)));
      else op1 = (Cell)(ebreg-1);
    }
    //    printf("allo %p,%p,%p,%p,",hreg,ereg,ebreg,cp_ebreg(breg));
    *(CPtr *)((CPtr) op1) = ereg;
    *((byte **) (CPtr)op1-1) = cpreg;
    ereg = (CPtr)op1; 
    //    printf("%p,%p\n",ereg,lpcreg);
    {/* initialize all permanent variables not in the first chunk to unbound */
      Integer  i = ((Cell)op3) - op2;
      CPtr p = ((CPtr)op1) - op2;
      while (i--) {
	bld_free(p);
        p--;
      }
    }
  XSB_End_Instr()

/* This is obsolete and is only kept for backwards compatibility for < 2.0 */
  XSB_Start_Instr(allocate,_allocate) /* PPP */
    Def1op
    ADVANCE_PC(size_xxx);
    if (efreg_on_top(ereg))
      op1 = (Cell)(efreg-1);
    else {
      if (ereg_on_top(ereg)) op1 = (Cell)(ereg - *(cpreg-(2*sizeof(Cell)-3)));
      else op1 = (Cell)(ebreg-1);
    }
    *(CPtr *)((CPtr) op1) = ereg;
    *((byte **) (CPtr)op1-1) = cpreg;
    ereg = (CPtr)op1; 
    { /* for old object files initialize pessimisticly but safely */
      int  i = 256;
      CPtr p = ((CPtr)op1)-2;
      while (i--) {
	bld_free(p);
        p--;
      }
    }
  XSB_End_Instr()

  XSB_Start_Instr(deallocate,_deallocate) /* PPP */
    ADVANCE_PC(size_xxx);
    cpreg = *((byte **)ereg-1);
    //    printf("dealloc %p,%p,",hreg,ereg);
    ereg = *(CPtr *)ereg;
    //    printf("%p,%p\n",ereg,lpcreg);
  XSB_End_Instr()

  XSB_Start_Instr(deallocate_gc,_deallocate_gc) /* PPA -- instruction no longer used! */
    ADVANCE_PC(size_xxx);
    cpreg = *((byte **)ereg-1);
    ereg = *(CPtr *)ereg;
    /* following required for recursive loops that build structure on way back up. */
    if (heap_local_overflow(OVERFLOW_MARGIN)) { 
      Def1op
      Op1((lpcreg[-1]));  // already advanced pc, so look back
      if (gc_heap(CTXTc (int)op1,FALSE)) { // no regs, garbage collection potentially modifies hreg 
	if (heap_local_overflow(OVERFLOW_MARGIN)) {
	  glstack_realloc(CTXTc resize_stack(glstack.size,OVERFLOW_MARGIN),(int)op1);
	  /*
	  if (pflags[STACK_REALLOC]) {
	    if (glstack_realloc(CTXTc resize_stack(glstack.size,OVERFLOW_MARGIN),(int)op1) != 0) {
	      xsb_memory_error("memory","Cannot Expand Local and Global Stacks");
	    }
	  } else {
	    xsb_warn(CTXTc "Reallocation is turned OFF !");
	    xsb_memory_error("memory","Cannot Expand Local and Global Stacks");
	  }
	  */
	}
      }
    }
  XSB_End_Instr()

  XSB_Start_Instr(proceed,_proceed)  /* PPP */
     proceed_sub;
  /*  {Psc psc = *(((Psc *)lpcreg)-1); byte call_inst = *((byte *)lpcreg-8);
    if (call_inst == call) printf("proc %s/%d (h:%p,e:%p,pc:%p)\n",get_name(psc),get_arity(psc),hreg,ereg,lpcreg);
    else if (call_inst == calld) printf("proc to calld (h:%p,e:%p,pc:%p,addr:%p)\n",hreg,ereg,lpcreg,psc);
    else printf("proc illegal?\n"); } */
  XSB_End_Instr()

  XSB_Start_Instr(proceed_gc,_proceed_gc) /* PPP */
    /* following required for recursive loops that build structure on way back up. */
    if (heap_local_overflow(OVERFLOW_MARGIN)) { 
      if (gc_heap(CTXTc 0,FALSE)) { // no regs, garbage collection potentially modifies hreg 
	if (heap_local_overflow(OVERFLOW_MARGIN)) {
	  glstack_realloc(CTXTc resize_stack(glstack.size,OVERFLOW_MARGIN),0);
	  /*
	  if (pflags[STACK_REALLOC]) {
	    if (glstack_realloc(CTXTc resize_stack(glstack.size,OVERFLOW_MARGIN),0) != 0) {
	      xsb_memory_error("memory","Cannot Expand Local and Global Stacks");
	    }
	  } else {
	    xsb_warn(CTXTc "Reallocation is turned OFF !");
	    xsb_memory_error("memory","Cannot Expand Local and Global Stacks");
	  }
	  */
	}
      }
    }
    proceed_sub;
    /*  {Psc psc = *(((Psc *)lpcreg)-1); byte call_inst = *((byte *)lpcreg-8);
    if (call_inst == call) printf("prgc %s/%d (h:%p,e:%p,pc:%p)\n",get_name(psc),get_arity(psc),hreg,ereg,lpcreg);
    else if (call_inst == calld) printf("prgc to calld (h:%p,e:%p,pc:%p,addr:%p)\n",hreg,ereg,lpcreg,psc);
    else printf("prgc illegal? (h:%p,e:%p,pc:%p,addr:%p)\n",hreg,ereg,lpcreg,psc); } */
  XSB_End_Instr()

  XSB_Start_Instr(restore_dealloc_proceed,_restore_dealloc_proceed) /* PPP */
    Cell term = cell(ereg-3);
    XSB_Deref(term);
    if (isconstr(term)) {
      int  disp;
      char *addr;
      Psc psc = get_str_psc(term);
      addr = (char *)(clref_val(term));
      for (disp = 1; disp <= (int)get_arity(psc); ++disp) {
	bld_copy(reg+disp, cell((CPtr)(addr)+disp));
      }
    }
    cpreg = *((byte **)ereg-1);
    lpcreg = *((byte **)ereg-2);
    ereg = *(CPtr *)ereg;
  XSB_End_Instr()

    /* This is the WAM-execute.  Name was changed because of conflict
       with some system files for pthreads. */
  XSB_Start_Instr(xsb_execute,_xsb_execute) /* PPP-S */
    Def1op
    Psc psc;

    Op1(get_xxxs);
    //    ADVANCE_PC(size_xxxX);
    psc = (Psc)op1;
#ifdef MULTI_THREAD_LOGGING
    log_rec(CTXTc psc, "exec");
#endif
#ifdef CP_DEBUG
    pscreg = psc;
#endif
    //    if (flags[PIL_TRACE]) fprintf(logfile,"%ld:exec %s/%d (h:%p,e:%p,pc:%p,b:%p,hs:%lx)\n",(Integer)(cpu_time()*1000),get_name(psc),get_arity(psc),hreg,ereg,lpcreg,breg,top_of_localstk-hreg);
    call_sub(psc);
  XSB_End_Instr()

  XSB_Start_Instr(jump,_jump)   /* PPP-L */
    lpcreg = (byte *)get_xxxl;
  XSB_End_Instr()

  XSB_Start_Instr(jumpz,_jumpz)   /* PPR-L */
    Def1op
    Op1(Register(get_xxr));
    if (isointeger(op1)) {
        if (oint_val(op1) == 0) {
            lpcreg = (byte *)get_xxxl;   
        } else {ADVANCE_PC(size_xxxX);}
    } else if (isofloat(op1)) {
        if (ofloat_val(op1) == 0.0) {
           lpcreg = (byte *)get_xxxl;
        } else {ADVANCE_PC(size_xxxX);}
    }
  XSB_End_Instr()

  XSB_Start_Instr(jumpnz,_jumpnz)    /* PPR-L */
    Def1op
    Op1(Register(get_xxr));
    if (isointeger(op1)) {
        if (oint_val(op1) != 0) {
            lpcreg = (byte *)get_xxxl;   
        } else {ADVANCE_PC(size_xxxX);}
    } else if (isofloat(op1)) {
        if (ofloat_val(op1) != 0.0) {
           lpcreg = (byte *)get_xxxl;
        } else {ADVANCE_PC(size_xxxX);}
    }
  XSB_End_Instr()

  XSB_Start_Instr(jumplt,_jumplt)    /* PPR-L */
    Def1op
    Op1(Register(get_xxr));
    if (isointeger(op1)) {
      if (oint_val(op1) < 0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } else if (isofloat(op1)) {
      if (ofloat_val(op1) < 0.0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } 
  XSB_End_Instr() 

  XSB_Start_Instr(jumple,_jumple)    /* PPR-L */
    Def1op
    Op1(Register(get_xxr));
    if (isointeger(op1)) {
      if (oint_val(op1) <= 0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } else if (isofloat(op1)) {
      if (ofloat_val(op1) <= 0.0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } 
  XSB_End_Instr() 

  XSB_Start_Instr(jumpgt,_jumpgt)    /* PPR-L */
    Def1op
    Op1(Register(get_xxr));
    if (isointeger(op1)) {
      if (oint_val(op1) > 0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } else if (isofloat(op1)) {
      if (ofloat_val(op1) > 0.0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } 
  XSB_End_Instr()

  XSB_Start_Instr(jumpge,_jumpge)    /* PPR-L */
    Def1op
    Op1(Register(get_xxr));
    if (isointeger(op1)) {
      if (oint_val(op1) >= 0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } else if (isofloat(op1)) {
      if (ofloat_val(op1) >= 0.0) lpcreg = (byte *)get_xxxl;
      else {ADVANCE_PC(size_xxxX);}
    } 
  XSB_End_Instr() 

  XSB_Start_Instr(fail,_fail)    /* PPP */
    Fail1; 
  XSB_End_Instr()

  XSB_Start_Instr(dynfail,_dynfail)    /* PPP */
#ifdef MULTI_THREAD
    if (i_have_dyn_mutex) {
      SYS_MUTEX_UNLOCK(MUTEX_DYNAMIC);
      i_have_dyn_mutex = 0;
    }
#endif
    Fail1; 
  XSB_End_Instr()

  XSB_Start_Instr(noop,_noop)  /* PPA */
    Def1op
    Op1(get_xxa);
    ADVANCE_PC(size_xxx);
    lpcreg += (int)op1;
    lpcreg += (int)op1;
  XSB_End_Instr()

  XSB_Start_Instr(dynnoop,_dynnoop)  /* PPA */
    Def1op
    Op1(get_xxa);
    ADVANCE_PC(size_xxx);
    lpcreg += (int)op1;
    lpcreg += (int)op1;
#ifdef MULTI_THREAD
    if (i_have_dyn_mutex) {
      SYS_MUTEX_UNLOCK(MUTEX_DYNAMIC);
      i_have_dyn_mutex = 0;
    }
#endif
  XSB_End_Instr()

    /* for an explanation of the code below see the documentation
       "Using the API with the MT engine" in cinterf.c */

  XSB_Start_Instr(halt,_halt)  /* PPP */
    ADVANCE_PC(size_xxx);
    pcreg = lpcreg; 
    current_inst = lpcreg;
#ifdef MULTI_THREAD
    if (xsb_mode == C_CALLING_XSB && th != main_thread_gl) {
      int pthread_cond_wait_err;
      xsb_ready = XSB_IN_C;
      pthread_cond_wait_err = xsb_cond_signal(&xsb_done_cond, "emuloop", __FILE__, __LINE__);
      while ((XSB_IN_C == xsb_ready) && (!pthread_cond_wait_err))
      	pthread_cond_wait_err = xsb_cond_wait(&xsb_started_cond, &xsb_synch_mut, "emuloop", __FILE__, __LINE__);
    } else  
#endif
    return(0);	/* not "goto contcase"! */
  XSB_End_Instr()

  XSB_Start_Instr(builtin,_builtin)
    Def1op
    Op1(get_xxa);
    ADVANCE_PC(size_xxx);
    pcreg=lpcreg; 
    if (builtin_call(CTXTc (byte)(op1))) {lpcreg=pcreg;}
    else Fail1;
  XSB_End_Instr()

#define jump_cond_fail(Condition) \
      if (Condition) {ADVANCE_PC(size_xxxX);} else lpcreg = (byte *)get_xxxl

  XSB_Start_Instr(jumpcof,_jumpcof)
    Def2ops
    Op1(get_xax);
    Op2(get_xxr);
    XSB_Deref(op2);
    switch (op1) {
    case ATOM_TEST:
      jump_cond_fail(isatom(op2));
      break;
    case INTEGER_TEST:
      jump_cond_fail(isointeger(op2));
      break;
    case REAL_TEST:
      jump_cond_fail(isofloat(op2));
      break;
    case NUMBER_TEST:
      jump_cond_fail(xsb_isnumber(op2) || isboxedinteger(op2) || isboxedfloat(op2));
      break;
    case ATOMIC_TEST:
      jump_cond_fail(isatomic(op2) || isboxedinteger(op2) || isboxedfloat(op2));
      break;
    case COMPOUND_TEST:
      jump_cond_fail(((isconstr(op2) && (get_str_psc(op2) != box_psc)) ||
		      (islist(op2))));
      break;
    case CALLABLE_TEST:
      jump_cond_fail(iscallable(op2));
      break;
    case DIRECTLY_CALLABLE_TEST: {
      //      printf("op2: %x, %x %x cut_psc %x %x\n",op2,cs_val(op2),get_str_psc(op2),cut_psc,cut_string);
      if (isstring(op2)) jump_cond_fail((char *) cs_val(op2) != cut_string);
      else jump_cond_fail(is_directly_callable(op2));
      break;
    }
    case IS_LIST_TEST:
      jump_cond_fail(is_proper_list(op2));
      break;
    case IS_MOST_GENERAL_TERM_TEST:
      jump_cond_fail(is_most_general_term(op2));
      break;
    case IS_ATTV_TEST:
      jump_cond_fail(isattv(op2));
      break;
    case VAR_TEST:
      jump_cond_fail(isref(op2) || isattv(op2));
      break;
    case NONVAR_TEST:
      jump_cond_fail(isnonvar(op2) && !isattv(op2));
      break;
    case IS_NUMBER_ATOM_TEST:
      jump_cond_fail(is_number_atom(op2));
      break;
    case GROUND_TEST:
      jump_cond_fail(ground(op2));
      break;
    default: 
      xsb_error("Undefined jumpcof condition");
      Fail1;
    }
  XSB_End_Instr()

  XSB_Start_Instr(unifunc,_unifunc)   /* PAR */
    Def2ops
    Op1(get_xax);
    Op2(get_xxr);
    ADVANCE_PC(size_xxx);
    if (unifunc_call(CTXTc (int)(op1), (CPtr)op2) == 0) {
      unifunc_abort(CTXTc (int)(op1), (CPtr)op2);
      //      xsb_error("Error in unary function call");
      //      Fail1;
    }
  XSB_End_Instr()

    /* Calls internal code -- does not go through psc record and omits
       interrupt checks.  Not sure if profile_interrupt should be here...*/
  XSB_Start_Instr(calld,_calld)   /* PPA-L */
    ADVANCE_PC(size_xxx); /* this is ok */
    cpreg = lpcreg+sizeof(Cell); 
    /*check_glstack_overflow(MAX_ARITY, lpcreg,OVERFLOW_MARGIN);  try eliminating?? */
    handle_xsb_profile_interrupt;
    lpcreg = *(pb *)lpcreg;
  XSB_End_Instr()

  XSB_Start_Instr(logshiftr,_logshiftr)  /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1); 
    XSB_Deref(op2);
    if (isointeger(op1)) {
      if (isointeger(op2)) {
        Integer temp = oint_val(op2) >> oint_val(op1);
        bld_oint(op3, temp); 
      }
      else {
	FltInt fiop2;
	if (xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	  Integer temp = fiint_val(fiop2) >> oint_val(op1);
	  bld_oint(op3,temp);
	} else {arithmetic_abort(CTXTc op2, "'>>'", op1);}
      }
    }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop2) >> fiint_val(fiop1);
	bld_oint(op3,temp);
      } else {arithmetic_abort(CTXTc op2, "'>>'", op1);}
    }
  XSB_End_Instr() 

  XSB_Start_Instr(logshiftl,_logshiftl)   /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1); 
    XSB_Deref(op2);
    if (isointeger(op1)) {
      if (isointeger(op2)) {
        Integer temp = oint_val(op2) << oint_val(op1);
        bld_oint(op3, temp); 
      }
      else {
	FltInt fiop2;
	if (xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	  Integer temp = fiint_val(fiop2) << oint_val(op1);
	  bld_oint(op3,temp);
	} else {arithmetic_abort(CTXTc op2, "'<<'", op1);}
      }
    }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop2) << fiint_val(fiop1);
	bld_oint(op3,temp);
      } else {arithmetic_abort(CTXTc op2, "'<<'", op1);}
    }
  XSB_End_Instr() 

  XSB_Start_Instr(or,_or)   /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1); 
    XSB_Deref(op2);
    if (isointeger(op1)) {
      if (isointeger(op2)) {
        Integer temp = (oint_val(op2)) | (oint_val(op1));
        bld_oint(op3, temp); 
      }
      else {
	FltInt fiop2;
	if (xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	  Integer temp = fiint_val(fiop2) | oint_val(op1);
	  bld_oint(op3,temp);
	} else {arithmetic_abort(CTXTc op2, "'\\/'", op1);}
      }
    }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop2) | fiint_val(fiop1);
	bld_oint(op3,temp);
      } else {arithmetic_abort(CTXTc op2, "'\\/'", op1);}
    }
  XSB_End_Instr() 

  XSB_Start_Instr(and,_and)   /* PRR */
    Def3ops
    Op1(Register(get_xrx));
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op2 = *(op3);
    XSB_Deref(op1); 
    XSB_Deref(op2);
    if (isointeger(op1)) {
      if (isointeger(op2)) {
        Integer temp = (oint_val(op2)) & (oint_val(op1));
        bld_oint(op3, temp); 
      }
      else {
	FltInt fiop2;
	if (xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	  Integer temp = fiint_val(fiop2) & oint_val(op1);
	  bld_oint(op3,temp);
	} else {arithmetic_abort(CTXTc op2, "'/\\'", op1);}
      }
    }
    else {
      FltInt fiop1,fiop2;
      if (xsb_eval(CTXTc op1,&fiop1) && isfiint(fiop1)
	  && xsb_eval(CTXTc op2,&fiop2) && isfiint(fiop2)) {
	Integer temp = fiint_val(fiop2) & fiint_val(fiop1);
	bld_oint(op3,temp);
      } else {arithmetic_abort(CTXTc op2, "'/\\'", op1);}
    }
  XSB_End_Instr() 

  XSB_Start_Instr(negate,_negate)   /* PPR */
    DefOps13
    Op3(get_xxr);
    ADVANCE_PC(size_xxx);
    op1 = *(op3);
    XSB_Deref(op1);
    if (isointeger(op1)) { bld_oint(op3, ~(oint_val(op1))); }
    else {
      FltInt fiop1;
      if (xsb_eval(CTXTc op1, &fiop1) && isfiint(fiop1)) {
	Integer temp = ~(fiint_val(fiop1));
	bld_oint(op3, temp); 
      } else { arithmetic_abort1(CTXTc "'\\'", op1); }
    }
  XSB_End_Instr() 

/** Execution of the sob_jump_out instruction  -- **Ken **/
  /** when we reach here, we are falling through a switch on bound
   * instruction and should build an index using the new DI scheme **/
  XSB_Start_Instr(sob_jump_out, _sob_jump_out) /* PPL */
    if (flags[LOG_UNINDEXED]) {
      Integer chainlen;
      if (flags[LOG_UNINDEXED] == 1) *lpcreg = jump; // change inst so unindex test is done only once
      chainlen = length_dyntry_chain(*(((byte **)(lpcreg))+1));
      if (chainlen > 20) {  /* only if chain is > 20 long */
	if (xsb_profiling_enabled) {
	  int i;
	  Psc psc = psc_from_code_addr(lpcreg);
	  fprintf(stdout,"%s/%d searched %"Intfmt" unindexed clauses\n",get_name(psc),get_arity(psc),chainlen);
	  fprintf(stdout,"  %s(",get_name(psc));
	  for (i=1; i<=get_arity(psc); i++) {
	    if (isref(reg[i])) {
	      fprintf(stdout,"_");  // dont care what vars
	    } else {
	      printterm(stdout,reg[i],10);
	    }
	    if (i<get_arity(psc)) printf(",");
	  }
	  printf(")\n");
	  if ((Psc)flags[LOG_UNINDEXED] == psc) print_xsb_backtrace(CTXT);
	} else {
	  //	  printf("Unknown predicate searched %d unindexed clauses\n",chainlen);
	}
      }
      lpcreg = (byte *)get_xxxl;
    } else {
      lpcreg = (byte *)get_xxxl;  // do jump
    }
  XSB_End_Instr()

#ifndef JUMPTABLE_EMULOOP
  default: {
    char message[80];
    sprintf(message, "Illegal opcode hex %x (& %p)", *lpcreg,lpcreg); 
    xsb_exit(CTXTc message);
  }
} /* end of switch */
#else
  _no_inst:
    {
      char message[80];
      sprintf(message, "Illegal opcode hex %x (& %p)", *lpcreg,lpcreg);
      xsb_exit(message);
    }
#endif

return 0;

} /* end of emuloop() */

/*======================================================================*/
/*======================================================================*/
/* TLS: changes so that XSB does not cause a process exit when called by C */

DllExport int call_conv xsb(CTXTdeclc int flag, int argc, char *argv[])
{ 
   char *startup_file;
   FILE *fd;
   unsigned int magic_num;
   static double realtime;	/* To retain its value across invocations */

   extern void dis(xsbBool);
   extern char *init_para(CTXTdeclc int, int, char **);
   extern void perform_IO_Redirect(CTXTdeclc int, char **);
   extern void init_machine(CTXTdeclc int, int, int, int), init_symbols(CTXTdecl);
#ifdef FOREIGN
#ifndef FOREIGN_ELF
#ifndef FOREIGN_WIN32
   extern char tfile[];
#endif
#endif
#endif

#ifdef MULTI_THREAD
extern pthread_mutexattr_t attr_rec_gl ;
#endif

   if (flag == XSB_INIT || flag == XSB_C_INIT) {  /* initialize xsb */

     if (flag == XSB_C_INIT)
     		xsb_mode = C_CALLING_XSB;
     else
     {
     		xsb_mode = DEFAULT; /* MAY BE CHANGED LATER */
#ifdef MULTI_THREAD
/* this has to be initialized here and not in main_xsb.c because
   of windows linkage problems for the variable main_thread_gl
 */
   main_thread_gl = th ;
#endif
     }

     //     logfile = fopen("XSB_LOGFILE.txt","w");
     /* Set the name of the executable to the real name.
	The name of the executable could have been set in cinterf.c:xsb_init
	if XSB is called from C. In this case, we don't want `executable'
	to be overwritten, so we check if it is initialized. */
	perform_IO_Redirect(CTXTc argc, argv);

#ifdef SIMPLESCALAR
	strcpy(executable_path_gl,argv[0]);
#else
      	if (executable_path_gl[0] == '\0')
	  xsb_executable_full_path(argv[0]);
#endif

	/* set install_dir, xsb_config_file and user_home */
	set_install_dir();
	set_config_file();
	set_user_home();

	realtime = real_time();
	setbuf(stdout, NULL);
	startup_file = init_para(CTXTc flag, argc, argv);	/* init parameters */

#ifdef MULTI_THREAD
	/* init c-calling-xsb mutexes, after init_para() */
	pthread_mutex_init( &(th->_xsb_ready_mut), &attr_rec_gl ) ;
	pthread_mutex_init( &(th->_xsb_synch_mut), &attr_rec_gl ) ;
	pthread_mutex_init( &(th->_xsb_query_mut), &attr_rec_gl ) ;
	xsb_ready = XSB_IN_Prolog;
#endif

	init_machine(CTXTc 0, 0, 0, 0);	/* init space, regs, stacks */
	init_inst_table();		/* init table of instruction types */
	init_symbols(CTXT);		/* preset symbols in PSC table; initialize Proc-level globals */
	init_interrupt();		/* catch ^C interrupt signal */

	/* "b" does nothing in UNIX, denotes binary file in Windows -- 
	   needed in Windows for reading byte-code files */
	fd = fopen(startup_file, "rb");

	if (!fd) {
	  char message[MAXPATHLEN + 50];
	  /* TLS: doing an snprintf here to avoid trouncing memory if XSB is called from C */
	  snprintf(message, MAXPATHLEN + 50, "The startup file, %s, could not be found!",
		  startup_file);
	  xsb_initialization_exit(message); 
	}
	magic_num = read_magic(fd);
	fclose(fd);
	if (magic_num == 0x11121307  || magic_num == 0x11121305 || magic_num == 0x1112130a) {
	  inst_begin_gl = loader(CTXTc startup_file,0);
	}
	else 
	  xsb_initialization_exit("Incorrect startup file format");

	if (!inst_begin_gl)
	  xsb_initialization_exit("Error in loading startup file");

	if (xsb_mode == DISASSEMBLE) {
	  if (magic_num == 0x1112130a) {
	    exit(0);
	  }
	  dis(1);
	  exit(0);  /* This raw exit ok -- wont be called by C_CALLING_XSB */
	}

	/* do it after initialization, so that typing 
	   xsb -v or xsb -h won't create .xsb directory */
	set_xsbinfo_dir();

	current_inst = inst_begin_gl;   // current_inst is thread-specific.

	init_message_queue(&mq_table[0], MQ_CHECK_FLAGS);
	init_message_queue(&mq_table[max_threads_glc], MQ_CHECK_FLAGS);

	return(0);

   } else if (flag == XSB_EXECUTE) {  /* continue execution */

     return(emuloop(CTXTc current_inst));

   } else if (flag == XSB_SETUP_X) {  /* initialize for call to XSB, saving argc regs */
     Psc term_psc;
     CPtr term_ptr;
     term_psc = get_ret_psc((byte)argc);
     term_ptr = (CPtr)build_call(CTXTc term_psc);
     bld_cs((reg+1),((Cell)term_ptr));

     return(emuloop(CTXTc (pb)get_ep(c_callloop_psc)));

   } else if (flag == XSB_SHUTDOWN) {  /* shutdown xsb */

#ifdef FOREIGN
#ifndef FOREIGN_ELF
#ifndef FOREIGN_WIN32
     if (fopen(tfile, "r")) unlink(tfile);
#endif
#endif
#endif

     if (xsb_mode != C_CALLING_XSB && flags[BANNER_CTL]%NOBANNER) {
       realtime = real_time() - realtime;
       fprintf(stdmsg, "\nEnd XSB (cputime %.2f secs, elapsetime ",
	       cpu_time());
       if (realtime < 600.0)
	 fprintf(stdmsg, "%.2f secs)\n", realtime);
       else
	 fprintf(stdmsg, "%.2f mins)\n", realtime/60.0);
     }
     return(0);
   }
   return(1);
}  /* end of xsb() */

/*======================================================================*/
