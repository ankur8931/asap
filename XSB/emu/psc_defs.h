/* File:      psc_defs.h
** Author(s): Jiyang Xu, Terry Swift, Kostis Sagonas
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
** $Id: psc_defs.h,v 1.22 2013-05-06 21:10:25 dwarren Exp $
** 
*/

#ifndef _PSC_DEFS_
#define _PSC_DEFS_

#define T_VISIBLE	0
#define T_HIDDEN	1
#define T_UNLOADED	2
#define T_ENV		3
#define T_TABLED	12 /* both bits may be on if it is known that pred is tabled, but not
			      yet determined whether it is variant or subsumptive tabled */
#define T_TABLED_VAR	8 /* Variant tabled (unless T_TABLED_SUB is also on, in which case
			     it is tabled, but not yet determined which kind.) */
#define T_TABLED_SUB	4 /* Subsumptive tabled (unless T_TABLED_VAR is also on, ...*/
#define T_TABLED_SUB_LOADFILE 64 /* Bit in the t_env byte in .xwam (objcode) file symtab 
				    that is used to set T_TABLED_SUB in the psc.  
				    (Nec since T_GLOBAL took 4, which would have been used.) */
#define T_SHARED	32 /* 0x20 (decimal for Prolog include) On: thread shared, Off: private */
#define T_SHARED_DET	16 /* 0x10 (decimal for Prolog include) On if T_SHARED bit determines
			      sharedness.  Off means sharedness not yet determined. 
			      Nec for error messages */
#define T_SPY		192 // 0xc0 use decimal for Prolog include

/* === type definition: (entry_type) ============================== */

#define T_ORDI  0	/* constant-type: no ep definition, may be set later */
#define T_DYNA	1	/* constant-type: dynamic, code in buffer */
#define T_PRED  2	/* constant-type: ep points to compiled code */

#define T_MODU  4	/* ep field is used to hold psc-list */

/* TLS: PSCs should no longer represent files (or file descriptors).
Use the stream table instead.  By the way, I think I've gotten all of
the PSC files out of the system, but now and then I come across one
I've forgotten.  */
//#define T_FILE  5	/* ep field could be file descriptor (not now) */

#define T_UDEF 12	/* unloaded T_PRED */
#define T_FORN 13	/* predicate in foreign language */

#define T_DEFI 128      /* bit to pass from compiler to loader that pred has clauses */
#define T_DEFA 64	/* bit to pass from compiler to loader for import-as (defined-as) */
/* === loader definitions (module information) ==================== */

#define T_EXPORTED 0
#define T_LOCAL    1
#define T_IMPORTED 2
#define T_GLOBAL   4

#define T_NONTABLED 0
#define T_VARIANT      8
#define T_SUBSUMPTIVE  4
#define T_TABLED_UNDETERMINED 12

#define T_INCR       3
//#define T_INTERN     4  /* if clauses to be interned for dynamic pred */
#define T_INTERN     1  /* if clauses to be interned for dynamic pred */

/* Global module */

#define USERMOD_PSC    1

#endif /* _PSC_DEFS_ */

