#include "xsb_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#ifdef WIN_NT
#define XSB_DLL
#endif

#include "auxlry.h"
/* context.h is necessary for the type of a thread context. */
#include "context.h"
#include "cinterf.h"

/* call as: change_char(+String,+CharPos,+ReplacementString,-ResultString)
   Will take String and replace the character at position CharPos with the
   first character in ReplacementString. Both String and ReplacementString must
   be atoms.
*/
DllExport int call_conv change_char(CTXTdecl)
{
   char	*str_in; 
   int	pos;
   char *c, *str_out;

	str_in = (char *) extern_ptoc_string(1);
	str_out = (char *) malloc(strlen(str_in)+1);
	strcpy(str_out, str_in);
	pos = extern_ptoc_int(2);
	c = (char *) extern_ptoc_string(3);
	str_out[pos-1] = c[0];

	/* Now that we have constructed a new symbol, we must ensure that it
	   appears in the symbol table.  This can be done using function
	   string_find() that searches the symbol table for the symbol, and
	   if the symbol does not appear there, it inserts it.  If we are 
	   sure that the symbol already appeared in the symbol table there
	   is no need to use string_find(). 
	 */

	//	extern_ctop_string(4, (char *) string_find(str_out,4));
		extern_ctop_string(CTXTc 4, str_out);

	return TRUE;
}

DllExport int call_conv  my_sqrt(CTXTdecl)
{
   int i = extern_ptoc_int(1);

   extern_ctop_float(2, (float) pow((double)i, 0.5));
   return TRUE;
}

DllExport int call_conv minus_one(CTXTdecl)
{
  int i = extern_ptoc_int(1);
  extern_ctop_int(2,i-1);
  return TRUE;
}

