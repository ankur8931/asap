/*  -*-c-*-  Make sure this file comes up in the C mode of emacs */ 
/* File:      fetch.i
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
** $Id: fetch_xsb_i.h,v 1.6 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


switch (cursorHandle) {
 case 0:
  EXEC SQL FOR :arrayLength FETCH C0 USING DESCRIPTOR sliDesc;  break;
 case 1:
  EXEC SQL FOR :arrayLength FETCH C1 USING DESCRIPTOR sliDesc;  break;
 case 2:
  EXEC SQL FOR :arrayLength FETCH C2 USING DESCRIPTOR sliDesc;  break;
 case 3:
  EXEC SQL FOR :arrayLength FETCH C3 USING DESCRIPTOR sliDesc;  break;
 case 4:
  EXEC SQL FOR :arrayLength FETCH C4 USING DESCRIPTOR sliDesc;  break;
 case 5:
  EXEC SQL FOR :arrayLength FETCH C5 USING DESCRIPTOR sliDesc;  break;
 case 6:
  EXEC SQL FOR :arrayLength FETCH C6 USING DESCRIPTOR sliDesc;  break;
 case 7:
  EXEC SQL FOR :arrayLength FETCH C7 USING DESCRIPTOR sliDesc;  break;
 case 8:
  EXEC SQL FOR :arrayLength FETCH C8 USING DESCRIPTOR sliDesc;  break;
 case 9:
  EXEC SQL FOR :arrayLength FETCH C9 USING DESCRIPTOR sliDesc;  break;
 case 10:
  EXEC SQL FOR :arrayLength FETCH C10 USING DESCRIPTOR sliDesc;  break;
 case 11:
  EXEC SQL FOR :arrayLength FETCH C11 USING DESCRIPTOR sliDesc;  break;
 case 12:
  EXEC SQL FOR :arrayLength FETCH C12 USING DESCRIPTOR sliDesc;  break;
 case 13:
  EXEC SQL FOR :arrayLength FETCH C13 USING DESCRIPTOR sliDesc;  break;
 case 14:
  EXEC SQL FOR :arrayLength FETCH C14 USING DESCRIPTOR sliDesc;  break;
 case 15:
  EXEC SQL FOR :arrayLength FETCH C15 USING DESCRIPTOR sliDesc;  break;
 case 16:
  EXEC SQL FOR :arrayLength FETCH C16 USING DESCRIPTOR sliDesc;  break;
 case 17:
  EXEC SQL FOR :arrayLength FETCH C17 USING DESCRIPTOR sliDesc;  break;
 case 18:
  EXEC SQL FOR :arrayLength FETCH C18 USING DESCRIPTOR sliDesc;  break;
 case 19:
  EXEC SQL FOR :arrayLength FETCH C19 USING DESCRIPTOR sliDesc;  break;
 case 20:
  EXEC SQL FOR :arrayLength FETCH C20 USING DESCRIPTOR sliDesc;  break;
}
