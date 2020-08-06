/* File:      table_status_defs.h
** Author(s): Ernie Johnson
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
** $Id: table_status_defs.h,v 1.9 2011-04-01 18:23:56 tswift Exp $
** 
*/


/*
 * Builtin codes indicating the type of tabled evaluation used for
 * a given predicate.  These values are shared with Prolog builtins
 * which allow users to select and change the evaluation method.
 */

#define UNTABLED_PREDICATE	 -1
#define VARIANT_EVAL_METHOD	  0x0
#define SUBSUMPTIVE_EVAL_METHOD	  0x1
#define DISPATCH_BLOCK	  0x3


/*
 * Builtin codes indicating the type of a given tabled call.
 */

#define UNDEFINED_CALL	   -1	/* due to untabled predicate or error */
#define PRODUCER_CALL	    0
#define SUBSUMED_CALL 	    1
#define NO_CALL_ENTRY	    2   /* for subsumptive predicates, does not
				   necessarily mean that such a call was
				   never made */

/*
 * Builtin codes indicating the status of a given tabled call's answer
 * set.
 */

#define INCR_NEEDS_REEVAL        -2
#define UNDEFINED_ANSWER_SET	 -1   /* due to untabled predicate, error,
				         or the call not having an entry
				         AND also no subsumer in the table */
#define COMPLETED_ANSWER_SET	  0
#define INCOMPLETE_ANSWER_SET	  1
