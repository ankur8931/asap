/* File:      oracle_xsb_i.h
** Author(s): Hasan Davulcu, Ernie Johnson
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
** Copyright (C) ECRC, Germany, 1990
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
** $Id: oracle_xsb_i.h,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
** 
*/



#ifdef ORACLE

case ORACLE_QUERY:
  switch (ptoc_int(1)) {
   case ORA_ORACLE_CONNECT:
     oracle_connect();
     break;
   case ORA_ORACLE_DISCONNECT:
     oracle_disconnect();
     break;
   case ORA_ALLOCATE_CURSOR:
     allocate_cursor();
     break;
   case ORA_DEALLOCATE_CURSOR:
     deallocate_cursor();
     break;
   case ORA_CLOSE_CURSOR:
     close_cursor();
     break;
   case ORA_RECLAIM_CURSORS:
     reclaim_cursors();
     break;
   case ORA_SET_INPUT_VALUE:
     set_input_value();
     break;
   case ORA_EXECUTE_CURSOR:
     execute_cursor();
     break;
   case ORA_FLUSH_CURSOR:
     flush_cursor();
     break;
   case ORA_FETCH_FROM_CURSOR:
     fetch_from_cursor();
     break;
   case ORA_GET_OUTPUT_VALUE:
     get_output_value();
     break;
   case ORA_EXECUTE_IMMEDIATE:
     execute_immediate();
     break;
   case ORA_SQLCAID:
     db_sqlcaid();
     break;
   case ORA_SQLABC:
     db_sqlabc();
     break;
   case ORA_SQLCODE:
     db_sqlcode();
     break;
   case ORA_SQLERRML:
     db_sqlerrml();
     break;
   case ORA_SQLERRMC:
     db_sqlerrmc();
     break;
   case ORA_SQLERRP:
     db_sqlerrp();
     break;
   case ORA_SQLERRD0:
     db_sqlerrd0();
     break;
   case ORA_SQLERRD1:
     db_sqlerrd1();
     break;
   case ORA_SQLERRD2:
     db_sqlerrd2();
     break;
   case ORA_SQLERRD3:
     db_sqlerrd3();
     break;
   case ORA_SQLERRD4:
     db_sqlerrd4();
     break;
   case ORA_SQLERRD5:
     db_sqlerrd5();
     break;
   case ORA_SQLWARN0:
     db_sqlwarn0();
     break;
   case ORA_SQLWARN1:
     db_sqlwarn1();
     break;
   case ORA_SQLWARN3:
     db_sqlwarn3();
     break;
   case ORA_SQLWARN4:
     db_sqlwarn4();
     break;
   case ORA_SQLWARN5:
     db_sqlwarn5();
     break;
   case ORA_SQLEXT:
     db_sqlext();
     break;
   default:
     /* Put an error message here */
     break;
  }
  break;

#endif
