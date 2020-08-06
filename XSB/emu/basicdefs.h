/* File:      basicdefs.h
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: basicdefs.h,v 1.15 2013-04-18 22:55:43 tswift Exp $
** 
*/


#ifndef BASICDEFS_INCLUDED

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE  (!FALSE)
#endif

#ifndef NO
#define NO  FALSE
#endif
#ifndef YES
#define YES  TRUE
#endif


#ifdef WIN_NT
#define SLASH '\\'
#else
#define SLASH '/'
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN   1024  /* SYSV */
#endif

#ifndef MAXFILENAME
#define MAXFILENAME   255  /* SYSV */
#endif

#ifndef MAXBUFSIZE
#define MAXBUFSIZE   2048  /* used when a large string buffer is needed */
#endif

#define MAX_IO_BUFSIZE  4096 /* 1 page */

#define K   1024  /* please make sure that K stays divisible by sizeof(Cell) */

//#define MAXTERMBUFSIZE   64*K  /* Terms may be big...*/
#define MAXTERMBUFSIZE   256*K  /* Terms may be big...*/

#define XSB_STYLE_DCG  0    /* use XSB style DCG grammars */
#define STANDARD_DCG   1    /* use standard DCG grammars */


#ifndef xsb_max
#define xsb_max(p1,p2) ((p1)>=(p2)?(p1):(p2))
#endif
#ifndef xsb_min
#define xsb_min(p1,p2) ((p1)<=(p2)?(p1):(p2))
#endif

#define MOD %

#define IsNULL(ptr)      ( (ptr) == NULL )
#define IsNonNULL(ptr)   ( (ptr) != NULL )

#define XSB_INIT 0 
#define XSB_EXECUTE 1
#define XSB_SHUTDOWN 2
#define XSB_C_INIT 3
#define XSB_SETUP_X 4

#define XSBINITERRLEN 2048

#define UNUSED(x) ((void)(x))

#endif  /* BASICDEFS_INCLUDED */

#define BASICDEFS_INCLUDED

