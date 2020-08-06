/* File:      odbc_xsb_i.h
** Author(s): Lily Dong
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
** $Id: odbc_xsb_i.h,v 1.15 2012-12-08 18:21:37 dwarren Exp $
**
*/


#ifdef XSB_ODBC

case ODBC_EXEC_QUERY:
switch (ptoc_int(CTXTc 1)) {
 case ODBC_CONNECT:
   ODBCConnect(CTXT);
   break;
 case ODBC_PARSE:
   Parse(CTXT);
   break;
 case ODBC_SET_BIND_VAR_NUM:
   SetBindVarNum(CTXT);
   break;
 case ODBC_FETCH_NEXT_ROW:
   FetchNextRow(CTXT);
   break;
 case ODBC_GET_COLUMN:
   return GetColumn(CTXT);
   break;
 case ODBC_SET_BIND_VAL:
   SetBindVal(CTXT);
   break;
 case ODBC_FIND_FREE_CURSOR:
   FindFreeCursor(CTXT);
   break;
 case ODBC_DISCONNECT:
   ODBCDisconnect(CTXT);
   break;
 case ODBC_SET_CURSOR_CLOSE: {
   SetCursorClose((struct ODBC_Cursor *)ptoc_int(CTXTc 2));
   break;
 }
 case ODBC_COMMIT:
   ODBCCommit(CTXT);
   break;
 case ODBC_ROLLBACK:
   ODBCRollback(CTXT);
   break;
 case ODBC_COLUMNS:
   ODBCColumns(CTXT);
   break;
 case ODBC_TABLES:
   ODBCTables(CTXT);
   break;
 case ODBC_USER_TABLES:
   ODBCUserTables(CTXT);
   break;
 case ODBC_DESCRIBE_SELECT:
   ODBCDescribeSelect(CTXT);
   break;
 case ODBC_CONNECT_OPTION:
   ODBCConnectOption(CTXT);
   break;
 case ODBC_DATA_SOURCES:
   ODBCDataSources(CTXT);
   break;
 case ODBC_DRIVERS:
   ODBCDrivers(CTXT);
   break;
 case ODBC_GET_INFO:
   ODBCGetInfo(CTXT);
   break;
 case ODBC_ROW_COUNT:
   ODBCRowCount(CTXT);
   break;
 default:
   xsb_error("Unknown or unimplemented ODBC request type");
   /* Put an error message here */
   break;
}
break;

#endif


