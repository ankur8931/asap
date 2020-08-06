/*  -*-c-*-  Make sure this file comes up in the C mode of emacs */ 
/* File:      open.i
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
** $Id: open_xsb_i.h,v 1.6 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


/*
 * OPEN a Cursor
 * -------------
 *  OPEN allocates an Oracle cursor, binds input values to placeholders,
 *  executes the associated SQL statement, and if this statement is a
 *  query, identifies the active set and positions the cursor on its
 *  first row.  The rows-processed count of the SQLCA is also zeroed-out.
 */

  switch (cursorHandle) {
  case  0:
    EXEC SQL FOR :numTuples OPEN C0 USING DESCRIPTOR bindDesc;
    break;
  case  1:
    EXEC SQL FOR :numTuples OPEN C1 USING DESCRIPTOR bindDesc;
    break;
  case  2:
    EXEC SQL FOR :numTuples OPEN C2 USING DESCRIPTOR bindDesc;
    break;
  case  3:
    EXEC SQL FOR :numTuples OPEN C3 USING DESCRIPTOR bindDesc;
    break;
  case  4:
    EXEC SQL FOR :numTuples OPEN C4 USING DESCRIPTOR bindDesc;
    break;
  case  5:
    EXEC SQL FOR :numTuples OPEN C5 USING DESCRIPTOR bindDesc;
    break;
  case  6:
    EXEC SQL FOR :numTuples OPEN C6 USING DESCRIPTOR bindDesc;
    break;
  case  7:
    EXEC SQL FOR :numTuples OPEN C7 USING DESCRIPTOR bindDesc;
    break;
  case  8:
    EXEC SQL FOR :numTuples OPEN C8 USING DESCRIPTOR bindDesc;
    break;
  case  9:
    EXEC SQL FOR :numTuples OPEN C9 USING DESCRIPTOR bindDesc;
    break;
  case 10:
    EXEC SQL FOR :numTuples OPEN C10 USING DESCRIPTOR bindDesc;
    break;
  case 11:
    EXEC SQL FOR :numTuples OPEN C11 USING DESCRIPTOR bindDesc;
    break;
  case 12:
    EXEC SQL FOR :numTuples OPEN C12 USING DESCRIPTOR bindDesc;
    break;
  case 13:
    EXEC SQL FOR :numTuples OPEN C13 USING DESCRIPTOR bindDesc;
    break;
  case 14:
    EXEC SQL FOR :numTuples OPEN C14 USING DESCRIPTOR bindDesc;
    break;
  case 15:
    EXEC SQL FOR :numTuples OPEN C15 USING DESCRIPTOR bindDesc;
    break;
  case 16:
    EXEC SQL FOR :numTuples OPEN C16 USING DESCRIPTOR bindDesc;
    break;
  case 17:
    EXEC SQL FOR :numTuples OPEN C17 USING DESCRIPTOR bindDesc;
    break;
  case 18:
    EXEC SQL FOR :numTuples OPEN C18 USING DESCRIPTOR bindDesc;
    break;
  case 19:
    EXEC SQL FOR :numTuples OPEN C19 USING DESCRIPTOR bindDesc;
    break;
  case 20:
    EXEC SQL FOR :numTuples OPEN C20 USING DESCRIPTOR bindDesc;
    break;
  }
