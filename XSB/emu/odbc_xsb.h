/* File:      odbc_xsb.h
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
** $Id: odbc_xsb.h,v 1.14 2012-12-08 18:21:36 dwarren Exp $
**
*/

#ifndef __ODBC_XSB_H__
#define __ODBC_XSB_H__

#ifdef XSB_ODBC

#include "odbc_def_xsb.h"

//the function declarations are kept seperate from the constant and structure declarations
//of ODBC_XSB since each function depends on CTXTdecl from context.h, but context.h depends on the 
//structure declarations of ODBC_XSB.
extern void ODBCConnect(CTXTdecl);
extern void ODBCDisconnect(CTXTdecl);
extern void SetBindVarNum(CTXTdecl);
extern void FindFreeCursor(CTXTdecl);
extern void SetBindVal(CTXTdecl);
extern void Parse(CTXTdecl);
extern int  GetColumn(CTXTdecl);
extern void SetCursorClose(struct ODBC_Cursor *);
extern void FetchNextRow(CTXTdecl);
extern void ODBCCommit(CTXTdecl);
extern void ODBCRollback(CTXTdecl);
extern void ODBCColumns(CTXTdecl);
extern void ODBCTables(CTXTdecl);
extern void ODBCUserTables(CTXTdecl);
extern void ODBCDescribeSelect(CTXTdecl);
extern void ODBCConnectOption(CTXTdecl);
extern void ODBCDataSources(CTXTdecl);
extern void ODBCDrivers(CTXTdecl);
extern void ODBCGetInfo(CTXTdecl);
extern void ODBCRowCount(CTXTdecl);

#endif /*XSB_ODBC defined*/

#endif /*this header included*/

