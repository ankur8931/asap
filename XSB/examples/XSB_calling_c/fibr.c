/******************************************************************
This is a C function in a foreign module that shows how a foreign
function called from XSB can in turn call a predicate back in XSB.
This function is to be called from XSB as fibr(+N,-F).  It receives a
nonnegative integer is register 1 (the first argument) and returns
fibonacci of that number in register 2 (the second argument.)  It uses
the horribly inefficient doubly recursive algorithm.  It first checks
if the input is 1 or less and if so, directly returns 1.  Otherwise,
it calls a predicate in XSB, fibp/2, for N-1 and N-2, adds the
results, and returns that value.  fibp/2 is defined in XSB in a
similar way: for input less than 2, it returns 1 directly; otherwise
if call this foreign function fibr on the two smaller values, adds
them, and returns them as the result.
*******************************************************************/

#include <stdio.h>
#include "xsb_config.h"
#include "cinterf.h"
#include "context.h"
#ifndef TRUE
#define TRUE 1
#endif

DllExport int call_conv fibr(CTXTdecl) {
  int inarg, f1, f2;

  inarg = p2c_int(reg_term(CTXTc 1));  // get the input argument

  if (inarg <= 1) { // if small, return answer directly 
    c2p_int(CTXTc 1,reg_term(CTXTc 2));
    return TRUE;
  }

  xsb_query_save(CTXTc 2);  // prepare for calling XSB
  c2p_functor(CTXTc "fibp",2,reg_term(CTXTc 1));  // setup call
  c2p_int(CTXTc inarg-1,p2p_arg(reg_term(CTXTc 1),1)); // and inp arg
  if (xsb_query(CTXT)) {  // call XSB
    printf("Error calling fibp 1.\n");
    fflush(stdout);
    return FALSE;
 }
  f1 = p2c_int(p2p_arg(reg_term(CTXTc 1),2));  // get answer
  if (xsb_close_query(CTXT)) {  // throw away other (nonexistent) answers
    printf("Error closing fibp 1.\n");
    fflush(stdout);
    return FALSE;
  }

  c2p_functor(CTXTc "fibp",2,reg_term(CTXTc 1)); // prepare for 2nd call to XSB
  c2p_int(CTXTc inarg-2,p2p_arg(reg_term(CTXTc 1),1)); // and its inp arg
  if (xsb_query(CTXT)) { // and call query
    printf("Error calling fibp 2.\n");
    fflush(stdout);
    return FALSE;
  }
  f2 = p2c_int(p2p_arg(reg_term(CTXTc 1),2)); // and get its answer
  if (xsb_next(CTXT) != XSB_FAILURE) { // get next answer, which must NOT exist
    printf("Error getting next fibp 2.\n");
    fflush(stdout);
    return FALSE;
  }

  if (xsb_query_restore(CTXT)) {  // restore regs to prepare for exit
    printf("Error finishing.\n");
    fflush(stdout);
    return FALSE;
  }
  c2p_int(CTXTc f1+f2,reg_term(CTXTc 2));  // set our answer
  return TRUE;  // and return successfully
}
