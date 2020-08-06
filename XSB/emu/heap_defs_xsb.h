/* File:      heap_defs_xsb.h
** Author(s): Kostis Sagonas
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999.
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
** $Id: heap_defs_xsb.h,v 1.12 2013-05-06 21:10:24 dwarren Exp $
** 
*/


/*--- The following is for the type of garbage collection to be used ---*/

#define NO_GC           0
#define SLIDING_GC      1
#define COPYING_GC      2
#define INDIRECTION_SLIDE_GC   3

#define GC_GC_HEAP		1
#define GC_GC_STRINGS		2
#define GC_GC_CLAUSES		4
#define GC_GC_TABLED_PREDS	8

#define ABOLISH_TABLES_TRANSITIVELY  0
#define ABOLISH_TABLES_SINGLY        1
#define ABOLISH_TABLES_DEFAULT       2

/*--- The following are used for string-space collection ---------------*/

#define mark_string_safe(tstr,msg)		\
     do {char *str = (tstr);			\
         Integer *pptr = ((Integer *)(str))-1;  \
     if (!( *(pptr) & 7)) *(pptr) |= 1;		\
     } while(0)    

#define mark_string(tstr,msg) 				\
  do {char *str = (tstr);				\
      if (str && string_find_safe(str) == str) {	\
         Integer *pptr = ((Integer *)(str))-1;		\
         if (!( *(pptr) & 7)) *(pptr) |= 1;		\
     } else if (str) 					\
	printf("Not interned (wrongly GC-ed?): %s: '%p',%s\n",msg,str,str); \
  } while(0)

#define mark_if_string(tcell,msg) 		\
  do {Cell acell = (tcell);			\
      if (isstring(acell)) 			\
	mark_string(string_val(acell),msg);	\
  } while(0)
/*----------------------------------------------------------------------*/
