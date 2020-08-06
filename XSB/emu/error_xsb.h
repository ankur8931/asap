/* File:      error_xsb.h
** Author(s): Kostis F. Sagonas
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
** $Id: error_xsb.h,v 1.61 2013-02-14 22:03:53 tswift Exp $
** 
*/

#include "basictypes.h"
#include "setjmp_xsb.h"
#include "export.h"
#include "context.h"
#include "psc_xsb.h"

/*----------------------------------------------------------------------*/
/* The following is a list of errors as defined by the Prolog ISO	*/
/* standard.  It is clear that today (October 1993), XSB does NOT	*/
/* catch all of them.  In fact it does not even report half of them.	*/
/* However, one fine day it might!  The following list tries to become	*/
/* a first step towards the uniform treatment of errors by XSB.		*/
/*----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#define CALCULATION	 0
#define DATABASE	 1
#define EVALUATION	 2
#define IMPLEMENTATION	 3
#define INSTANTIATION	 4
#define IO_CONTROL	 5
#define IO_END_OF_FILE	 6
#define IO_FORMATTING	 7
#define OPERATOR	 8
#define XSBOVERFLOW	 9  /* renamed from OVERFLOW. This def isn't used, but
			       OVERFLOW clashes with some C compilers */
#define RANGE		10
#define SYNTAX		11
  //#define TYPE		12
#define UNDEFINED_PRED	13
#define UNDEFINED_VAL	14
#define XSBUNDERFLOW	15  /* renamed from UNDERFLOW. This def isn't used, but
			       UNDERFLOW clashes with some C compilers */
#define ZERO_DIVIDE	16 

#define EVALUATION_DOMAIN_ERROR 0
#define EVALUATION_INSTANTIATION_ERROR 1
#define EVALUATION_UNDERFLOW_ERROR 2
#define EVALUATION_OVERFLOW_ERROR 3
#define EVALUATION_TYPE_ERROR 4

#define pow_domain_error(op1,op2)	{			\
  XSB_StrSet(&(*tsgSBuff1),"");					\
  XSB_StrSet(&(*tsgSBuff2),"");					\
  print_pterm(CTXTc op1, TRUE, &(*tsgSBuff1));			\
  print_pterm(CTXTc op2, TRUE, &(*tsgSBuff2));			\
  xsb_evaluation_error(CTXTc EVALUATION_DOMAIN_ERROR,				\
		       "Wrong domain in evaluable function **/2: %s is " \
		       "negative and finite, while its exponent (%s) is not an integer", \
		       (*tsgSBuff2).string, (*tsgSBuff1).string);	\
  }

/* TLS: used for determing the offset of a putpvar + call so that
   the pc register can be saved so that the proper choice point can be
   (in a necessarily roundabout manner) determined. 
*/
#ifdef BITS64
#define THROWPAD 24
#else 
#define THROWPAD 12
#endif 

#define MAX_CMD_LEN 8192

DllExport extern void call_conv xsb_exit(char *, ...);
DllExport extern void call_conv xsb_initialization_exit(char *, ...);
  DllExport extern void call_conv exit_xsb(char *);

DllExport extern void call_conv xsb_abort(char *, ...);
DllExport extern void call_conv abort_xsb(char *);

DllExport extern void call_conv xsb_bug(char *, ...);
DllExport extern void call_conv bug_xsb(char *);
void call_conv xsb_basic_abort(char *);

DllExport extern void call_conv xsb_warn(CTXTdeclc char *, ...);
DllExport extern void call_conv warn_xsb(char *);

DllExport extern void call_conv xsb_mesg(char *, ...);
DllExport extern void call_conv mesg_xsb(char *);

DllExport extern void call_conv xsb_error(char *, ...);
DllExport extern void call_conv error_xsb(char *);

DllExport extern void call_conv xsb_dbgmsg1(int, char *, ...);
DllExport extern void call_conv dbgmsg1_xsb(int, char *);

  extern void unifunc_abort(CTXTdeclc int, CPtr);
extern void addintfastuni_abort(CTXTdeclc Cell , Cell);
extern void arithmetic_abort(CTXTdeclc Cell, char *, Cell);
extern void arithmetic_abort1(CTXTdeclc char *, Cell);
extern void arithmetic_comp_abort(CTXTdeclc Cell, char *, Cell);
extern void err_handle(CTXTdeclc int, int, char *, int, char *, Cell);

extern FILE *stdmsg;	    	/* Stream for XSB messages     	         */
extern FILE *stdwarn;	    	/* Stream for XSB warnings     	         */
extern FILE *stddbg;	    	/* Stream for XSB debugging msgs         */
extern FILE *stdfdbk;	    	/* Stream for XSB feedback msgs         */

extern char *xsb_default_segfault_msg;

extern char *xsb_segfault_message; /* Put your segfault message here prior to
				      executing the command that might
				      segfault. Then restore it to
				      xsb_default_segfault_message */ 

extern void (*xsb_default_segfault_handler)(int); /* where the previous value
						     of the SIGSEGV handler is
						     saved */ 

extern int print_xsb_backtrace(CTXTdecl);

/* SIGSEGV handler that catches segfaults; used unless configured with DEBUG */
extern void xsb_segfault_catcher (int);
extern void xsb_segfault_quitter(int);

int unwind_stack(CTXTdecl);

DllExport extern void call_conv xsb_domain_error(CTXTdeclc char *, Cell, const char *, int) ;
  DllExport extern void call_conv xsb_evaluation_error(CTXTdeclc int,char * , ...);
DllExport extern void call_conv xsb_existence_error(CTXTdeclc char *,Cell, const char *,int, int) ;
DllExport extern void call_conv xsb_instantiation_error(CTXTdeclc const char *, int) ;
DllExport extern void call_conv xsb_misc_error(CTXTdeclc char*,const char*,int) ; 
DllExport extern void call_conv xsb_permission_error(CTXTdeclc char *,char *,Cell,const char *,int) ;
DllExport extern void call_conv xsb_resource_error(CTXTdeclc char *,const char *, int) ;
DllExport extern void call_conv xsb_representation_error(CTXTdeclc char*,Cell, const char*,int) ; 
 DllExport extern void call_conv xsb_resource_error_nopred(char *,char *,...) ;
DllExport extern void call_conv xsb_syntax_error(CTXTdeclc char *) ;
DllExport extern void call_conv xsb_syntax_error_non_compile(CTXTdeclc Cell,
							     const char *,int) ;
DllExport extern void call_conv xsb_table_error(CTXTdeclc char *) ;
DllExport extern void call_conv xsb_table_error_vargs(CTXTdeclc char *, char *, ...) ;
DllExport extern void call_conv xsb_new_table_error(CTXTdeclc char *, char *,char*, ...) ;
DllExport extern void call_conv xsb_type_error(CTXTdeclc char *,Cell , const char *,int) ;

extern void call_conv xsb_memory_error(char *, char *);
DllExport void call_conv xsb_throw_memory_error(int);

DllExport void call_conv xsb_throw_error(CTXTdeclc char *, char *);
DllExport void call_conv xsb_throw(CTXTdeclc prolog_term);

extern prolog_term build_xsb_backtrace(CTXTdecl);
extern char abort_file_gl[];

DllExport extern void  call_conv xsb_log(char *description, ...);

#ifdef __cplusplus
}
#endif

