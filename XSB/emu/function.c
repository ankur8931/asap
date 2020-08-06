/* File:      function.c
** Author(s): Jiyang Xu, David S. Warren
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
** $Id: function.c,v 1.41 2013-01-09 20:15:34 dwarren Exp $
** 
*/

#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "register.h"
#include "memory_xsb.h"
#include "deref.h"
#include "heap_xsb.h"
#include "binding.h"
#include "error_xsb.h"
#include "function.h"
#include "cell_xsb_i.h"

#define FUN_PLUS   1
#define FUN_MINUS  2
#define FUN_TIMES  3
#define FUN_DIVIDE 4
#define FUN_AND    5
#define FUN_OR     6
#define FUN_sin    9
#define FUN_cos   10
#define FUN_tan   11

#define FUN_float 13
#define FUN_floor 14
#define FUN_exp   15
#define FUN_log   16
#define FUN_log10 17
#define FUN_sqrt  18
#define FUN_asin  19
#define FUN_acos  20
#define FUN_atan  21
#define FUN_abs  22
#define FUN_truncate  23
#define FUN_round  24
#define FUN_ceiling  25
#define FUN_sign 26
#define FUN_min  27
#define FUN_lgamma  28
#define FUN_erf  29
//#define FUN_atan2  30

char * function_names[30] = {"","+","-","*","/","/\\","\\/","","","sin",
                             "cos","tan","","float","floor","exp","log","log10","sqrt","asin",
                             "acos","atan","abs","truncate","round","ceiling","sign","min","lgamma","erf"};

/* --- returns 1 when succeeds, and returns 0 when there is an error --	*/

#define set_fvalue_from_value do {					\
    if (isointeger(value)) fvalue = (Float) oint_val(value);		\
    else if (isofloat(value)) fvalue = ofloat_val(value);		\
    else {								\
      FltInt fiop1;							\
      if (xsb_eval(CTXTc value, &fiop1)) {				\
	if (isfiint(fiop1)) fvalue = (Float)fiint_val(fiop1);		\
	else fvalue = fiflt_val(fiop1);					\
      } else return 0;							\
    }									\
  } while (0)

extern void inline bld_boxedfloat(CTXTdeclc CPtr, Float);

int  unifunc_call(CTXTdeclc int funcnum, CPtr regaddr)
{
  Cell value;
  Float fvalue; 
  prolog_int ivalue;

  value = cell(regaddr);
  XSB_Deref(value);
  switch (funcnum) {
  case FUN_float:
    set_fvalue_from_value;
    bld_boxedfloat(CTXTc regaddr, fvalue);
    break;
  case FUN_floor:
    set_fvalue_from_value;
    ivalue = (prolog_int) floor(fvalue);
    bld_oint(regaddr, ivalue);
    break;
  case FUN_PLUS:
  case FUN_MINUS:
  case FUN_TIMES:
  case FUN_DIVIDE:
  case FUN_AND:
  case FUN_OR:
    return 0;		/* should not come here */
  case FUN_sin:
      set_fvalue_from_value;
      fvalue = (Float)sin(fvalue);
      bld_boxedfloat(CTXTc regaddr, fvalue);
  break;
  case FUN_cos:
      set_fvalue_from_value;
      fvalue = (Float)cos(fvalue);
      bld_boxedfloat(CTXTc regaddr, fvalue);
  break;
  case FUN_tan:
      set_fvalue_from_value;
      fvalue = (Float)tan(fvalue);
      bld_boxedfloat(CTXTc regaddr, fvalue);
  break;
  case FUN_exp:
      set_fvalue_from_value;
      fvalue = (Float)exp(fvalue);
      bld_boxedfloat(CTXTc regaddr, fvalue);
      break;
  case FUN_log:
      set_fvalue_from_value;
      if (fvalue > 0) {               /* tls -- shd be able to use errno, but I cant seem to get it to work(?) */
	fvalue = (Float)log(fvalue);
	bld_boxedfloat(CTXTc regaddr, fvalue);
      }
      else /* NaN */
	xsb_evaluation_error(CTXTc EVALUATION_DOMAIN_ERROR,"in log/1");
  break;
  case FUN_log10:
    set_fvalue_from_value;
    if (fvalue > 0) {
      fvalue = (Float)log10(fvalue);
      bld_boxedfloat(CTXTc regaddr, fvalue);
    }
    else /* NaN */
      xsb_evaluation_error(CTXTc EVALUATION_DOMAIN_ERROR,"in log10/1");
  break;
  case FUN_sqrt:
      set_fvalue_from_value;
      fvalue = (Float)sqrt(fvalue);
      if (fvalue == fvalue) 
	bld_boxedfloat(CTXTc regaddr, fvalue);
      else /* NaN */
	xsb_evaluation_error(CTXTc EVALUATION_DOMAIN_ERROR,"sqrt/1 returned NaN");
      break;
  case FUN_asin:
      set_fvalue_from_value;
      fvalue = (Float)asin(fvalue);
      if (fvalue == fvalue) 
	bld_boxedfloat(CTXTc regaddr, fvalue);
      else /* NaN */
	xsb_evaluation_error(CTXTc EVALUATION_DOMAIN_ERROR,"asin/1 returned NaN");
  break;
  case FUN_acos:
    set_fvalue_from_value;
    fvalue = (Float)acos(fvalue);
    if (fvalue == fvalue) 
      bld_boxedfloat(CTXTc regaddr, fvalue);
    else /* NaN */
      xsb_evaluation_error(CTXTc EVALUATION_DOMAIN_ERROR,"acos/1 returned NaN");
    break;
  case FUN_atan:
    set_fvalue_from_value;
    fvalue = (Float)atan(fvalue);
    bld_boxedfloat(CTXTc regaddr, fvalue);
    break;
  case FUN_abs:
    if (isointeger(value)) {
      ivalue = oint_val(value);
      if (ivalue > 0) {
	bld_oint(regaddr,ivalue);
      } else bld_oint(regaddr,-ivalue);
    } 
    else if (isofloat(value) ) {
      fvalue = ofloat_val(value);
      if (fvalue > 0) {
          bld_boxedfloat(CTXTc regaddr,fvalue);
      } else {
          fvalue = -fvalue;
          bld_boxedfloat(CTXTc regaddr,fvalue);
      }
    } else {
      FltInt fiop1;
      if (xsb_eval(CTXTc value, &fiop1)) {
	if (isfiint(fiop1)) 
	  if (fiint_val(fiop1) >= 0) {
	    ivalue = fiint_val(fiop1);
	    bld_oint(regaddr,ivalue);
	  }
	  else {
	    ivalue = -fiint_val(fiop1);
	    bld_oint(regaddr,ivalue);
	  }
	else
	  if (fiflt_val(fiop1) >= 0) {
	    fvalue = fiflt_val(fiop1);
	    bld_boxedfloat(CTXTc regaddr, fvalue);
	  }
	  else {
	    fvalue = -fiflt_val(fiop1);
	    bld_boxedfloat(CTXTc regaddr,fvalue);
	  }
      } else return 0;
    }
    break;
  case FUN_truncate:
    if (isointeger(value)) { 
      ivalue = oint_val(value);
      bld_oint(regaddr,ivalue);
    }
    else if (isofloat(value)) {
      fvalue = ofloat_val(value);
      if (fvalue > 0) 
      {
          ivalue = (prolog_int) floor(fvalue);
          bld_oint(regaddr,ivalue);
      }
      else 
      {
          ivalue = (prolog_int) -floor(-fvalue);
          bld_oint(regaddr,ivalue);
      }
    } else {
      FltInt fiop1;
      if (xsb_eval(CTXTc value, &fiop1)) {
	if (isfiint(fiop1)) {
	  ivalue = fiint_val(fiop1);
	  bld_oint(regaddr,ivalue);
	}
	else
	  if (fiflt_val(fiop1) > 0) {
	    ivalue = (prolog_int) floor(fiflt_val(fiop1));
	    bld_oint(regaddr,ivalue);
	  }
	  else {
	    ivalue = (prolog_int) -floor(-fiflt_val(fiop1));
	    bld_oint(regaddr,ivalue);
	  }
      } else return 0;
    }
    break;
  case FUN_round:
    if (isointeger(value)) { 
      ivalue = oint_val(value);
      bld_oint(regaddr,ivalue);
    }
    else if (isofloat(value)) {
      fvalue = ofloat_val(value);
      ivalue = (prolog_int) floor(fvalue+0.5);
      bld_oint(regaddr, ivalue);
    } else {
      FltInt fiop1;
      if (xsb_eval(CTXTc value, &fiop1)) {
	if (isfiint(fiop1)) {
	  ivalue = fiint_val(fiop1);
	  bld_oint(regaddr,ivalue);
	}
	else {
	  ivalue = (prolog_int) floor(fiflt_val(fiop1)+0.5);
	  bld_oint(regaddr,ivalue);
	}
      } else return 0;
    }
    break;
  case FUN_ceiling:
    if (isointeger(value)) { 
      ivalue = oint_val(value);
      bld_oint(regaddr,ivalue);
    }
    else if (isofloat(value)) {
      fvalue = ofloat_val(value);
      ivalue = (prolog_int) -floor(-fvalue);
      bld_oint(regaddr,ivalue);
    } else {
      FltInt fiop1;
      if (xsb_eval(CTXTc value, &fiop1)) {
	if (isfiint(fiop1)) {
	  ivalue = fiint_val(fiop1);
	  bld_oint(regaddr,ivalue);
	}
	else {
	  ivalue = (prolog_int) -floor(-fiflt_val(fiop1));
	  bld_oint(regaddr,ivalue);
	}
      } else return 0;
    }
    break;
  case FUN_sign:
    set_fvalue_from_value;
    //    printf("value %d fvalue %f\n",value,fvalue);
    if (fvalue > 0) 
      bld_int(regaddr,1);
    else if (fvalue == 0) 
      bld_int(regaddr,0);
    else if (fvalue < 0) {
      bld_int(regaddr,-1);
    }
    break;
  case FUN_lgamma:
    set_fvalue_from_value;
#if defined(WIN_NT)
    xsb_warn(CTXTc "lgamma function NOT defined");
    fvalue = 0.0;
#else
    fvalue = (Float)lgamma(fvalue);
#endif
    bld_boxedfloat(CTXTc regaddr, fvalue);
    break;
  case FUN_erf:
    set_fvalue_from_value;
#if defined(WIN_NT)
    xsb_warn(CTXTc "lgamma function NOT defined");
    fvalue = 0.0;
#else
    fvalue = (Float)erf(fvalue);
#endif
    bld_boxedfloat(CTXTc regaddr, fvalue);
    break;

  default:  return 0;
  }
  return 1;
}


static double xsb_calculate_epsilon(void)
{
	double ep = 1.0, one = 1.0 ;
	for( ; one + ep > one ; ep /= 2 ) ;
	return ep * 2 ;
}


/* xsb_eval evaluates a Prolog term representing an arithmetic
   expression and returns its value as an integer or float. */

int xsb_eval(CTXTdeclc Cell expr, FltInt *value) {

  XSB_Deref(expr);
  if (isointeger(expr)) set_int_val(value,oint_val(expr));
  else if (isofloat(expr)) 
    set_flt_val(value,ofloat_val(expr));
  else if (isconstr(expr)) {
    Psc op_psc = get_str_psc(expr);
    int arity = get_arity(op_psc);
    if (arity == 2) {
      Cell op1, op2;
      FltInt fiop1, fiop2;
      op1 = get_str_arg(expr,1);
      op2 = get_str_arg(expr,2);
      /* this evaluates recursively even before it checks whether the operator 
	 is valid.  May be a problem. */
      if (xsb_eval(CTXTc op1, &fiop1) && xsb_eval(CTXTc op2, &fiop2)) {
	switch (get_name(op_psc)[0]) {
	case '+':
	  if (strcmp(get_name(op_psc),"+")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_int_val(value,fiint_val(fiop1)+fiint_val(fiop2));
	      else set_flt_val(value,fiint_val(fiop1)+fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) set_flt_val(value,fiflt_val(fiop1)+fiint_val(fiop2));
	      else set_flt_val(value,fiflt_val(fiop1)+fiflt_val(fiop2));
	  }
	  break;
	case '*':
	  if (strcmp(get_name(op_psc),"*")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_int_val(value,fiint_val(fiop1)*fiint_val(fiop2));
	      else set_flt_val(value,fiint_val(fiop1)*fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) set_flt_val(value,fiflt_val(fiop1)*fiint_val(fiop2));
	      else set_flt_val(value,fiflt_val(fiop1)*fiflt_val(fiop2));
	    break;
	  } else if (strcmp(get_name(op_psc),"**")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_flt_val(value,(Float)pow((Float)fiint_val(fiop1),(Float)fiint_val(fiop2)));
	      else set_flt_val(value,(Float)pow((Float)fiint_val(fiop1),fiflt_val(fiop2)));
	    } else {
	      if (isfiint(fiop2)) set_flt_val(value,(Float)pow(fiflt_val(fiop1),(Float)fiint_val(fiop2)));
	      else if (fiflt_val(fiop1) < 0) return 0;
	      else set_flt_val(value,(Float)pow(fiflt_val(fiop1),fiflt_val(fiop2)));
	    }
	    break;
	  } else set_and_return_fail(value);

	case '^':
	  if (strcmp(get_name(op_psc),"^")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_int_val(value,(Integer)pow((Float)fiint_val(fiop1),(Float)fiint_val(fiop2)));
	      else set_flt_val(value,(Float)pow((Float)fiint_val(fiop1),fiflt_val(fiop2)));
	    } else {
	      if (isfiint(fiop2)) set_flt_val(value,(Float)pow(fiflt_val(fiop1),(Float)fiint_val(fiop2)));
	      else if (fiflt_val(fiop1) < 0) return 0;
	      else set_flt_val(value,(Float)pow(fiflt_val(fiop1),fiflt_val(fiop2)));
	    }
	    break;
	  } else set_and_return_fail(value);

	case 'a':
	  if (strcmp(get_name(op_psc),"atan")==0 || strcmp(get_name(op_psc),"atan2")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_flt_val(value,(Float)atan2((Float)fiint_val(fiop1),(Float)fiint_val(fiop2)));
	      else set_flt_val(value,(Float)atan2((Float)fiint_val(fiop1),fiflt_val(fiop2)));
	    } else
	      if (isfiint(fiop2)) set_flt_val(value,(Float)atan2(fiflt_val(fiop1),(Float)fiint_val(fiop2)));
	      else set_flt_val(value,(Float)atan2(fiflt_val(fiop1),fiflt_val(fiop2)));
	    break;
	  } else set_and_return_fail(value);

	case 'r':
	  if (strcmp(get_name(op_psc),"rem")==0) { /* % is C rem */
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_int_val(value,fiint_val(fiop1)%fiint_val(fiop2));
	      else set_int_val(value,fiint_val(fiop1)%(Integer)fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) set_int_val(value,(Integer)fiflt_val(fiop1)%fiint_val(fiop2));
	      else set_int_val(value,(Integer)fiflt_val(fiop1)%(Integer)fiflt_val(fiop2));
	    break;
	  } else set_and_return_fail(value);

	case 'd':
	  if (strcmp(get_name(op_psc),"div")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,(Integer)floor((Float)fiint_val(fiop1) / (Float)fiint_val(fiop2)));
	    } else arithmetic_abort(CTXTc op2,"div",op1);
	    break;
	  } else set_and_return_fail(value);
	  
	case 'm':
	  if (strcmp(get_name(op_psc),"mod")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) 
		set_int_val(value,fiint_val(fiop1)-(Integer)floor((Float)fiint_val(fiop1)/fiint_val(fiop2))*fiint_val(fiop2));
	      else set_flt_val(value,fiint_val(fiop1)-(Float)floor(fiint_val(fiop1)/fiflt_val(fiop2))*fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) 
		set_flt_val(value,fiflt_val(fiop1)-(Float)floor(fiflt_val(fiop1)/fiint_val(fiop2))*fiint_val(fiop2));
	      else set_flt_val(value,fiflt_val(fiop1)-(Float)floor(fiflt_val(fiop1)/fiflt_val(fiop2))*fiflt_val(fiop2));
	    break;
	  } else if (strcmp(get_name(op_psc),"min")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) 
		if (fiint_val(fiop1)<fiint_val(fiop2)) set_int_val(value,fiint_val(fiop1));
		else set_int_val(value,fiint_val(fiop2));
	      else 
		if (fiint_val(fiop1)<fiflt_val(fiop2)) set_int_val(value,fiint_val(fiop1));
		else set_flt_val(value,fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) 
		if (fiflt_val(fiop1)<fiint_val(fiop2)) set_flt_val(value,fiflt_val(fiop1));
		else set_int_val(value,fiint_val(fiop2));
	      else 
		if (fiflt_val(fiop1)<fiflt_val(fiop2)) set_flt_val(value,fiflt_val(fiop1));
		else set_flt_val(value,fiflt_val(fiop2));
	    break;
	  } else if (strcmp(get_name(op_psc),"max")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) 
		if (fiint_val(fiop1)>fiint_val(fiop2)) set_int_val(value,fiint_val(fiop1));
		else set_int_val(value,fiint_val(fiop2));
	      else 
		if (fiint_val(fiop1)>fiflt_val(fiop2)) set_int_val(value,fiint_val(fiop1));
		else set_flt_val(value,fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) 
		if (fiflt_val(fiop1)>fiint_val(fiop2)) set_flt_val(value,fiflt_val(fiop1));
		else set_int_val(value,fiint_val(fiop2));
	      else 
		if (fiflt_val(fiop1)>fiflt_val(fiop2)) set_flt_val(value,fiflt_val(fiop1));
		else set_flt_val(value,fiflt_val(fiop2));
	    break;
	  } else set_and_return_fail(value);

	case '-':
	  if (strcmp(get_name(op_psc),"-")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_int_val(value,fiint_val(fiop1)-fiint_val(fiop2));
	      else set_flt_val(value,fiint_val(fiop1)-fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) set_flt_val(value,fiflt_val(fiop1)-fiint_val(fiop2));
	      else set_flt_val(value,fiflt_val(fiop1)-fiflt_val(fiop2));
	    break;
	  } else set_and_return_fail(value);

	case '/':
	  if (strcmp(get_name(op_psc),"/")==0) {
	    if (isfiint(fiop1)) {
	      if (isfiint(fiop2)) set_flt_val(value,(Float)fiint_val(fiop1)/(Float)fiint_val(fiop2));
	      else set_flt_val(value,(Float)fiint_val(fiop1)/fiflt_val(fiop2));
	    } else
	      if (isfiint(fiop2)) set_flt_val(value,fiflt_val(fiop1)/(Float)fiint_val(fiop2));
	      else set_flt_val(value,fiflt_val(fiop1)/fiflt_val(fiop2));
	    break;
	  } else if (strcmp(get_name(op_psc),"//")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) / fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else if (strcmp(get_name(op_psc),"/\\")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) & fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else set_and_return_fail(value);

	case '<':
	  if (strcmp(get_name(op_psc),"<<")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) << fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else set_and_return_fail(value);

	case '>':
	  if (strcmp(get_name(op_psc),"><")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) ^ fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else if (strcmp(get_name(op_psc),">>")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) >> fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else set_and_return_fail(value);

	case '\\':
	  if (strcmp(get_name(op_psc),"\\/")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) | fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else set_and_return_fail(value);
	case 'x':
	  if (strcmp(get_name(op_psc),"xor")==0) {
	    if (isfiint(fiop1) && isfiint(fiop2)) {
	      set_int_val(value,fiint_val(fiop1) ^ fiint_val(fiop2));
	    } else return 0;
	    break;
	  } else set_and_return_fail(value);
	default:
	  set_and_return_fail(value);
	} /* end switch */
      } else set_and_return_fail(value);

    } else if (arity == 1) {
      Cell op1;
      FltInt fiop1;
      op1 = get_str_arg(expr,1);
      if (xsb_eval(CTXTc op1, &fiop1)) {
      /***
	  builtin_function(sin, 1, 9).
	  builtin_function(cos, 1, 10).
	  builtin_function(tan, 1, 11).
	  builtin_function(float, 1, 13).
	  builtin_function(floor, 1, 14).
	  builtin_function(exp, 1, 15).
	  builtin_function(log, 1, 16).
	  builtin_function(log10, 1, 17).
	  builtin_function(sqrt, 1, 18).
	  builtin_function(asin, 1, 19).
	  builtin_function(acos, 1, 20).
	  builtin_function(atan, 1, 21).
	  builtin_function(abs, 1, 22).
	  builtin_function(truncate, 1, 23).
	  builtin_function(round, 1, 24).
	  builtin_function(ceiling, 1, 25).
	  builtin_function(sign, 1, 26).
      **/
	switch (get_name(op_psc)[0]) {
	case '-':
	  if (strcmp(get_name(op_psc),"-")==0) {
	    if (isfiint(fiop1)) set_int_val(value,-fiint_val(fiop1));
	    else set_flt_val(value,-fiflt_val(fiop1));
	    break;
	  } else set_and_return_fail(value);

	case '+':
	  if (strcmp(get_name(op_psc),"+")==0) {
	    if (isfiint(fiop1)) set_int_val(value,fiint_val(fiop1));
	    else set_flt_val(value,fiflt_val(fiop1));
	    break;
	  } else set_and_return_fail(value);

	case '\\':
	  if (strcmp(get_name(op_psc),"\\")==0) {
	    if (isfiint(fiop1)) set_int_val(value,~fiint_val(fiop1));
	    else set_int_val(value,~((Integer)fiint_val(fiop1)));
	    break;
	  } else set_and_return_fail(value);

	case 's':
	  if (strcmp(get_name(op_psc),"sin")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)sin((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)sin(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"sqrt")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)sqrt((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)sqrt(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"sign")==0) {
	    if (isfiint(fiop1)) 
	      if (fiint_val(fiop1) > 0) set_int_val(value,1);
	      else if (fiint_val(fiop1) == 0) set_int_val(value,0);
	      else set_int_val(value,-1);
	    else if (fiflt_val(fiop1) > 0.0) set_int_val(value,1);
	    else if (fiflt_val(fiop1) == 0.0) set_int_val(value,0);
	    else set_int_val(value,-1);
	    break;
	  } else if (strcmp(get_name(op_psc),"sinh")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)sinh((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)sinh(fiflt_val(fiop1)));
	    break;
	  } else set_and_return_fail(value);

	case 'c':
	  if (strcmp(get_name(op_psc),"cos")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)cos((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)cos(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"ceiling")==0) {
	    if (isfiint(fiop1)) set_int_val(value,fiint_val(fiop1));
	    else  set_int_val(value,-(Integer)floor(-fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"cosh")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)cosh((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)cosh(fiflt_val(fiop1)));
	    break;
	  } else set_and_return_fail(value);

	case 't':
	  if (strcmp(get_name(op_psc),"tan")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)tan((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)tan(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"truncate")==0) {
	    if (isfiint(fiop1)) set_int_val(value,fiint_val(fiop1));
	    else if (fiflt_val(fiop1) > 0) set_int_val(value,(Integer)floor(fiflt_val(fiop1)));
	    else set_int_val(value,-(Integer)floor(-fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"tanh")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)tanh((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)tanh(fiflt_val(fiop1)));
	    break;
	  } else set_and_return_fail(value);

	case 'f':
	  if (strcmp(get_name(op_psc),"float")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)fiint_val(fiop1));
	    else set_flt_val(value,fiflt_val(fiop1));
	    break;
	  } else if (strcmp(get_name(op_psc),"floor")==0) {
	    if (isfiint(fiop1)) set_int_val(value,fiint_val(fiop1));
	    else set_int_val(value,(Integer)floor(fiflt_val(fiop1)));
	    break;
	  } else set_and_return_fail(value);

	case 'e':
	  if (strcmp(get_name(op_psc),"exp")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)exp((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)exp(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"erf")==0) {
#ifdef WIN_NT
	    xsb_warn(CTXTc "erf function NOT defined");
	    set_flt_val(value,0.0);
#else
	    if (isfiint(fiop1)) set_flt_val(value,(Float)erf((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)exp(fiflt_val(fiop1)));
#endif
	    break;
	  } else set_and_return_fail(value);

	case 'l':
	  if (strcmp(get_name(op_psc),"log")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)log((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)log(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"log10")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)log10((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)log10(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"lgamma")==0) {
#ifdef WIN_NT
	    xsb_warn(CTXTc "lgamma function NOT defined");
	    set_flt_val(value,0.0);
#else
	    if (isfiint(fiop1)) set_flt_val(value,(Float)lgamma((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)lgamma(fiflt_val(fiop1)));
#endif
	    break;
	  } else set_and_return_fail(value);

	case 'a':
	  if (strcmp(get_name(op_psc),"asin")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)asin((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)asin(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"acos")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)acos((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)acos(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"atan")==0) {
	    if (isfiint(fiop1)) set_flt_val(value,(Float)atan((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)atan(fiflt_val(fiop1)));
	    break;
	  } else if (strcmp(get_name(op_psc),"abs")==0) {
	    if (isfiint(fiop1)) 
	      if (fiint_val(fiop1) >= 0) set_int_val(value,fiint_val(fiop1));
	      else set_int_val(value,-fiint_val(fiop1));
	    else
	      if (fiflt_val(fiop1) >= 0) set_flt_val(value,fiflt_val(fiop1));
	      else set_flt_val(value,-fiflt_val(fiop1));
	    break;
	  } else if (strcmp(get_name(op_psc),"asinh")==0) {
#ifdef WIN_NT
	    xsb_warn(CTXTc "asinh function NOT defined");
	    set_flt_val(value,0.0);
#else
	    if (isfiint(fiop1)) set_flt_val(value,(Float)asinh((Float)fiint_val(fiop1)));
	    else set_flt_val(value,(Float)asinh(fiflt_val(fiop1)));
#endif
	    break;
//		PM: the following two function definitions are incomplete as they need to
//			handle out of range errors but I haven't figured out yet how to do it!
//	  } else if (strcmp(get_name(op_psc),"acosh")==0) {
//	    if (isfiint(fiop1)) set_flt_val(value,(Float)acosh((Float)fiint_val(fiop1)));
//	    else set_flt_val(value,(Float)acosh(fiflt_val(fiop1)));
//	    break;
//	  } else if (strcmp(get_name(op_psc),"atanh")==0) {
//	    if (isfiint(fiop1)) set_flt_val(value,(Float)atanh((Float)fiint_val(fiop1)));
//	    else set_flt_val(value,(Float)atanh(fiflt_val(fiop1)));
//	    break;
	  } else set_and_return_fail(value);

	case 'r':
	  if (strcmp(get_name(op_psc),"round")==0) {
	    if (isfiint(fiop1)) set_int_val(value,fiint_val(fiop1));
	    else  set_int_val(value,(Integer)floor(fiflt_val(fiop1)+0.5));
	    break;
	  } else set_and_return_fail(value);

	default:
	  set_and_return_fail(value);
	} /* end switch */
      } else  /* bad subexpression */
	set_and_return_fail(value);
    } else  /* not binary or unary functor or string */
      set_and_return_fail(value);
  } else if (isstring(expr)) {
    if (strcmp(string_val(expr),"pi")==0) {
      set_flt_val(value,(const Float)3.1415926535897932384);
    } else if (strcmp(string_val(expr),"e")==0) {
      set_flt_val(value,(const Float)2.7182818284590452354);
    } else if (strcmp(string_val(expr),"epsilon")==0) {
      set_flt_val(value,(const Float)xsb_calculate_epsilon());
    } else set_and_return_fail(value);      
  } else set_and_return_fail(value);
  return 1;
}
