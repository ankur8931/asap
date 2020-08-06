/* File:      context.h
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
** $Id: context.h,v 1.93 2013-05-06 21:10:24 dwarren Exp $
**
*/

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "cell_def_xsb.h"
#include "basictypes.h"
#include "basicdefs.h"

#include "setjmp_xsb.h"
#include "flag_defs_xsb.h"
#include "hashtable_xsb.h"
#include "conc_compl.h"

/* Note that ClRef pointers typically point to the end of a ClRef, so
   to access the components, the pointer must be decremented! */

typedef struct ClRefHdr
{	UInteger buflen ;
	struct ClRefHdr *prev ;
}	*ClRef, ClRefData, ClRefHdr ;

#define ClRef_Buflen(CLREF)        ( (CLREF)->buflen )
#define ClRef_Prev(CLREF)          ( (CLREF)->prev )

struct xsb_token_t {
  int type;
  char *value;
  int nextch;
};

struct funstktype {
    char *fun;		/* functor name */
    int funop;	        /* index into opstk of first operand */
    int funtyp; 	/* 0 if functor, 1 if list, 2 if dotted-tail list */
};

struct opstktype {
    int typ;
    prolog_term op;
};

#define MAXVAR 1000
struct vartype {
  Cell varid;
  prolog_term varval;
};

struct sort_par_spec {
  long sort_num_pars;
  long sort_par_dir[10];
  long sort_par_ind[10];
};

struct random_seeds_t {
  short IX;
  short IY;
  short IZ;

  double TX;
  double TY;
  double TZ;
};

struct asrtBuff_t {
 char *Buff;
 int Buff_size;
 int *Loc;
 int BLim;
 int Size;
};

#define ERRTYPELEN 1024
#define ERRMSGLEN 4096

struct ccall_error_t {
  char ccall_error_type[ERRTYPELEN];
  char ccall_error_message[ERRMSGLEN];
  //  char ccall_error_backtrace
};

typedef struct Message_Queue_Cell *MQ_Cell_Ptr;
typedef struct Message_Queue_Cell {
  MQ_Cell_Ptr          next;
  MQ_Cell_Ptr          prev;
  int                  size;
} MQ_Cell;

#define MAX_RETRACTED_CLAUSES   20

#ifdef MULTI_THREAD

#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>

#include "debugs/debug_attv.h"

#include "basictypes.h"
#include "memory_xsb.h"
#include "varstring_xsb.h"
#include "dynamic_stack.h"
#include "psc_xsb.h"
#include "tries.h"
#include "choice.h"
#include "tab_structs.h"
#include "token_defs_xsb.h"
#include "odbc_def_xsb.h"
#include "findall.h"
#include "struct_manager.h"

//BELOW INCLUDES ARE FOR SQL Interfaces
#ifdef CYGWIN
#define FAR
#include "sql.h"
#else
#ifdef WIN_NT
#include <windows.h>
#include <sql.h>
#endif
#endif
//end of SQL related includes.

#define MAX_REGS 257

/************************************************************************/
struct th_context
{
/* System & user Flags */

  int _call_intercept ;

/* The SLG-WAM data regions
   ------------------------ */

  System_Stack	_pdl,           /* PDL                   */
    _glstack,	/* Global + Local Stacks */
    _tcpstack,	/* Trail + CP Stack      */
    _complstack;	/* Completion Stack  */

/* Argument Registers
   ------------------ */
  Cell _reg[MAX_REGS];


/* Special Registers
   ----------------- */
  CPtr _ereg;		/* last activation record       */
  CPtr _breg;		/* last choice point            */
  CPtr _hreg;		/* top of heap                  */
  CPtr *_trreg;		/* top of trail stack           */
  CPtr _hbreg;		/* heap back track point        */
  CPtr _sreg;		/* current build or unify field */
  byte *_cpreg;		/* return point register        */
  byte *_pcreg;		/* program counter              */
  CPtr _ebreg;		/* breg into environment stack	*/

  CPtr _efreg;
  CPtr _bfreg;
  CPtr _hfreg;
  CPtr *_trfreg;
  CPtr _pdlreg;
  CPtr _openreg;
  xsbBool _neg_delay;
  int _xwammode;
  int _level_num;
  CPtr _root_address;

  CPtr _ptcpreg;
  CPtr _delayreg;

  int _asynint_code;
  int _asynint_val;

  CPtr _smodels;
  CPtr _api;
  CPtr _atoms;
  int _curatom;
  int _totatoms;

  int _callAbsStk_index;
  AbstractionFrame *_callAbsStk;
  int _callAbsStk_size;
  int _can_abstract;
  int _vcs_tnot_call;

  CTptr _cycle_trail;
  int _cycle_trail_size;
  int _cycle_trail_top;

  /* Used for by trie instructions */
  Cell *_trieinstr_unif_stk;
  CPtr _trieinstr_unif_stkptr;
  Integer  _trieinstr_unif_stk_size;

  int  _num_heap_term_vars;
  CPtr *_var_addr;
  int  _var_addr_arraysz;

  Cell *_VarEnumerator;

  int _current_num_trievars;

  CPtr *_trieinstr_vars;
  int  _trieinstr_vars_num ;

  CPtr *_VarEnumerator_trail;
  CPtr *_VarEnumerator_trail_top;

  int _addr_stack_index;
  CPtr *_Addr_Stack;
  size_t _addr_stack_size;

  int  _term_stack_index;
  Cell *_term_stack;
  byte *_term_mod_stack;
  size_t _term_stacksize;

  int *_depth_stack;
  int _depth_stacksize;

  int _global_trieinstr_vars_num;

  Cell _TrieVarBindings[DEFAULT_NUM_TRIEVARS];

  struct tif_list _private_tif_list;
  DelTFptr _private_deltf_chain_begin;
  DelCFptr _private_delcf_chain_begin;

  BTNptr  _NodePtr,
    _Last_Nod_Sav;

  int     _delay_it;

  int	_AnsVarCtr;
  CPtr	_ans_var_pos_reg;

/* Flag used in the locking of called tries */
  int	trie_locked;

  IGRptr _IGRhead;

  /* Variables for subsumptive and TST tries.  */
struct VariantContinuation *_variant_cont;
struct tstCCPStack_t *_tstCCPStack;
struct tstCPStack_t *_tstCPStack;
CPtr *_trail_base;
CPtr *_orig_trreg;
CPtr _orig_hreg;
CPtr _orig_hbreg;
CPtr _orig_ebreg;

DynamicStack  _tstTermStack;
DynamicStack  _tstTermStackLog;
DynamicStack  _tstSymbolStack;
DynamicStack  _tstTrail;

  /* Error checking for TST unification */
  BTNptr _gAnsLeaf;
  CPtr _gAnsTmplt;
  int _gSizeTmplt;

  /* for skiplist (1-level) to speed up tsiOrderedInsert */
  SL_node_ptr _SL_header;

  /* delay, simplification, etc. */
  Cell _cell_array[MAXTERMBUFSIZE];
  CPtr *_var_addr_accum;
  int _var_addr_accum_num;
  int _var_addr_accum_arraysz;

#define MAX_SIMPLIFY_NEG_FAILS_STACK 10
  VariantSF _simplify_neg_fails_stack[MAX_SIMPLIFY_NEG_FAILS_STACK];
  Integer _simplify_neg_fails_stack_top;
  int _in_simplify_neg_fails;

  /* Variables for table traversal for abolishing tables */
  int _trans_abol_answer_stack_top;
  BTNptr * _trans_abol_answer_stack;
  int _trans_abol_answer_stack_size;

int      _trans_abol_answer_array_top;
BTNptr * _trans_abol_answer_array;
int      _trans_abol_answer_array_size;

  int _ta_done_subgoal_stack_top;
  VariantSF * _ta_done_subgoal_stack;
  int _ta_done_subgoal_stack_size;
  //  int _ta_done_subgoal_stack_current_pos;

  int _done_tif_stack_top;
  TIFptr * _done_tif_stack;
  int _done_tif_stack_size;

  /********* Variables for array of interned tries *********/
  int _itrie_array_first_free;
  int _itrie_array_last_free;
  int _itrie_array_first_trie;
  struct interned_trie_t* _itrie_array;

  /* for backtrackable updates & assoc arrays (storage_xsb) */
  xsbHashTable _bt_storage_hash_table;

  /********** variables for findall buffers **********/
findall_solution_list *_findall_solutions;  /*= NULL;*/
findall_solution_list *_current_findall;
int 	_nextfree ; /* nextfree index in findall array */
CPtr 	_gl_bot, _gl_top ;
f_tr_chunk *_cur_tr_chunk ;
CPtr 	*_cur_tr_top ;
CPtr 	*_cur_tr_limit ;

VarString **_LSBuff; /* 30 buffers for making longstring in ctop_longstring */

  /********** Global thread-specific charstring buffers for local use
	      within io-builtins **********/

VarString *_last_answer;  /* for c-calling-xsb interface */
VarString *_tsgLBuff1;
VarString *_tsgLBuff2;
VarString *_tsgSBuff1;
VarString *_tsgSBuff2;
/* read_canonical stacks */
int _opstk_size;
int _funstk_size;
struct funstktype *_funstk;
struct opstktype *_opstk;
struct vartype *_rc_vars;

  forestLogBuffer _forest_log_buffer_1;
  forestLogBuffer _forest_log_buffer_2;
  forestLogBuffer _forest_log_buffer_3;

  forest_log_buffer_struct _fl_buffer_1;
  forest_log_buffer_struct _fl_buffer_2;
  forest_log_buffer_struct _fl_buffer_3;

  /********** Global variables for tokenizing **********/
struct xsb_token_t *_token;
int     _lastc; // = ' ';    /* previous character */
byte*   _strbuff; // = NULL;  /* Pointer to token buffer; Will be allocated on first call to GetToken */
int     _strbuff_len; // = InitStrLen;  /* first allocation size, doubled on subsequent overflows */
double  _double_v;
Integer	_rad_int;
int _token_too_long_warning;

struct sort_par_spec _par_spec;		/* spec for par_sort */

  /********** Global variables for assert / retract **********/
  /* used for C-level longjumps in assert */
jmp_buf _assertcmp_env;

ClRef _retracted_buffer[MAX_RETRACTED_CLAUSES+1];
ClRef *_OldestCl;
ClRef *_NewestCl;
struct asrtBuff_t *_asrtBuff;	/* assert code buffer */
int    _i_have_dyn_mutex;	/* This thread has dynamic mutex, for asserted code read */

struct random_seeds_t *_random_seeds;	/* struct containing seeds for random num gen */

  /* Pointers to common structure managers (table vs. assert) */
  struct Structure_Manager *_smBTN;
  struct Structure_Manager *_smBTHT;

  /* private structure managers */
  /* for tables */
  Structure_Manager *_private_smTableBTN;
  Structure_Manager *_private_smTableBTHT;
  Structure_Manager *_private_smAssertBTN;
  Structure_Manager *_private_smAssertBTHT;
  Structure_Manager *_private_smTableBTHTArray;
  Structure_Manager *_private_smTSTN;
  Structure_Manager *_private_smTSTHT;
  Structure_Manager *_private_smTSIN;
  Structure_Manager *_private_smVarSF;
  Structure_Manager *_private_smProdSF;
  Structure_Manager *_private_smConsSF;
  Structure_Manager *_private_smALN;
  Structure_Manager *_private_smASI;

  /* for dynamic code */
  Structure_Manager *_private_smDelCF;

  int    _threads_current_sm;

  char *_private_current_de_block;
  char *_private_current_dl_block;
  char *_private_current_pnde_block;

  DE _private_released_des;
  DL _private_released_dls;
  PNDE _private_released_pndes;

  DE _private_next_free_de;
  DL _private_next_free_dl;
  PNDE _private_next_free_pnde;

  DE _private_current_de_block_top;
  DL _private_current_dl_block_top;
  PNDE _private_current_pnde_block_top;

  /********** Error handling  **********/

byte *_catch_scope_marker;
jmp_buf _xsb_abort_fallback_environment;

  /********** cinterf stuff  **********/
  jmp_buf _cinterf_env;
  byte *_current_inst;
  volatile int _xsb_inquery;
  volatile int _xsb_ready;
  pthread_cond_t _xsb_started_cond;
  pthread_cond_t _xsb_done_cond;
  pthread_mutex_t _xsb_synch_mut;
  pthread_mutex_t _xsb_ready_mut;
  pthread_mutex_t _xsb_query_mut;
  struct ccall_error_t _ccall_error;

  /********** Message Queue State  **********/
  MQ_Cell_Ptr _current_mq_cell;

 /************ Pointers to cursor information used by
 odbc_xsb.c context-local cursor table ***********/

struct ODBC_Cursor *_FCursor;  /* root of curser chain*/
struct ODBC_Cursor *_LCursor;  /* tail of curser chain*/
struct NumberofCursors *_FCurNum;

#define MAX_BIND_VALS 30
char *_term_string[MAX_BIND_VALS];

/* Private flags */

Cell _pflags[MAX_PRIVATE_FLAGS];

/* Thread Id (for fast access) */

  Thread_T tid;

/* stuff for deadlock detection in completion */
#ifdef SHARED_COMPL_TABLES
  Integer waiting_for_tid;
	int is_deadlock_leader;
int reset_thread;
struct th_context *tmp_next;
struct subgoal_frame *	waiting_for_subgoal;
#endif

#ifdef CONC_COMPL
int completing ;
int completed ;
int last_ans ;
CPtr cc_leader ;
ThreadDepList TDL ;
pthread_cond_t cond_var ;
#endif

int _num_gc;
double _total_time_gc;
UInteger _total_collected;

/* Heap realloc and garbage collection stuff */

CPtr	_heap_bot;
CPtr	_heap_top;
CPtr	_ls_bot;
CPtr 	_ls_top;
CPtr	_tr_bot;
CPtr	_tr_top;
CPtr	_cp_bot;
CPtr	_cp_top;
CPtr	_compl_top;
CPtr	_compl_bot;

size_t _heap_marks_size;

char	*_heap_marks;
char	*_ls_marks;
char	*_tr_marks;
char	*_cp_marks;

CPtr	 *_slide_buf;
UInteger _slide_top;
int	_slide_buffering;
UInteger _slide_buf_size;

int	_gc_offset;
CPtr	_gc_scan;
CPtr	_gc_next;

/* enabling and disabling thread_cancel */
unsigned int	enable_cancel : 1 ;
unsigned int	to_be_cancelled :  1 ;

/* allowing blocked threads to be wake up by signals */
pthread_cond_t * cond_var_ptr ;
} ;

typedef struct th_context th_context ;


#define xsb_thread_id           (Integer)(th -> tid)
#define xsb_thread_entry        (Integer)(THREAD_ENTRY(th -> tid))

#define call_intercept		(th->_call_intercept)

#define pdl			(th->_pdl)
#define glstack			(th->_glstack)
#define tcpstack		(th->_tcpstack)
#define complstack		(th->_complstack)

#define reg			(th->_reg)

#define ereg			(th->_ereg)
#define breg			(th->_breg)
#define hreg			(th->_hreg)
#define trreg			(th->_trreg)
#define hbreg			(th->_hbreg)
#define sreg			(th->_sreg)
#define cpreg			(th->_cpreg)
#define pcreg			(th->_pcreg)
#define ebreg			(th->_ebreg)

#define efreg			(th->_efreg)
#define bfreg			(th->_bfreg)
#define hfreg			(th->_hfreg)
#define trfreg			(th->_trfreg)

#define pdlreg			(th->_pdlreg)
#define openreg			(th->_openreg)
#define neg_delay		(th->_neg_delay)
#define xwammode		(th->_xwammode)
#define level_num		(th->_level_num)
#define root_address		(th->_root_address)

#define ptcpreg			(th->_ptcpreg)
#define delayreg		(th->_delayreg)

#define asynint_code		(th->_asynint_code)
#define asynint_val		(th->_asynint_val)

#define trieinstr_unif_stk		(th->_trieinstr_unif_stk)
#define trieinstr_unif_stkptr		(th->_trieinstr_unif_stkptr)
#define trieinstr_unif_stk_size		(th->_trieinstr_unif_stk_size)

#define callAbsStk_index          (th-> _callAbsStk_index)
#define callAbsStk                (th->_callAbsStk)
#define callAbsStk_size           (th-> _callAbsStk_size)
#define can_abstract              (th-> _can_abstract)
#define vcs_tnot_call             (th-> _vcs_tnot_call)

#define cycle_trail               (th-> _cycle_trail)
#define cycle_trail_size          (th-> _cycle_trail_size)
#define cycle_trail_top           (th-> _cycle_trail_top)

#define trieinstr_vars		(th->_trieinstr_vars)
#define trieinstr_vars_num	(th->_trieinstr_vars_num)

#define current_num_trievars    (th->_current_num_trievars)

#define num_heap_term_vars	(th->_num_heap_term_vars)
#define var_addr		(th->_var_addr)
#define var_addr_arraysz	(th->_var_addr_arraysz)
#define var_addr_accum_arraysz	(th->_var_addr_accum_arraysz)

#define VarEnumerator		(th->_VarEnumerator)
#define TrieVarBindings		(th->_TrieVarBindings)

#define VarEnumerator_trail	(th->_VarEnumerator_trail)
#define VarEnumerator_trail_top	(th->_VarEnumerator_trail_top)

#define addr_stack_index	(th->_addr_stack_index)
#define Addr_Stack		(th->_Addr_Stack)
#define addr_stack_size		(th->_addr_stack_size)

#define term_stack_index		(th->_term_stack_index)
#define term_stack		(th->_term_stack)
#define term_mod_stack		(th->_term_mod_stack)
#define term_stacksize		(th->_term_stacksize)

#define depth_stack		(th->_depth_stack)
#define depth_stacksize		(th->_depth_stacksize)

#define global_trieinstr_vars_num		(th->_global_trieinstr_vars_num)

#define private_tif_list        (th-> _private_tif_list)
#define private_deltf_chain_begin (th-> _private_deltf_chain_begin)
#define private_delcf_chain_begin (th-> _private_delcf_chain_begin)

#define NodePtr			(th->_NodePtr)
#define Last_Nod_Sav		(th->_Last_Nod_Sav)

#define delay_it		(th->_delay_it)

#define IGRhead                 (th->_IGRhead)

#define variant_cont		(*(th->_variant_cont))
#define a_variant_cont		(th->_variant_cont)
#define tstCCPStack		(*(th->_tstCCPStack))
#define a_tstCCPStack		(th->_tstCCPStack)
#define tstCPStack		(*(th->_tstCPStack))
#define a_tstCPStack		(th->_tstCPStack)

#define trail_base		(th->_trail_base)
#define orig_trreg		(th->_orig_trreg)
#define orig_hreg		(th->_orig_hreg)
#define orig_hbreg		(th->_orig_hbreg)
#define orig_ebreg		(th->_orig_ebreg)

#define findall_solutions	(th->_findall_solutions)
#define current_findall		(th->_current_findall)
#define nextfree		(th->_nextfree)
#define gl_bot			(th->_gl_bot)
#define gl_top			(th->_gl_top)
#define cur_tr_chunk		(th->_cur_tr_chunk)
#define cur_tr_top		(th->_cur_tr_top)
#define cur_tr_limit		(th->_cur_tr_limit)

#define fl_buffer_1             (th->_fl_buffer_1)
#define fl_buffer_2             (th->_fl_buffer_2)
#define fl_buffer_3             (th->_fl_buffer_3)

#define forest_log_buffer_1     (th->_forest_log_buffer_1)
#define forest_log_buffer_2     (th->_forest_log_buffer_2)
#define forest_log_buffer_3     (th->_forest_log_buffer_3)

#define LSBuff			(th->_LSBuff)

#define last_answer           (th->_last_answer)
#define tsgLBuff1		(th->_tsgLBuff1)
#define tsgLBuff2		(th->_tsgLBuff2)
#define tsgSBuff1		(th->_tsgSBuff1)
#define tsgSBuff2		(th->_tsgSBuff2)

#define opstk_size		(th->_opstk_size)
#define funstk_size		(th->_funstk_size)
#define funstk			(th->_funstk)
#define opstk			(th->_opstk)
#define rc_vars			(th->_rc_vars)

#define token			(th->_token)
#define lastc			(th->_lastc)
#define strbuff			(th->_strbuff)
#define strbuff_len		(th->_strbuff_len)
#define double_v		(th->_double_v)
#define rad_int			(th->_rad_int)
#define token_too_long_warning  (th->_token_too_long_warning)

#define par_spec		(th->_par_spec)

#define assertcmp_env		(th->_assertcmp_env)
#define cinterf_env             (th->_cinterf_env)
#define current_inst            (th->_current_inst)
#define xsb_inquery             (th-> _xsb_inquery)
#define xsb_ready               (th-> _xsb_ready)
#define xsb_done_cond           (th-> _xsb_done_cond)
#define xsb_started_cond        (th-> _xsb_started_cond)
#define xsb_synch_mut           (th-> _xsb_synch_mut)
#define xsb_ready_mut           (th-> _xsb_ready_mut)
#define xsb_query_mut           (th-> _xsb_query_mut)
#define ccall_error             (th-> _ccall_error)

#define current_mq_cell         (th->_current_mq_cell)

#define retracted_buffer	(th->_retracted_buffer)
#define OldestCl		(th->_OldestCl)
#define NewestCl		(th->_NewestCl)

#define xsb_abort_fallback_environment (th->_xsb_abort_fallback_environment)
#define catch_scope_marker            (th->_catch_scope_marker)

#define random_seeds		(th->_random_seeds)

#define asrtBuff              (th->_asrtBuff)
#define i_have_dyn_mutex      (th->_i_have_dyn_mutex)

#define AnsVarCtr		(th->_AnsVarCtr)
#define ans_var_pos_reg		(th->_ans_var_pos_reg)

/******/

#define smBTN			(th->_smBTN)
#define smBTHT			(th->_smBTHT)

#define private_smTableBTN        (th->_private_smTableBTN)
#define private_smTableBTHT       (th->_private_smTableBTHT)
#define private_smAssertBTN        (th->_private_smAssertBTN)
#define private_smAssertBTHT       (th->_private_smAssertBTHT)
#define private_smTableBTHTArray       (th->_private_smTableBTHTArray)
#define private_smTSTN          (th-> _private_smTSTN)
#define private_smTSTHT         (th-> _private_smTSTHT)
#define private_smTSIN          (th-> _private_smTSIN)
#define private_smVarSF         (th-> _private_smVarSF)
#define private_smProdSF        (th-> _private_smProdSF)
#define private_smConsSF        (th-> _private_smConsSF)
#define private_smALN           (th-> _private_smALN)
#define private_smASI           (th-> _private_smASI)

#define private_smDelCF        (th -> _private_smDelCF)

#define subsumptive_smALN       (*private_smALN)
#define subsumptive_smBTN       (*private_smTableBTN)
#define subsumptive_smBTHT      (*private_smTableBTHT)

#define private_current_de_block    (th-> _private_current_de_block)
#define private_current_dl_block    (th-> _private_current_dl_block)
#define private_current_pnde_block  (th-> _private_current_pnde_block)

#define private_released_des       (th-> _private_released_des)
#define private_released_dls       (th-> _private_released_dls)
#define private_released_pndes     (th-> _private_released_pndes)

#define private_next_free_de  	   (th-> _private_next_free_de)
#define private_next_free_dl	   (th-> _private_next_free_dl)
#define private_next_free_pnde	   (th-> _private_next_free_pnde)

#define private_current_de_block_top     (th-> _private_current_de_block_top)
#define private_current_dl_block_top     (th-> _private_current_dl_block_top)
#define private_current_pnde_block_top   (th-> _private_current_pnde_block_top)

#define threads_current_sm      (th->_threads_current_sm)

/* For now, Subsumptive-tables are all private*/
#define smProdSF                (*private_smProdSF)
#define smConsSF                (*private_smConsSF)
#define smTSTN                  (*private_smTSTN)
#define smTSIN                  (*private_smTSIN)
#define smTSTHT                 (*private_smTSTHT)

/******/

#define  tstTermStack		(th->_tstTermStack)
#define  tstTermStackLog	(th->_tstTermStackLog)
#define  tstSymbolStack		(th->_tstSymbolStack)
#define  tstTrail		(th->_tstTrail)

#define  gAnsLeaf               (th->_gAnsLeaf)
#define  gAnsTmplt              (th->_gAnsTmplt)
#define  gSizeTmplt             (th->_gSizeTmplt)
#define  SL_header		(th->_SL_header)

#define  cell_array                         (th->_cell_array)
#define  var_addr_accum                     (th->_var_addr_accum)
#define  var_addr_accum_num                 (th->_var_addr_accum_num)

#define  simplify_neg_fails_stack           (th->_simplify_neg_fails_stack)
#define  simplify_neg_fails_stack_top       (th->_simplify_neg_fails_stack_top)
#define  in_simplify_neg_fails              (th->_in_simplify_neg_fails)

#define  trans_abol_answer_stack_top                  (th->_trans_abol_answer_stack_top)
#define  trans_abol_answer_stack                      (th->_trans_abol_answer_stack)
#define  trans_abol_answer_stack_size                 (th->_trans_abol_answer_stack_size)

#define  trans_abol_answer_array_top                  (th->_trans_abol_answer_array_top)
#define  trans_abol_answer_array                      (th->_trans_abol_answer_array)
#define  trans_abol_answer_array_size                 (th->_trans_abol_answer_array_size)

#define  ta_done_subgoal_stack_top           (th->_ta_done_subgoal_stack_top)
#define  ta_done_subgoal_stack               (th->_ta_done_subgoal_stack)
#define  ta_done_subgoal_stack_size          (th->_ta_done_subgoal_stack_size)

#define  done_tif_stack_top           (th->_done_tif_stack_top)
#define  done_tif_stack               (th->_done_tif_stack)
#define  done_tif_stack_size          (th->_done_tif_stack_size)

#define  itrie_array_first_free       (th->_itrie_array_first_free)
#define  itrie_array_last_free        (th->_itrie_array_last_free)
#define  itrie_array_first_trie       (th->_itrie_array_first_trie)
#define  itrie_array                  (th->_itrie_array)

#define  bt_storage_hash_table             (th-> _bt_storage_hash_table)

#define FCursor (th->_FCursor)
#define LCursor (th->_LCursor)
#define FCurNum (th->_FCurNum)
#define term_string (th->_term_string)

#define  pflags			(th->_pflags)

#define num_gc                  (th->_num_gc)
#define total_time_gc           (th->_total_time_gc)
#define total_collected         (th->_total_collected)

#define	heap_bot		(th->_heap_bot)
#define	heap_top		(th->_heap_top)
#define	ls_bot			(th->_ls_bot)
#define ls_top			(th->_ls_top)
#define	tr_bot			(th->_tr_bot)
#define	tr_top			(th->_tr_top)
#define	cp_bot			(th->_cp_bot)
#define	cp_top			(th->_cp_top)
#define	compl_top		(th->_compl_top)
#define	compl_bot		(th->_compl_bot)
#define heap_marks_size		(th->_heap_marks_size)

#define	heap_marks		(th->_heap_marks)
#define	ls_marks		(th->_ls_marks)
#define	tr_marks		(th->_tr_marks)
#define	cp_marks		(th->_cp_marks)

#define	slide_buf		(th->_slide_buf)
#define slide_top		(th->_slide_top)
#define	slide_buffering		(th->_slide_buffering)
#define slide_buf_size		(th->_slide_buf_size)

#define gc_offset		(th->_gc_offset)
#define gc_scan			(th->_gc_scan)
#define gc_next			(th->_gc_next)


#define CTXT			th
#define CTXTc			th ,

#define CTXTdecl		th_context *th
#define CTXTdeclc		th_context *th ,

#define CTXTdecltype		th_context *
#define CTXTdecltypec		th_context *,

#else

#define CTXT
#define CTXTc

#define CTXTdecl
#define CTXTdeclc

#define CTXTdecltype
#define CTXTdecltypec

#define subsumptive_smBTN        smTableBTN
#define subsumptive_smBTHT       smTableBTHT


#endif /* MULTI_THREAD */

#ifdef MULTI_THREAD
extern xsbBucket *search_bucket(th_context *th, Cell name, xsbHashTable *tbl,
				enum xsbHashSearchOp search_op);
#else
extern xsbBucket *search_bucket(Cell name, xsbHashTable *tbl,
				enum xsbHashSearchOp search_op);
#endif

#endif /* __CONTEXT_H__ */
