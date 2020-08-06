/* File:      builtin.c
** Author(s): Xu, Warren, Sagonas, Swift, Freire, Johnson
** Contact:   xsb-contact@cs.sunysb.edu
**
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1999
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
** $Id: builtin.c,v 1.400 2013-05-06 21:10:24 dwarren Exp $
**
*/

#include "xsb_config.h"
#include <stdio.h>
#include "xsb_debug.h"

/* Private debugs */
#include "debugs/debug_delay.h"
#include "context.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>

#ifdef WIN_NT
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <process.h>
#include <stdarg.h>
#include <winsock.h>
#include "wsipx.h"
#include <tchar.h>
#else /* Unix */
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <fcntl.h>

#include "auxlry.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "psc_xsb.h"

#include "ubi_BinTree.h"
#include "ubi_SplayTree.h"

#include "hash_xsb.h"
#include "tries.h"
#include "choice.h"
#include "deref.h"
#include "memory_xsb.h"
#include "heap_xsb.h"
#include "register.h"
#include "flags_xsb.h"
#include "loader_xsb.h"
#include "binding.h"
#include "tab_structs.h"
#include "builtin.h"
#include "sig_xsb.h"
#include "subp.h"
#include "tr_utils.h"
#include "trassert.h"
#include "dynload.h"
#include "cinterf.h"
#include "residual.h"
#include "tables.h"
#include "trie_internals.h"
#include "table_status_defs.h"
#include "rw_lock.h"
#include "deadlock.h"
#include "struct_intern.h"
#include "trace_xsb.h"
#ifdef ORACLE
#include "oracle_xsb.h"
#endif

#ifdef XSB_ODBC
#include "odbc_xsb.h"
#endif

#ifdef XSB_INTERPROLOG
#include "interprolog_xsb.h"
#endif

#ifdef PROFILE
#include "inst_xsb.h"
#include "subinst.h"
#endif

#include "io_builtins_xsb.h"
#include "storage_xsb.h"

/* wind2unix.h must be included after sys/stat.h */
#include "wind2unix.h"
#include "system_xsb.h"
#include "random_xsb.h"
#include "thread_xsb.h"
#ifdef DEMAND
#include "demand.h"
#endif
#include "debug_xsb.h"

#include "thread_xsb.h"
/* incremental evaluation */

#include "incr_xsb.h"
#include "call_graph_xsb.h"

#include "table_stats.h"
#include "url_encode.h"
#include "table_inspection_defs.h"

#include "unify_xsb.h"
#include "token_xsb.h"
#include "cell_xsb_i.h"

int mem_flag;

/*======================================================================*/
extern struct xsb_token_t *GetToken(CTXTdeclc int, int);

extern int  sys_syscall(CTXTdeclc int);
extern xsbBool sys_system(CTXTdeclc int);
extern xsbBool formatted_io(CTXTdecl), read_canonical(CTXTdecl);
extern xsbBool private_builtin(void);

extern void xsb_segfault_quitter(int err);
extern void alt_print_cp(CTXTdeclc char *);
extern int xsb_profiling_enabled;

extern char *canonical_term(CTXTdeclc Cell, int);
extern int prolog_call0(CTXTdeclc Cell);
extern int prolog_code_call(CTXTdeclc Cell, int);

extern void init_psc_ep_info(Psc psc);
extern Integer intern_term_size(CTXTdeclc Cell);

int is_cyclic(CTXTdeclc Cell);
int ground_cyc(CTXTdeclc Cell, int);

#ifdef WIN_NT
extern xsbBool startInterruptThread(SOCKET intSocket);
#endif

// Externs for profiler

Integer if_profiling = 0;
Integer profile_thread_started = 0;
Integer prof_gc_count = 0;
Integer prof_int_count = 0;
static Integer prof_unk_count = 0;

static Integer total_prog_segments = 0;
static Integer prof_table_length = 0;
static Integer prof_table_count = 0;

/*static Psc colon_psc = NULL;*/

extern xsbBool startProfileThread();
extern void dump_prof_table();
extern void retrieve_prof_table();

// Externs for assert/retract
extern xsbBool assert_code_to_buff(CTXTdecl), assert_buff_to_clref(CTXTdecl);
extern xsbBool gen_retract_all(CTXTdecl), db_retract0(CTXTdecl),
  db_get_clause(CTXTdecl);
extern xsbBool db_get_last_clause(CTXTdecl);
extern xsbBool db_build_prref(CTXTdecl), db_abolish0(CTXTdecl),
	       db_reclaim0(CTXTdecl), db_get_prref(CTXTdecl);
extern xsbBool dynamic_code_function(CTXTdecl);
extern xsbBool table_inspection_function(CTXTdecl);

extern char *dirname_canonic(char *);
extern xsbBool almost_search_module(CTXTdeclc char *);
extern char *expand_filename(char *filename);
extern char *existing_file_extension(char *);
extern char *tilde_expand_filename(char *filename);
extern xsbBool is_absolute_filename(char *filename);
extern void parse_filename(char *filenam, char **dir, char **base, char **ext);

int print_xsb_backtrace(CTXTdecl);
prolog_term build_xsb_backtrace(CTXTdecl);

extern xsbBool xsb_socket_request(CTXTdecl);

extern int  findall_init(CTXTdecl), findall_add(CTXTdecl),
  findall_get_solutions(CTXTdecl);
extern int  copy_term(CTXTdecl);
extern int  copy_term_3(CTXTdecl);

extern xsbBool substring(CTXTdecl);
extern xsbBool string_substitute(CTXTdecl);
extern xsbBool str_cat(CTXTdecl);
extern xsbBool str_sub(void);
extern xsbBool str_match(CTXTdecl);

// For force_truth_value (which may not be used much)
extern void force_answer_true(BTNptr);
extern void force_answer_false(BTNptr);

// catch/throw.
extern int set_scope_marker(CTXTdecl);
extern int unwind_stack(CTXTdecl);
extern int clean_up_block(CTXTdeclc int);

extern double realtime_count_gl; /* from subp.c */

extern BTNptr trie_asserted_trienode(CPtr clref);
extern int gc_dynamic(CTXTdecl);

extern int sha1_string(prolog_term, char *);
extern int md5_string(prolog_term, char *);

/* ------- variables also used in other parts of the system -----------	*/

Cell flags[MAX_FLAGS];			  /* System flags + user flags */
#ifndef MULTI_THREAD
Cell pflags[MAX_PRIVATE_FLAGS];		  /* Thread private flags */
#endif

/* ------- utility routines -------------------------------------------	*/


#include "ptoc_tag_xsb_i.h"


DllExport prolog_int call_conv ptoc_int(CTXTdeclc int regnum)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);

  switch (cell_tag(addr)) {
  case XSB_STRUCT:
    if (isboxedinteger(addr)) return(boxedint_val(addr));
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
  case XSB_LIST:
  case XSB_FLOAT: xsb_abort("[PTOC_INT] Argument %d of illegal type: %s\n",
			    regnum, canonical_term(CTXTc addr, 0));
  case XSB_STRING: return (prolog_int)string_val(addr);	/* dsw */
  case XSB_INT: return int_val(addr);
  default: xsb_abort("[PTOC_INT] Argument %d of unknown type: %d",regnum,cell_tag(addr));
  }
  return FALSE;
}

/* TLS: unlike ptoc_int, this does NOT cast strings to integers */
DllExport prolog_int call_conv iso_ptoc_int(CTXTdeclc int regnum,const char * PredString)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);

  switch (cell_tag(addr)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV: xsb_instantiation_error(CTXTc PredString,regnum);
  case XSB_STRUCT:
    if (isboxedinteger(addr)) return(boxedint_val(addr));
  case XSB_LIST:
  case XSB_STRING:
  case XSB_FLOAT: xsb_type_error(CTXTc "integer",addr,PredString,regnum);

  case XSB_INT: return int_val(addr);
  default: xsb_abort("[PTOC_INT] Argument %d of unknown type: %d",regnum,cell_tag(addr));
  }
  return FALSE;
}

/* TLS: unlike ptoc_int, this does NOT cast strings to integers.  Like
   iso_ptoc_int, but passes explicit argument for error message -- for
   use by thread_request and other non-builtin builtins.
*/
DllExport prolog_int call_conv iso_ptoc_int_arg(CTXTdeclc int regnum,const char * PredString, int arg)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);

  switch (cell_tag(addr)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV: xsb_instantiation_error(CTXTc PredString,arg);
  case XSB_STRUCT:
    if (isboxedinteger(addr)) return(boxedint_val(addr));
  case XSB_LIST:
  case XSB_STRING:
  case XSB_FLOAT: xsb_type_error(CTXTc "integer",addr,PredString,arg);

  case XSB_INT: return int_val(addr);
  default: xsb_abort("[PTOC_INT] Argument of unknown type: %d",cell_tag(addr));
  }
  return FALSE;
}

/** now a macro
Psc get_mod_for_psc(Psc psc) {
  Cell psc_data = (Cell)get_data(psc);
  if (isstring(psc_data)) return global_mod;
  else return (Psc)psc_data;
  } **/

char *get_filename_for_psc(Psc psc) {
  struct psc_rec *psc_data = get_data(psc);
  if (IsNULL(psc_data)) return "unknown";
  if (isstring((Cell)psc_data)) return string_val((Cell)psc_data);
  return string_val(get_ep(psc_data));
}

inline Cell iso_ptoc_callable(CTXTdeclc int regnum,const char * PredString)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);

  if ((isconstr(addr) && !isboxed(addr)) || isstring(addr) || islist(addr))
    return addr;
  else if (isref(addr))  xsb_instantiation_error(CTXTc PredString,regnum);
  else xsb_type_error(CTXTc "callable",addr,PredString,regnum);
  return FALSE;
}

/* TLS: this one is designed to pass through Prolog register offsets
   in PredString and arg -- that way ptocs for them need only be done
   if theres an error */
inline Cell iso_ptoc_callable_arg(CTXTdeclc int regnum,const int PredString,const int arg)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);

  if ((isconstr(addr) && !isboxed(addr)) || isstring(addr) || islist(addr))
    return addr;
  else if (isref(addr))  xsb_instantiation_error(CTXTc ptoc_string(CTXTc PredString),
						 (int)ptoc_int(CTXTc arg));
  else xsb_type_error(CTXTc "callable",addr,ptoc_string(CTXTc PredString),
		      (int)ptoc_int(CTXTc arg));
  return FALSE;
}

inline void iso_check_var(CTXTdeclc int regnum,const char * PredString) {
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  if (!isref(addr)) xsb_type_error(CTXTc "variable",addr,PredString,regnum);
}



DllExport prolog_float call_conv ptoc_float(CTXTdeclc int regnum)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref( addr );
  switch (cell_tag(addr)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
  case XSB_LIST:
  case XSB_INT:
  case XSB_STRING:
    xsb_abort("[PTOC_FLOAT] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
  case XSB_STRUCT:
      if (!isboxedfloat(addr))
	xsb_abort("[PTOC_FLOAT] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
  case XSB_FLOAT: return (prolog_float)ofloat_val(addr);
  default:
    xsb_abort("[PTOC_FLOAT] Argument %d of unknown type: %d",regnum,cell_tag(addr));
  }
  return 0.0;
}

/* TLS: unlike ptoc_string, this throws an error if given a boxed int. */
DllExport char* call_conv iso_ptoc_string(CTXTdeclc int regnum,char * PredString)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);
  switch (cell_tag(addr)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV: xsb_instantiation_error(CTXTc PredString,regnum);
  case XSB_STRUCT:
  case XSB_LIST:
  case XSB_INT:
  case XSB_FLOAT: xsb_type_error(CTXTc "atom",addr,PredString,regnum);
  case XSB_STRING: return string_val(addr);
  default: xsb_abort("[PTOC_INT] Argument %d of unknown type: %d",regnum,cell_tag(addr));
  }
  return FALSE;
}

DllExport char* call_conv ptoc_string(CTXTdeclc int regnum)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */

  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);
  switch (cell_tag(addr)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
  case XSB_LIST:
  case XSB_FLOAT:
    xsb_abort("[PTOC_STRING] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
  case XSB_STRUCT:  /* tentative approach to fix boxed ints --lfcastro */
    if (isboxedinteger(addr))
      return (char *)boxedint_val(addr);
    else
      xsb_abort("[PTOC_STRING] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
  case XSB_INT:
      return (char *)int_val(addr);
  case XSB_STRING:
      return string_val(addr);
  default:
    xsb_abort("[PTOC_STRING] Argument %d of unknown type: %d",regnum,cell_tag(addr));
  }
  return "";
}

/* Used to pass integer or float values to math functions
   that do the conversion. */
DllExport prolog_float call_conv ptoc_number(CTXTdeclc int regnum)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  /* XSB_Deref and then check the type */
  XSB_Deref(addr);
  switch (cell_tag(addr)) {
  case XSB_STRUCT:
    if (isboxedfloat(addr)) return(boxedfloat_val(addr));
    if (isboxedinteger(addr)) return((prolog_float)boxedint_val(addr));
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
  case XSB_LIST: xsb_abort("[PTOC_NUMBER] Float-convertable argument %d expected, found: %s",
			   regnum,canonical_term(CTXTc addr, 0));
  case XSB_FLOAT: return (prolog_float)float_val(addr);
  case XSB_STRING: return (prolog_float)(prolog_int)string_val(addr);	/* dsw */
  case XSB_INT: return (prolog_float)int_val(addr);
  default: xsb_abort("[PTOC_NUMBER] Argument %d of unknown type: %d",regnum,cell_tag(addr));
  }
  return 0.0;
}


#define MAXSBUFFS 30 /* also defined in init_xsb.c (for mt), so if change here.... */
#ifndef MULTI_THREAD
static VarString *LSBuff[MAXSBUFFS] = {NULL};
#endif

#define str_op1 (*tsgSBuff1)


/* construct a long string from prolog... concatenates atoms,
flattening lists and comma-lists, and treating small ints as ascii
codes.  Puts result in a fixed buffer (if nec.) automatically extended */

void constructString(CTXTdeclc Cell addr, int ivstr)
{
  int val;

 constructStringBegin:
  XSB_Deref(addr);
  switch (cell_tag(addr)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
  case XSB_FLOAT:
    xsb_abort("[PTOC_LONGSTRING] Argument %d of illegal type: %s",ivstr,canonical_term(CTXTc addr, 0));
  case XSB_STRUCT:
    if (get_str_psc(addr) == comma_psc) {
      constructString(CTXTc get_str_arg(addr,1),ivstr);
      addr = get_str_arg(addr,2);  /* tail recursion opt */
      goto constructStringBegin;
    } else xsb_abort("[PTOC_LONGSTRING] Argument %d of illegal type: %s",ivstr,canonical_term(CTXTc addr, 0));
  case XSB_LIST:
    constructString(CTXTc get_list_head(addr),ivstr);
    addr = get_list_tail(addr);  /* tail recursion opt */
    goto constructStringBegin;
  case XSB_INT:
    val = (int)int_val(addr);
    if (val < 256 && val >= 0) {
      XSB_StrAppendC(LSBuff[ivstr],(char)val);
      return;
    } else xsb_abort("[PTOC_LONGSTRING] Argument %d of illegal type: %s",ivstr,canonical_term(CTXTc addr, 0));
  case XSB_STRING:
    if (isnil(addr)) return;
    XSB_StrAppend(LSBuff[ivstr],string_val(addr));
    return;
  default:
    xsb_abort("[PTOC_LONGSTRING] Argument %d of unknown type: %d",ivstr,cell_tag(addr));
  }
}

DllExport char* call_conv ptoc_longstring(CTXTdeclc int regnum)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);
  XSB_Deref(addr);
  if (isstring(addr)) return string_val(addr);
  if (isointeger(addr)) return (char *)oint_val(addr);

  if (LSBuff[regnum]==NULL) {
    XSB_StrCreate(&LSBuff[regnum]);
  }
  XSB_StrSet(LSBuff[regnum],"");
  constructString(CTXTc addr,regnum);
  return(LSBuff[regnum]->string);
}

/*
 *  For decoding object pointers, like PSC, PSC-PAIR and Subgoal frames.
 */
#define ptoc_addr(regnum)	(void *)ptoc_int(CTXTc regnum)
#define is_encoded_addr(term)	(isointeger(term))
#define decode_addr(term)	(void *)oint_val(term)


/*
 *  Deref's the variable of register `regnum', trails the binding,
 *  creates an INT Cell containing `value', and binds the variable to it.
 */
DllExport void call_conv ctop_int(CTXTdeclc int regnum, prolog_int value)
{
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  if (isref(addr)) {
    bind_oint(vptr(addr),value);
  }
  else {
    xsb_abort("[CTOP_INT] Argument %d of illegal type: %s",
	      regnum, canonical_term(CTXTc addr, 0));
  }
}

/* from float value form an int node */
DllExport void call_conv ctop_float(CTXTdeclc int regnum, prolog_float value)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  if (isref(addr)) {
    bind_boxedfloat(vptr(addr), (Float)value);
  }
  else xsb_abort("[CTOP_FLOAT] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
}

/* take a C string, form a string node */
DllExport void call_conv ctop_string(CTXTdeclc int regnum, const char *value)
{
  /* reg is global array in register.h in the single-threaded engine
   * and is defined as a thread-specific macro in context.h in the
   * multi-threaded engine
   */
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  if (isref(addr)) {
    bind_string(vptr(addr), string_find(value,1));  //?? did not intern before??
  }
  else
    xsb_abort("[CTOP_STRING] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
}

/* TLS: do not need to intern here, as ctop_string is now interning */
DllExport void call_conv extern_ctop_string(CTXTdeclc int regnum, const char *value)
{
  ctop_string(CTXTc regnum,value) ;
}

inline static void ctop_constr(CTXTdeclc int regnum, Pair psc_pair)
{				/* from psc_pair ptr form an constr node */
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  if (isref(addr)) {
    bind_cs(vptr(addr), psc_pair);
  }
  else xsb_abort("[CTOP_CONSTR] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
}


/*
 *  For encoding object pointer, like PSC, PSC-PAIR and Subgoal frames.
 */
#define ctop_addr(regnum, val)    ctop_int(CTXTc regnum, (prolog_int)val)

/* --------------------------------------------------------------------	*/

int list_unifiable(CTXTdeclc Cell term) {
  XSB_Deref(term);
  while (islist(term)) {
    term = get_list_tail(term);
    XSB_Deref(term);
  }
  if (isref(term) || isnil(term)) return TRUE;
  else return FALSE;
}

/* --------------------------------------------------------------------	*/

UInteger val_to_hash(Cell term)
{
  UInteger value;

  switch(cell_tag(term)) {
    case XSB_INT:
      value = int_val(term);
      break;
    case XSB_FLOAT:
      value = int_val(term);
      break;
    case XSB_LIST:
      value = (UInteger)list_pscPair;
      break;
    case XSB_STRUCT:
      //to make a hash val for a boxed int, we take the int value inside the box and cast it
      //to a Cell.
      if (isboxedinteger(term))
      {
          value = boxedint_val(term);
          break;
      }
      //to make a hash val for a boxed float, we take the int values inside the 3 boxes for
      //the float bits, and XOR them together
      else if (isboxedfloat(term))
      {
	value = int_val(get_str_arg(term,1)) ^
	  int_val(get_str_arg(term,2)) ^
	  int_val(get_str_arg(term,3));
          break;
      }
      //but if this structure isn't any special boxed representation, then we use its PSC as
      //a hash value.
      value = (UInteger)get_str_psc(term);
      break;
    case XSB_STRING:
      value = (UInteger)string_val(term);
      break;
    default: xsb_abort("[term_hash/3] Indexing on illegal argument: (%p)",term);
      value = 0;
      break;
  }
  return value;
}

/* -------------------------------------------------------------------- */

UInteger  det_val_to_hash(Cell term)
{
  UInteger value;
  Psc psc;

  switch(cell_tag(term)) {
    case XSB_INT:
      value = int_val(term);
      break;
    case XSB_FLOAT:
      value = int_val(term);
      break;
    case XSB_LIST:
      value = (UInteger)list_pscPair;
      break;
    case XSB_STRUCT:
      //to make a hash val for a boxed int, we take the int value inside the box and cast it
      //to a Cell.
      if (isboxedinteger(term))
      {
          value = boxedint_val(term);
          break;
      }
      //to make a hash val for a boxed float, we take the int values inside the 3 boxes for
      //the float bits, and XOR them together
      else if (isboxedfloat(term))
      {
	value = int_val(get_str_arg(term,1)) ^
	  int_val(get_str_arg(term,2)) ^
	  int_val(get_str_arg(term,3));
          break;
      }
      //but if this structure isn't any special boxed representation, then we hash its name for
      //a hash value.
      psc = get_str_psc(term);
      value = hash(get_name(psc),get_arity(psc),4194301);
      break;
    case XSB_STRING:
      value = hash(string_val(term),0,4194301);
      break;
    default: xsb_abort("[term_hash/3] Indexing on illegal argument: (%p)",term);
      value = 0;
      break;
  }
  // TLS: not sure what this was for, but it will never evaluate to true.
  //  if ((UInteger)value < 0) printf("Bad Hash4");
  return value;
}

/* -------------------------------------------------------------------- */

int ground(Cell temp)
{
 int j, arity;
 groundBegin:
  XSB_Deref(temp);
  switch(cell_tag(temp)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
    return FALSE;

  case XSB_STRING:
  case XSB_INT:
  case XSB_FLOAT:
    return TRUE;

  case XSB_LIST:
    if (isinternstr_really(temp)) return TRUE;
    if (!ground(get_list_head(temp)))
      return FALSE;
    temp = get_list_tail(temp);
    goto groundBegin;

  case XSB_STRUCT:
    if (isinternstr_really(temp)) return TRUE;
    arity = (int) get_arity(get_str_psc(temp));
    if (arity == 0) return TRUE;
    for (j=1; j < arity ; j++)
      if (!ground(get_str_arg(temp,j)))
	return FALSE;
	  temp = get_str_arg(temp,arity);
    goto groundBegin;

  default:
    xsb_abort("[ground/1] Term with unknown tag (%d)",
	      (int)cell_tag(temp));
    return -1;	/* so that g++ does not complain */
  }
}

/* --------------------------------------------------------------------	*/

int is_proper_list(Cell term)	/* for standard preds */
{
  register Cell addr;

  addr = term;
  XSB_Deref(addr);
  while (islist(addr)) {
    addr = get_list_tail(addr);
    XSB_Deref(addr);
  }
  return isnil(addr);
}

/* --------------------------------------------------------------------	*/
//  Need to expand this.
#define MAX_LOCAL_TRAIL_SIZE 32*K

struct ltrail {
  CPtr *ltrail_top;
  CPtr ltrail_vals[MAX_LOCAL_TRAIL_SIZE];
};

#define local_bind_var(term,local_trail)	\
  follow(term) = makenil;			\
  *(++(local_trail)->ltrail_top) = (CPtr)term;

#define undo_ltrail_bindings(local_trail,lbase)	\
  while ((local_trail)->ltrail_top != lbase) {	\
    untrail(*(local_trail)->ltrail_top);	\
    (local_trail)->ltrail_top--;		\
    }

# define ltrail_base(local_trail) &(local_trail.ltrail_vals[0])-1;


int is_most_general_term(Cell term)
{
  struct ltrail mini_trail;

  XSB_Deref(term);
  switch (cell_tag(term)) {
  case XSB_STRING:
    return TRUE;
  case XSB_STRUCT:
    {
      Psc psc;
      CPtr taddr;
      int i, arity;
      register Cell addr;
      void *mini_trail_base;

      mini_trail.ltrail_top = mini_trail_base = ltrail_base(mini_trail);
      psc = get_str_psc(term);
      taddr = clref_val(term);
      arity = (int) get_arity(psc);

      for (i = 1; i <= arity ; ++i) {
	addr = cell(taddr+i);
	XSB_Deref(addr);
	if (isnonvar(addr)) {
	  undo_ltrail_bindings(&mini_trail,mini_trail_base);
	  return FALSE;
	} else {
	  local_bind_var(addr,&mini_trail);
	}
      }
      undo_ltrail_bindings(&mini_trail,mini_trail_base);
      return TRUE;
    }
  case XSB_LIST:
    {
      register Cell addr;
      void *mini_trail_base;

      mini_trail.ltrail_top = mini_trail_base = ltrail_base(mini_trail);
      while (islist(term)) {
	addr = get_list_head(term);
	XSB_Deref(addr);
	if (isnonvar(addr)) {
	  undo_ltrail_bindings(&mini_trail,mini_trail_base);
	  return FALSE;
	} else {
	  local_bind_var(addr,&mini_trail);
	  term = get_list_tail(term);
	  XSB_Deref(term);
	}
      }
      undo_ltrail_bindings(&mini_trail,mini_trail_base);
      return isnil(term);
    }
  default:
    return FALSE;
  }
}

int count_variable_occurrences(Cell term) {
  int count = 0;
 begin_count_variable_occurrences:
  XSB_Deref(term);
  switch (cell_tag(term)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_ATTV:
    return count + 1;
  case XSB_STRUCT: {
    int i, arity;
    arity = (int) get_arity(get_str_psc(term));
    if (arity == 0) return count;
    for (i = 1; i < arity; i++) {
      count += count_variable_occurrences(get_str_arg(term,i));
    }
    term = get_str_arg(term,arity);
  }
    goto begin_count_variable_occurrences;
  case XSB_LIST:
    count += count_variable_occurrences(get_list_head(term));
    term = get_list_tail(term);
    goto begin_count_variable_occurrences;
  case XSB_FLOAT:
  case XSB_STRING:
  case XSB_INT:
    return count;
  default: xsb_abort("[COUNT_VARIABLE_OCCURRENCES] Argument of unknown type (%p)",term);
  }
  return count; // to quiet compiler
}


/* bind all the variables in term to nil, local-trailing them */
int make_ground(Cell term, struct ltrail *templ_trail) {
 begin_make_ground:
  XSB_Deref(term);
  switch (cell_tag(term)) {
  case XSB_ATTV: {
    term = dec_addr(term);
  case XSB_FREE:
  case XSB_REF1:
    local_bind_var(term,templ_trail);
    return TRUE;
  }
  case XSB_STRUCT: {
    int i, arity;
    arity = (int) get_arity(get_str_psc(term));
    for (i = 1; i < arity; i++) {
      make_ground(get_str_arg(term,i), templ_trail);
    }
    term = get_str_arg(term,arity);
  }
    goto begin_make_ground;
  case XSB_LIST:
    make_ground(get_list_head(term), templ_trail);
    term = get_list_tail(term);
    goto begin_make_ground;
  case XSB_FLOAT:
  case XSB_STRING:
  case XSB_INT: return TRUE;
  default: xsb_abort("[MAKE_GROUND] Argument of unknown type (%p)",term);
  }
  return TRUE;
}

CPtr excess_vars(CTXTdeclc Cell term, CPtr varlist, int ifExist,
		 struct ltrail *templ_trail, struct ltrail *var_trail) {
 begin_excess_vars:
  XSB_Deref(term);
  switch (cell_tag(term)) {
  case XSB_ATTV:
    term = dec_addr(term);
  case XSB_FREE:
  case XSB_REF1:
    cell(varlist) = makelist(hreg);
    cell(hreg) = term;
    local_bind_var(term,var_trail);
    hreg += 2;
    return (CPtr)hreg-1;
  case XSB_STRUCT: {
    int i, arity;
    Psc psc;
    psc = get_str_psc(term);
    arity = (int) get_arity(psc);
    if (arity == 0) return varlist;
    if (psc == caret_psc && ifExist) {
      CPtr *save_templ_base;
      save_templ_base = templ_trail->ltrail_top;
      make_ground(get_str_arg(term,1), templ_trail);
      varlist = excess_vars(CTXTc get_str_arg(term,2), varlist, ifExist, templ_trail, var_trail);
      undo_ltrail_bindings(templ_trail,save_templ_base);
      return varlist;
    }
    if (ifExist && (psc == setof_psc || psc == bagof_psc)) {
      CPtr *save_templ_base;
      save_templ_base = templ_trail->ltrail_top;
      make_ground(get_str_arg(term,1), templ_trail);
      varlist = excess_vars(CTXTc get_str_arg(term,2),varlist,ifExist,templ_trail,var_trail);
      varlist = excess_vars(CTXTc get_str_arg(term,3),varlist,ifExist,templ_trail,var_trail);
      undo_ltrail_bindings(templ_trail,save_templ_base);
      return varlist;
    }
      for (i = 1; i < arity; i++) {
	varlist = excess_vars(CTXTc get_str_arg(term,i), varlist, ifExist, templ_trail, var_trail);
      }
      term = get_str_arg(term,arity);
  }
    goto begin_excess_vars;
  case XSB_LIST:
    varlist = excess_vars(CTXTc get_list_head(term), varlist, ifExist, templ_trail, var_trail);
    term = get_list_tail(term);
    goto begin_excess_vars;
  case XSB_FLOAT:
  case XSB_STRING:
  case XSB_INT:
    return varlist;
  default: xsb_abort("[EXCESS_VARS] Argument 1 of unknown type (%p)",term);
  }
  return NULL;
}


int is_number_atom(Cell term) {
  char hack_char;
  Integer c;

  XSB_Deref(term);
  if (!isstring(term)) return FALSE;
  if (sscanf(string_val(term), "%" Intfmt "%c", &c, &hack_char) == 1) {
    return TRUE;
  } else {
    Float float_temp;
    //TODO: Refactor the below few lines of code once the "Floats are always double?"
    //situation is resolved.
#ifndef FAST_FLOATS
    if (sscanf(string_val(term), "%lf%c", &float_temp, &hack_char) == 1)
#else
      if (sscanf(string_val(term), "%f%c", &float_temp, &hack_char) == 1)
#endif
	return TRUE;
      else return FALSE;	/* fail */
  }
}

/* --------------------------------------------------------------------	*/

#include "term_psc_xsb_i.h"
#include "conget_xsb_i.h"

/* -------------------------------------------------------------------- */

inline static void xsb_fprint_variable(CTXTdeclc FILE *fptr, CPtr var)
{
  if (var >= (CPtr)glstack.low && var <= top_of_heap)
    fprintf(fptr, "_h%" Cellfmt , ((Cell)var-(Cell)glstack.low+1)/sizeof(CPtr));
  else {
    if (var >= top_of_localstk && var <= (CPtr)glstack.high)
      fprintf(fptr, "_l%" Cellfmt , ((Cell)glstack.high-(Cell)var+1)/sizeof(CPtr));
    else fprintf(fptr, "_%p", var);   /* Should never happen */
  }
}

/* --------------------------------------------------------------------	*/

Cell builtin_table[BUILTIN_TBL_SZ][2];

#define BuiltinName(Code)	( (char *)builtin_table[Code][0] )
#define set_builtin_table(Code,String)		\
   builtin_table[Code][0] = (Cell)(String);

void init_builtin_table(void)
{
  int i;

  for (i = 0; i < BUILTIN_TBL_SZ; i++) builtin_table[i][1] = 0;

  set_builtin_table(PSC_NAME, "psc_name");
  set_builtin_table(PSC_ARITY, "psc_arity");
  set_builtin_table(PSC_TYPE, "psc_type");
  set_builtin_table(PSC_PROP, "psc_prop");
  set_builtin_table(PSC_MOD, "psc_mod");
  set_builtin_table(PSC_SET_TYPE, "psc_set_type");
  set_builtin_table(PSC_SET_PROP, "psc_set_prop");
  set_builtin_table(CONGET_TERM, "conget");
  set_builtin_table(CONSET_TERM, "conset");
  set_builtin_table(PSC_SET_SPY, "psc_set_spy");
  set_builtin_table(PSC_EP, "psc_ep");
  set_builtin_table(PSC_SET_EP, "psc_set_ep");

  set_builtin_table(TERM_NEW_MOD, "term_new_mod");
  set_builtin_table(TERM_PSC, "term_psc");
  set_builtin_table(TERM_TYPE, "term_type");
  set_builtin_table(TERM_COMPARE, "term_compare");
  set_builtin_table(TERM_NEW, "term_new");
  set_builtin_table(TERM_ARG, "term_arg");
  set_builtin_table(TERM_SET_ARG, "term_set_arg");
  set_builtin_table(STAT_FLAG, "stat_flag");
  set_builtin_table(STAT_SET_FLAG, "stat_set_flag");
  set_builtin_table(BUFF_ALLOC, "buff_alloc");
  set_builtin_table(BUFF_WORD, "buff_word");
  set_builtin_table(BUFF_SET_WORD, "buff_set_word");
  set_builtin_table(BUFF_BYTE, "buff_byte");
  set_builtin_table(BUFF_SET_BYTE, "buff_set_byte");
  set_builtin_table(CODE_CALL, "code_call");

  set_builtin_table(STR_LEN, "str_len");
  set_builtin_table(SUBSTRING, "substring");
  set_builtin_table(STR_CAT, "str_cat");
  set_builtin_table(STR_CMP, "str_cmp");
  set_builtin_table(STRING_SUBSTITUTE, "string_substitute");

  set_builtin_table(CALL0, "call0");
  set_builtin_table(STAT_STA, "stat_sta");
  set_builtin_table(STAT_CPUTIME, "stat_cputime");
  set_builtin_table(CODE_LOAD, "code_load");
  set_builtin_table(BUFF_SET_VAR, "buff_set_var");
  set_builtin_table(BUFF_DEALLOC, "buff_dealloc");
  set_builtin_table(BUFF_CELL, "buff_cell");
  set_builtin_table(BUFF_SET_CELL, "buff_set_cell");
  set_builtin_table(COPY_TERM,"copy_term");
  set_builtin_table(XWAM_STATE,"xwam_state");

  set_builtin_table(STR_MATCH, "str_match");
  set_builtin_table(DIRNAME_CANONIC, "dirname_canonic");

  set_builtin_table(PSC_INSERT, "psc_insert");
  set_builtin_table(PSC_IMPORT, "psc_import");
  set_builtin_table(PSC_IMPORT_AS, "psc_import_as");
  set_builtin_table(PSC_DATA, "psc_data");
  set_builtin_table(PSC_INSERTMOD, "psc_insertmod");
  set_builtin_table(CALLN, "calln");

  set_builtin_table(FILE_GETTOKEN, "file_gettoken");
  set_builtin_table(FILE_PUTTOKEN, "file_puttoken");
  set_builtin_table(TERM_HASH, "term_hash");
  set_builtin_table(UNLOAD_SEG, "unload_seg");
  set_builtin_table(LOAD_OBJ, "load_obj");

  set_builtin_table(GETENV, "getenv");
  set_builtin_table(SYS_SYSCALL, "sys_syscall");
  set_builtin_table(SYS_SYSTEM, "sys_system");
  set_builtin_table(SYS_GETHOST, "sys_gethost");
  set_builtin_table(SYS_ERRNO, "sys_errno");
  set_builtin_table(PUTENV, "putenv");
  set_builtin_table(FILE_WRITEQUOTED, "file_writequoted");
  set_builtin_table(GROUND, "ground");

  set_builtin_table(INTERN_STRING, "intern_string");
  set_builtin_table(EXPAND_FILENAME, "expand_filename");
  set_builtin_table(TILDE_EXPAND_FILENAME, "tilde_expand_filename");
  set_builtin_table(IS_ABSOLUTE_FILENAME, "is_absolute_filename");
  set_builtin_table(PARSE_FILENAME, "parse_filename");
  set_builtin_table(ALMOST_SEARCH_MODULE, "almost_search_module");
  set_builtin_table(EXISTING_FILE_EXTENSION, "existing_file_extension");

  set_builtin_table(DO_ONCE, "do_once");
  set_builtin_table(INTERN_TERM, "intern_term");

  set_builtin_table(INCR_EVAL_BUILTIN, "incr_eval"); /* incremental evaluation */

  set_builtin_table(GET_DATE, "get_date");
  set_builtin_table(STAT_WALLTIME, "stat_walltime");

  set_builtin_table(PSC_INIT_INFO, "psc_init_info");
  set_builtin_table(PSC_GET_SET_ENV_BYTE, "psc_get_set_env_byte");
  set_builtin_table(PSC_ENV, "psc_env");
  set_builtin_table(PSC_SPY, "psc_spy");
  set_builtin_table(PSC_TABLED, "psc_tabled");
  set_builtin_table(PSC_SET_TABLED, "psc_set_tabled");

  set_builtin_table(IS_INCOMPLETE, "is_incomplete");

  set_builtin_table(GET_PTCP, "get_ptcp");
  set_builtin_table(GET_PRODUCER_CALL, "get_producer_call");
  set_builtin_table(DEREFERENCE_THE_BUCKET, "dereference_the_bucket");
  set_builtin_table(PAIR_PSC, "pair_psc");
  set_builtin_table(PAIR_NEXT, "pair_next");
  set_builtin_table(NEXT_BUCKET, "next_bucket");

  set_builtin_table(SLG_NOT, "slg_not");
  set_builtin_table(IS_XWAMMODE, "is_xwammode");
  set_builtin_table(CLOSE_OPEN_TABLES, "close_open_tables");

  set_builtin_table(FILE_FUNCTION, "file_function");
  set_builtin_table(SLASH_BUILTIN, "slash");

  set_builtin_table(ABOLISH_ALL_TABLES, "abolish_all_tables");
  set_builtin_table(ABOLISH_MODULE_TABLES, "abolish_module_tables");
  set_builtin_table(ZERO_OUT_PROFILE, "zero_out_profile");
  set_builtin_table(WRITE_OUT_PROFILE, "write_out_profile");
  set_builtin_table(ASSERT_CODE_TO_BUFF, "assert_code_to_buff");
  set_builtin_table(ASSERT_BUFF_TO_CLREF, "assert_buff_to_clref");

  set_builtin_table(FILE_READ_CANONICAL, "file_read_canonical");
  set_builtin_table(GEN_RETRACT_ALL, "gen_retract_all");

  set_builtin_table(DB_GET_LAST_CLAUSE, "db_get_last_clause");
  set_builtin_table(DB_RETRACT0, "db_retract0");
  set_builtin_table(DB_GET_CLAUSE, "db_get_clause");
  set_builtin_table(DB_BUILD_PRREF, "db_build_prref");
  set_builtin_table(DB_GET_PRREF, "db_get_prref");
  set_builtin_table(DB_ABOLISH0, "db_abolish0");
  set_builtin_table(DB_RECLAIM0, "db_reclaim0");

  set_builtin_table(FORMATTED_IO, "formatted_io");
  set_builtin_table(TABLE_STATUS, "table_status");
  set_builtin_table(GET_DELAY_LISTS, "get_delay_lists");
  set_builtin_table(ANSWER_COMPLETION_OPS, "answer_completion_ops");

  set_builtin_table(ABOLISH_TABLE_PREDICATE, "abolish_table_pred");
  set_builtin_table(ABOLISH_TABLE_CALL, "abolish_table_call");
  set_builtin_table(TRIE_ASSERT, "trie_assert");
  set_builtin_table(TRIE_RETRACT, "trie_retract");
  set_builtin_table(TRIE_RETRACT_SAFE, "trie_retract_safe");
  set_builtin_table(TRIE_DELETE_RETURN, "trie_delete_return");
  set_builtin_table(TRIE_GET_RETURN, "trie_get_return");
  set_builtin_table(TRIE_ASSERT_HDR_INFO, "trie_assert_hdr_info");


  /* Note: TRIE_GET_CALL previously used for get_calls/1, before get_call/3
     was made a builtin itself. */
  set_builtin_table(TRIE_UNIFY_CALL, "get_calls");
  set_builtin_table(GET_LASTNODE_CS_RETSKEL, "get_lastnode_cs_retskel");
  set_builtin_table(TRIE_GET_CALL, "get_call");
  set_builtin_table(BREG_RETSKEL,"breg_retskel");

  set_builtin_table(TRIMCORE, "trimcore");
  set_builtin_table(NEWTRIE, "newtrie");
  set_builtin_table(TRIE_INTERN, "trie_intern");
  set_builtin_table(TRIE_INTERNED, "trie_interned");
  set_builtin_table(TRIE_UNINTERN, "trie_unintern");
  set_builtin_table(BOTTOM_UP_UNIFY, "bottom_up_unify");
  set_builtin_table(TRIE_TRUNCATE, "trie_truncate");
  set_builtin_table(TRIE_DISPOSE_NR, "trie_dispose_nr");
  set_builtin_table(TRIE_UNDISPOSE, "trie_undispose");
  set_builtin_table(RECLAIM_UNINTERNED_NR, "reclaim_uninterned_nr");
  set_builtin_table(GLOBALVAR, "globalvar");
  set_builtin_table(CCALL_STORE_ERROR, "ccall_store_error");

  set_builtin_table(SET_TABLED_EVAL, "set_tabled_eval_method");
  set_builtin_table(UNIFY_WITH_OCCURS_CHECK, "unify_with_occurs_check");

  set_builtin_table(PUT_ATTRIBUTES, "put_attributes");
  set_builtin_table(GET_ATTRIBUTES, "get_attributes");
  set_builtin_table(DELETE_ATTRIBUTES, "delete_attributes");
  set_builtin_table(ATTV_UNIFY, "attv_unify");
  set_builtin_table(PRIVATE_BUILTIN, "private_builtin");
  set_builtin_table(SEGFAULT_HANDLER, "segfault_handler");
  set_builtin_table(GET_BREG, "get_breg");

  set_builtin_table(FLOAT_OP, "float_op");
  set_builtin_table(IS_ATTV, "is_attv");
  set_builtin_table(VAR, "var");
  set_builtin_table(NONVAR, "nonvar");
  set_builtin_table(ATOM, "atom");
  set_builtin_table(INTEGER, "integer");
  set_builtin_table(REAL, "real");
  set_builtin_table(NUMBER, "number");
  set_builtin_table(ATOMIC, "atomic");
  set_builtin_table(COMPOUND, "compound");
  set_builtin_table(CALLABLE, "callable");
  set_builtin_table(IS_LIST, "is_list");

  set_builtin_table(FUNCTOR, "functor");
  set_builtin_table(ARG, "arg");
  set_builtin_table(UNIV, "univ");
  set_builtin_table(IS_MOST_GENERAL_TERM, "is_most_general_term");
  set_builtin_table(HiLog_ARG, "hilog_arg");
  set_builtin_table(HiLog_UNIV, "hilog_univ");
  set_builtin_table(EXCESS_VARS, "excess_vars");
  set_builtin_table(ATOM_CODES, "atom_codes");
  set_builtin_table(ATOM_CHARS, "atom_chars");
  set_builtin_table(NUMBER_CHARS, "number_chars");
  set_builtin_table(NUMBER_CODES, "number_codes");
  set_builtin_table(IS_CHARLIST, "is_charlist");
  set_builtin_table(NUMBER_DIGITS, "number_digits");
  set_builtin_table(IS_NUMBER_ATOM, "is_number_atom");
  set_builtin_table(TERM_DEPTH, "term_depth");
  set_builtin_table(IS_CYCLIC, "is_cyclic");
  set_builtin_table(GROUND_CYC, "ground_cyc");
  set_builtin_table(TERM_SIZE, "term_sizew");
  set_builtin_table(TERM_SIZE_LIMIT, "term_size_limit");

  set_builtin_table(PUT, "put");
  set_builtin_table(TAB, "tab");
  set_builtin_table(SORT, "sort");
  set_builtin_table(KEYSORT, "keysort");
  set_builtin_table(PARSORT, "parsort");

  set_builtin_table(URL_ENCODE_DECODE, "url_encode_decode");
  set_builtin_table(CHECK_CYCLIC, "check_cyclic");

  set_builtin_table(ORACLE_QUERY, "oracle_query");
  set_builtin_table(ODBC_EXEC_QUERY, "odbc_exec_query");
  set_builtin_table(SET_SCOPE_MARKER, "set_scope_marker");
  set_builtin_table(UNWIND_STACK, "unwind_stack");
  set_builtin_table(CLEAN_UP_BLOCK, "clean_up_block");

  set_builtin_table(THREAD_REQUEST, "thread_request");
  set_builtin_table(MT_RANDOM_REQUEST, "mt_random_request");

  set_builtin_table(COPY_TERM_3,"copy_term_3");
  set_builtin_table(MARK_HEAP, "mark_heap");
  set_builtin_table(GC_STUFF, "gc_stuff");
  set_builtin_table(FINDALL_INIT, "$$findall_init");
  set_builtin_table(FINDALL_ADD, "$$findall_add");
  set_builtin_table(FINDALL_GET_SOLS, "$$findall_get_solutions");

#ifdef HAVE_SOCKET
  set_builtin_table(SOCKET_REQUEST, "socket_request");
#endif

  set_builtin_table(JAVA_INTERRUPT, "setupJavaInterrupt");
  set_builtin_table(FORCE_TRUTH_VALUE, "force_truth_value");
  set_builtin_table(INTERPROLOG_CALLBACK, "interprolog_callback");
}

/* --------------------------------------------------------------------	*/

#if defined(PROFILE) && !defined(MULTI_THREAD)
static void write_out_profile(void)
{
  UInteger i, isum, ssum, tot;
  double rat1, rat2;

  isum = ssum = tot = 0;
  for (i = 0; i < BUILTIN_TBL_SZ; i++) {
    if (inst_table[i][0] != 0) isum = isum + inst_table[i][5];
  }
  for (i = 0; i < BUILTIN_TBL_SZ; i++) {
    if (subinst_table[i][0] != 0) ssum = ssum + subinst_table[i][1];
  }
    tot = isum + ssum;
  if (tot!=0) {
    fprintf(stdout,
	    "max subgoals %u max completed %u max consumers in ascc %u max compl_susps in ascc %u\n",
	          max_subgoals,max_completed,max_consumers_in_ascc,
	          max_compl_susps_in_ascc);
    rat1 = isum / tot;
    rat2 = ssum / tot;
    fprintf(stdout,
	    "trapped Prolog choice point memory (%d bytes).\n",trapped_prolog_cps);
    fprintf(stdout,
	    "summary(total(%d),inst(%d),pct(%f),subinst(%d),pct(%f)).\n",
	    tot,isum,rat1,ssum,rat2);
    for (i = 0; i < BUILTIN_TBL_SZ; i++) {
      if (inst_table[i][5] != 0)
	fprintf(stdout,"instruction(%s,%x,%d,%.3f).\n",
		(char *) inst_table[i][0],i,
	        inst_table[i][5],(((float)inst_table[i][5])/(float)tot));
    }
/*      fprintf(stdout,"_______________subinsts_______________\n"); */
    for (i = 0; i < BUILTIN_TBL_SZ; i++) {
      if (subinst_table[i][0] != 0) {
	ssum = subinst_table[i][1];
	rat1 = ssum/tot;
	fprintf(stdout,"subinst(%s,%x,%d,%g).\n",
		(char *) subinst_table[i][0],i,
		subinst_table[i][1],rat1);
      }
    }
/*      fprintf(stdout,"_______________builtins_______________\n"); */
    for (i = 0; i < BUILTIN_TBL_SZ; i++)
      if (builtin_table[i][1] > 0 && builtin_table[i][0] != 0)
	fprintf(stdout,"builtin(%s,%d,%d).\n",
		BuiltinName(i), i, builtin_table[i][1]);
    fprintf(stdout,"switch_envs(%d).\n",
	    num_switch_envs);
    fprintf(stdout,"switch_envs_iter(%d).\n",
	    num_switch_envs_iter);
  }
  else
    fprintf(stdout,"Instruction profiling not turned On\n");
}
#endif

/*----------------------------------------------------------------------*/

/* inlined definition of file_function */
#include "io_builtins_xsb_i.h"

/* inlined functions for prolog standard builtins */
#include "std_pred_xsb_i.h"

/* --- built in predicates --------------------------------------------	*/

int builtin_call(CTXTdeclc byte number)
{
  switch (number) {
  case PSC_NAME: {	/* R1: +PSC; R2: -String */
    Psc psc = (Psc)ptoc_addr(1);
    //    ctop_string(CTXTc 2, get_name(psc));  TLS -- avoid interning already interned string
    extern_ctop_string(CTXTc 2, get_name(psc));
    break;
  }
  case PSC_ARITY: {	/* R1: +PSC; R2: -int */
    Psc psc = (Psc)ptoc_addr(1);
    ctop_int(CTXTc 2, (Integer)get_arity(psc));
    break;
  }
  case PSC_TYPE: {	/* R1: +PSC; R2: -int */
			/* type: see psc_xsb.h, `entry_type' field defs */
    Psc psc = (Psc)ptoc_addr(1);
    ctop_int(CTXTc 2, (Integer)get_type(psc));
    break;
  }
  case PSC_SET_TYPE: {	/* R1: +PSC; R2: +type (int): see psc_xsb.h */
    Psc psc = (Psc)ptoc_addr(1);
    set_type(psc, (byte)ptoc_int(CTXTc 2));
    break;
  }
  case PSC_PROP: {	/* R1: +PSC; R2: -term */
			/* prop: as a buffer pointer */
    Psc psc = (Psc)ptoc_addr(1);
    if ((get_type(psc) == T_PRED || get_type(psc) == T_DYNA) && get_env(psc) != T_IMPORTED) {
      char str[100];
      snprintf(str,100,"[psc_prop/2] Cannot get property of predicate: %s/%d\n",
	      get_name(psc),get_arity(psc));
      xsb_warn(CTXTc str);
      return FALSE;
    }
    ctop_int(CTXTc 2, (Integer)get_data(psc));
    break;
  }
  case PSC_MOD: {	/* R1: +PSC; R2: -term */
			/* prop: as a buffer pointer */
    Psc psc = (Psc)ptoc_addr(1);
    if ((get_type(psc) == T_PRED || get_type(psc) == T_DYNA) && get_env(psc) != T_IMPORTED) {
      char str[100];
      snprintf(str,100,"[psc_mod/2] Cannot get property of predicate: %s/%d\n",
	      get_name(psc),get_arity(psc));
      xsb_warn(CTXTc str);
      return FALSE;
    }
    ctop_int(CTXTc 2, (Integer)(get_mod_for_psc(psc)));
    break;
  }
  case PSC_SET_PROP: {	       /* R1: +PSC; R2: +int */
    Psc psc = (Psc)ptoc_addr(1);
    if (get_type(psc) == T_PRED || get_type(psc) == T_DYNA) {
      xsb_warn(CTXTc "[psc_set_prop/2] Cannot set property of predicate.\n");
      return FALSE;
    }
    set_data(psc, (Psc)ptoc_int(CTXTc 2));
    break;
  }

  case CONGET_TERM: {
    Integer res = conget((Cell)ptoc_tag(CTXTc 1));
    prolog_term arg2 = reg_term(CTXTc 2);
    if (isref(arg2)) {
      c2p_int(CTXTc res,arg2);
      return TRUE;
    } else {
      return (int_val(arg2) == res);
    }
  }
  case CONSET_TERM: {
    return conset((Cell)ptoc_tag(CTXTc 1), (Integer)ptoc_int(CTXTc 2));
  }
  case PSC_EP: {	/* R1: +PSC; R2: -term */
			/* prop: as a buffer pointer */
    Psc psc = (Psc)ptoc_addr(1);
    /* ep contains filename ptr if its a module psc */
    if (isstring(get_ep(psc))) ctop_string(CTXTc 2, string_val(get_ep(psc)));
    else ctop_int(CTXTc 2, (Integer)get_ep(psc));
    break;
  }
  case PSC_SET_EP: {	       /* R1: +PSC; R2: +int */
    Psc psc = (Psc)ptoc_addr(1);
    byte *ep = (pb)ptoc_int(CTXTc 2);
    if (ep == (byte *)NULL) set_ep(psc,(byte *)(&(psc->load_inst)));
    else if (ep == (byte *)4) set_ep(psc,(byte *)&fail_inst);
    break;
  }

  case PSC_SET_SPY: { 	       /* R1: +PSC; R2: +int */
    Psc psc = (Psc)ptoc_addr(1);
    set_spy(psc, (byte)ptoc_int(CTXTc 2));
    break;
  }

  case FILE_FUNCTION:  /* file_open/close/put/get/truncate/seek/pos */
  { int tmp ;
    //    SYS_MUTEX_LOCK( MUTEX_IO );
    tmp = file_function(CTXT);
    //    SYS_MUTEX_UNLOCK( MUTEX_IO );
    return tmp;
  }

  case TERM_PSC: {		/* R1: +term; R2: -PSC */
    /* Assumes that `term' is a XSB_STRUCT-tagged Cell. */
    /*    ctop_addr(2, get_str_psc(ptoc_tag(CTXTc 1))); */
    Psc temp =  term_psc((Cell)(ptoc_tag(CTXTc 1)));
    ctop_addr(2, temp);
    //    ctop_addr(2, term_psc((Cell)(ptoc_tag(CTXTc 1))));
    break;
  }
  case CONPSC:
    {int new;
      Cell term = ptoc_tag(CTXTc 1);
      if (isstring(term)) {
	ctop_addr(2, pair_psc(insert(string_val(term), 0, (Psc)flags[CURRENT_MODULE], &new)));
      }
      else if (isconstr(term))
	ctop_addr(2, term_psc(term));
      else return FALSE;
      return TRUE;
    }
  case TERM_TYPE: {	/* R1: +term; R2: tag (-int)			  */
			/* <0 - var, 1 - cs, 2 - int, 3 - list, 7 - ATTV> */
    Cell term = ptoc_tag(CTXTc 1);
    if (isref(term)) {
        ctop_int(CTXTc 2, XSB_FREE);
    }
    else {
        if (isboxedinteger(term)) {
            ctop_int(CTXTc 2, XSB_INT);
            break;
        }
        if (isboxedfloat(term)) {
            ctop_int(CTXTc 2, XSB_FLOAT);
            break;
        }
        ctop_int(CTXTc 2, cell_tag(term));
    }
    break;
  }
  case TERM_COMPARE:	/* R1, R2: +term; R3: res (-int) */
    ctop_int(CTXTc 3, compare(CTXTc (void *)ptoc_tag(CTXTc 1), (void *)ptoc_tag(CTXTc 2)));
    break;
  case TERM_NEW_MOD: {  /* R1: +ModName, R2: +Term, R3: -NewTerm */
    int new, disp;
    Psc termpsc, modpsc, newtermpsc;
    Cell arg, term = ptoc_tag(CTXTc 2);
    /* XSB_Deref(term); not nec since ptoc_tag derefs */
    if (isref(term)) {
      xsb_instantiation_error(CTXTc "term_new_mod/3",2);
      break;
    }
    termpsc = term_psc(term);
    modpsc = pair_psc(insert_module(0,ptoc_string(CTXTc 1)));
    /*    if (!colon_psc) colon_psc = pair_psc(insert(":",2,global_mod,&new));*/
    while (termpsc == colon_psc) {
      term = get_str_arg(term,2);
      XSB_Deref(term);
      termpsc = term_psc(term);
    }
    newtermpsc = pair_psc(insert(get_name(termpsc),get_arity(termpsc),modpsc,&new));
    if (new) {
      set_data(newtermpsc, modpsc);
      env_type_set(CTXTc newtermpsc, T_IMPORTED, T_ORDI, (xsbBool)new);
    }
    ctop_constr(CTXTc 3, (Pair)hreg);
    new_heap_functor(hreg, newtermpsc);
    for (disp=1; disp <= get_arity(newtermpsc); disp++) {
      arg = get_str_arg(term,disp);
      nbldval_safe(arg); /* but really isnt.  should change to nbldval and resolve issues. */
    }
  }
  break;
  case TERM_NEW: {		/* R1: +PSC, R2: -term */
    int disp;
    Psc psc = (Psc)ptoc_addr(1);
    sreg = hreg;
    hreg += get_arity(psc) + 1;
    ctop_constr(CTXTc 2, (Pair)sreg);
    new_heap_functor(sreg, psc);
    for (disp=0; disp < (int)get_arity(psc); sreg++,disp++) {
      bld_free(sreg);
    }
    break;
  }
  case TERM_ARG: {	/* R1: +term; R2: index (+int); R3: arg (-term) */
    size_t  disp = ptoc_int(CTXTc 2);
    Cell term = ptoc_tag(CTXTc 1);
    ctop_tag(CTXTc 3, get_str_arg(term,disp));
    break;
  }

    /* TLS: it turns out that term_set_arg, and the perm. flag are
       still used in array.P.  Added error conditions for form in
       constraintLib */
  case TERM_SET_ARG: {	/* R1: +term; R2: index (+int) */
			/* R3: newarg (+term) */
    /* used in file_read.P, array.P, array1.P */
    //    int  disp = ptoc_int(CTXTc 2);
  //    Cell term = ptoc_tag(CTXTc 1);
    int  disp = (int) iso_ptoc_int(CTXTc 2,"term_set_arg/3");
    Cell term = iso_ptoc_callable(CTXTc 1,"term_set_arg/3");
    CPtr arg_loc = clref_val(term)+disp;
    Cell new_val = cell(reg+3);
    int perm_flag = (int)ptoc_int(CTXTc 4);
    if (disp < 1) xsb_domain_error(CTXTc "positive_integer",ptoc_tag(CTXTc 2),"set_arg/3",2);
    if (perm_flag == 0) {
      pushtrail(arg_loc,new_val);
    } else if (perm_flag < 0) {
      push_pre_image_trail(arg_loc,new_val);
    }
    bld_copy(arg_loc, new_val);
    break;
  }
  case STAT_FLAG: {	/* R1: flagname(+int); R2: value(-int) */
    int flagname = (int)ptoc_int(CTXTc 1);
    prolog_int flagval;
    if (flagname < MAX_PRIVATE_FLAGS ) flagval = pflags[flagname];
    else flagval = flags[flagname];
    ctop_int(CTXTc 2, flagval);
    break;
  }
  case STAT_SET_FLAG: {	/* R1: flagname(+int); R2: value(+int); */
    prolog_int flagval = ptoc_int(CTXTc 2);
    int flagname = (int)ptoc_int(CTXTc 1);
    if (flagname < MAX_PRIVATE_FLAGS )
    	pflags[flagname] = flagval;
    else flags[flagname] = flagval;
    if (flags[DEBUG_ON]||flags[HITRACE]||pflags[CLAUSE_INT])
      asynint_val |= MSGINT_MARK;
    else asynint_val &= ~MSGINT_MARK;
    break;
  }
  case BUFF_ALLOC: {	/* R1: size (+integer); R2: -buffer; */
	           /* the length of the buffer is also stored at position 0 */
    char *addr;
    size_t  value = ((ptoc_int(CTXTc 1)+7)>>3)<<3;
    value *= ZOOM_FACTOR ;
    addr = (char *)mem_alloc(value,BUFF_SPACE);
    value /= ZOOM_FACTOR ;
    *(Integer *)addr = value;	/* store buffer size at buf[0] */
    ctop_int(CTXTc 2, (Integer)addr);	/* use "integer" type now! */
    break;
  }
  case BUFF_DEALLOC: {	/* R1: +buffer; R2: +oldsize; R3: +newsize; */
    size_t  value;
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t  disp = ((ptoc_int(CTXTc 2)+7)>>3)<<3;
    disp *= ZOOM_FACTOR ;
    value = ((ptoc_int(CTXTc 3)+7)>>3)<<3;	/* alignment */
    value *= ZOOM_FACTOR ;
    if (value > disp) {
      xsb_warn(CTXTc "[BUFF_DEALLOC] New Buffer Size (%d) Cannot exceed the old one (%d)!!",
	       value, disp);
      break;
    }
    mem_dealloc((byte *)(addr+value), disp-value,BUFF_SPACE);
    break;
  }
  case BUFF_WORD: {     /* R1: +buffer; r2: displacement(+integer); */
			/* R3: value (-integer) */
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t  disp = ptoc_int(CTXTc 2);
    disp *= ZOOM_FACTOR ;
    ctop_int(CTXTc 3, *(Integer *)(addr+disp));
    break;
  }
  case BUFF_SET_WORD: {	/* R1: +buffer; r2: displacement(+integer); */
			/* R3: value (+integer) */
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t  disp = ptoc_int(CTXTc 2);
    disp *= ZOOM_FACTOR ;
    *(CPtr)(addr+disp) = ptoc_int(CTXTc 3);
    break;
  }
  case BUFF_BYTE: {	/* R1: +buffer; r2: displacement(+integer); */
			/* R3: value (-integer) */
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t disp = ptoc_int(CTXTc 2);
    ctop_int(CTXTc 3, (Integer)(*(byte *)(addr+disp)));
    break;
  }
  case BUFF_SET_BYTE: {	/* R1: +buffer; R2: displacement(+integer); */
			/* R3: value (+integer) */
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t  disp = ptoc_int(CTXTc 2);
    *(pb)(addr+disp) = (byte)ptoc_int(CTXTc 3);
    break;
  }
  case BUFF_CELL: {	/* R1: +buffer; R2: displacement(+integer); */
                        /* R3: -Cell at that location */
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t disp = ptoc_int(CTXTc 2);
    disp *= ZOOM_FACTOR ;
    ctop_tag(CTXTc 3, (Cell)(addr+disp));
    break;
  }
  case BUFF_SET_CELL: {	/* R1: +buffer; R2: displacement(+integer);*/
			/* R3: type (+integer); R4: +term */
    /* When disp<0, set the type of the buff itself */
    /* The last function is not implemented */
    size_t value;
    char *addr = (char *) ptoc_int(CTXTc 1);
    size_t disp = ptoc_int(CTXTc 2);
    disp *= ZOOM_FACTOR ;
    value = ptoc_int(CTXTc 3);
    switch (value) {
    case XSB_REF:
    case XSB_REF1:
      bld_ref(vptr(addr+disp), (CPtr)ptoc_int(CTXTc 4)); break;
    case XSB_INT: {
      size_t tmpval = ptoc_int(CTXTc 4);
      bld_int(vptr(addr+disp), tmpval); break;
    }
    case XSB_FLOAT:
      bld_float(vptr(addr+disp),(float)ptoc_float(CTXTc 4)); break;
    case XSB_STRUCT:
      bld_cs(vptr(addr+disp), (Pair)ptoc_int(CTXTc 4)); break;
    case XSB_STRING:
      bld_string(vptr(addr+disp), (char *)ptoc_int(CTXTc 4)); break;
    case XSB_LIST:
      bld_list(vptr(addr+disp), (CPtr)ptoc_int(CTXTc 4)); break;
    default:
      xsb_warn(CTXTc "[BUFF_SET_CELL] Type %d is not implemented", value);
    }
    break;
  }
  case BUFF_SET_VAR: {
    size_t disp;
    Cell term;
    char *addr;
    /* This procedure is used to make an external variable pointing to the
       buffer. The linkage inside the buffer will not be trailed so remains
       after backtracking. */
    /* R1: +buffer; R2: +disp; */
    /* R3: +buffer length; R4: External var */
    addr = (char *) ptoc_int(CTXTc 1);
    disp = ptoc_int(CTXTc 2);
    disp *= ZOOM_FACTOR;
    term = ptoc_tag(CTXTc 4);
    bld_free(vptr(addr+disp));
    if ((Cell)term < (Cell)addr ||
	(Cell)term > (Cell)addr+ptoc_int(CTXTc 3)) { /* var not in buffer, trail */
      bind_ref(vptr(term), (CPtr)(addr+disp));
    } else {		/* already in buffer */
      bld_ref(vptr(term), (CPtr)(addr+disp));
    }
    break;
  }
  case COPY_TERM: /* R1: +term to copy; R2: -variant; R3: -list_of_attvars */
    return copy_term(CTXT);

  case CALL0: {			/* R1: +Term, the call to be made */
    /* Note: this procedure does not save cpreg, hence is more like */
    /* an "execute" instruction, and must be used as the last goal!!!*/
    Cell term = ptoc_tag(CTXTc 1);
    /* in call_xsb_i.h */
    return prolog_call0(CTXTc term);
  }

  case CALLN: { /* R1 is K: Number of arguments to add,
		   R2-R(2+K-1) are the added arguments,
		   R(K+2) is the Goal. */
    int i,new;
    Psc newpsc;
    int k = (int)ptoc_int(CTXTc 1);
    Cell goal = ptoc_tag(CTXTc (k+2));
    if (k == 0) {
      return prolog_call0(CTXTc goal);
    } else if (isstring(goal)) {
      for (i = 1; i <= k; i++) {
	bld_copy(reg+i,cell(reg+i+1));
      }
      newpsc = pair_psc(insert(string_val(goal),(byte)k,(Psc)flags[CURRENT_MODULE],&new));
      pcreg = get_ep(newpsc);
      if (asynint_val) intercept(CTXTc newpsc);
      return TRUE;
    } else if (isconstr(goal)) {
      Psc modpsc, psc = get_str_psc(goal);
      int arity;
      char *goalname;
      CPtr addr;
      if (psc == colon_psc) {
	//printf("found colon_psc\n");
	Cell modstring = get_str_arg(goal,1);
	XSB_Deref(modstring);
	if (!isstring(modstring)) {
	  xsb_type_error(CTXTc "module",goal,"call/n",1);
	  return FALSE;
	}
	modpsc = pair_psc(insert_module(0,string_val(modstring)));
	//printf("modpsc1 %s\n",get_name(modpsc));
	goal = get_str_arg(goal,2);
	XSB_Deref(goal);
	if (!isstring(goal)) {
	  psc = get_str_psc(goal);
	  //printf("here00 %s/%d\n",get_name(psc),get_arity(psc));
	} else {
	  //printf("isstring\n");
	  psc = pair_psc(insert(string_val(goal),(byte)k,modpsc,&new));
	}
      } else {
	//printf("string\n");
	modpsc = get_mod_for_psc(psc);
      }
      //printf("here0 %s/%d\n",get_name(psc),get_arity(psc));
      if (isstring(goal)) {
	for (i = 1; i <= k; i++) {
	  bld_copy(reg+i,cell(reg+i+1));
	}
	//printf("inserting\n");  
	newpsc = pair_psc(insert(string_val(goal),(byte)k,modpsc,&new));
	pcreg = get_ep(newpsc);
	if (asynint_val) intercept(CTXTc newpsc);
	return TRUE;
      }
      arity = get_arity(psc);
      if (arity == 0) {
	for (i = 1; i <= k; i++) {
	  bld_copy(reg+i,cell(reg+i+1));
	}
      } else if (arity >= 1) {
	for (i = k+1; i > 1; i--) {
	  bld_copy(reg+i+arity-1,cell(reg+i));
	}
      }
      addr = (clref_val(goal));
      for (i = 1; i <= arity; i++) {
	bld_copy(reg+i,cell(addr+i));
      }
      goalname = get_name(psc);
      if (!modpsc) modpsc = (Psc)flags[CURRENT_MODULE];
      if (!modpsc) modpsc = global_mod;
      //printf("inserting %s/%d into modpsc %s\n",goalname,get_arity(psc),get_name(modpsc));
      newpsc = pair_psc(insert(goalname,(byte)(arity+k),modpsc,&new));
      if (new) {
	set_data(newpsc, modpsc);
	set_env(newpsc,T_UNLOADED);
	set_type(newpsc, T_ORDI);
      }
      pcreg = get_ep(newpsc);
      if (asynint_val) intercept(CTXTc newpsc);
      return TRUE;
    } else {
      if (isnonvar(goal))
	xsb_type_error(CTXTc "callable",goal,"call/n",1);
      else xsb_instantiation_error(CTXTc "call/n",1);
      return FALSE;
    }
  }

  case CODE_CALL: {		/* R1: +Code (addr), the code address */
				/* R2: +Term, the call to be made */
				/* R3: +Type, code type (same as psc->type)  */
				/* may need to resume interrupt testing here */
    /* Note: this procedure does not save cpreg, hence is more like */
    /* an "execute" instruction, and must be used as the last goal!!!*/
    Cell term = ptoc_tag(CTXTc 2);
    int  value = (int)ptoc_int(CTXTc 3);  /* Cannot be delayed! R3 may be reused */
    pcreg = (byte *)ptoc_int(CTXTc 1);

    /* in call_xsb_i.h */
    return prolog_code_call(CTXTc term,value);
  }
  case SUBSTRING: /* R1: +String; R2,R3: +begin/end offset; R4: -OutSubstr */
    return substring(CTXT);
  case STRING_SUBSTITUTE: /* R1: +Str, R2: [s(a1,b1),s(a2,b2),...],
			     R3: [str1,str2,...], R4: -OutStr */
    return string_substitute(CTXT);
  case STR_LEN:	{	/* R1: +String; R2: -Length */
    Cell term = ptoc_tag(CTXTc 1);
    Cell num = ptoc_tag(CTXTc 2);
    if (isstring(term)) {
      char *addr = string_val(term);
      if (isref(num) || (isointeger(num) && oint_val(num) >= 0))
	return int_unify(CTXTc makeint(strlen(addr)), num);
      else if (!isointeger(num)) xsb_type_error(CTXTc "integer",num,"atom_length/2",2);
      else xsb_domain_error(CTXTc "not_less_than_zero",num,"atom_length/2",2);
    } else if (isref(term)) {
      xsb_instantiation_error(CTXTc "atom_length/2",1);
    } else {
      xsb_type_error(CTXTc "atom",term,"atom_length/2",1);
    }
    return FALSE;
  }
  case STR_CAT:		/* R1: +Str1; R2: +Str2: R3: -Str3 */
    return str_cat(CTXT);
  case STR_CMP:		/* R1: +Str1; R2: +Str2: R3: -Res */
    ctop_int(CTXTc 3, strcmp(ptoc_string(CTXTc 1), ptoc_string(CTXTc 2)));
    break;
  case STR_MATCH:
    return str_match(CTXT);
  case INTERN_STRING: /* R1: +String1; R2: -String2 ; Intern string */
    //    ctop_string(CTXTc 2, string_find(ptoc_string(CTXTc 1), 1));
    ctop_string(CTXTc 2, ptoc_string(CTXTc 1));  // TLS: let's just intern once
    break;
  case STAT_STA: {		/* R1: +Amount */
    int value = (int)ptoc_int(CTXTc 1);
    print_statistics(CTXTc value);
    break;
  }
  case STAT_CPUTIME: {	/* R1: -cputime, in miliseconds */
    int value = (int)(cpu_time() * 1000);
    ctop_int(CTXTc 1, value);
    break;
  }
  case GET_DATE: {
    int year=0, month=0, day=0, hour=0, minute=0, second=0;
    get_date((int)ptoc_int(CTXTc 1),&year,&month,&day,&hour,&minute,&second);
    ctop_int(CTXTc 2,year);
    ctop_int(CTXTc 3,month);
    ctop_int(CTXTc 4,day);
    ctop_int(CTXTc 5,hour);
    ctop_int(CTXTc 6,minute);
    ctop_int(CTXTc 7,second);
    break;
  }
  case STAT_WALLTIME: {
    int value;
    value = (int) ((real_time() - realtime_count_gl) * 1000);
    ctop_int(CTXTc 1, value);
    break;
  }
  case XWAM_STATE: { /* return info about xwam state: R1: +InfoCode, R2: -ReturnedValue */
    switch (ptoc_int(CTXTc 1)) { /* extend as needed */
    case 0: /* current trail size */
      ctop_int(CTXTc 2, (pb)trreg-(pb)tcpstack.low);
      break;
    case 1: /* current CP Stack size */
      ctop_int(CTXTc 2, (pb)tcpstack.high - (pb)breg);
      break;
    case 2: /* value of delayreg */
      ctop_int(CTXTc 2, (UInteger)delayreg);
      break;
    case 3: /* current interned trie space used, SLOW */
      {NodeStats abtn;
	HashStats abtht;
	size_t trieassert_used;
	abtn = node_statistics(&smAssertBTN);
	abtht = hash_statistics(CTXTc &smAssertBTHT);
	trieassert_used =
	  NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);
	ctop_int(CTXTc 2, (Integer)trieassert_used);
	}
      break;
    case 4: /* current Global Stack size */
      ctop_int(CTXTc 2, (pb)top_of_heap - (pb)glstack.low);
      break;
    case 5: { /* current approximate memory usage */
      Integer pspacetot = 0;
      int i;
      for (i=0; i<NUM_CATS_SPACE; i++) pspacetot += pspacesize[i];
      // ignoring de, dl, pnde space for now...
      ctop_int(CTXTc 2, pspacetot + (pdl.size + glstack.size + tcpstack.size + complstack.size) * K);
      break;
    }
    default: xsb_domain_error(CTXTc "xwam_state_case",ptoc_tag(CTXTc 1),"xwam_state/2",1);
    }
    break;
  }
  case CODE_LOAD:		/* R1: +FileName, bytecode file to be loaded */
				/* R2: -int, addr of 1st instruction;	     */
				/*	0 indicates an error                 */
				/* R3 = 1 if exports to be exported, 0 otw   */
    SYS_MUTEX_LOCK( MUTEX_LOADER );
    ctop_int(CTXTc 2, (Integer)loader(CTXTc ptoc_longstring(CTXTc 1), (int)ptoc_int(CTXTc 3)));
    SYS_MUTEX_UNLOCK( MUTEX_LOADER );
    break;

  case PSC_INSERT: {	/* R1: +String, symbol name
			   R2: +Arity
			   R3: -PSC, the new PSC
			   R4: +String, module to be inserted */
    /* inserts or finds a symbol in a given module.	*/
    /* When the given module is 0 (null string), current module is used. */
    Psc  psc, sym;
    int  value = 0;
    char *addr = ptoc_string(CTXTc 4);
    if (addr)
      psc = pair_psc(insert_module(0, addr));
    else
      psc = (Psc)flags[CURRENT_MODULE];
    sym = pair_psc(insert(ptoc_string(CTXTc 1), (char)ptoc_int(CTXTc 2), psc, &value));
    if (value) {
      set_data(sym, psc);
      env_type_set(CTXTc sym, (addr?T_IMPORTED:T_GLOBAL) , T_ORDI, (xsbBool)value);
    }
    ctop_addr(3, sym);
    break;
  }

  case PSC_IMPORT: {    /* R1: +String, functor name to be imported
			   R2: +Arity
			   R3: +String, Module name where functor lives  */
    /*
     * Creates a PSC record for a predicate and its module (if they
     * don't already exist) and links the predicate into usermod.
     */
    int  value;
    Psc  psc = pair_psc(insert_module(0, ptoc_string(CTXTc 3)));
    Pair sym = insert(ptoc_string(CTXTc 1), (char)ptoc_int(CTXTc 2), psc, &value);
    if (value)       /* if predicate is new */
      set_data(pair_psc(sym), (psc));
    env_type_set(CTXTc pair_psc(sym), T_IMPORTED, T_ORDI, (xsbBool)value);
    if (flags[CURRENT_MODULE]) /* in case before flags is initted */
      link_sym(CTXTc pair_psc(sym), (Psc)flags[CURRENT_MODULE]);
    else link_sym(CTXTc pair_psc(sym), global_mod);
    break;
  }

  case PSC_IMPORT_AS: {    /* R1: PSC addr of source psc, R2 PSC addr of target psc */
    set_psc_ep_to_psc(CTXTc (Psc)ptoc_int(CTXTc 2),(Psc)ptoc_int(CTXTc 1));
    break;
  }


  case PSC_DATA:  {	/* R1: +PSC; R2: -int */
    Psc psc = (Psc)ptoc_addr(1);
    if (isstring(get_data(psc))) ctop_string(CTXTc 2, string_val(get_data(psc)));
    else ctop_int(CTXTc 2, (Integer)get_data(psc));
    break;
  }

    /* TLS: No MUTEX in FILE_GETTOKEN.  Its assumed that this is
       called from some other predicate with a stream lock, such as
       file_read. */

  case FILE_GETTOKEN: {    /* R1: +File, R2: +PrevCh, R3: -Type; */
                                /* R4: -Value, R5: -NextCh */

    token = GetToken(CTXTc (int)ptoc_int(CTXTc 1),(int)ptoc_int(CTXTc 2));
    if (token->type == TK_ERROR) {
      //      pcreg = (pb)&fail_inst;
      return FALSE;
    }
    else {
      ctop_int(CTXTc 3, token->type);
      ctop_int(CTXTc 5, token->nextch);
      switch (token->type) {
        case TK_ATOM : case TK_FUNC : case TK_STR : case TK_LIST :
        case TK_VAR : case TK_VVAR : case TK_VARFUNC : case TK_VVARFUNC :
	  // TLS 070416 -- change to fix double interning (ctop_string calls string_find)
	  //	  ctop_string(CTXTc 4, string_find(token->value,1));  // NOT INTERNED, CALLER MUST DO SO SOON!!
	  ctop_string(CTXTc 4, token->value);
	  break;
        case TK_INT : case TK_INTFUNC :
	  ctop_int(CTXTc 4, *(Integer *)(token->value));
	  break;
        case TK_REAL : case TK_REALFUNC :
	  ctop_float(CTXTc 4, *(double *)(token->value));
	  break;
        case TK_PUNC : case TK_HPUNC :
	  ctop_int(CTXTc 4, *(token->value)); break;
        case TK_EOC : case TK_EOF :
	  ctop_int(CTXTc 4, 0); break;
      }
    }
    break;
  }
    /* TLS: No MUTEX in FILE_PUTTOKEN.  Its assumed that this is
       called from some other predicate with a stream lock, such as
       file_write. */

  case FILE_PUTTOKEN: {	/* R1: +File, R2: +Type, R3: +Value; */
    FILE* fptr;
    int io_port = (int)ptoc_int(CTXTc 1);
    int charset;
    //    SYS_MUTEX_LOCK(MUTEX_IO);
    SET_FILEPTR_CHARSET(fptr,charset,io_port);
    switch (ptoc_int(CTXTc 2)) {
    case XSB_FREE   : {
      CPtr var = (CPtr)ptoc_tag(CTXTc 3);
      xsb_fprint_variable(CTXTc fptr, var);
      break;
    }
    case XSB_ATTV   : {
      CPtr var = (CPtr)dec_addr(ptoc_tag(CTXTc 3));
      xsb_fprint_variable(CTXTc fptr, var);
      break;
    }
    case XSB_INT    :
      if (fprintf(fptr, "%" Intfmt, (Integer)ptoc_int(CTXTc 3)) < 0)
	xsb_permission_error(CTXTc strerror(errno),"file write",
			     open_file_name(io_port),"file_puttoken",3);

      break;
    case XSB_STRING :
      write_string_code(fptr,charset,(byte *)ptoc_string(CTXTc 3));
      break;
    case XSB_FLOAT  : fprintf(fptr, "%2.4f", ptoc_float(CTXTc 3)); break;
    case TK_INT_0  : {
      int tmp = (int) ptoc_int(CTXTc 3);
      fix_bb4((byte *)&tmp);
      if (fwrite(&tmp, 4, 1, fptr) != 1)
	xsb_permission_error(CTXTc strerror(errno),"file write",
			     open_file_name(io_port),"file_puttoken",3);
      break;
    }
    case TK_FLOAT_0: {
      //printf("TK_FLOAT_0 case in put token entered\n");
      float ftmp = (float)ptoc_float(CTXTc 3);
      fix_bb4((byte *)&ftmp);
      if (fwrite(&ftmp, 4, 1, fptr) != 1)
	xsb_permission_error(CTXTc strerror(errno),"file write",
			     open_file_name(io_port),"file_puttoken",3);
      //printf("TK_FLOAT_0 case in put token left\n");
      break;
    }
    case TK_DOUBLE_0: {
      double ftmp = ptoc_float(CTXTc 3);
      if (fwrite(&ftmp, 8, 1, fptr) != 1)
	xsb_permission_error(CTXTc strerror(errno),"file write",
			     open_file_name(io_port),"file_puttoken",3);
      break;
    }
    case TK_PREOP  : print_op(fptr, charset, ptoc_string(CTXTc 3), 1); break;
    case TK_INOP   : print_op(fptr, charset, ptoc_string(CTXTc 3), 2); break;
    case TK_POSTOP : print_op(fptr, charset, ptoc_string(CTXTc 3), 3); break;
    case TK_QATOM  : print_qatom(fptr, charset, ptoc_string(CTXTc 3)); break;
    case TK_AQATOM : print_aqatom(fptr, charset, ptoc_string(CTXTc 3)); break;
    case TK_QSTR   : print_dqatom(fptr, charset, ptoc_string(CTXTc 3)); break;
    case TK_TERML  : print_term_canonical(CTXTc fptr, charset, ptoc_tag(CTXTc 3), 1); break;
    case TK_TERM   : print_term_canonical(CTXTc fptr, charset, ptoc_tag(CTXTc 3), 0); break;
    default : //printf("flg: %ld\n",(long)ptoc_int(CTXTc 2));
      xsb_abort("[FILE_PUTTOKEN] Unknown token type %ld",(long)ptoc_int(CTXTc 2));
    }
    //    SYS_MUTEX_UNLOCK(MUTEX_IO);
    break;
  }
  case PSC_INSERTMOD: { /* R1: +String, Module name */
                        /* R2: +Def (4 - is a definition; 0 -not) */
                        /* R3: -PSC of the Module entry */
    Pair sym = insert_module((int)ptoc_int(CTXTc 2), ptoc_string(CTXTc 1));
    ctop_addr(3, pair_psc(sym));
    break;
  }
  case TERM_HASH: {		/* R1: +Term	*/
				/* R2: +Size (of hash table) */
				/* R3: -HashVal */
    ctop_int(CTXTc 3, ihash(det_val_to_hash(ptoc_tag(CTXTc 1)),ptoc_int(CTXTc 2)));
    break;
  }
  case UNLOAD_SEG:	/* R1: -Code buffer */
    unload_seg((pseg)ptoc_int(CTXTc 1));
    break;
  case LOAD_OBJ:		/* R1: +FileName, R2: +Module (Psc) */
	    			/* R3: +ld option, R4: -InitAddr */
#ifdef FOREIGN
    ctop_int(CTXTc 4, (Integer)load_obj(CTXTc ptoc_string(CTXTc 1),(Psc)ptoc_addr(2),
				  ptoc_string(CTXTc 3)));
#else
    xsb_abort("Loading foreign object files is not implemented for this configuration");
#endif
    break;

  case WH_RANDOM:		/* R1: +Type of operation */
    switch (ptoc_int(CTXTc 1)) {
    case RET_RANDOM:		/* return a random float in [0.0, 1.0) */
      return ret_random(CTXT);
      break;
    case GET_RAND:		/* getrand */
      return getrand(CTXT);
      break;
    case SET_RAND:		/* setrand */
      setrand(CTXT);
      break;
    }
    break;

  case EXPAND_FILENAME:	       /* R1: +FileName, R2: -ExpandedFileName */
    {char *filename = expand_filename(ptoc_longstring(CTXTc 1));
      //    ctop_string(CTXTc 2, string_find(filename,1));
    ctop_string(CTXTc 2, filename);
    mem_dealloc(filename,MAXPATHLEN,OTHER_SPACE);
    }
    break;
  case TILDE_EXPAND_FILENAME:  /* R1: +FileN, R2: -TildeExpanded FN */
    /* TLS: we might be able to change this to a ctop_string without the string find? */
    ctop_string(CTXTc 2, tilde_expand_filename(ptoc_longstring(CTXTc 1)));
    break;
  case IS_ABSOLUTE_FILENAME: /* R1: +FN. Ret 1 if name is absolute, 0 else */
    return is_absolute_filename(ptoc_longstring(CTXTc 1));
 case PARSE_FILENAME: {    /* R1: +FN, R2: -Dir, R3: -Basename, R4: -Ext */
    char *dir, *basename, *extension;
    parse_filename(ptoc_longstring(CTXTc 1), &dir, &basename, &extension);
    ctop_string(CTXTc 2, dir);
    ctop_string(CTXTc 3, basename);
    ctop_string(CTXTc 4, extension);
    break;
  }
  case ALMOST_SEARCH_MODULE: /* R1: +FileName, R2: -Dir, R3: -Mod,
				R4: -Ext, R5: -BaseName */
    return almost_search_module(CTXTc ptoc_longstring(CTXTc 1));
  case EXISTING_FILE_EXTENSION: { /* R1: +FileN, R2: ?Ext */
    char *extension = existing_file_extension(ptoc_longstring(CTXTc 1));
    if (extension == NULL) return FALSE;
    else {
      extension = string_find(extension,1);
      return atom_unify(CTXTc makestring(extension), ptoc_tag(CTXTc 2));
    }
  }

  case DO_ONCE: { /* R1: +Breg */
#ifdef DEMAND
    perform_once();
#else
    xsb_abort("This XSB executable was not compiled with support for demand.\n");
#endif
    break;
  }
  case GETENV:  {	/* R1: +environment variable */
			/* R2: -value of that environment variable */
    char *env = getenv(ptoc_longstring(CTXTc 1));
    if (env == NULL)
      /* otherwise, string_find dumps core */
      return FALSE;
    else
      ctop_string(CTXTc 2, env);
    break;
  }
  case SYS_SYSCALL:	/* R1: +int (call #, see <syscall.h> */
				/* R2: -int, returned value */
	    			/* R3, ...: Arguments */
    ctop_int(CTXTc 2, sys_syscall(CTXTc (int)ptoc_int(CTXTc 1)));
    break;
  case SYS_SYSTEM:	/* R1: call mubler, R2: +String (of command);
			   R3: -Int (res), or mode: read/write;
			   R4: undefined or Stream used for output/input
			   from/to the shell command. */
    {
      xsbBool sys_system_return;
      sys_system_return = sys_system(CTXTc (int)ptoc_int(CTXTc 1));
      return sys_system_return;
    }
  case SYS_GETHOST: {
    /* +R1: a string indicating the host name  */
    /* +R2: a buffer (of length 16) for returned structure */
#ifdef HAVE_GETHOSTBYNAME
    static struct hostent *hostptr;
    hostptr = gethostbyname(ptoc_longstring(CTXTc 1));
#ifdef DARWIN	/* OS X returns an array of hostnames in h_addr_list */
    memmove(ptoc_longstring(CTXTc 2), hostptr->h_addr_list[0], hostptr->h_length);
#else
    memmove(ptoc_longstring(CTXTc 2), hostptr->h_addr, hostptr->h_length);
#endif
#else
    xsb_abort("[SYS_GETHOST] Operation not available for this XSB configuration");
#endif
    break;
  }
  case SYS_ERRNO:			/* R1: -Int (errno) */
    ctop_int(CTXTc 1, errno);
    break;

  case PUTENV:
    {
      return !putenv(ptoc_longstring(CTXTc 1));
      break;
    }

    /* TLS: file_writequoted is intended for use within l_write.  Do
       not use it directly -- as it should have its streams locked. */
  case FILE_WRITEQUOTED: {
    FILE *fptr;
    int   io_port = (int)ptoc_int(CTXTc 1);
    int   charset;
    SET_FILEPTR_CHARSET(fptr,charset,io_port);
    write_quotedname(fptr,charset,ptoc_string(CTXTc 2));
    break;
  }
  case GROUND:
    return ground(ptoc_tag(CTXTc 1));

  case PSC_INIT_INFO: {
    Psc psc = (Psc)ptoc_addr(1);
    init_psc_ep_info(psc);
    break;
  }

  case PSC_GET_SET_ENV_BYTE: { /* reg 1: +PSC, reg 2: +And-bits, reg 3: +Or-bits, reg 4: -Result */
    Psc psc = (Psc)ptoc_addr(1);
    psc->env = (psc->env & (byte)ptoc_int(CTXTc 2)) | (byte)ptoc_int(CTXTc 3);
    ctop_int(CTXTc 4, (Integer)(psc->env));
    break;
  }

  case PSC_ENV:	{       /* reg 1: +PSC; reg 2: -int */
    /* env: 0 = exported, 1 = local, 2 = imported */
    Psc psc = (Psc)ptoc_addr(1);
    ctop_int(CTXTc 2, (Integer)get_env(psc));
    break;
  }
  case PSC_SPY:	{	/* reg 1: +PSC; reg 2: -int */
				/* env: 0 = non-spied else spied */
    Psc psc = (Psc)ptoc_addr(1);
    ctop_int(CTXTc 2, (Integer)get_spy(psc));
    break;
  }
  case PSC_TABLED: {	/* reg 1: +PSC; reg 2: -int */
    Psc psc = (Psc)ptoc_addr(1);
    ctop_int(CTXTc 2, get_tabled(psc));  // Returns both bits -- subsumptive / variant
    break;
  }
  case PSC_SET_TABLED: {	/* reg 1: +PSC; reg 2: +int */
    Psc psc = (Psc)ptoc_addr(1);
    if (ptoc_int(CTXTc 2)) set_tabled(psc,0x08);
    else psc->env = psc->env & ~0x8; /* turn off */
    break;
  }
    //  case PSC_ENV: {	/* reg 1: +PSC; reg 2: +int-anded; reg 3: +int-orred; reg 4: -Result*/
    //    Psc psc = (Psc)ptoc_addr(1);
    //    psc->env = ((psc->env & ptoc_int(CTXTc 2)) | ptoc_int(CTXTc 3));
    //    ctop_int(CTXTc 4, psc->env);
    //    break;
    //  }



/*----------------------------------------------------------------------*/

#include "bineg_xsb_i.h"

/*----------------------------------------------------------------------*/

  case GET_PRODUCER_CALL: {
    const int Arity = 3;
    const int regCallTerm = 1;  /* in: tabled subgoal */
    const int regSF       = 2;  /* out: subgoal frame of producer from
				        which subgoal can consume */
    const int regRetTerm  = 3;  /* out: answer template in ret/N form */

    Cell term;
    Psc  psc;
    TIFptr tif;
    void *sf;
    Cell retTerm;

    term = ptoc_tag(CTXTc regCallTerm);
    if ( isref(term) ) {
      xsb_instantiation_error(CTXTc "get_producer_call/3",regCallTerm);
      break;
    }
    psc = term_psc(term);
    if ( IsNULL(psc) ) {
      xsb_type_error(CTXTc "callable",term,"get_producer_call/3",regCallTerm);
      break;
    }
    tif = get_tip(CTXTc psc);
    if ( IsNULL(tif) )
      xsb_abort("Illegal table operation\n\t Untabled predicate (%s/%d)"
		"\n\t In argument %d of %s/%d",
		get_name(psc), get_arity(psc), regCallTerm,
		BuiltinName(GET_PRODUCER_CALL), Arity);

    if ( IsSubsumptivePredicate(tif) )
      sf = get_subsumer_sf(CTXTc term, tif, &retTerm);
    else
      sf = get_variant_sf(CTXTc term, tif, &retTerm);
    if ( IsNULL(sf) )
      return FALSE;
    ctop_addr(regSF, sf);
    ctop_tag(CTXTc regRetTerm, retTerm);
    break;
  }

  case DEREFERENCE_THE_BUCKET:
    /*
     * Given an index into the symbol table, return the first Pair
     * in that bucket's chain.
     */
    ctop_int(CTXTc 2, (Integer)(symbol_table.table[ptoc_int(CTXTc 1)]));
    break;
  case PAIR_PSC:
    ctop_addr(2, pair_psc((Pair)ptoc_addr(1)));
    break;
  case PAIR_NEXT:
    ctop_addr(2, pair_next((Pair)ptoc_addr(1)));
    break;
  case NEXT_BUCKET: {     /* R1: +Index of Symbol Table Bucket. */
    /* R2: -Next Index (0 if end of Hash Table) */
    size_t value = ptoc_int(CTXTc 1);
    // TLS: fixing clang errors:
    //    if ( ((unsigned int)value >= (symbol_table.size - 1)) || (value < 0) )
    if  ((unsigned int)value >= (symbol_table.size - 1))
      ctop_int(CTXTc 2, 0);
    else
      ctop_int(CTXTc 2, (value + 1));
    break;
  }

  case IS_XWAMMODE:     /* R1: -int flag for xwammode */

    if (xwammode) ctop_int(CTXTc 1,1);
    else ctop_int(CTXTc 1,0);
    break;

  case CLOSE_OPEN_TABLES:
    //    printf("close open tables... %d\n",ptoc_int(CTXTc 1));
    remove_incomplete_tables_reset_freezes(CTXTc (int)ptoc_int(CTXTc 1));
#ifdef MULTI_THREAD
    release_held_mutexes(CTXT);
#endif
    break;

  case ABOLISH_ALL_TABLES:
    abolish_all_tables(CTXTc ERROR_ON_INCOMPLETES);
    break;

  case ZERO_OUT_PROFILE:
#if defined(PROFILE) && !defined(MULTI_THREAD)
    {
      int i;
      for (i = 0 ; i <= BUILTIN_TBL_SZ ; i++) {
	inst_table[i][5] = 0;
	builtin_table[i][1] = 0;
	subinst_table[i][1] = 0;
      }
      num_switch_envs=0;
    }
    break;
#else
    xsb_abort("Profiling is not enabled for this XSB configuration");
#endif
case WRITE_OUT_PROFILE:
#if defined(PROFILE) && !defined(MULTI_THREAD)
    write_out_profile();
    break;
#else
    xsb_abort("Profiling is not enabled for this XSB configuration");
#endif
  case ASSERT_CODE_TO_BUFF:
    assert_code_to_buff(CTXT);
    break;
  case ASSERT_BUFF_TO_CLREF:
    assert_buff_to_clref(CTXT);
    break;
  case DIRNAME_CANONIC: /* R1: +Dirname, R2: -Canonicized Dirname:
			   If file is a directory, add trailing slash and
			   rectify filename (delete multiple slashes, '..' and
			   '.'. */
    ctop_string(CTXTc 2, dirname_canonic(ptoc_longstring(CTXTc 1)));
    break;
  case SLASH_BUILTIN: {  /* R1: -Slash. Tells what kind of slash the OS uses */
    static char slash_string[2];
    slash_string[0] = SLASH;
    slash_string[1] = '\0';
    ctop_string(CTXTc 1, slash_string);
    break;
  }
  case FORMATTED_IO:
    return formatted_io(CTXT);

  case GET_TRIE_LEAF:
    ctop_int(CTXTc 1, (Integer)Last_Nod_Sav);
    break;

  case FILE_READ_CANONICAL:
    return read_canonical(CTXT);

  case GEN_RETRACT_ALL:
    return gen_retract_all(CTXT);
  case DB_GET_LAST_CLAUSE:
    return db_get_last_clause(CTXT);
    break;
  case DB_RETRACT0:
    db_retract0(CTXT);
    break;
  case DB_GET_CLAUSE:
    db_get_clause(CTXT);
    break;
  case DB_BUILD_PRREF:
    db_build_prref(CTXT);
    break;
  case DB_GET_PRREF:
    db_get_prref(CTXT);
    break;
  case DB_ABOLISH0:
    db_abolish0(CTXT);
    break;
  case DB_RECLAIM0:
    db_reclaim0(CTXT);
    break;

  case EXCESS_VARS: {
    Cell term, templ, startvlist, ovar, retlist;
    CPtr anslist, tanslist, tail;
    int max_num_vars;
    struct ltrail templ_trail, var_trail;
    void *templ_trail_base, *var_trail_base;
    templ_trail.ltrail_top = templ_trail_base = ltrail_base(templ_trail);
    var_trail.ltrail_top = var_trail_base = ltrail_base(var_trail);

    max_num_vars = count_variable_occurrences(ptoc_tag(CTXTc 1))+count_variable_occurrences(ptoc_tag(CTXTc 3));
    if (max_num_vars > MAX_LOCAL_TRAIL_SIZE) xsb_error("Buffer overflow in EXCESS_VARS: %d\n",max_num_vars);
    check_glstack_overflow(4,pcreg,1+max_num_vars*sizeof(Cell)*2);

    anslist = tanslist = hreg++;
    startvlist = ptoc_tag(CTXTc 3);
    XSB_Deref(startvlist);
    while (!isnil(startvlist)) {
      if (islist(startvlist)) {
	ovar = get_list_head(startvlist);
	XSB_Deref(ovar);
	if (isref(ovar) || isattv(ovar)) {
	  if (isattv(ovar)) ovar = dec_addr(ovar);
	  cell(tanslist) = makelist(hreg);
	  bld_ref(hreg,ovar);
	  local_bind_var(ovar, &var_trail);
	  tanslist = hreg+1;
	  hreg += 2;
	  startvlist = get_list_tail(startvlist);
	  XSB_Deref(startvlist);
	} else {
	  xsb_type_error(CTXTc "list of variables",ptoc_tag(CTXTc 3),"excess_vars/4",3);
	  break;
	}
      } else {
	  xsb_type_error(CTXTc "list of variables",ptoc_tag(CTXTc 3),"excess_vars/4",3);
	break;
      }
    }
    templ = ptoc_tag(CTXTc 2);
    make_ground(templ, &var_trail);
    term = ptoc_tag(CTXTc 1);
    tail = excess_vars(CTXTc term,tanslist,(int)ptoc_int(CTXTc 4),&templ_trail,&var_trail);
    cell(tail) = makenil;
    undo_ltrail_bindings(&templ_trail,templ_trail_base);
    undo_ltrail_bindings(&var_trail,var_trail_base);
    retlist = ptoc_tag(CTXTc 5);
    if (list_unifiable(CTXTc retlist)) return unify(CTXTc (Cell)anslist,retlist);
    else xsb_type_error(CTXTc "list",retlist,"term_variables/2",2);
  }
  break;

/*----------------------------------------------------------------------*/

#include "std_cases_xsb_i.h"

#ifdef ORACLE
#include "oracle_xsb_i.h"
#endif

#ifdef XSB_ODBC
#include "odbc_xsb_i.h"
#else
  case ODBC_EXEC_QUERY: {
    xsb_abort("[ODBC] XSB not compiled with ODBC support.\nReconfigure using the option --with-odbc and then recompile XSB.\n");
  }
#endif

#ifdef XSB_INTERPROLOG
#include "interprolog_xsb_i.h"
#endif

/*----------------------------------------------------------------------*/

  case TABLE_STATUS: {
    Cell goalTerm;
    TableStatusFrame TSF;

    goalTerm = ptoc_tag(CTXTc 1);
    if ( isref(goalTerm) ) {
      xsb_instantiation_error(CTXTc "table_status/4",1);
      break;
    }

    // table_status() now also used by tnot/1
    table_status(CTXTc goalTerm, &TSF);

    ctop_int(CTXTc 2,TableStatusFrame_pred_type(TSF));
    ctop_int(CTXTc 3,TableStatusFrame_goal_type(TSF));
    ctop_int(CTXTc 4,TableStatusFrame_answer_set_status(TSF));
    ctop_addr(5, TableStatusFrame_subgoal(TSF));
    return TRUE;
  }

  case ANSWER_COMPLETION_OPS: {
    switch (ptoc_int(CTXTc 1)) {
    case 1:  // 1 is reset needs_completion
      answer_complete_subg(ptoc_int(CTXTc 2));
      break;
    case 2:  // 2 is get needs_answer_completion flag
      if (subg_is_answer_completed(ptoc_int(CTXTc 2)))
	ctop_int(CTXTc 3, 1);
      else ctop_int(CTXTc 3, 0);
      break;
    default: 
      xsb_abort("builtin(ANSWER_COMPLETION_OPS): illegal op: %d",ptoc_int(CTXTc 1));
    }
    return TRUE;
  }

  case ABOLISH_TABLE_PREDICATE: {
    const int regTerm = 1;   /* in: tabled predicate as term */
    Cell term;
    Psc psc;

    term = ptoc_tag(CTXTc regTerm);

    psc = term_psc(term);
    if ( IsNULL(psc) ) {
      xsb_domain_error(CTXTc "predicate_or_term_indicator",term,
  		       "abolish_table_pred/1", 1) ;
      break;
    }
    abolish_table_predicate(CTXTc psc,(int)ptoc_int(CTXTc 2));
    return TRUE;
  }

  case ABOLISH_TABLE_CALL: {
    VariantSF subg=(VariantSF) ptoc_int(CTXTc 1);
    abolish_table_call(CTXTc subg, (int)ptoc_int(CTXTc 2));
    return TRUE;
  }

  case ABOLISH_MODULE_TABLES: {
    char *module_name;

    module_name = ptoc_string(CTXTc 1);
    if (!strcmp(module_name,"usermod") || !strcmp(module_name,"global"))
      return abolish_usermod_tables(CTXT);
    else
      return abolish_module_tables(CTXTc module_name);
    break;
  }
  case TRIE_ASSERT:
    if (trie_assert(CTXT))
      return TRUE;
    else
      xsb_exit("Failure of trie_assert/1");
  case TRIE_RETRACT:
    if (trie_retract(CTXT))
      return TRUE;
    else
      xsb_exit("Failure of trie_retract/1");

  case TRIE_RETRACT_SAFE:
    return trie_retract_safe(CTXT);

  case TRIE_DELETE_RETURN: {
    const int regSF = 1;   /* in: subgoal frame ref */
    const int regReturnNode = 2;   /* in: answer trie node */
    //    const int Arity = 2;
    VariantSF sf;
    BTNptr leaf;

    sf = ptoc_addr(regSF);

    leaf = ptoc_addr(regReturnNode);

    if (IsVariantSF(sf)) {
      /*
	| 	if ( ! smIsValidStructRef(*smBTN,leaf) )
	| 	  xsb_abort("Invalid Return Handle\n\t Argument %d of %s/%d",
	| 		    regReturnNode, BuiltinName(TRIE_DELETE_RETURN), Arity);
	| 	if ( (! smIsAllocatedStruct(*smBTN,leaf)) ||
	| 	     (subg_ans_root_ptr(sf) != get_trie_root(leaf)) ||
	|	     (! IsLeafNode(leaf)) )
      */
      /*      if ( (! smIsAllocatedStruct(*smBTN,leaf)) ||
	   (subg_ans_root_ptr(sf) != get_trie_root(leaf)) ||
	   (! IsLeafNode(leaf)) )
	   return FALSE;*/

      if (!subg_is_completed(sf) || ptoc_int(CTXTc 3) == USER_DELETE) {
	//	printf("subg is not ec-scheduled %d\n",subg_is_completed(sf));
	SET_TRIE_ALLOCATION_TYPE_SF(sf); /* set to private/shared SM */
	delete_return(CTXTc leaf,sf,VARIANT_EVAL_METHOD);
      }
      else {
	//	printf("subg is ec-scheduled\n");
	return FALSE;
      }
    }
      else delete_return(CTXTc leaf,sf,SUBSUMPTIVE_EVAL_METHOD);
    break;
  }

  case TRIE_GET_RETURN: {
    const int regTableEntry = 1;   /* in: subgoal frame ref */
    const int regRetTerm    = 2;   /* in/out: ret/n term to unify against
				              answer substitutions */
    VariantSF sf;
    Cell retTerm;

    sf = ptoc_addr(regTableEntry);
#ifdef DEBUG_ASSERTIONS
  /* Need to change for MT: smVarSF can be private or shared
|    if ( ! smIsValidStructRef(smVarSF,sf) &&
|	 ! smIsValidStructRef(smProdSF,sf) &&
|	 ! smIsValidStructRef(smConsSF,sf) )
|      xsb_abort("Invalid Table Entry Handle\n\t Argument %d of %s/%d",
|		regTableEntry, BuiltinName(TRIE_GET_RETURN), Arity);
  */
#endif
    retTerm = ptoc_tag(CTXTc regRetTerm);
    if ( isref(retTerm) ) {
      xsb_instantiation_error(CTXTc "trie_get_return/2",regRetTerm);
      break;
    }
    pcreg = trie_get_return(CTXTc sf, retTerm, (int)ptoc_int(CTXTc 3));
    break;
  }

  case TRIE_ASSERT_HDR_INFO: /* r1: 0 -> r2: +TrieNodeAddr, r3: -RootOfCall
				r1: 1 -> r2: +Clref, r3: -trieNodeAddr,
					     fail if Clref not for a trie. */
    switch (ptoc_int(CTXTc 1)) {
    case 0:  /* r1: 0 -> r2: +TrieNodeAddr, r3: -RootOfCall */
      ctop_int(CTXTc 3,(Integer)(((BTNptr)(ptoc_int(CTXTc 2)))->child));
      break;
    case 1: {
      BTNptr trienode = trie_asserted_trienode((CPtr)ptoc_int(CTXTc 2));
      if (trienode) ctop_int(CTXTc 3, (Integer)trienode);
      else return FALSE;
      break;
    }
    }
    break;

  case TRIE_UNIFY_CALL: /* r1: +call_term */
    pcreg = trie_get_calls(CTXT);
    break;

  case GET_LASTNODE_CS_RETSKEL: {
    const int regCallTerm  = 1;   /* in: call of a tabled predicate */
    const int regTrieLeaf  = 2;   /* out: a unifying trie term handle */
    const int regLeafChild = 3;   /* out: usually to get subgoal frame */
    const int regRetTerm   = 4;   /* out: term in ret/N form:
				     Call Trie -> answer template
				     Other Trie -> variable vector */
    Cell call_term = ptoc_tag(CTXTc regCallTerm);
    if (isconstr(call_term)) {
      Psc psc = term_psc(call_term);
      if (get_incr(psc) && (get_type(psc) == T_DYNA)) {
	xsb_abort("get_calls/3 called with incremental dynamic predicate: %s/%d",
		  get_name(psc),get_arity(psc));
      }
    }
    ctop_int(CTXTc regTrieLeaf, (Integer)Last_Nod_Sav);
    ctop_int(CTXTc regLeafChild, (Integer)BTN_Child(Last_Nod_Sav));
    ctop_tag(CTXTc regRetTerm, get_lastnode_cs_retskel(CTXTc call_term));
    return TRUE;
  }

  case TRIE_GET_CALL: {
    const int regCallTerm = 1;   /* in:  tabled call to look for */
    const int regSF       = 2;   /* out: corresponding subgoal frame */
    const int regRetTerm  = 3;   /* out: answer template in ret/N form */

    Cell ret;
    VariantSF sf;

    sf = get_call(CTXTc ptoc_tag(CTXTc regCallTerm), &ret);
    if ( IsNonNULL(sf) ) {
      ctop_int(CTXTc regSF, (Integer)sf);
      ctop_tag(CTXTc regRetTerm, ret);
      return TRUE;
    }
    else
      return FALSE;
  }

  case BREG_RETSKEL:
    breg_retskel(CTXT);
    break;

  case TRIMCORE:

    /*
     * In each case, check whether the initial size of the data area is
     * large enough to contain the currently used portion of the data area.
     */
    if (tcpstack.size != tcpstack.init_size)
      if ( (unsigned int)((tcpstack.high - (byte *)top_of_cpstack) +
		     ((byte *)top_of_trail - tcpstack.low))
	   < tcpstack.init_size * K - OVERFLOW_MARGIN )
	tcpstack_realloc(CTXTc tcpstack.init_size);

    if (complstack.size != complstack.init_size)
      if ( (unsigned int)(complstack.high - (byte *)openreg)
	   < complstack.init_size * K - OVERFLOW_MARGIN )
	complstack_realloc(CTXTc complstack.init_size);

    if (glstack.size != glstack.init_size)
	glstack_realloc(CTXTc glstack.init_size,0);

    tstShrinkDynStacks(CTXT);
    break;

  case NEWTRIE:
    ctop_int(CTXTc 1,newtrie(CTXTc (int)ptoc_int(CTXTc 2)));
    break;
  case TRIE_INTERN:
    return(private_trie_intern(CTXT));
    break;
  case TRIE_INTERNED:
    return(private_trie_interned(CTXT));
  case TRIE_UNINTERN:
    private_trie_unintern(CTXT);
    break;
  case TRIE_DISPOSE_NR:
    trie_dispose_nr(CTXT);
    break;
  case TRIE_UNDISPOSE:
    trie_undispose(CTXTc iso_ptoc_int(CTXTc 1,"unmark_uninterned_nr/2"),
		   (BTNptr) iso_ptoc_int(CTXTc 2,"unmark_uninterned_nr/2"));
    break;
  case RECLAIM_UNINTERNED_NR:
    reclaim_uninterned_nr(CTXTc iso_ptoc_int(CTXTc 1,"reclaim_uninterned_nr/1"));
    break;
  case GLOBALVAR:
    ctop_tag(CTXTc 1, cell((CPtr)glstack.low+2));
    break;
  case CCALL_STORE_ERROR: {
#ifdef MULTI_THREAD
    create_ccall_error(find_context(xsb_thread_id), ptoc_string(CTXTc 1),ptoc_string(CTXTc 2));
#else
    create_ccall_error(ptoc_string(CTXTc 1),ptoc_string(CTXTc 2));
#endif
    break;
  }

  case STORAGE_BUILTIN: {
    STORAGE_HANDLE *storage_handle =
      storage_builtin(CTXTc (int)ptoc_int(CTXTc 1), (Cell)ptoc_tag(CTXTc 2), reg_term(CTXTc 3));
    if (storage_handle != NULL) {
      ctop_int(CTXTc 4, (Integer)storage_handle->handle);
      ctop_int(CTXTc 5, (Integer)storage_handle->snapshot_number);
      ctop_int(CTXTc 6, (Integer)storage_handle->changed);
    }
    break;
  }

  case INCR_EVAL_BUILTIN: {
    return(incr_eval_builtin(CTXT));
    break;
  }

  case BOTTOM_UP_UNIFY:
    return ( bottom_up_unify(CTXT) );

  case TRIE_TRUNCATE:
    // TLS: dont know why arg 2 is checked
    trie_truncate(CTXTc  iso_ptoc_int(CTXTc 1,"trie_truncate/1"));
    break;

    case SET_TABLED_EVAL: { /* reg 1=psc, reg 2=eval method to use */
    Psc psc;
    Cell term = ptoc_tag(CTXTc 1);
    int eval_meth = (int)ptoc_int(CTXTc 2);

    if ( isref(term) ) {
      xsb_instantiation_error(CTXTc "set_tabled_eval/2",1);
      break;
    }
    psc = term_psc(term);
    if ( IsNULL(psc) ) {
      xsb_type_error(CTXTc "predicate_indicator",term,"set_tabled_eval/2",1);
      break;
    }
    /* TLS (April 2010): experimental approach that allows a change in
       tabling method even if a predicate already has tables.  The
       idea is that new tables will get the method from the psc or
       tip, while incomplete tables will get the method from the
       subgoal frame.  Let's see if it works ...    */
    /* changing to variant */
    if ((eval_meth == VARIANT_EVAL_METHOD) && (get_tabled(psc) != T_TABLED_VAR)) {
      set_tabled(psc,T_TABLED_VAR);
      //      if (get_tabled(psc) == T_TABLED)
      if (get_tip(CTXTc psc)) {
	TIF_EvalMethod(get_tip(CTXTc psc)) = VARIANT_EVAL_METHOD;
	if (TIF_CallTrie(get_tip(CTXTc psc))) {
	  xsb_warn(CTXTc "Change to variant tabling method for predicate with tabled subgoals: %s/%d",
		   get_name(psc),get_arity(psc));
	}
      }
    } else if ((eval_meth == SUBSUMPTIVE_EVAL_METHOD) && (get_tabled(psc) != T_TABLED_SUB)) {
      if (get_incr(psc))
	xsb_abort("cannot change %s/%n to use call subsumption as it is tabled incremental\n",
		  get_name(psc),get_arity(psc));
      set_tabled(psc,T_TABLED_SUB);
      /* T_TABLED if the predicate is known to be tabled, but no specific eval method */
      if (get_tip(CTXTc psc)) {
	TIF_EvalMethod(get_tip(CTXTc psc)) = SUBSUMPTIVE_EVAL_METHOD;
	if (TIF_CallTrie(get_tip(CTXTc psc))) {
	  xsb_warn(CTXTc "Change to subsumptive tabling method for predicate with tabled subgoals: %s/%d",
		   get_name(psc),get_arity(psc));
	}
      }
    }

    /***    tif = get_tip(CTXTc psc);
    if ( IsNULL(tif) ) {
      xsb_warn(CTXTc "Predicate %s/%d is not tabled", get_name(psc), get_arity(psc));
      return FALSE;
    }
    if ( IsNonNULL(TIF_CallTrie(tif)) ) {
      xsb_warn(CTXTc "Cannot change tabling method for tabled predicate %s/%d\n"
	       "\t   Calls to %s/%d have already been issued\n",
	       get_name(psc), get_arity(psc), get_name(psc), get_arity(psc));
      return FALSE;
    }
    TIF_EvalMethod(tif) = (TabledEvalMethod)ptoc_int(CTXTc regTEM);
***/
    return TRUE;
  }

    case UNIFY_WITH_OCCURS_CHECK:
      return unify_with_occurs_check(CTXTc cell(reg+1),cell(reg+2));

  case XSB_PROFILE:
    {
      if (xsb_profiling_enabled) {
	int call_type = (int)ptoc_int(CTXTc 1);
	if (call_type == 1) { /* turn profiling on */
	  if (!profile_thread_started) {
	    if (!startProfileThread()) {
	      xsb_abort("[XSB_PROFILE] Profiling thread does not start");
	    } else profile_thread_started = TRUE;
	  }
	  if_profiling = 1;
	} else if (call_type == 2) {
	  if_profiling = 0;
	} else if (call_type == 3) {
	  retrieve_prof_table();
	} else {
	  xsb_abort("[XSB_PROFILE] Unknown profiling command");
	}
	return TRUE;
      } else return FALSE;
    }

  case XSB_BACKTRACE:
    switch (ptoc_int(CTXTc 1)) {
    case 1:
      print_xsb_backtrace(CTXT);
      break;
    case 2:
      return unify(CTXTc ptoc_tag(CTXTc 2),build_xsb_backtrace(CTXT));
      break;
    }
    break;

  case COPY_TERM_3:
    return copy_term_3(CTXT);

    // Does not appear to be used
  case EXP_HEAP: glstack_realloc(CTXTc glstack.size + 1,0) ; return TRUE ;

  case MARK_HEAP: {
    size_t tmpval;
    mark_heap(CTXTc (int)ptoc_int(CTXTc 1),&tmpval);
    return TRUE;
  }

    /* TLS: changed && -> & */
  case GC_STUFF: {
    int gc = (int)ptoc_int(CTXTc 1);
    int ret_val = 0;
    if (gc & GC_GC_STRINGS) {
      gc &= ~GC_GC_HEAP;
      ret_val |= gc_heap(CTXTc 2,TRUE);
    }
    if (gc & GC_GC_HEAP) ret_val |= gc_heap(CTXTc 2,FALSE);
    if (gc & GC_GC_CLAUSES) ret_val |= gc_dynamic(CTXT);
    if (gc & GC_GC_TABLED_PREDS) {
#ifndef MULTI_THREAD
      double timer = cpu_time();
#endif
      int local_ret_val = 0;

      local_ret_val = gc_tabled_preds(CTXT);

#ifndef MULTI_THREAD
      total_table_gc_time =  total_table_gc_time + (cpu_time() - timer);
#endif

      ret_val |= local_ret_val;

    }

    ctop_int(CTXTc 2, ret_val);
    return TRUE;
  }
  case FLOAT_OP:
#ifdef FAST_FLOATS
    {xsb_error("Builtin float_op/10 not implemented in this configuration (FAST_FLOATS)");}
#else
  {
    char * operator = ptoc_string(CTXTc 1);
    Float result;
    switch((*operator))
    {
    case '+':
    result =
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 2)), (ptoc_int(CTXTc 3)), (ptoc_int(CTXTc 4))))
        +
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 5)), (ptoc_int(CTXTc 6)), (ptoc_int(CTXTc 7))));
        break;
    case '-':
    result =
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 2)), (ptoc_int(CTXTc 3)), (ptoc_int(CTXTc 4))))
        -
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 5)), (ptoc_int(CTXTc 6)), (ptoc_int(CTXTc 7))));
        break;
    case '*':
    result =
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 2)), (ptoc_int(CTXTc 3)), (ptoc_int(CTXTc 4))))
        *
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 5)), (ptoc_int(CTXTc 6)), (ptoc_int(CTXTc 7))));
        break;
    case '/':
    result =
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 2)), (ptoc_int(CTXTc 3)), (ptoc_int(CTXTc 4))))
        /
        (EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 5)), (ptoc_int(CTXTc 6)), (ptoc_int(CTXTc 7))));
        break;
    case 'c': {
      Cell addr = ptoc_tag(CTXTc 5);
      if (isref(addr)) {
	result = ((Float)(EXTRACT_FLOAT_FROM_16_24_24((ptoc_int(CTXTc 2)), (ptoc_int(CTXTc 3)), (ptoc_int(CTXTc 4)))));
	/* XSB_Deref(addr); unnec since ptoc_tag derefs */
	bind_boxedfloat(((CPtr)addr), result);
      } else {
	result = ptoc_float(CTXTc 5);
	ctop_int(CTXTc 2, (FLOAT_HIGH_16_BITS(result)));
	ctop_int(CTXTc 3, (FLOAT_MIDDLE_24_BITS(result)));
	ctop_int(CTXTc 4, (FLOAT_LOW_24_BITS(result)));
      }
      return TRUE;
    }
    default:
        result = 0.0;
        xsb_abort("[float_op] unsupported operator: %s\n", operator);
        return FALSE;
    }
    ctop_int(CTXTc 8, ((ID_BOXED_FLOAT << BOX_ID_OFFSET ) | FLOAT_HIGH_16_BITS(result) ));
    ctop_int(CTXTc 9, (FLOAT_MIDDLE_24_BITS(result)));
    ctop_int(CTXTc 10, (FLOAT_LOW_24_BITS(result)));
    return TRUE;
  }
#endif

    /* This is the builtin where people should put their private, experimental
       builtin code. SEE THE EXAMPLE IN private_builtin.c to UNDERSTAND HOW TO
       DO IT. Note: even though this is a single builtin, YOU CAN SIMULATE ANY
       NUMBER OF BUILTINS WITH IT.  */

  case PRIVATE_BUILTIN:
  {
    //    private_builtin();
    XSB_StrSet(&str_op1,"");
    print_pterm(CTXTc ptoc_tag(CTXTc 1),FALSE,&str_op1);
    printf("%s\n",str_op1.string);
    return TRUE;
  }

  case INTERN_TERM:
  {
    Integer termsize;
    prolog_term term;

    term = ptoc_tag(CTXTc 1);
    XSB_Deref(term);
    if (!isinternstr_really(term)) {
      termsize = intern_term_size(CTXTc term);
      check_glstack_overflow(2,pcreg,termsize*sizeof(Cell));
      term = intern_term(CTXTc ptoc_tag(CTXTc 1));
    }
    if (term) {
      //printf("o %p\n",term);
      return unify(CTXTc term, ptoc_tag(CTXTc 2));
    } else {
      //printf("of\n");
      return FALSE;
    }
  }

  case SEGFAULT_HANDLER: { /* Set the desired segfault handler:
			      +Arg1:  none  - don't catch segfaults;
				      warn  - warn and exit;
				      catch - try to recover */
    char *type = ptoc_string(CTXTc 1);
    switch (*type) {
    case 'w': /* warn: Warn and exit */
      xsb_default_segfault_handler = xsb_segfault_quitter;
      break;
    case 'n': /* none: Don't handle segfaults */
      xsb_default_segfault_handler = SIG_DFL;
      break;
    case 'c': /* catch: Try to recover from all segfaults */
      xsb_default_segfault_handler = xsb_segfault_catcher;
      break;
    default:
      xsb_warn(CTXTc "Request for unsupported type of segfault handling, %s", type);
      return TRUE;
    }
#ifdef SIGBUS
    signal(SIGBUS, xsb_default_segfault_handler);
#endif
    signal(SIGSEGV, xsb_default_segfault_handler);
    return TRUE;
  }

    /* For call_cleanup -- need offset to avoid problems w. stack reallocation */
    case GET_BREG: {
      //      printf("get_breg %x %x\n",breg,*breg);
      ctop_addr(1,((pb)tcpstack.high - (pb)breg));
      break;
    }
  case IS_CHARLIST: {
    prolog_term size_var;
    int size;
    xsbBool retcode;
    size_var = reg_term(CTXTc 2);
    if (! isref(size_var)) {
      xsb_abort("[IS_CHARLIST] Arg 2 must be a variable: %s",canonical_term(CTXTc size_var, 0));
    }
    retcode = is_charlist(reg_term(CTXTc 1), &size);
    c2p_int(CTXTc size,size_var);
    return retcode;
  }

    case DYNAMIC_CODE_FUNCTION: {
      return dynamic_code_function(CTXT);
    }

    case TABLE_INSPECTION_FUNCTION: {
      return table_inspection_function(CTXT);
      break;
    }

    case URL_ENCODE_DECODE: {
      Integer url_func_type = ptoc_int(CTXTc 1);
      char *url_str = ptoc_string(CTXTc 2);
      char *out_url;

      if (url_func_type == URL_ENCODE)
	out_url = url_encode(url_str);
      else
	out_url = url_decode(url_str);

      ctop_string(CTXTc 3, out_url);
      return TRUE;
      break;
    }

    case CHECK_CYCLIC: {
      if (is_cyclic(CTXTc (Cell) ptoc_tag(CTXTc 1))) {
	sprintCyclicTerm(CTXTc forest_log_buffer_1, (Cell) ptoc_tag(CTXTc 1));
	xsb_abort("Illegal cyclic term in arg %d of %s: %s\n",ptoc_int(CTXTc 3),ptoc_string(CTXTc 2),
		  forest_log_buffer_1->fl_buffer);
      }
      return TRUE;
      break;
    }

    case FINDALL_FREE:
      findall_free(CTXTc (int)ptoc_int(CTXTc 1));
      return TRUE;
      break;
    case FINDALL_INIT: return(findall_init(CTXT)) ;
    case FINDALL_ADD: return(findall_add(CTXT)) ;
    case FINDALL_GET_SOLS: return(findall_get_solutions(CTXT)) ;

#ifdef HAVE_SOCKET
    case SOCKET_REQUEST: {
      xsbBool xsb_socket_request_return;
      xsb_socket_request_return = xsb_socket_request(CTXT);
      return xsb_socket_request_return;
    }
#endif /* HAVE_SOCKET */

#ifdef WIN_NT
  case JAVA_INTERRUPT:
    return( startInterruptThread( (SOCKET)ptoc_int(CTXTc 1) ) );
#endif

  case FORCE_TRUTH_VALUE: { /* +R1: AnsLeafPtr; +R2: TruthValue */
    BTNptr as_leaf = (BTNptr)ptoc_addr(1);
    char *tmpstr = ptoc_string(CTXTc 2);
    if (!strcmp(tmpstr, "true"))
      force_answer_true(as_leaf);
    else if (!strcmp(tmpstr, "false"))
      force_answer_false(as_leaf);
    else xsb_abort("[FORCE_TRUTH_VALUE] Argument 2 has unknown truth value (%s)",tmpstr);
    break;
  }

  case PUT_ATTRIBUTES: { /* R1: -Var; R2: +List */
    Cell attv = ptoc_tag(CTXTc 1);
    Cell atts = ptoc_tag(CTXTc 2);
    if (isref(attv)) {		/* attv is a free var */
      if (!isnil(atts)) {
	bind_attv((CPtr)attv, hreg);
	bld_free(hreg);
	bld_copy(hreg+1, atts); hreg += 2;
      }
    }
    else if (isattv(attv)) {	/* attv is already an attv */
      if (isnil(atts)) {	/* change it back to normal var */
	bind_ref((CPtr)dec_addr(attv), hreg);
	bld_free(hreg); hreg++;
      }
      else {			/* update the atts (another copy) */
	/*** Doesn't work for attv into and out of tables
|	     CPtr attv_attr = ((CPtr)dec_addr(attv))+1;
|	     push_pre_image_trail(attv_attr,atts);
|	     bld_copy(attv_attr,atts);
	***/
	bind_attv((CPtr)dec_addr(attv), hreg);
	bld_free(hreg);
	bld_copy(hreg+1, atts); hreg += 2;
      }
    }
    else xsb_abort("[PUT_ATTRIBUTES] Argument 1 is nonvar: %s",canonical_term(CTXTc attv, 0));
    break;
  }

  case GET_ATTRIBUTES: { /* R1: +Var; R2: -List */
    Cell attv = ptoc_tag(CTXTc 1);
    if (isref(attv)) {		/* a free var */
	return FALSE;
    }
    else if (isattv(attv)) {
      CPtr list;
      list = (CPtr)dec_addr(attv) + 1;
      ctop_tag(CTXTc 2, cell(list));
    }
    else return FALSE;
    //    else xsb_abort("[GET_ATTRIBUTES] Argument 1 is not an attributed variable: %s",
    //		   canonical_term(CTXTc attv, 0));
    break;
  }

  case DELETE_ATTRIBUTES: { /* R1: -Var */
    Cell attv = ptoc_tag(CTXTc 1);
    if (isattv(attv)) {
      bind_ref((CPtr)dec_addr(attv), hreg);
      bld_free(hreg); hreg++;
    }
    break;

  }

  /*
   * attv_unify/1 is an internal builtin for binding an attv to a value
   * (it could be another attv or a nonvar term).  The users can call
   * this builtin in verify_attributes/2 to bind an attributed var
   * without triggering attv interrupt.
   */
  case ATTV_UNIFY: { /* R1: +Var; R2: +Value */
    Cell attv = ptoc_tag(CTXTc 1);
    if (isattv(attv)) {
      bind_copy((CPtr)dec_addr(attv), ptoc_tag(CTXTc 2));
    } else {
      return FALSE;
    }
    break;
  }

  case SLEEPER_THREAD_OPERATION: {
#ifndef MULTI_THREAD
    Integer selection = ptoc_int(CTXTc 1);
    if (selection == START_SLEEPER_THREAD) {
      //printf("starting sleeper thread\n");
      startSleeperThread(CTXTc (int)ptoc_int(CTXTc 2));
    }
    else if (selection == CANCEL_SLEEPER_THREAD) {
      //printf("cancelling sleeper thread\n");
      cancelSleeperThread(CTXT);
    }
    else
      xsb_abort("sleeper thread operation called with bad operation number: %d\n",selection);
#else
    xsb_abort("timed_call/3 not implemented for multi-threaded engine.  Please use "
              "thread signalling");
#endif
    break;
}

  case MARK_TERM_CYCLIC: {
    mark_cyclic(CTXTc ptoc_tag(CTXTc 1));
    return TRUE;
  }

  case SET_SCOPE_MARKER: {
    if (set_scope_marker(CTXT)) return TRUE; else return FALSE;
    break;
  }
  case UNWIND_STACK: {
    if (flags[CTRACE_CALLS])  {
      if (ptcpreg)
	sprint_subgoal(CTXTc forest_log_buffer_1,0, (VariantSF)ptcpreg);
      else sprintf(forest_log_buffer_1->fl_buffer,"null");
      fprintf(fview_ptr,"err(%s,%d).\n",forest_log_buffer_1->fl_buffer,
	      ctrace_ctr++);
    }

    if (unwind_stack(CTXT)) return TRUE; else return FALSE;
    break;
  }
  case CLEAN_UP_BLOCK: {
    //    if (clean_up_block(CTXT)) return TRUE; else return FALSE;
    clean_up_block(CTXTc (int)ptoc_int(CTXTc 1));
    //    clean_up_block(CTXT);
    return TRUE;
    break;
  }

  case THREAD_REQUEST: {

    return xsb_thread_request(CTXT) ;
  }

  case MT_RANDOM_REQUEST: {
    return mt_random_request(CTXT) ;
  }

  case CRYPTO_HASH:
    /* Arg 1: int: type of hash - 1 (MD5) or 2 (SHA1)
       Arg 2: input string: - atom or file(filename)
       Arg 3: output: atom
    */
    {
      Integer type = ptoc_int(CTXTc 1);
      prolog_term InputTerm = reg_term(CTXTc 2);
      prolog_term Output = reg_term(CTXTc 3);
      // SHA1 hash has 40 characters; MD5 has less
      char *Result = (char *)mem_alloc(41,BUFF_SPACE);
      int retcode;

      switch (type) {
      case MD5: {
	retcode = md5_string(InputTerm,Result);
	break;
      }
      case SHA1: {
	retcode = sha1_string(InputTerm,Result);
	break;
      }
      default: {
	xsb_error("crypto_hash: unknown hash function type");
	return FALSE;
      }
      }
      return retcode && atom_unify(CTXTc makestring(string_find(Result,1)),Output);
    }

  default:
    xsb_abort("Builtin #%d is not implemented", number);
    break;

  } /* switch */

  return TRUE; /* catch for every break from switch */
}

/* Prolog Profiling (NOT thread-safe) */

ubi_btRoot TreeRoot;
ubi_btRootPtr RootPtr = NULL;
ubi_btNodePtr prof_table;
ubi_btNodePtr prof_table_free = NULL;

typedef struct psc_profile_count_struct {
  Psc psc;
  int prof_count;
} psc_profile_count;

/* could use a splay tree to store and quickly find these entries if
   this were to be too slow, or if we added returning to psc and so
   got more. */

static psc_profile_count *psc_profile_count_table = NULL;
static int psc_profile_count_max = 0;
static int psc_profile_count_num = 0;
#define initial_psc_profile_count_size 100

void add_to_profile_count_table(Psc apsc, int count) {
  int i;
  if (psc_profile_count_num >= psc_profile_count_max) {
    if (psc_profile_count_table == NULL) {
      psc_profile_count_max = initial_psc_profile_count_size;
      psc_profile_count_table = (psc_profile_count *)
	mem_alloc(psc_profile_count_max*sizeof(psc_profile_count),PROFILE_SPACE);
    } else {
      psc_profile_count_max = 2*psc_profile_count_max;
      psc_profile_count_table = (psc_profile_count *)
	mem_realloc(psc_profile_count_table,
		    (psc_profile_count_max/2)*sizeof(psc_profile_count),
		    psc_profile_count_max*sizeof(psc_profile_count),PROFILE_SPACE);
    }
  }
  for (i=0; i<psc_profile_count_num; i++)
    if (psc_profile_count_table[i].psc == apsc) {
      psc_profile_count_table[i].prof_count += count;
      return;
    }
  psc_profile_count_table[psc_profile_count_num].psc = apsc;
  psc_profile_count_table[psc_profile_count_num].prof_count = count;
  psc_profile_count_num++;
}

int compareItemNode(ubi_btItemPtr itemPtr, ubi_btNodePtr nodePtr) {
  if (*itemPtr < nodePtr->code_begin) return -1;
  else if (*itemPtr == nodePtr->code_begin) return 0;
  else return 1;
}

void log_prog_ctr(CTXTdeclc byte *lpcreg) {
  ubi_btNodePtr uNodePtr;

  uNodePtr = ubi_sptLocate(RootPtr, &lpcreg, ubi_trLE);
  if (uNodePtr == NULL) prof_unk_count += prof_int_count;
  else if (lpcreg <= uNodePtr->code_end) {
    uNodePtr->i_count += (int)prof_int_count;
  } else prof_unk_count += prof_int_count;
  prof_int_count = 0;
}

#define prof_tab_incr 10000

void add_prog_seg(Psc psc, byte *code_addr, size_t code_len) {
  ubi_btNodePtr newNode;

  if (!RootPtr) RootPtr = ubi_btInitTree(&TreeRoot,compareItemNode,ubi_trOVERWRITE);

  if (prof_table_free != NULL) {
    newNode = prof_table_free;
    prof_table_free = prof_table_free->Link[0];
  } else if (prof_table_count >= prof_table_length) {
    /* printf("Allocating another Profile Table segment\n"); */
    prof_table = (ubi_btNodePtr)mem_alloc(prof_tab_incr*sizeof(ubi_btNode),PROFILE_SPACE);
    prof_table_length = prof_tab_incr;
    newNode = prof_table;
    prof_table_count = 1;
  } else {
    newNode = prof_table+prof_table_count;
    prof_table_count++;
  }
  newNode->code_begin = code_addr;
  newNode->code_end = code_addr+code_len;
  newNode->code_psc = psc;
  newNode->i_count = 0;
  ubi_sptInsert(RootPtr,newNode,&(newNode->code_begin),NULL);
  //  printf("Adding segment for: %s/%d\n",get_name(newNode->code_psc),get_arity(newNode->code_psc));
  total_prog_segments++;
}

void remove_prog_seg(byte *code_addr) {
  ubi_btNodePtr oldNodePtr;

  oldNodePtr = ubi_sptFind(RootPtr,&code_addr);
  if (oldNodePtr == NULL) fprintf(stdout,"Error: code to delete not found: %p\n", code_addr);
  else {
    //    printf("Removing segment for: %s/%d\n",get_name(oldNodePtr->code_psc),get_arity(oldNodePtr->code_psc));
    if (oldNodePtr->i_count != 0)
      add_to_profile_count_table(oldNodePtr->code_psc, oldNodePtr->i_count);
    ubi_sptRemove(RootPtr,oldNodePtr);
    oldNodePtr->Link[0] = prof_table_free;
    prof_table_free = oldNodePtr;
    total_prog_segments--;
  }
}

Psc p3psc = NULL;

void retrieve_prof_table(CTXTdecl) { /* r2: +NodePtr, r3: -p(PSC,ModPSC,Cnt), r4: -NextNodePtr */
  ubi_btNodePtr uNodePtr;
  CPtr pscptrloc, modpscptrloc;
  Cell arg3;
  Integer i;
  int tmp;
  Psc apsc;

  i = ptoc_int(CTXTc 2);
  if (i == 0) { // fill table
    uNodePtr = ubi_btFirst(RootPtr->root);
    while (uNodePtr != NULL) {
      if (uNodePtr->i_count != 0) {
	add_to_profile_count_table(uNodePtr->code_psc,uNodePtr->i_count);
	uNodePtr->i_count = 0;
      }
      uNodePtr = ubi_btNext(uNodePtr);
    }
  }

  if (p3psc == NULL) p3psc = insert("p",3,(Psc)flags[CURRENT_MODULE],&tmp)->psc_ptr;
  arg3 = ptoc_tag(CTXTc 3);
  bind_cs((CPtr)arg3,hreg);
  new_heap_functor(hreg,p3psc);
  pscptrloc = hreg;
  modpscptrloc = hreg+1;
  hreg += 2;
  if (i < psc_profile_count_num) {
    follow(hreg++) = makeint(psc_profile_count_table[i].prof_count);
    apsc = psc_profile_count_table[i].psc;
    bld_oint(pscptrloc,(Integer)(apsc));
    bld_oint(modpscptrloc,(Integer)(get_mod_for_psc(apsc)));
    ctop_int(CTXTc 4,i+1);
  } else if (i == psc_profile_count_num) {
    follow(hreg++) = makeint(prof_gc_count);
    bld_int(pscptrloc,1);
    bld_int(modpscptrloc,0);
    ctop_int(CTXTc 4,i+1);
  } else {
    follow(hreg++) = makeint(prof_unk_count);
    bld_int(pscptrloc,0);
    bld_int(modpscptrloc,0);
    ctop_int(CTXTc 4,0);
    psc_profile_count_num = 0; // clear table
    prof_unk_count = 0;
    prof_gc_count = 0;
  }
}

/*----------------------------------------------------------------------*/
/* backtrace printer DSW */
Psc psc_from_code_addr(byte *code_addr) {
  ubi_btNodePtr uNodePtr;

  uNodePtr = ubi_sptLocate(RootPtr, &code_addr, ubi_trLE);
  if (uNodePtr == NULL) return NULL;
  if (code_addr <= uNodePtr->code_end) return uNodePtr->code_psc;
  return NULL;
}

#define MAX_BACKTRACE_LENGTH 50
int print_xsb_backtrace(CTXTdecl) {
  Psc tmp_psc, called_psc;
  byte *tmp_cpreg;
  byte instruction;
  CPtr tmp_ereg, tmp_breg;
  Integer backtrace_length = 0;
  if (xsb_profiling_enabled) {
    // print forward continuation
    fprintf(stdout,"Forward Continuation...\n");
    tmp_psc = psc_from_code_addr(pcreg);
    if (tmp_psc) fprintf(stdout,"... %s/%d  From %s\n",get_name(tmp_psc),get_arity(tmp_psc),
			 get_filename_for_psc(tmp_psc));
    else fprintf(stdout,"... unknown/?   pc=%p\n",pcreg);
    tmp_ereg = ereg;
    tmp_cpreg = cpreg;
    instruction = *(tmp_cpreg-2*sizeof(Cell));
    while (tmp_cpreg && (instruction == call || instruction == trymeorelse) &&
	   (backtrace_length++ < MAX_BACKTRACE_LENGTH)) {
      if (instruction == call) {
	called_psc = *((Psc *)tmp_cpreg - 1);
	if (called_psc != tmp_psc) {
	  fprintf(stdout,"..* %s/%d  From %s\n",get_name(called_psc),get_arity(called_psc),
		  get_filename_for_psc(called_psc));
	}
      }
      tmp_psc = psc_from_code_addr(tmp_cpreg);
      if (tmp_psc) fprintf(stdout,"... %s/%d  From %s\n",get_name(tmp_psc),get_arity(tmp_psc),
			   get_filename_for_psc(tmp_psc));
      else fprintf(stdout,"... unknown/?   pc=%p\n",tmp_cpreg);
      tmp_cpreg = *((byte **)tmp_ereg-1);
      tmp_ereg = *(CPtr *)tmp_ereg;
      instruction = *(tmp_cpreg-2*sizeof(Cell));
    }

    // print backward continuation
    fprintf(stdout,"Backward Continuation...\n");
    tmp_breg = breg;
    while (tmp_breg && tmp_breg != cp_prevbreg(tmp_breg)) {
      tmp_psc = psc_from_code_addr(cp_pcreg(tmp_breg));
      if (tmp_psc) fprintf(stdout,"... %s/%d  From %s\n",
			   get_name(tmp_psc),get_arity(tmp_psc),
			   get_filename_for_psc(tmp_psc));
      else fprintf(stdout,"... unknown/?   i=%x, pc=%p\n",*cp_pcreg(tmp_breg),cp_pcreg(tmp_breg));
      tmp_breg = cp_prevbreg(tmp_breg);
    }
  } else {
    fprintf(stdout,"Partial Forward Continuation...\n");
    if ((pb)top_of_localstk < (pb)top_of_heap+256*ZOOM_FACTOR) {
      fprintf(stdout,"  Local Stack clobbered, no backtrace available (h:%p,e:%p)\n",hreg,ereg);
      return TRUE;
    }
    tmp_ereg = ereg;
    tmp_cpreg = cpreg;
    if (tmp_cpreg) instruction = *(tmp_cpreg-2*sizeof(Cell));
    else instruction = (unsigned char)fail_inst;
    while (tmp_cpreg && (instruction == call || instruction == trymeorelse) &&
	   (backtrace_length++ < MAX_BACKTRACE_LENGTH)) {
      if (instruction == call) {
	called_psc = *((Psc *)tmp_cpreg - 1);
	fprintf(stdout,"... %s/%d  From %s\n",get_name(called_psc),get_arity(called_psc),
		get_filename_for_psc(called_psc));
      }
      if (!tmp_ereg) {
	fprintf(stdout,"... error in backtrace \n");
	break;
      }
      tmp_cpreg = *((byte **)tmp_ereg-1);
      tmp_ereg = *(CPtr *)tmp_ereg;
      if (tmp_cpreg) instruction = *(tmp_cpreg-2*sizeof(Cell));
    }
  }
  return TRUE;
}

#define MAX_BACKTRACE_LEN 25
prolog_term build_xsb_backtrace(CTXTdecl) {
  Psc tmp_psc, called_psc;
  byte *tmp_cpreg;
  byte instruction;
  CPtr tmp_ereg, tmp_breg, forward, backward, threg;
  prolog_term backtrace;
  int backtrace_cnt = 0;

  if (heap_local_overflow(MAX_BACKTRACE_LEN*2*sizeof(Cell))
      || !pflags[BACKTRACE]) {
    return makenil;
  }

  backtrace = makelist(hreg);
  forward = hreg;
  backward = hreg+1;
  hreg += 2;
  if (xsb_profiling_enabled) {
    tmp_psc = psc_from_code_addr(pcreg);
    follow(forward) = makelist(hreg);
    threg = hreg;
    forward = hreg+1;
    hreg += 2;
    bld_oint(threg,tmp_psc);
    tmp_ereg = ereg;
    tmp_cpreg = cpreg;
    instruction = *(tmp_cpreg-2*sizeof(Cell));
    while (backtrace_cnt++ < MAX_BACKTRACE_LEN
	   && tmp_cpreg
	   && (instruction == call || instruction == trymeorelse)
	   && (pb)top_of_localstk > (pb)top_of_heap + 96) {
      if (instruction == call) {
	called_psc = *((Psc *)tmp_cpreg - 1);
	if (called_psc != tmp_psc) {
	  follow(forward) = makelist(hreg);
	  threg = hreg;
	  forward = hreg+1;
	  hreg += 2;
	  bld_oint(threg,called_psc);
	}
      }
      tmp_psc = psc_from_code_addr(tmp_cpreg);
      follow(forward) = makelist(hreg);
      threg = hreg;
      forward = hreg+1;
      hreg += 2;
      bld_oint(threg,tmp_psc);
      tmp_cpreg = *((byte **)tmp_ereg-1);
      tmp_ereg = *(CPtr *)tmp_ereg;
      instruction = *(tmp_cpreg-2*sizeof(Cell));
    }
    follow(forward) = makenil;

    tmp_breg = breg;
    while (tmp_breg && tmp_breg != cp_prevbreg(tmp_breg)
	   && (pb)top_of_localstk > (pb)top_of_heap + 48) {
      tmp_psc = psc_from_code_addr(cp_pcreg(tmp_breg));
      follow(backward) = makelist(hreg);
      threg = hreg;
      backward = hreg+1;
      hreg += 2;
      bld_oint(threg,tmp_psc);
      tmp_breg = cp_prevbreg(tmp_breg);
    }
    follow(backward) = makenil;

  } else {
    tmp_ereg = ereg;
    tmp_cpreg = cpreg;
    instruction = *(tmp_cpreg-2*sizeof(Cell));
    while (tmp_cpreg && (instruction == call || instruction == trymeorelse)
	   && (pb)top_of_localstk > (pb)top_of_heap + 48) {
      if (instruction == call) {
	called_psc = *((Psc *)tmp_cpreg - 1);
	follow(forward) = makelist(hreg);
	threg = hreg;
	forward = hreg+1;
	hreg += 2;
	bld_oint(threg,called_psc);
      }
      tmp_cpreg = *((byte **)tmp_ereg-1);
      tmp_ereg = *(CPtr *)tmp_ereg;
      instruction = *(tmp_cpreg-2*sizeof(Cell));
    }
    follow(forward) = makenil;
    follow(backward) = makenil;
  }
  return backtrace;
}


/*------------------------- end of builtin.c -----------------------------*/


