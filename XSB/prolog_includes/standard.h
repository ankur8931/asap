/* File:      prolog_includes/standard.h
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
** $Id: standard.h,v 1.9 2010-08-19 15:03:40 spyrosh Exp $
** 
*/



#define STDIN	     0
#define STDOUT	     1
#define STDERR	     2
#define STDWARN	     3    /* output stream for xsb warnings  */
#define STDMSG	     4    /* output for regular xsb messages */
#define STDDBG	     5    /* output for debugging info       */
#define STDFDBK	     6    /* output for XSB feedback
			     (prompt/yes/no/Aborting/answers) */

#define AF_INET     0	  /* XSB-side socket request for Internet domain */
#define AF_UNIX     1     /* XSB-side socket request for UNIX domain */
