/* File:      io_builtins_xsb.c
** Author(s): David S. Warren, kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1998
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
** $Id: io_builtins_xsb.c,v 1.104 2013-01-09 20:15:34 dwarren Exp $
** 
*/

#include "xsb_config.h"
#include "xsb_debug.h"

#ifdef WIN_NT
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>

#include "setjmp_xsb.h"
#include "wind2unix.h"
#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "cinterf.h"
#include "memory_xsb.h"
#include "psc_xsb.h"
#include "heap_xsb.h"
#include "register.h"
#include "flags_xsb.h"
#include "inst_xsb.h"
#include "loader_xsb.h" /* for ZOOM_FACTOR */
#include "subp.h"
#include "tries.h"
#include "choice.h"
#include "tab_structs.h"
#include "io_builtins_xsb.h"
#include "binding.h"
#include "deref.h"
#include "findall.h"
#include "heap_defs_xsb.h"
#include "cell_xsb_i.h"

//extern FILE *logfile;

stream_record open_files[MAX_OPEN_FILES]; /* open file table, protected by MUTEX IO */

// static FILE *fptr;			/* working variable */
    
struct fmt_spec {
  char type; 	     	     	 /* i(nteger), f(loat), s(tring) */
  /* size: in case of a write op a the * format specifier (e.g., %*.*d), size
     is 1, 2, or 3, depending the number of *'s in that particular format.
     If there are no *'s, then this format correspnds to exactly one argument,
     so size=1. If there is one "*", then this format corresponds to 2
     arguments, etc.
     This tells how manu arguments to expect.
     In case of a read operation, size can be 0, since here '*' means
     assignment suppression. */
  char size;
  char *fmt;
};

struct next_fmt_state {
  size_t _current_substr_start;
  VarString *_workspace;
  char _saved_char;
};
#define current_substr_start (fmt_state->_current_substr_start)
#define workspace (*(fmt_state->_workspace))
#define saved_char (fmt_state->_saved_char)

void next_format_substr(CTXTdeclc char*, struct next_fmt_state*, struct fmt_spec *, int, int);
char *p_charlist_to_c_string(CTXTdeclc prolog_term, VarString*, char*, char*);

/* type is a char: 's', 'i', 'f' */
#define TYPE_ERROR_CHK(ch_type, Label) \
        if (current_fmt_spec->type != ch_type) { \
	    xsb_abort("[%s] in argument value %d. Expected %c, got %c.", Label, i,current_fmt_spec->type,ch_type); \
        }

/* these should incorporate the charset of the target file */
#define PRINT_ARG(arg) switch (current_fmt_spec->size) { \
        case 1: fprintf(fptr, current_fmt_spec->fmt, arg); \
	        break; \
	case 2: fprintf(fptr, current_fmt_spec->fmt, width, arg); \
	        break; \
	case 3: fprintf(fptr, current_fmt_spec->fmt, width, precision, arg); \
	        break; \
	}
#define CHECK_ARITY(i, Arity, Label) if (i > Arity) { \
	      xsb_abort("[%s] Not enough arguments for given format", Label); \
	}

#ifdef HAVE_SNPRINTF
/* like PRINT_ARG, but uses snprintf */
#define SPRINT_ARG(arg) \
        switch (current_fmt_spec->size) {	\
	/* the 1st snprintf in each case finds the #bytes to be formatted */ \
        case 1: bytes_formatted=snprintf(NULL,0,current_fmt_spec->fmt,arg); \
	  XSB_StrEnsureSize(&OutString,OutString.length+(int)bytes_formatted+1); \
	        bytes_formatted=snprintf(OutString.string+OutString.length, \
					 bytes_formatted+1, \
					 current_fmt_spec->fmt, arg); \
	        break; \
	case 2: bytes_formatted=snprintf(NULL,0,current_fmt_spec->fmt,width,arg); \
	  XSB_StrEnsureSize(&OutString,OutString.length+(int)bytes_formatted+1); \
	        bytes_formatted=snprintf(OutString.string+OutString.length,\
					 bytes_formatted+1, \
					 current_fmt_spec->fmt, width, arg); \
	        break; \
	case 3: bytes_formatted=snprintf(NULL,0,current_fmt_spec->fmt,width,precision,arg); \
	  XSB_StrEnsureSize(&OutString,OutString.length+(int)bytes_formatted+1); \
                bytes_formatted=snprintf(OutString.string+OutString.length, \
					 bytes_formatted+1, \
					 current_fmt_spec->fmt, \
					 width, precision, arg); \
	        break; \
	}			      \
        OutString.length += (int)bytes_formatted;		\
        XSB_StrNullTerminate(&OutString);

#elif defined(WIN_NT)
#define SPRINT_ARG(arg) \
        switch (current_fmt_spec->size) {	\
        case 1: bytes_formatted=_scprintf(current_fmt_spec->fmt,arg); \
	  XSB_StrEnsureSize(&OutString,OutString.length+(int)bytes_formatted+1); \
	        _snprintf_s(OutString.string+OutString.length, \
			    bytes_formatted+1,		       \
			    _TRUNCATE,				\
			    current_fmt_spec->fmt, arg);	\
	        break; \
	case 2: bytes_formatted=_scprintf(current_fmt_spec->fmt,width,arg); \
	  XSB_StrEnsureSize(&OutString,OutString.length+(int)bytes_formatted+1); \
	        _snprintf_s(OutString.string+OutString.length,\
			    bytes_formatted+1,		      \
			    _TRUNCATE,					\
			    current_fmt_spec->fmt, width, arg);		\
	        break; \
	case 3: bytes_formatted=_scprintf(current_fmt_spec->fmt,width,precision,arg); \
	  XSB_StrEnsureSize(&OutString,OutString.length+(int)bytes_formatted+1); \
                _snprintf_s(OutString.string+OutString.length, \
			    bytes_formatted+1,		       \
			    _TRUNCATE,				 \
			    current_fmt_spec->fmt,		 \
			    width, precision, arg);		 \
	        break; \
	}			      \
        OutString.length += (int)bytes_formatted;		\
        XSB_StrNullTerminate(&OutString);

#else
/* like PRINT_ARG, but uses sprintf -- used with old compilers that don't have
   snprintf.  This is error-prone: don't use broken compilers!!!
   In some systems sprintf returns it's first argument, so have to use
   strlen to count bytes formatted, for portability.
   */
#define SPRINT_ARG(arg) \
    	XSB_StrEnsureSize(&OutString, OutString.length+SAFE_OUT_SIZE); \
     	switch (current_fmt_spec->size) { \
        case 1: sprintf(OutString.string+OutString.length, \
			current_fmt_spec->fmt, arg); \
		bytes_formatted = strlen(OutString.string+OutString.length); \
	        break; \
	case 2: sprintf(OutString.string+OutString.length, \
			current_fmt_spec->fmt, width, arg); \
		bytes_formatted = strlen(OutString.string+OutString.length); \
	        break; \
	case 3: sprintf(OutString.string+OutString.length, \
			current_fmt_spec->fmt, \
			width, precision, arg); \
		bytes_formatted = strlen(OutString.string+OutString.length); \
	        break; \
	} \
        if (bytes_formatted >= SAFE_OUT_SIZE)				      \
	    xsb_memory_error("memory","Buffer overflow in fmt_write_string"); \
        OutString.length += (int)bytes_formatted;	\
        XSB_StrNullTerminate(&OutString);
#endif


xsbBool fmt_write(CTXTdecl);
xsbBool fmt_write_string(CTXTdecl);
xsbBool fmt_read(CTXTdecl);


#include "ptoc_tag_xsb_i.h"
    	    

xsbBool formatted_io (CTXTdecl)
{
  switch (ptoc_int(CTXTc 1)) {
  case FMT_WRITE: return fmt_write(CTXT);
  case FMT_WRITE_STRING: return fmt_write_string(CTXT);
  case FMT_READ: return fmt_read(CTXT);
  default:
    xsb_abort("[FORMATTED_IO] Invalid operation number: %d", ptoc_int(CTXTc 1));
  }
  return TRUE; /* just to get rid of compiler warning */
}

/*----------------------------------------------------------------------
    like fprintf
     C invocation: formatted_io(FMT_WRITE, IOport, Format, ValTerm)
     Prolog invocation: fmt_write(+IOport, +Format, +ValTerm)
       IOport: XSB I/O port
       Format: format as atom or string;
       ValTerm: term whose args are vars to receive values returned.
----------------------------------------------------------------------*/
/* The following definitions are to use the threadsafe char buffers,
   but use more reasonable names.  These buffers will just grow
   (unless they are explicitly shrunk.)  They provide global buffers
   without having to malloc them every time.  The names are undef'ed
   at the end. */
#define FmtBuf (*tsgSBuff1)
#define StrArgBuf (*tsgSBuff2)

xsbBool fmt_write(CTXTdecl)
{
  FILE *fptr;			/* working variable */
  struct next_fmt_state fmt_state;
  char *Fmt=NULL, *str_arg;
  char aux_msg[50];
  prolog_term ValTerm, Arg, Fmt_term;
  int i, Arity=0;
  Integer int_arg;     	     	     	      /* holder for int args         */
  double float_arg;    	     	     	      /* holder for float args       */
  struct fmt_spec *current_fmt_spec = (struct fmt_spec *)mem_alloc(sizeof(struct fmt_spec),LEAK_SPACE);
  int width=0, precision=0;    	     	      /* these are used in conjunction
						 with the *.* format         */
  XSB_StrSet(&FmtBuf,"");
  XSB_StrSet(&StrArgBuf,"");
  fmt_state._workspace = tsgLBuff2;
  XSB_StrSet(fmt_state._workspace,"");

  SET_FILEPTR(fptr, ptoc_int(CTXTc 2));
  Fmt_term = reg_term(CTXTc 3);
  if (is_list(Fmt_term))
    Fmt = p_charlist_to_c_string(CTXTc Fmt_term,&FmtBuf,"FMT_WRITE","format string");
  else if (isstring(Fmt_term))
    Fmt = string_val(Fmt_term);
  else
    xsb_type_error(CTXTc "character string or atom",Fmt_term,"fmt_write/[2,3]",2);
  ValTerm = reg_term(CTXTc 4);
  if (isconstr(ValTerm) && !isboxed(ValTerm))
    Arity = get_arity(get_str_psc(ValTerm));
  else if (isref(ValTerm))
    /* Var in the argument position means, no arguments */
    Arity = 0;
  else {
    /* assume single argument; convert ValTerm into arg(val) */
    prolog_term TmpValTerm=p2p_new(CTXT);

    c2p_functor(CTXTc "arg", 1, TmpValTerm);
    if (isstring(ValTerm))
      c2p_string(CTXTc string_val(ValTerm), p2p_arg(TmpValTerm,1));
    else if (isointeger(ValTerm))
      c2p_int(CTXTc oint_val(ValTerm), p2p_arg(TmpValTerm,1));
     else if (isofloat(ValTerm))
      c2p_float(CTXTc ofloat_val(ValTerm), p2p_arg(TmpValTerm,1));
    else
      xsb_abort("Usage: fmt_write([+IOport,] +FmtStr, +args(A1,A2,...))");
    ValTerm = TmpValTerm;
    Arity = 1;
  }

  next_format_substr(CTXTc Fmt, &fmt_state,current_fmt_spec,
		     1,   /* initialize    	      	     */
		     0);  /* write    	      	     */
  xsb_segfault_message =
    "++FMT_WRITE: Argument type doesn't match format specifier\n";
  signal(SIGSEGV, &xsb_segfault_catcher);
  
  i=0;
  while (i <= Arity) {
    /* last format substring (and has no conversion spec) */
    if (current_fmt_spec->type == '.') {
      PRINT_ARG("");
      if (i < Arity)
	xsb_warn(CTXTc "[FMT_WRITE] More arguments than format specifiers");
      goto EXIT_WRITE;
    }

    i++; /* increment after checking the last format segment */

    if (current_fmt_spec->size >  1) {
      Arg = p2p_arg(ValTerm,i++);
      width = (int) int_val(Arg);
    } 
    CHECK_ARITY(i, Arity, "FMT_WRITE");

    if (current_fmt_spec->size == 3) {
      Arg = p2p_arg(ValTerm,i++);
      precision = (int) int_val(Arg);
    }
    CHECK_ARITY(i, Arity, "FMT_WRITE");

    Arg = p2p_arg(ValTerm,i);

    if (current_fmt_spec->type == '!') { /* ignore field */
    } else if (current_fmt_spec->type == 'S') {
      /* Any type: print as a string */
      XSB_StrSet(&StrArgBuf,"");
      print_pterm(CTXTc Arg, TRUE, &StrArgBuf);
      PRINT_ARG(StrArgBuf.string);
    } else if (isstring(Arg) && !isnil(Arg)) {
      TYPE_ERROR_CHK('s', "FMT_WRITE");
      str_arg = string_val(Arg);
      PRINT_ARG(str_arg);
    } else if (islist(Arg) || isnil(Arg)) {
      TYPE_ERROR_CHK('s', "FMT_WRITE");
      sprintf(aux_msg, "argument %d", i);
      str_arg = p_charlist_to_c_string(CTXTc Arg, &StrArgBuf, "FMT_WRITE", aux_msg);
      PRINT_ARG(str_arg);
    } else if (isointeger(Arg)) {
      TYPE_ERROR_CHK('i', "FMT_WRITE");
      int_arg = oint_val(Arg);
      PRINT_ARG(int_arg);
    } else if (isofloat(Arg)) {
      TYPE_ERROR_CHK('f', "FMT_WRITE")
      float_arg = ofloat_val(Arg);
      PRINT_ARG(float_arg);
    } else {
      sprintf(aux_msg,"format_string %%%c",current_fmt_spec->type);
      xsb_domain_error(CTXTc aux_msg,Arg,"fmt_write/2",2);
    }
    next_format_substr(CTXTc Fmt, &fmt_state,current_fmt_spec,
		       0 /* don't initialize */,
		       0 /* write */ );
  }

  /* print the remainder of the format string, if it exists */
  if (current_fmt_spec->type == '.')
      PRINT_ARG("");

 EXIT_WRITE:
  xsb_segfault_message = xsb_default_segfault_msg;
  signal(SIGSEGV, xsb_default_segfault_handler);
  
  mem_dealloc(current_fmt_spec,sizeof(struct fmt_spec),LEAK_SPACE);
  return TRUE;
}

#undef FmtBuf
#undef StrArgBuf

/*----------------------------------------------------------------------
   like sprintf:
    C invocation: formatted_io(FMT_WRITE_STRING, String, Format, ValTerm)
    Prolog invocation: fmt_write_string(-String, +Format, +ValTerm)
      String: string buffer
      Format: format as atom or string;
      ValTerm: Term whose args are vars to receive values returned.
----------------------------------------------------------------------*/

#define MAX_SPRINTF_STRING_SIZE MAX_IO_BUFSIZE

/* If no snprintf, we fill only half of OutString, to be on the safe side;
no, need more space if no snprintf... */
#ifdef HAVE_SNPRINTF
#define SAFE_OUT_SIZE MAX_SPRINTF_STRING_SIZE
int (sprintf)(char *s, const char *format, /* args */ ...);
#else
#define SAFE_OUT_SIZE MAX_SPRINTF_STRING_SIZE
#endif

#define OutString (*tsgLBuff1)
#define FmtBuf (*tsgSBuff1)
#define StrArgBuf (*tsgSBuff2)

xsbBool fmt_write_string(CTXTdecl)
{
  char *Fmt=NULL, *str_arg;
  struct next_fmt_state fmt_state;
  char aux_msg[50];
  prolog_term ValTerm, Arg, Fmt_term;
  int i, Arity;
  Integer int_arg;     	     	     	    /* holder for int args     	    */
  double float_arg;    	     	     	    /* holder for float args   	    */
  struct fmt_spec *current_fmt_spec = (struct fmt_spec *)mem_alloc(sizeof(struct fmt_spec),LEAK_SPACE);
  int width=0, precision=0;      	    /* these are used in conjunction
					       with the *.* format     	    */
  size_t bytes_formatted=0;       	       	    /* the number of bytes formatted as
					       returned by sprintf/snprintf */
  XSB_StrSet(&OutString,"");
  XSB_StrSet(&FmtBuf,"");
  XSB_StrSet(&StrArgBuf,"");
  fmt_state._workspace = tsgLBuff2;
  XSB_StrSet(fmt_state._workspace,"");

  if (isnonvar(reg_term(CTXTc 2)))
    xsb_abort("[FMT_WRITE_STRING] Arg 1 must be an unbound variable");
  
  Fmt_term = reg_term(CTXTc 3);
  if (islist(Fmt_term))
    Fmt = p_charlist_to_c_string(CTXTc Fmt_term, &FmtBuf,
				 "FMT_WRITE_STRING", "format string");
  else if (isstring(Fmt_term))
    Fmt = string_val(Fmt_term);
  else
    xsb_abort("[FMT_WRITE_STRING] Format must be an atom or a character string");

  ValTerm = reg_term(CTXTc 4);
  if (isconstr(ValTerm) && ! isboxed(ValTerm))
    Arity = get_arity(get_str_psc(ValTerm));
  else if (isref(ValTerm))
    /* Var in the argument position means, no arguments */
    Arity = 0;
  else {
    /* assume single argument; convert ValTerm into arg(val) */
    prolog_term TmpValTerm=p2p_new(CTXT);

    c2p_functor(CTXTc "arg", 1, TmpValTerm);
    if (isstring(ValTerm))
      c2p_string(CTXTc string_val(ValTerm), p2p_arg(TmpValTerm,1));
    else if (isointeger(ValTerm))
      c2p_int(CTXTc oint_val(ValTerm), p2p_arg(TmpValTerm,1));
    else if (isofloat(ValTerm))
      c2p_float(CTXTc ofloat_val(ValTerm), p2p_arg(TmpValTerm,1));
    else
      xsb_abort("Usage: fmt_write_string(-OutStr, +FmtStr, +args(A1,A2,...))");

    ValTerm = TmpValTerm;
    Arity = 1;
  }

  next_format_substr(CTXTc Fmt, &fmt_state,current_fmt_spec,
		     1,  /* initialize     	      	     */
		     0); /* write     	      	     */
  xsb_segfault_message =
    "++FMT_WRITE_STRING: Argument type doesn't match format specifier\n";
  signal(SIGSEGV, &xsb_segfault_catcher);
  
  i=0;
  while (i <= Arity) {
    /* last string (and has no conversion spec) */
    if (current_fmt_spec->type == '.') {
      SPRINT_ARG("");
      if (i < Arity)
	xsb_warn(CTXTc "[FMT_WRITE_STRING] More arguments than format specifiers");
      goto EXIT_WRITE_STRING;
    }

    i++; /* increment after checking the last format segment */

    if (current_fmt_spec->size >  1) {
      Arg = p2p_arg(ValTerm,i++);
      width = (int) int_val(Arg);
    } 
    CHECK_ARITY(i, Arity, "FMT_WRITE_STRING");

    if (current_fmt_spec->size == 3) {
      Arg = p2p_arg(ValTerm,i++);
      precision = (int) int_val(Arg);
    }
    CHECK_ARITY(i, Arity, "FMT_WRITE_STRING");

    Arg = p2p_arg(ValTerm,i);

    if (current_fmt_spec->type == '!') { /* ignore field */
    } else if (current_fmt_spec->type == 'S') {
      /* Any type: print as a string */
      XSB_StrSet(&StrArgBuf,"");
      print_pterm(CTXTc Arg, TRUE, &StrArgBuf);
      SPRINT_ARG(StrArgBuf.string);
    } else if (isstring(Arg)) {
      TYPE_ERROR_CHK('s', "FMT_WRITE_STRING");
      str_arg = string_val(Arg);
      SPRINT_ARG(str_arg);
    } else if (islist(Arg)) {
      TYPE_ERROR_CHK('s', "FMT_WRITE_STRING");
      sprintf(aux_msg, "argument %d", i);
      str_arg = p_charlist_to_c_string(CTXTc Arg, &StrArgBuf,
				       "FMT_WRITE_STRING", aux_msg);
      SPRINT_ARG(str_arg);
    } else if (isointeger(Arg)) {
      TYPE_ERROR_CHK('i', "FMT_WRITE_STRING");
      int_arg = oint_val(Arg);
      SPRINT_ARG(int_arg);
    } else if (isofloat(Arg)) {
      TYPE_ERROR_CHK('f', "FMT_WRITE_STRING");
      float_arg = ofloat_val(Arg);
      SPRINT_ARG(float_arg);
    } else {
      xsb_abort("[FMT_WRITE_STRING] Argument %d has illegal type", i);
    }
    next_format_substr(CTXTc Fmt, &fmt_state,current_fmt_spec,
		       0 /* don't initialize */,
		       0 /* write */ );
  }

  /* print the remainder of the format string, if it exists */
  if (current_fmt_spec->type == '.') {
      SPRINT_ARG("");
  }

 EXIT_WRITE_STRING:
  xsb_segfault_message = xsb_default_segfault_msg;
  signal(SIGSEGV, xsb_default_segfault_handler);

  mem_dealloc(current_fmt_spec,sizeof(struct fmt_spec),LEAK_SPACE);
  /* fmt_write_string is used in places where interning of the string is needed
     (such as constructing library search paths)
     Therefore, must use string_find(..., 1). */
  ctop_string(CTXTc 2, OutString.string);
  
  return TRUE;
}
#undef OutString
#undef FmtBuf
#undef StrArgBuf


/*----------------------------------------------------------------------
   like fscanf
     C invocation: formatted_io(FMT_READ, IOport, Format, ArgTerm, Status)
     Prolog invocation: fmt_read(+IOport, +Format, -ArgTerm, -Status)
      IOport: XSB I/O port
      Format: format as atom or string;
      ArgTerm: Term whose args are vars to receive values returned.
      Status: 0 OK, -1 eof 
----------------------------------------------------------------------*/
#define FmtBuf (*tsgSBuff1)
#define StrArgBuf (*tsgSBuff2)
#define aux_fmt (*tsgLBuff1)     	      /* auxiliary fmt holder 	     */

xsbBool fmt_read(CTXTdecl)
{
  FILE *fptr;			/* working variable */
  char *Fmt=NULL;
  struct next_fmt_state fmt_state;
  prolog_term AnsTerm, Arg, Fmt_term;
  int i ;
  Integer int_arg;     	     	     	      /* holder for int args         */
  float float_arg;    	     	     	      /* holder for float args       */
  struct fmt_spec *current_fmt_spec = (struct fmt_spec *)mem_alloc(sizeof(struct fmt_spec),LEAK_SPACE);
  int Arity=0;
  int number_of_successes=0, curr_assignment=0;
  int cont; /* continuation indicator */
  Integer chars_accumulator=0, curr_chars_consumed=0;
  int dummy; /* to squash return arg warnings */

  XSB_StrSet(&FmtBuf,"");
  XSB_StrSet(&StrArgBuf,"");
  XSB_StrSet(&aux_fmt,"");
  fmt_state._workspace = tsgLBuff2;
  XSB_StrSet(fmt_state._workspace,"");

  SET_FILEPTR(fptr, ptoc_int(CTXTc 2));
  Fmt_term = reg_term(CTXTc 3);
  if (islist(Fmt_term))
    Fmt = p_charlist_to_c_string(CTXTc Fmt_term,&FmtBuf,"FMT_READ","format string");
  else if (isstring(Fmt_term))
    Fmt = string_val(Fmt_term);
  else
    xsb_abort("[FMT_READ] Format must be an atom or a character string");

  AnsTerm = reg_term(CTXTc 4);
  if (isconstr(AnsTerm))
    Arity = get_arity(get_str_psc(AnsTerm));
  else if (isref(AnsTerm)) {
    /* assume that only one input val is reuired */
    prolog_term TmpAnsTerm=p2p_new(CTXT), TmpArg;

    Arity = 1;
    c2p_functor(CTXTc "arg", 1, TmpAnsTerm);
    /* The following is a bit tricky: Suppose AnsTerm was X.
       We unify AnsTerm (which is avriable) with
       TmpArg, the argument of the new term TmpAnsTerm.
       Then the variable AnsTerm is reset to TmpAnsTerm so that the rest of the
       code would think that AnsTerm was arg(X).
       Eventually, X will get bound to the result */
    TmpArg = p2p_arg(TmpAnsTerm,1);
    p2p_unify(CTXTc TmpArg, AnsTerm);
    AnsTerm = TmpAnsTerm;
  } else
    xsb_abort("Usage: fmt_read([IOport,] FmtStr, args(A1,A2,...), Feedback)");

  /* status variable */
  if (isnonvar(reg_term(CTXTc 5)))
    xsb_abort("[FMT_READ] Arg 4 must be an unbound variable");

  next_format_substr(CTXTc Fmt, &fmt_state,current_fmt_spec,
		     1,   /* initialize    	      	     */
		     1);  /* read    	      	     */
  XSB_StrSet(&aux_fmt, current_fmt_spec->fmt);
  XSB_StrAppend(&aux_fmt,"%n");

  for (i = 1; (i <= Arity); i++) {
    Arg = p2p_arg(AnsTerm,i);
    cont = 0;
    curr_chars_consumed=0;

    /* if there was an assignment suppression spec, '*' */
    if (current_fmt_spec->size == 0)
      current_fmt_spec->type = '-';

    switch (current_fmt_spec->type) {
    case '-':
      /* we had an assignment suppression character: just count how 
	 many chars were scanned, don't skip to the next scan variable */
      dummy = fscanf(fptr, aux_fmt.string, &curr_chars_consumed);
      curr_assignment = 0;
      i--; /* don't skip scan variable */
      cont = 1; /* don't leave the loop */
      break;
    case '.': /* last format substring (and has no conversion spec) */
      curr_assignment = fscanf(fptr,  current_fmt_spec->fmt, "");
      if (isref(Arg))
	xsb_warn(CTXTc "[FMT_READ] More arguments than format specifiers");
      goto EXIT_READ;
    case 's':
      XSB_StrEnsureSize(&StrArgBuf, MAX_IO_BUFSIZE);
      curr_assignment = fscanf(fptr, aux_fmt.string,
			       StrArgBuf.string,
			       &curr_chars_consumed);
      /* if no match, leave prolog variable uninstantiated;
	 if it is a prolog constant, then return FALSE (no unification) */
      if (curr_assignment <= 0) {
	if (isref(Arg)) break;
	else goto EXIT_READ_FALSE;
      }
      if (isref(Arg))
	c2p_string(CTXTc StrArgBuf.string,Arg);
      else if (strcmp(StrArgBuf.string, string_val(Arg)))
	goto EXIT_READ_FALSE;
      break;
    case 'n':
      int_arg = -1;
      curr_assignment = fscanf(fptr, current_fmt_spec->fmt, &int_arg);
      if (int_arg < 0) break; /* scanf failed before reaching %n */
      cont = 1; /* don't leave the loop */
      curr_chars_consumed = int_arg;
      int_arg += chars_accumulator;
      if (isref(Arg))
	c2p_int(CTXTc int_arg,Arg);
      else xsb_abort("[FMT_READ] Argument %i must be a variable", i);
      break;
    case 'i':
      curr_assignment = fscanf(fptr, aux_fmt.string,
			       &int_arg, &curr_chars_consumed);
      /* if no match, leave prolog variable uninstantiated;
	 if it is a prolog constant, then return FALSE (no unification) */
      if (curr_assignment <= 0) {
	if (isref(Arg)) break;
	else goto EXIT_READ_FALSE;
      }
      if (isref(Arg))
	c2p_int(CTXTc int_arg,Arg);
      else if (int_arg != (Integer)oint_val(Arg)) goto EXIT_READ_FALSE;
      break;
    case 'f':
      curr_assignment = fscanf(fptr, aux_fmt.string,
			       &float_arg, &curr_chars_consumed);
      /* floats never unify with anything */
      if (!isref(Arg)) goto EXIT_READ_FALSE;
      /* if no match, leave prolog variable uninstantiated */
      if (curr_assignment <= 0) break;
      c2p_float(CTXTc float_arg, Arg);
      break;
    default:
      xsb_abort("[FMT_READ] Unsupported format specifier for argument %d", i);
    }

    chars_accumulator +=curr_chars_consumed;

    /* format %n shouldn't cause us to leave the loop */
    if (curr_assignment > 0 || cont)
      number_of_successes =
	(curr_assignment ? number_of_successes+1 : number_of_successes);
    else
      break;

    next_format_substr(CTXTc Fmt, &fmt_state,current_fmt_spec,
		       0 /* don't initialize */,
		       1 /* read */ );
    XSB_StrSet(&aux_fmt, current_fmt_spec->fmt);
    XSB_StrAppend(&aux_fmt,"%n");
  }

  /* if there are format specifiers beyond what corresponds to the last
     variable then we make use of %* (suppression) and of non-format
     strings. The leftover format specifiers are ignored. */
  /* last format substr without conversion spec */
  if (current_fmt_spec->type == '.')
    curr_assignment = fscanf(fptr, current_fmt_spec->fmt, "");
  /* last format substr with assignment suppression (spec size=0) */
  if (current_fmt_spec->size == 0)
    dummy = fscanf(fptr, aux_fmt.string, &curr_chars_consumed);

  /* check for end of file */
  if ((number_of_successes == 0) && (curr_assignment < 0))
    number_of_successes = -1;

 EXIT_READ:
  mem_dealloc(current_fmt_spec,sizeof(struct fmt_spec),LEAK_SPACE);
  ctop_int(CTXTc 5, number_of_successes);
  return TRUE;

 EXIT_READ_FALSE:
  mem_dealloc(current_fmt_spec,sizeof(struct fmt_spec),LEAK_SPACE);

  SQUASH_LINUX_COMPILER_WARN(dummy) ; 
  return FALSE;
}
#undef FmtBuf
#undef StrArgBuf
#undef aux_fmt


/**********
In scanning a canonical term, we maintain a functor stack, and an
operand stack. The functor stack has the name of the functor and a
pointer to its first operand on the operand stack. The operand stack
is just a stack of operands. They are Prolog terms. (How to handle
variables remains to be seen.)
***/

/* prevpsc is kept as an optimization to save a lookup in prolog.  If
   it has changed, it is reset to 0.  So it doesn't need to be
   protected for multithreading, since it is self-correcting. */
static Psc prevpsc = 0;


/* ----- handle read_cannonical errors: print msg and scan to end -----	*/
/***
reallocate op stack.
add clear findall stack at toploop
***/

CPtr init_term_buffer(CTXTdeclc int *findall_chunk_index) {
  *findall_chunk_index = findall_init_c(CTXT);
  current_findall = findall_solutions + *findall_chunk_index;
  return current_findall->top_of_chunk ;
}

#define ensure_term_space(ptr,size) \
  if ((ptr+size) > (current_findall->current_chunk + FINDALL_CHUNCK_SIZE -1)) {\
	if (!get_more_chunk(CTXT)) xsb_abort("Cannot allocate space for term buffer") ;\
	ptr = current_findall->top_of_chunk ;\
  }

#define free_term_buffer() findall_free(CTXTc findall_chunk_index)

static int read_can_error(CTXTdeclc int stream, int prevchar, Cell prologvar, int findall_chunk_index)
{
  char *ptr;

  xsb_error("READ_CAN_ERROR: illegal format. Next tokens:");
  while ((token->type != TK_EOC) && (token->type != TK_EOF)) {
    ptr = token->value;
    switch (token->type) {
    case TK_PUNC	: fprintf(stderr,"%c ", *ptr); break;
    case TK_VARFUNC	: fprintf(stderr,"%s ", ptr); break;
    case TK_VAR		: fprintf(stderr,"%s ", ptr); break;
    case TK_FUNC	: fprintf(stderr,"%s ", ptr); break;
    case TK_INT		: fprintf(stderr,"%d ", *(int *)ptr); break;
    case TK_ATOM	: fprintf(stderr,"%s ", ptr); break;
    case TK_VVAR	: fprintf(stderr,"%s ", ptr); break;
    case TK_VVARFUNC	: fprintf(stderr,"%s ", ptr); break;
    case TK_REAL	: fprintf(stderr,"%f ", *(double *)ptr); break;
    case TK_STR		: fprintf(stderr,"%s ", ptr); break;
    case TK_LIST	: fprintf(stderr,"%s ", ptr); break;
    case TK_HPUNC	: fprintf(stderr,"%c ", *ptr); break;
    case TK_INTFUNC	: fprintf(stderr,"%d ", *(int *)ptr); break;
    case TK_REALFUNC	: fprintf(stderr,"%f ", *(double *)ptr); break;
    }
    token = GetToken(CTXTc stream,prevchar);
    prevchar = token-> nextch;
  }
  if (token->type == TK_EOC)
    fprintf(stderr,".\n");
  else
    fprintf(stderr,"\n");
  free_term_buffer();
  unify(CTXTc prologvar,makestring(string_find("read_canonical_error",1)));
  return 0;
}


/* Read a canonical term from XSB I/O port in r1 and put answer in variable in
   r2; r3 set to 0 if ground fact (non zero-ary), to 1 if variable or :-.
   Fail on EOF */

#define INIT_STK_SIZE 32
#define MAX_INIT_STK_SIZE 1000

#define FUNFUN 0
#define FUNLIST 1
#define FUNDTLIST 2
#define FUNCOMMALIST 3

#ifndef MULTI_THREAD
int opstk_size = 0;
int funstk_size = 0;
struct funstktype *funstk;
struct opstktype *opstk;
struct vartype rc_vars[MAXVAR];
#endif

#define setvar(loc,op1) \
    if (rc_vars[opstk[op1].op].varval) \
       cell(loc) = rc_vars[opstk[op1].op].varval; \
    else { \
	     cell(loc) = (Cell) loc; \
	     rc_vars[opstk[op1].op].varval = (Cell) loc; \
	 }

#define expand_opstk {\
    opstk_size = opstk_size+opstk_size;\
    opstk = (struct opstktype *)mem_realloc(opstk,(opstk_size/2)*sizeof(struct opstktype),\
					    opstk_size*sizeof(struct opstktype),READ_CAN_SPACE);\
    if (!opstk) xsb_abort("[READ_CANONICAL] Out of space for operand stack");\
    /*printf("RC opstk expanded to %d\n",opstk_size);*/ \
  }
#define expand_funstk {\
    funstk_size = funstk_size+funstk_size;\
    funstk = (struct funstktype *)mem_realloc(funstk,(funstk_size/2)*sizeof(struct funstktype),\
					  funstk_size*sizeof(struct funstktype),READ_CAN_SPACE);\
    if (!funstk) xsb_abort("[READ CANONICAL] Out of space for function stack");\
    /*printf("RC funstk expanded to %d\n",funstk_size);*/ \
  }

#define ctop_addr(regnum, val)    ctop_int(CTXTc regnum, (prolog_int)val)

int read_canonical(CTXTdecl)
{
  int tempfp;
  
  tempfp = (int)ptoc_int(CTXTc 1);
  if (tempfp == -1000) {
    prevpsc = 0;
    return TRUE;
  }

  ctop_addr(3,read_canonical_term(CTXTc tempfp, 1));
  return TRUE;
}

Cell read_canonical_return_var(CTXTdeclc int code) {
  if (code == 1) { /* from read_canonical */
    return (Cell)ptoc_tag(CTXTc 2);
  } else if (code == 2) { /* from odbc */
    Cell op1, op;
    op = ptoc_tag(CTXTc 4);
    op1 = get_str_arg(op,1);
    XSB_Deref(op1);
    return op1;
  } else return (Cell)NULL;
}

/* copied from emuloop.c and added param */
#ifndef FAST_FLOATS
static inline void bld_boxedfloat_here(CTXTdeclc CPtr *h, CPtr addr, Float value)
{
    Float tempFloat = value;
    new_heap_functor((*h),box_psc);
    bld_int(*h,((ID_BOXED_FLOAT << BOX_ID_OFFSET ) | FLOAT_HIGH_16_BITS(tempFloat) ));
    (*h)++;
    bld_int(*h,FLOAT_MIDDLE_24_BITS(tempFloat)); (*h)++;
    bld_int(*h,FLOAT_LOW_24_BITS(tempFloat)); (*h)++;
    cell(addr) = makecs((*h)-4);
}
#else
inline void bld_boxedfloat_here(CTXTdeclc CPtr *h, CPtr addr, Float value) {
static   bld_float(addr,value);
}
#endif

static inline void bld_boxedint_here(CTXTdeclc CPtr *h, CPtr addr, Integer value) {
  Integer temp_value = value;
  new_heap_functor((*h),box_psc);
  bld_int(*h,((ID_BOXED_INT << BOX_ID_OFFSET ) | 0 ));
  bld_int((*h)+1,INT_LOW_24_BITS(temp_value));
  bld_int((*h)+2,((temp_value) & LOW_24_BITS_MASK)); 
  *(h) = (*h)+3;
  cell(addr) = makecs((*h)-4);
}

/* read canonical term, and return prev psc pointer, if valid */
Integer read_canonical_term(CTXTdeclc int stream, int return_location_code)
{
  int findall_chunk_index;
  int funtop = 0;
  int optop = 0;
  int cvarbot = MAXVAR-1;
  int prevchar, arity, i, size, charset;
  CPtr h;
  int j, op1;
  Integer retpscptr;
  Pair sym;
  Float float_temp;
  Integer integer_temp;
  Psc headpsc, termpsc;
  char *cvar;
  int postopreq = FALSE, varfound = FALSE;
  prolog_term term;
  Cell prologvar = read_canonical_return_var(CTXTc return_location_code);
  
  if (stream >= 0) charset = charset(stream);
  else charset = UTF_8;

  if (opstk_size == 0) {
    opstk = 
      (struct opstktype *)mem_alloc(INIT_STK_SIZE*sizeof(struct opstktype),READ_CAN_SPACE);
    opstk_size = INIT_STK_SIZE;
    funstk = 
      (struct funstktype *)mem_alloc(INIT_STK_SIZE*sizeof(struct funstktype),READ_CAN_SPACE);
    funstk_size = INIT_STK_SIZE;
  }

  /* get findall buffer to read term into */
  h = init_term_buffer(CTXTc &findall_chunk_index);
  size = 0;

  prevchar = 10;
  while (1) {
    token = GetToken(CTXTc stream,prevchar);
    //print_token(token->type,token->value);
	prevchar = token->nextch;
	if (postopreq) {  /* must be an operand follower: , or ) or | or ] */
	    if (token->type == TK_PUNC) {
		if (*token->value == ')') {
		  CPtr this_term;
		  funtop--;
		  if (funstk[funtop].funtyp == FUNFUN) {
		    arity = optop - funstk[funtop].funop;
		    ensure_term_space(h,arity+1);
		    this_term = h;
		    op1 = funstk[funtop].funop;
		    if ((arity == 2) && !(strcmp(funstk[funtop].fun,"."))) {
		      if (opstk[op1].typ == TK_VAR) { setvar(h,op1) }
		      else cell(h) = opstk[op1].op;
		      h++;
		      if (opstk[op1+1].typ == TK_VAR) { setvar(h,op1+1) }
		      else cell(h) = opstk[op1+1].op;
		      h++;
		      opstk[op1].op = makelist(this_term);
		      opstk[op1].typ = TK_FUNC;
		      size += 2;
		    } else {
		      size += arity+1;
		      sym = (Pair)insert(funstk[funtop].fun,(char)arity,
					 (Psc)flags[CURRENT_MODULE],&i);
		      new_heap_functor(h, sym->psc_ptr);
		      for (j=op1; j<optop; h++,j++) {
			if (opstk[j].typ == TK_VAR) { setvar(h,j) }
			else cell(h) = opstk[j].op;
		      }
		      opstk[op1].op = makecs(this_term);
		      opstk[op1].typ = TK_FUNC;
		    }
		    optop = op1+1;
		  } else if (funstk[funtop].funtyp == FUNCOMMALIST) {
		    op1 = funstk[funtop].funop;
		    if ((op1+1) == optop) { /* no comma-list, just parens */
		    } else {	/* handle comma list... */
		      CPtr prev_tail;
		      ensure_term_space(h,3);
		      this_term = h;

		      new_heap_functor(h, comma_psc);

		      if (opstk[op1].typ == TK_VAR) { setvar(h,op1) }
		      else cell(h) = opstk[op1].op;
		      h++;
		      prev_tail = h;
		      h++;
		      size += 3;
		      for (j=op1+1; j<optop-1; j++) {
			ensure_term_space(h,3);
			cell(prev_tail) = makecs(h);
			new_heap_functor(h, comma_psc);
			if (opstk[j].typ == TK_VAR) { setvar(h,j) }
			else cell(h) = opstk[j].op;
			h++;
			prev_tail = h;
			h++;
			size += 3;
		      }
		      j = optop-1;
		      if (opstk[j].typ == TK_VAR) { setvar(prev_tail,j) }
		      else cell(prev_tail) = opstk[j].op;
		      opstk[op1].op = makecs(this_term);
		      opstk[op1].typ = TK_FUNC;
		      optop = op1+1;
		    }
		  } else {
  		    return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index); /* ')' ends a list? */
		  }
		} else if (*token->value == ']') {	/* end of list */
		  CPtr this_term, prev_tail;
		  funtop--;
		  if (funstk[funtop].funtyp == FUNFUN || funstk[funtop].funtyp == FUNCOMMALIST)
		    return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
		  ensure_term_space(h,2);
		  this_term = h;
		  op1 = funstk[funtop].funop;

		  if (opstk[op1].typ == TK_VAR) { setvar(h,op1) }
		  else cell(h) = opstk[op1].op;
		  h++;
		  size += 2;
		  if ((op1+1) == optop) {
			cell(h) = makenil;
			h++;
		  } else {
			prev_tail = h;
			h++;
			for (j=op1+1; j<optop-1; j++) {
			  ensure_term_space(h,2);
			  cell(prev_tail) = makelist(h);
			  if (opstk[j].typ == TK_VAR) { setvar(h,j) }
			  else cell(h) = opstk[j].op;
			  h++;
			  prev_tail = h;
			  h++;
			  size += 2;
			}
			j = optop-1;
			if (funstk[funtop].funtyp == FUNLIST) {
			  ensure_term_space(h,2);
			  cell(prev_tail) = makelist(h);
			  if (opstk[j].typ == TK_VAR) { setvar(h,j) }
			  else cell(h) = opstk[j].op;
			  h++;
			  prev_tail = h;
			  h++;
			  cell(prev_tail) = makenil;
			  size += 2;
			} else {
			  if (opstk[j].typ == TK_VAR) { setvar(prev_tail,j) }
			  else cell(prev_tail) = opstk[j].op;
			}
		  }
		  opstk[op1].op = makelist(this_term);
		  opstk[op1].typ = TK_FUNC;
		  optop = op1+1;
		} else if (*token->value == ',') {
		  postopreq = FALSE;
		} else if (*token->value == '|') {
		  postopreq = FALSE;
		  if (funstk[funtop-1].funtyp != FUNLIST)
		    return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
		  funstk[funtop-1].funtyp = FUNDTLIST;
		} else return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
	    } else {  /* check for neg numbers and backpatch if so */
	        if (opstk[optop-1].typ == TK_ATOM && 
				!strcmp("-",string_val(opstk[optop-1].op))) {
		  if (token->type == TK_INT) {
		    integer_temp = -(*(Integer *)(token->value));
		    if (int_overflow(integer_temp)) {
		      ensure_term_space(h,4);
		      size +=4; 
		      opstk[optop-1].typ = TK_FUNC;
		      bld_boxedint_here(CTXTc &h, &opstk[optop-1].op, integer_temp);
		    } else {
		      opstk[optop-1].typ = TK_INT;
		      opstk[optop-1].op = makeint(integer_temp);
		    }
		  } else if (token->type == TK_REAL) {
			float_temp = (Float) *(double *)(token->value);
#ifdef FAST_FLOATS
			opstk[optop-1].typ = TK_REAL;
			opstk[optop-1].op = makefloat((float)-float_temp); // lose precision FIX!!
#else
			ensure_term_space(h,4); // size of boxfloat
			size +=4; 
			opstk[optop-1].typ = TK_FUNC;
			bld_boxedfloat_here(CTXTc &h, &opstk[optop-1].op, -float_temp);
#endif
		  } else return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
		} else return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
	    }
	} else {  /* must be an operand */
      switch (token->type) {
      case TK_PUNC:
		if (*token->value == '[') {
		  if(token->nextch == ']') {
		        if (optop >= opstk_size) expand_opstk;
			token = GetToken(CTXTc stream,prevchar);
			//print_token(token->type,token->value);
			prevchar = token->nextch;
			opstk[optop].typ = TK_ATOM;
			opstk[optop].op = makenil;
			optop++;
			postopreq = TRUE;
		  } else {	/* beginning of a list */
		        if (funtop >= funstk_size) expand_funstk;
			funstk[funtop].funop = optop;
			funstk[funtop].funtyp = FUNLIST; /* assume regular list */
			funtop++;
		  }
		  break;
		} else if (*token->value == '(') { /* beginning of comma list */
		  if (funtop >= funstk_size) expand_funstk;
		  funstk[funtop].funop = optop;
		  funstk[funtop].funtyp = FUNCOMMALIST;
		  funtop++;
		  break;
		}
	  /* fall through to let a punctuation mark be a functor symbol */
      case TK_FUNC:
	        if (funtop >= funstk_size) expand_funstk;
		funstk[funtop].fun = (char *)string_find(token->value,1);
		funstk[funtop].funop = optop;
		funstk[funtop].funtyp = FUNFUN;	/* functor */
		funtop++;

		if (token->nextch != '(')
		  return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
		token = GetToken(CTXTc stream,prevchar); /* eat open par */
		if (token->nextch == ')') postopreq = TRUE; /* handle f() form */
		//print_token(token->type,token->value);
		prevchar = token->nextch;
		break;
      case TK_VVAR:
	        if ((token->value)[1] == 0) { /* anonymous var */
		  if (cvarbot < 0)
		    xsb_abort("[READ_CANONICAL] too many variables in term");
		  i = cvarbot;
		  rc_vars[cvarbot].varid = (Cell) "_";
		  rc_vars[cvarbot].varval = 0;
		  cvarbot--;
		  if (optop >= opstk_size) expand_opstk;
		  opstk[optop].typ = TK_VAR;
		  opstk[optop].op = (prolog_term) i;
		  optop++;
		  postopreq = TRUE;
		  break;
		}  /* else fall through and treat as regular var*/
      case TK_VAR:
		varfound = TRUE;
		cvar = (char *)string_find(token->value,1);
		i = MAXVAR-1;
		while (i>cvarbot) {
		  if (cvar == (char *)rc_vars[i].varid) break;
		  i--;
		}
		if (i == cvarbot) {
		  if (cvarbot < 0)
		    xsb_abort("[READ_CANONICAL] too many variables in term");
		  rc_vars[cvarbot].varid = (Cell) cvar;
		  rc_vars[cvarbot].varval = 0;
		  cvarbot--;
		}
		if (optop >= opstk_size) expand_opstk;
		opstk[optop].typ = TK_VAR;
		opstk[optop].op = (prolog_term) i;
		optop++;
		postopreq = TRUE;
		break;
      case TK_REAL:
	        if (optop >= opstk_size) expand_opstk;
		float_temp = (Float)* (double *)(token->value);
#ifdef FAST_FLOATS		
		opstk[optop].typ = TK_REAL;
		opstk[optop].op = makefloat((float)float_temp); // lose precision  FIX!!
		size += 1;
#else
		ensure_term_space(h,4); // size of boxfloat
		opstk[optop].typ = TK_FUNC;
		bld_boxedfloat_here(CTXTc &h, &opstk[optop].op, float_temp);
		size += 4; 
#endif
		optop++;
		postopreq = TRUE;
		break;
      case TK_INT:
	        if (optop >= opstk_size) expand_opstk;
		integer_temp = *(Integer *)(token->value);
		if (int_overflow(integer_temp)) {
		  ensure_term_space(h,4);
		  opstk[optop].typ = TK_FUNC;
		  bld_boxedint_here(CTXTc &h, &opstk[optop].op, integer_temp);
		} else {
		  opstk[optop].typ = TK_INT;
		  opstk[optop].op = makeint(integer_temp);
		}
		optop++;
		postopreq = TRUE;
		break;
      case TK_ATOM:
	        if (optop >= opstk_size) expand_opstk;
		opstk[optop].typ = TK_ATOM;
		opstk[optop].op = makestring((char *)string_find(token->value,1));
		optop++;
		postopreq = TRUE;
		break;
      case TK_LIST:  /* "-list */
	if (optop >= opstk_size) expand_opstk;
	if ((token->value)[0] == 0) {
	  opstk[optop].typ = TK_ATOM;
	  opstk[optop].op = makenil;
	  optop++;
	  postopreq = TRUE;
	  break;
	} else {
	  int code; /* utf-8 code nfz */
	  CPtr this_term, prev_tail;
	  byte *charptr = (byte *)token->value;
	  ensure_term_space(h,2);
	  this_term = h;
	  code = char_to_codepoint(charset,&charptr);  /* nfz */
	  cell(h) = makeint(code);                  /* nfz */
	  //	  cell(h) = makeint((int)*charptr); charptr++;
	  h++;
	  prev_tail = h;
	  h++;
	  size += 2;
	  while (*charptr != 0) {
	    ensure_term_space(h,2);
	    cell(prev_tail) = makelist(h);
	    code = char_to_codepoint(charset,&charptr); /* nfz */
	    cell(h) = makeint(code);                 /* nfz */
	    //	    cell(h) = makeint((int)*charptr); charptr++;
	    h++;
	    prev_tail = h;
	    h++;
	    size += 2;
	  }
	  cell(prev_tail) = makenil;
	  opstk[optop].op = makelist(this_term);
	  opstk[optop].typ = TK_FUNC;
	  optop++;
	  postopreq = TRUE;
	  break;
	}
      case TK_EOF:
	free_term_buffer();
	if (isnonvar(prologvar)) 
	  xsb_abort("[READ_CANONICAL] Argument must be a variable");
	unify(CTXTc prologvar,makestring(string_find("end_of_file",1)));
	return 0;
      default: return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
      }
	}
    if (token->type == TK_ATOM && !strcmp(token->value,"-")) {
    } else if (funtop == 0) {  /* term is finished */
      token = GetToken(CTXTc stream,prevchar);
      //print_token(token->type,token->value);
      prevchar = token->nextch; /* accept EOF as end_of_clause */
      if (token->type != TK_EOF && token->type != TK_EOC) {
	return read_can_error(CTXTc stream,prevchar,prologvar,findall_chunk_index);
      }
      if (opstk[0].typ != TK_VAR) {  /* if a variable, then a noop */
	if (isnonvar(prologvar)) 
	  xsb_abort("[READ_CANONICAL] Argument must be a variable");
	term = opstk[0].op;
	
	check_glstack_overflow(5, pcreg, (size+200)*sizeof(Cell)) ;
	/* get return location again, in case it moved, whole reasong for r_c_r_v */
	prologvar = read_canonical_return_var(CTXTc return_location_code); 
	gl_bot = (CPtr)glstack.low; gl_top = (CPtr)glstack.high;
	bind_ref((CPtr)prologvar,hreg);  /* build a new var to trail binding */
	new_heap_free(hreg);
	gl_bot = (CPtr)glstack.low; gl_top = (CPtr)glstack.high; /* so findall_copy* finds vars */
	findall_copy_to_heap(CTXTc term,(CPtr)prologvar,&hreg) ; /* this can't fail */
	free_term_buffer();

	XSB_Deref(prologvar);
	term = (prolog_term) prologvar;
	if (isointeger(term) || 
	    isofloat(term) || 
	    isstring(term) ||
	    varfound) {
	  retpscptr = 0;
	  prevpsc = 0;
	} else {
	  termpsc = get_str_psc(term);
	  if (termpsc == if_psc) {
	    if (!isconstr(p2p_arg(term,1))) headpsc = 0;
	    else headpsc = get_str_psc(p2p_arg(term,1));
	  } else {
	    headpsc = termpsc;
	  }
	  if (headpsc == prevpsc) {
	      retpscptr = (Integer)prevpsc;
	  } else {
	    prevpsc = headpsc;
	    retpscptr = 0;
	  }
	}
      } else {
	retpscptr = 0;
	prevpsc = 0;
      }

      if (opstk_size > MAX_INIT_STK_SIZE) {
	mem_dealloc(opstk,opstk_size*sizeof(struct opstktype),READ_CAN_SPACE); opstk = NULL;
	mem_dealloc(funstk,funstk_size*sizeof(struct funstktype),READ_CAN_SPACE); funstk = NULL;
	opstk_size = 0; funstk_size = 0;
      }
      return retpscptr;
    }
  }
}

/* Scan format string and return format substrings ending with a conversion
   spec. The return value is a ptr to a struct that has the type of conversion
   spec (i, f, s) and the format substring ('.' if the whole format string has
   been scanned).

   This function doesn't fully check the validity of the conversion
   specifier. In case of a mistake, the result is unpredictable.
   We insist that a single % is a beginning of a format specifier.

   FORMAT: format string, INITIALIZE: 1-process new fmt string; 0 - continue
   with old fmt string. READ: 1 if this is called for read op; 0 for write.  */
void next_format_substr(CTXTdeclc char *format, struct next_fmt_state *fmt_state,
				    struct fmt_spec *result,
				    int initialize, int read_op)
{
  size_t pos; int keep_going;
  char *ptr;
  //  static struct fmt_spec result;
  char *exclude, *expect; /* characters to exclude or expect */

  if (initialize) {
    current_substr_start = 0;
    XSB_StrSet(&workspace,format);
  } else {
    /* restore char that was replaced with \0 */
    workspace.string[current_substr_start] = saved_char;
  }

  pos = current_substr_start;
  result->type = '?';
  result->size = 1;

  /* done scanning format string */
  if (current_substr_start >= (size_t) workspace.length) {
    result->type = '.'; /* last substring (and has no conversion spec) */
    result->fmt  = "";
    return;
  }

  /* find format specification: % not followed by % */
  do {
    /* last substring (and has no conversion spec) */
    if ((ptr=strchr(workspace.string+pos, '%')) == NULL) {
      result->type = '.';  /* last substring with no type specifier */
      result->fmt = workspace.string+current_substr_start;
      current_substr_start = workspace.length;
      return;
    }

    pos = (ptr - workspace.string) + 1;
    if (workspace.string[pos] == '%')
      pos++;
    else break;
  } while (1);

  /* this doesn't do full parsing; it assumes anything that starts at % and
     ends at a valid conversion character is a conversion specifier. */ 
  keep_going = TRUE;
  expect = exclude = "";
  while ((pos < (size_t)workspace.length) && keep_going) {
    if (strchr(exclude, workspace.string[pos]) != NULL) {
      xsb_abort("[FMT_READ/WRITE] Illegal format specifier `%c' in: %s",
		workspace.string[pos],
		workspace.string+current_substr_start);
    }
    if (strlen(expect) && strchr(expect, workspace.string[pos]) == NULL) {
      xsb_abort("[FMT_READ/WRITE] Illegal format specifier `%c' in: %s",
		workspace.string[pos],
		workspace.string+current_substr_start);
    }

    expect = exclude = "";

    switch (workspace.string[pos++]) {
    case '1': /* flags, precision, etc. */
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      exclude = "+- #[]";
      break;
    case '.':
      exclude = "+- #[]";
      expect = "0123456789*";
      break;
    case '0':
    case '+':
    case '-':
      exclude = "+-[]";
      break;
    case 'h':
    case 'l':
      exclude = "+- #[]hL";  // modified to allow %lld for win64
      expect = "diouxXnl";
      break;
    case 'L':
      expect = "eEfgG";
      exclude = "+- #[]hlL";
      break;
    case ' ':
    case '#':
      exclude = "+- #[]hlL";
      break;
    case 'c':
      if (read_op)
	result->type = 's';
      else
	result->type = 'i';
      keep_going = FALSE;
      break;
    case 'd':
    case 'i':
    case 'u':
    case 'o':
    case 'x':
    case 'X':
      keep_going = FALSE;
      result->type = 'i'; /* integer or character */
      break;
    case 'e':
    case 'E':
    case 'f':
    case 'g':
    case 'G':
      keep_going = FALSE;
      result->type = 'f'; /* float */
      break;
    case 's':
      keep_going = FALSE;
      result->type = 's'; /* string */
      break;
    case 'S':
      keep_going = FALSE;
      result->type = 'S'; /* string */
      workspace.string[pos-1] = 's';
      break;
    case 'p':
      xsb_abort("[FMT_READ/WRITE] Format specifier %%p not supported: %s",
		workspace.string+current_substr_start);
    case 'n':
      if (read_op) {
	result->type = 'n'; /* %n is like integer, but in fmt_read we treat it
			      specially */
	keep_going = FALSE;
	break;
      }
      xsb_abort("[FMT_WRITE] Format specifier %%n not supported: %s",
		workspace.string+current_substr_start);
    case '[':
      /* scanf feature: [...] */
      if (!read_op) {
	xsb_abort("[FMT_WRITE] Format specifier [ is invalid for output: %s",
		  workspace.string+current_substr_start);
      }
      while ((pos < (size_t)workspace.length) && (workspace.string[pos++] != ']'));
      if (workspace.string[pos-1] != ']') {
	xsb_abort("[FMT_READ] Format specifier [ has no matching ] in: %s",
		  workspace.string+current_substr_start);
      }
      result->type = 's';
      keep_going = FALSE;
      break;

    case '*':
      if (read_op) {
	result->size = 0;
	break;
      }
      if (strncmp(workspace.string+pos, ".*", 2) == 0) {
	pos = pos+2;
	expect = "feEgEscdiuoxX";
	result->size = 3;
      } else if (workspace.string[pos] == '.') {
	pos++;
	expect = "0123456789";
	result->size = 2;
      } else {
	result->size = 2;
	expect = "feEgEscdiuoxX";
      }
      break;

    case '!':
      printf("set !\n");
      result->type = '!';
      keep_going = FALSE;
      break;

    default:
      xsb_abort("[FMT_READ/WRITE] Character `%c' in illegal format context: %s",
		workspace.string[pos-1],
		workspace.string+current_substr_start);
    }
  }

  saved_char = workspace.string[pos];
  workspace.string[pos] = '\0';
  result->fmt = workspace.string+current_substr_start;
  current_substr_start = pos;
  return;
}

/* TLS: changed the name of this function.  Here we are just checking
   whether a file pointer is present or not, rather than a file and
   I/O mode, as below. */

int xsb_intern_fileptr(CTXTdeclc FILE *fptr, char *context,char* name,char *strmode, int charset)
{
  int i;
  char mode = '\0';

  /*printf("xsb_intern_fileptr %x %s %s %s %d\n",fptr,context,name,strmode,charset);*/

  if (!fptr) return -1;

  if (!strcmp(strmode,"rb") || !strcmp(strmode,"r"))
    mode = 'r';
  else if (!strcmp(strmode,"wb")  || !strcmp(strmode,"w"))
    mode = 'w';
  else if (!strcmp(strmode,"ab")  || !strcmp(strmode,"a"))
    mode = 'a';
  else if (!strcmp(strmode,"rb+") || !strcmp(strmode,"r+") || !strcmp(strmode,"r+b"))
    mode = 's';  /* (i.e. r+) */
  else if (!strcmp(strmode,"wb+") || !strcmp(strmode,"w+") || !strcmp(strmode,"w+b"))
    mode = 'x'; /* (i.e. r+) */
  else if (!strcmp(strmode,"ab+") || !strcmp(strmode,"a+") || !strcmp(strmode,"a+b"))
    mode = 'b'; /* (i.e. a+) */

  for (i=MIN_USR_OPEN_FILE; i < MAX_OPEN_FILES && open_files[i].file_ptr != NULL; i++);
  if (i == MAX_OPEN_FILES) {
    xsb_warn(CTXTc "[%s] Too many open files", context);
    return -1;
  } else {
    open_files[i].file_ptr = fptr;
    open_files[i].file_name = string_find(name,1);
    open_files[i].io_mode = mode;
    open_files[i].charset = charset;
    return i;
  }
}

/* static int open_files_high_water = MIN_USR_OPEN_FILE+1;*/

/* Takes a file address, and mode and returns an ioport (in third
   argument) along with success/error.  The nonsense in the beginning
   is to handle possible Posix I/O modes, of which there is
   redundancy. */

int xsb_intern_file(CTXTdeclc char *context,char *addr, int *ioport,char *strmode,int opennew)
{
  FILE *fptr;			/* working variable */
  int i, first_null, stream_found; 
  char mode = '\0';

  /*  printf("xif Context %s Addr %s strmode %s\n",context,addr,strmode);*/
  
  if (!strcmp(strmode,"rb") || !strcmp(strmode,"r"))
    mode = 'r';
  else if (!strcmp(strmode,"wb")  || !strcmp(strmode,"w"))
    mode = 'w';
  else if (!strcmp(strmode,"ab")  || !strcmp(strmode,"a"))
    mode = 'a';
  else if (!strcmp(strmode,"rb+") || !strcmp(strmode,"r+") || !strcmp(strmode,"r+b"))
    mode = 's';  /* (i.e. r+) */
  else if (!strcmp(strmode,"wb+") || !strcmp(strmode,"w+") || !strcmp(strmode,"w+b"))
    mode = 'x'; /* (i.e. r+) */
  else if (!strcmp(strmode,"ab+") || !strcmp(strmode,"a+") || !strcmp(strmode,"a+b"))
    mode = 'b'; /* (i.e. a+) */

  for (i=MIN_USR_OPEN_FILE, stream_found = -1, first_null = -1; 
       i < MAX_OPEN_FILES; 
       i++) {
    if (open_files[i].file_ptr != NULL) {
      if (!opennew && open_files[i].file_name != NULL &&
	  !strcmp(addr,open_files[i].file_name) && 
	  open_files[i].io_mode == mode) {
	stream_found = i;
	break;
      } } else if (first_null < 0) {first_null = i; if (opennew) break;}
  }

  /*
  printf("stream_found %d file_ptr %x file_name %s first_null %d\n",
	 stream_found,open_files[stream_found].file_ptr,
	 open_files[stream_found].file_name,first_null);
  */

  if (stream_found < 0 && first_null < 0) {
  for (i=MIN_USR_OPEN_FILE; 
       i < MAX_OPEN_FILES ;
       i++) printf("File Ptr %p Name %s\n",open_files[i].file_ptr, open_files[i].file_name);

    xsb_warn(CTXTc "[%s] Too many open files", context);
    *ioport = 0;
    return -1;
  }
  else if (stream_found >= 0) { /* File already interned */
    fptr = open_files[i].file_ptr;
    *ioport = stream_found;
    return 0;
  } 
  else { /* try to intern new file */
    struct stat stat_buff;
    fptr = fopen(addr, strmode);
    //    fprintf(logfile,"opening: %s (%s)\n",addr,strmode);
    if (!fptr) {*ioport = 0; return -1;}
    if (flags[LOG_ALL_FILES_USED]) {
      char current_dir[MAX_CMD_LEN];
      char *dummy; /* to squash warnings */
      dummy = getcwd(current_dir, MAX_CMD_LEN-1);
      SQUASH_LINUX_COMPILER_WARN(dummy) ; 
      xsb_log("%s: %s\n",current_dir,addr);
    }
    if (!stat(addr, &stat_buff) && !S_ISDIR(stat_buff.st_mode)) {
	/* file exists and isn't a dir */
      open_files[first_null].file_ptr = fptr;
      open_files[first_null].file_name = string_find(addr,1);
      open_files[first_null].io_mode = mode;
      open_files[first_null].charset = CURRENT_CHARSET;
      *ioport = first_null;
      return 0;
    }  else {
	xsb_warn(CTXTc "FILE_OPEN: File %s is a directory, cannot open!", addr);
	fclose(fptr);
	return -1;
    }
  }
}

void mark_open_filenames() {
  int i;

  for (i=MIN_USR_OPEN_FILE; i < MAX_OPEN_FILES; i++) {
    if (open_files[i].file_ptr != NULL) {
      mark_string(open_files[i].file_name,"Filename");
    }
  }
}


/*----------------------- write_quotedname/2 ---------------------------*/

xsbBool quotes_are_needed(char *string)
{
  int nextchar;
  int ctr, flag;

  if (*string == '\0') return TRUE;
  if (!strcmp(string,"[]")) return FALSE;
  if (string[0] == '/' && string[1] == '*') return TRUE;
  ctr = 0;
  nextchar = (int) string[0];
  flag = 0;
  if (nextchar >= 97 && nextchar <= 122) {    /* 0'a=97, 0'z=122  */
    while (nextchar != '\0' && !flag) {
      if (nextchar < 48 
	  || (nextchar > 57 && nextchar < 65)
	  || ((nextchar > 90 && nextchar < 97) && nextchar != 95)
	  || (nextchar > 122))
	flag = 1;
      ctr++;
      nextchar = (int) string[ctr];
    }
    if (!flag) return FALSE;
  }

  if (string[1] == '\0') {
    if ((int) string[0] == 33)
      return FALSE;
    if ((int) string[0] == 46) return TRUE;
  }

  nextchar = (int) string[0];
  ctr = 0; 
  while (nextchar != '\0' && !flag) {
    switch(nextchar) {
    case 35: case 36: case 38: case 42: case 43: case 45: case 46:
    case 47: case 58: case 60: case 61: case 62: case 63: case 64: 
    case 92: case 94: case 96: case 126:
      nextchar++;
      break;
    default: 
      flag = 1;
    }
    ctr++;
    nextchar = (int) string[ctr];
  }
  return flag;
}

xsbBool string_contains_quotes(char *string) {
  if (strstr(string,"\'") == NULL && strstr(string,"\\") == NULL) return FALSE;
  return TRUE;
}

// TLS: changed return val to int so I can use it w. sprintf.
// int double_quotes(char *string, char *new_string)
// {
//   int ctr = 0, nctr = 0;
// 
//   while (string[ctr] != '\0') {
//     if (string[ctr] == '\'') {
//       new_string[nctr] = '\'';
//       nctr++;
//     } else if (string[ctr] == '\\') {
//       char ch = string[ctr+1];
//       if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || 
// 	  ch == 'r' || ch == 's' || ch == 't' || ch == 'v' || ch == 'x' || 
// 	  ch == '0' || ch == '1' || ch == '2' || ch == '3' || 
// 	  ch == '4' || ch == '5' || ch == '6' || ch == '7' || 
// 	  ch == '\\' || ch == '"' || ch == '`') {
//         new_string[nctr] = '\\';
//         nctr++;
//       }
//     }
//     new_string[nctr] = string[ctr];
//     nctr++; ctr++;
//   }
//   new_string[nctr] = '\0';
//   return nctr;
// }

// always double '\'
int double_quotes(char *string, char *new_string)
{
  int ctr = 0, nctr = 0;

  while (string[ctr] != '\0') {
    if (string[ctr] == '\'' || string[ctr] == '\\') {
      new_string[nctr] = string[ctr];
      nctr++;
    } 
    new_string[nctr] = string[ctr];
    nctr++; ctr++;
  }
  new_string[nctr] = '\0';
  return nctr;
}

void write_quotedname(FILE *file, int charset, char *string)
{
  if (*string == '\0') 
    //    fprintf(file,"''");
    write_string_code(file,charset,(byte *)"''");
  else {
    if (!quotes_are_needed(string)) {
      write_string_code(file,charset,(byte *)string);
    }
    else {
      size_t neededlen = 2*strlen(string)+1;
      if (neededlen < 1000) {
	char lnew_string[1000];
      	double_quotes(string,lnew_string);
	write_string_code(file,charset,(byte *)"\'");
	write_string_code(file,charset,(byte *)lnew_string);
	write_string_code(file,charset,(byte *)"\'");
      } else {
	char* new_string;
	new_string  = (char *)mem_alloc(neededlen,LEAK_SPACE);
	double_quotes(string,new_string);
	write_string_code(file,charset,(byte *)"\'");
	write_string_code(file,charset,(byte *)new_string);
	write_string_code(file,charset,(byte *)"\'");
	mem_dealloc(new_string,neededlen,LEAK_SPACE);
      }
    }
  }
}

/********************** write_canonical ****************/
//static Psc dollar_var_psc = NULL;
#define wcan_string tsgLBuff1
#define wcan_atombuff tsgLBuff2
#define wcan_buff tsgSBuff1

char *cvt_float_to_str(CTXTdeclc Float floatval) {
  sprintf(wcan_buff->string,"%1.17g",floatval);
  wcan_buff->length = (int)strlen(wcan_buff->string);
  if (!strchr(wcan_buff->string,'.')) {
    char *eloc = strchr(wcan_buff->string,'e');
    if (!eloc) XSB_StrAppend(wcan_buff,".0");
    else {	
      char exp[5],fstr[30];
      //	  printf("massage float %s\n", wcan_buff->string);
      strcpy(exp,eloc);
      eloc[0] = 0;
      strcpy(fstr,wcan_buff->string);
      XSB_StrSet(wcan_buff,fstr);
      XSB_StrAppend(wcan_buff,".0");
      XSB_StrAppend(wcan_buff,exp);
    }
  } 
  //printf("cvt-flt %s\n",wcan_buff->string);
  return wcan_buff->string;
}


/* write_canonical_term_rec is now tail recursive, to try to avoid C runstack oflow.
   It maintains a count of close parens to print on exit. */
int call_conv write_canonical_term_rec(CTXTdeclc Cell prologterm, int letter_flag)
{
  Integer close_paren_count = 0, cpi;
 write_canonical_term_rec_begin:
  XSB_Deref(prologterm);
  switch (cell_tag(prologterm)) 
    {
    case XSB_INT:
      sprintf(wcan_buff->string,"%" Intfmt,int_val(prologterm));
      XSB_StrAppend(wcan_string,wcan_buff->string);
      break;
    case XSB_STRING:
      if (quotes_are_needed(string_val(prologterm))) {
	if (string_contains_quotes(string_val(prologterm))) {
	  size_t len_needed = 2*strlen(string_val(prologterm))+1;
	  XSB_StrEnsureSize(wcan_atombuff,(int)len_needed);
	  double_quotes(string_val(prologterm),wcan_atombuff->string);
	  XSB_StrAppendC(wcan_string,'\'');
	  XSB_StrAppend(wcan_string,wcan_atombuff->string);
	  XSB_StrAppendC(wcan_string,'\'');
	} else {
	  XSB_StrAppendC(wcan_string,'\'');
	  XSB_StrAppend(wcan_string,string_val(prologterm));
	  XSB_StrAppendC(wcan_string,'\'');
	}
      } else {
	XSB_StrAppend(wcan_string,string_val(prologterm));
      }
      break;
    case XSB_FLOAT:
      XSB_StrAppend(wcan_string, cvt_float_to_str(CTXTc ofloat_val(prologterm)));
      break;
    case XSB_REF:
    case XSB_REF1: {
      UInteger varval;
      XSB_StrAppendC(wcan_string,'_');
      if (prologterm >= (Cell)glstack.low && prologterm <= (Cell)top_of_heap) {
	XSB_StrAppendC(wcan_string,'h');
	varval = ((prologterm-(Cell)glstack.low+1)/sizeof(CPtr));
      } else {
	if (prologterm >= (Cell)top_of_localstk && prologterm <= (Cell)glstack.high) {
	  XSB_StrAppendC(wcan_string,'l');
	  varval = (((Cell)glstack.high-prologterm+1)/sizeof(CPtr));
	} else varval = prologterm;   /* Should never happen */
      }
      sprintf(wcan_buff->string,"%" Intfmt,varval);
      XSB_StrAppend(wcan_string,wcan_buff->string);
    }
      break;
    case XSB_STRUCT: /* lettervar: i.e., print '$VAR'(i) terms as Cap Alpha-Num */
     //first, check to see if we are dealing with boxed int or boxed float, and if so,
     // do it's respective case, but with boxed value, then break. Otherwise, 
     // do the struct case
     if (isboxedinteger(prologterm))
     {
      sprintf(wcan_buff->string,"%" Intfmt,boxedint_val(prologterm));
      XSB_StrAppend(wcan_string,wcan_buff->string);
          break;         
     }
     else if (isboxedfloat(prologterm))
     {
       sprintf(wcan_buff->string,"%1.17g",boxedfloat_val(prologterm));
       wcan_buff->length = (int)strlen(wcan_buff->string);
       if (!strchr(wcan_buff->string,'.')) {
	 char *eloc = strchr(wcan_buff->string,'e');
	 if (!eloc) XSB_StrAppend(wcan_buff,".0");
	 else {	
	   char exp[5],fstr[30];
	   strcpy(exp,eloc);
	   eloc[0] = 0;
	   strcpy(fstr,wcan_buff->string);
	   XSB_StrSet(wcan_buff,fstr);
	   XSB_StrAppend(wcan_buff,".0");
	   XSB_StrAppend(wcan_buff,exp);
	 }
       }
       XSB_StrAppend(wcan_string,wcan_buff->string);
       break;         
     }        
      if (letter_flag && (get_str_psc(prologterm) == dollar_var_psc)) {
	int ival, letter;
	Cell tempi = get_str_arg(prologterm,1);
	XSB_Deref(tempi);
	if (!isointeger(tempi)) xsb_abort("[write_canonical]: illegal $VAR argument");
	ival = (int)int_val(tempi);
	letter = ival % 26;
	ival = ival / 26;
	XSB_StrAppendC(wcan_string,(char)(letter+'A'));
	if (ival != 0) {
	  sprintf(wcan_buff->string,"%d",ival);
	  XSB_StrAppend(wcan_string,wcan_buff->string);
	}
      } else { /* regular ole structured term */
	int i; 
	char *fnname = get_name(get_str_psc(prologterm));
	if (quotes_are_needed(fnname)) {
	  size_t len_needed = 2*strlen(fnname)+1;
	  XSB_StrEnsureSize(wcan_atombuff,(int)len_needed);
	  double_quotes(fnname,wcan_atombuff->string);
	  XSB_StrAppendC(wcan_string,'\'');
	  XSB_StrAppend(wcan_string,wcan_atombuff->string);
	  XSB_StrAppendC(wcan_string,'\'');
	} else XSB_StrAppend(wcan_string,fnname);
	XSB_StrAppendC(wcan_string,'(');
	if (get_arity(get_str_psc(prologterm))) {
	  for (i = 1; i < get_arity(get_str_psc(prologterm)); i++) {
	    if (!write_canonical_term_rec(CTXTc get_str_arg(prologterm,i),letter_flag)) {
	      XSB_StrAppend(wcan_string,"?ERROR?");
	      return FALSE;
	    }
	    XSB_StrAppendC(wcan_string,',');
	  }
	  close_paren_count++; /* count parens so can do tail recursion */
	  prologterm = get_str_arg(prologterm,i);
	  goto write_canonical_term_rec_begin;
	} else XSB_StrAppendC(wcan_string,')');
      }
      break;
    case XSB_LIST:
      {Cell tail;
      XSB_StrAppendC(wcan_string,'[');
      if (!write_canonical_term_rec(CTXTc get_list_head(prologterm),letter_flag)) {
	XSB_StrAppend(wcan_string,"?ERROR?");
	return FALSE;
      }
      tail = get_list_tail(prologterm);
      XSB_Deref(tail);
      while (islist(tail)) {
	XSB_StrAppendC(wcan_string,',');
	if (!write_canonical_term_rec(CTXTc get_list_head(tail),letter_flag)) {
	  XSB_StrAppend(wcan_string,"?ERROR?");
	  return FALSE;
	}
	tail = get_list_tail(tail);
	XSB_Deref(tail);
      } 
      if (!isnil(tail)) {
	XSB_StrAppendC(wcan_string,'|');
	if (!write_canonical_term_rec(CTXTc tail,letter_flag)) {
	  XSB_StrAppend(wcan_string,"?ERROR?");
	  return FALSE;
	}
      }
      XSB_StrAppendC(wcan_string,']');
      }
      break;
    default:
      return FALSE;
    }
  for (cpi=0; cpi<close_paren_count; cpi++) XSB_StrAppendC(wcan_string,')');
  return TRUE;
}

DllExport void call_conv write_canonical_term(CTXTdeclc Cell prologterm, int letter_flag)
{
  XSB_StrSet(wcan_string,"");
  XSB_StrEnsureSize(wcan_buff,40);
  if (write_canonical_term_rec(CTXTc prologterm, letter_flag)) return;
  else xsb_abort("[WRITE_CANONICAL_TERM] Illegal Prolog term: %s",wcan_string->string);
}

/* return a string containing the Prolog term in presented in canonical form */
char *canonical_term(CTXTdeclc Cell prologterm, int letter_flag) {
  XSB_StrSet(wcan_string,"");
  XSB_StrEnsureSize(wcan_buff,40);
  write_canonical_term_rec(CTXTc prologterm, letter_flag);
  return wcan_string->string;
}

void print_term_canonical(CTXTdeclc FILE *fptr, int charset, Cell prologterm, int letterflag)
{
  write_canonical_term(CTXTc prologterm, letterflag);
  write_string_code(fptr,charset,(byte *)wcan_string->string);
}

#undef wcan_string
#undef wcan_atombuff
#undef wcan_buff
