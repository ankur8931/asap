/*  -*-c-*-  Make sure this file comes up in the C mode of emacs */ 
/* File:      prepdecl.i
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
** $Id: prepdecl_xsb_i.h,v 1.6 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


/*
 * PREPARE and DECLARE a Cursor for a SQL Statement
 * ------------------------------------------------
 *  PREPARE parses the stmt and associates a symbolic name with it.
 *  DECLARE defines a symbolic name for a cursor and associates it with
 *  the specified SQL statement.
 */

  switch (cursorHandle) {
  case  0:  EXEC SQL PREPARE S0 FROM :sql_stmt;
             EXEC SQL DECLARE C0 CURSOR FOR S0;  break;
  case  1:  EXEC SQL PREPARE S1 FROM :sql_stmt;
             EXEC SQL DECLARE C1 CURSOR FOR S1;  break;
  case  2:  EXEC SQL PREPARE S2 FROM :sql_stmt;
             EXEC SQL DECLARE C2 CURSOR FOR S2;  break;
  case  3:  EXEC SQL PREPARE S3 FROM :sql_stmt;
             EXEC SQL DECLARE C3 CURSOR FOR S3;  break;
  case  4:  EXEC SQL PREPARE S4 FROM :sql_stmt;
             EXEC SQL DECLARE C4 CURSOR FOR S4;  break;
  case  5:  EXEC SQL PREPARE S5 FROM :sql_stmt;
             EXEC SQL DECLARE C5 CURSOR FOR S5;  break;
  case  6:  EXEC SQL PREPARE S6 FROM :sql_stmt;
             EXEC SQL DECLARE C6 CURSOR FOR S6;  break;
  case  7:  EXEC SQL PREPARE S7 FROM :sql_stmt;
             EXEC SQL DECLARE C7 CURSOR FOR S7;  break;
  case  8:  EXEC SQL PREPARE S8 FROM :sql_stmt;
             EXEC SQL DECLARE C8 CURSOR FOR S8;  break;
  case  9:  EXEC SQL PREPARE S9 FROM :sql_stmt;
             EXEC SQL DECLARE C9 CURSOR FOR S9;  break;
  case 10:  EXEC SQL PREPARE S10 FROM :sql_stmt;
             EXEC SQL DECLARE C10 CURSOR FOR S10;  break;
  case 11:  EXEC SQL PREPARE S11 FROM :sql_stmt;
             EXEC SQL DECLARE C11 CURSOR FOR S11;  break;
  case 12:  EXEC SQL PREPARE S12 FROM :sql_stmt;
             EXEC SQL DECLARE C12 CURSOR FOR S12;  break;
  case 13:  EXEC SQL PREPARE S13 FROM :sql_stmt;
             EXEC SQL DECLARE C13 CURSOR FOR S13;  break;
  case 14:  EXEC SQL PREPARE S14 FROM :sql_stmt;
             EXEC SQL DECLARE C14 CURSOR FOR S14;  break;
  case 15:  EXEC SQL PREPARE S15 FROM :sql_stmt;
             EXEC SQL DECLARE C15 CURSOR FOR S15;  break;
  case 16:  EXEC SQL PREPARE S16 FROM :sql_stmt;
             EXEC SQL DECLARE C16 CURSOR FOR S16;  break;
  case 17:  EXEC SQL PREPARE S17 FROM :sql_stmt;
             EXEC SQL DECLARE C17 CURSOR FOR S17;  break;
  case 18:  EXEC SQL PREPARE S18 FROM :sql_stmt;
             EXEC SQL DECLARE C18 CURSOR FOR S18;  break;
  case 19:  EXEC SQL PREPARE S19 FROM :sql_stmt;
             EXEC SQL DECLARE C19 CURSOR FOR S19;  break;
  case 20:  EXEC SQL PREPARE S20 FROM :sql_stmt;
             EXEC SQL DECLARE C20 CURSOR FOR S20;  break;
  }
