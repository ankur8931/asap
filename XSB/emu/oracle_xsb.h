/* File:      oracle_xsb.h
** Author(s): Hasan Davulcu, Ernie Johnson
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
** $Id: oracle_xsb.h,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


#ifdef ORACLE

/* Builtin Codes
   ------------- */
#define ORA_ORACLE_CONNECT	 1
#define ORA_ORACLE_DISCONNECT	 2
#define ORA_ALLOCATE_CURSOR	 3
#define ORA_DEALLOCATE_CURSOR	 4
#define ORA_CLOSE_CURSOR         5
#define ORA_RECLAIM_CURSORS      6
#define ORA_SET_INPUT_VALUE	 7
#define ORA_EXECUTE_CURSOR	 8
#define ORA_FLUSH_CURSOR	 9
#define ORA_FETCH_FROM_CURSOR	10
#define ORA_GET_OUTPUT_VALUE	11
#define ORA_EXECUTE_IMMEDIATE	12

#define ORA_SQLCAID             16
#define ORA_SQLABC              17
#define ORA_SQLCODE		18
#define ORA_SQLERRML		19
#define ORA_SQLERRMC		20
#define ORA_SQLERRP		21
#define ORA_SQLERRD0		22
#define ORA_SQLERRD1		23
#define ORA_SQLERRD2		24
#define ORA_SQLERRD3		25
#define ORA_SQLERRD4		26
#define ORA_SQLERRD5		27
#define ORA_SQLWARN0		28
#define ORA_SQLWARN1		29
#define ORA_SQLWARN3		30
#define ORA_SQLWARN4		31
#define ORA_SQLWARN5		32
#define ORA_SQLEXT		33


/* Function Prototypes
   ------------------- */
extern void oracle_connect();
extern void oracle_disconnect();
extern void allocate_cursor();
extern void deallocate_cursor();
extern void close_cursor();
extern void reclaim_cursors();
extern void set_input_value();
extern void execute_cursor();
extern void flush_cursor();
extern void fetch_from_cursor();
extern void get_output_value();
extern void execute_immediate();

extern void db_sqlcaid();
extern void db_sqlabc();
extern void db_sqlcode();
extern void db_sqlerrml();
extern void db_sqlerrmc();
extern void db_sqlerrp();
extern void db_sqlerrd0();
extern void db_sqlerrd1();
extern void db_sqlerrd2();
extern void db_sqlerrd3();
extern void db_sqlerrd4();
extern void db_sqlerrd5();
extern void db_sqlwarn0();
extern void db_sqlwarn1();
extern void db_sqlwarn3();
extern void db_sqlwarn4();
extern void db_sqlwarn5();
extern void db_sqlext();

#endif
