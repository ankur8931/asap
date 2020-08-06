/*  -*-c-*-  Make sure this file comes up in the C mode of emacs */ 
/* File:      close.i
** Author(s): Ernie Johnson
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1997-1998
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
** $Id: close_xsb_i.h,v 1.6 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

/*
 * CLOSE a Cursor
 * --------------
 *  CLOSEing a cursor disables it.  What resources are freed depends on
 *  the settings of the HOLD_CURSOR and RELEASE_CURSOR precompiler options.
 */

switch (cursorHandle) {
 case  0:  EXEC SQL CLOSE C0;  break;
 case  1:  EXEC SQL CLOSE C1;  break;
 case  2:  EXEC SQL CLOSE C2;  break;
 case  3:  EXEC SQL CLOSE C3;  break;
 case  4:  EXEC SQL CLOSE C4;  break;
 case  5:  EXEC SQL CLOSE C5;  break;
 case  6:  EXEC SQL CLOSE C6;  break;
 case  7:  EXEC SQL CLOSE C7;  break;
 case  8:  EXEC SQL CLOSE C8;  break;
 case  9:  EXEC SQL CLOSE C9;  break;
 case 10:  EXEC SQL CLOSE C10;  break;
 case 11:  EXEC SQL CLOSE C11;  break;
 case 12:  EXEC SQL CLOSE C12;  break;
 case 13:  EXEC SQL CLOSE C13;  break;
 case 14:  EXEC SQL CLOSE C14;  break;
 case 15:  EXEC SQL CLOSE C15;  break;
 case 16:  EXEC SQL CLOSE C16;  break;
 case 17:  EXEC SQL CLOSE C17;  break;
 case 18:  EXEC SQL CLOSE C18;  break;
 case 19:  EXEC SQL CLOSE C19;  break;
 case 20:  EXEC SQL CLOSE C20;  break;
}
