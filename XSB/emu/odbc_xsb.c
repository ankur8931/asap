/* File:      odbc_xsb.c
** Author(s): Lily Dong, David S. Warren
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
** $Id: odbc_xsb.c,v 1.83 2013-05-15 22:03:08 dwarren Exp $
**
*/

#include "xsb_config.h"
#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "cell_xsb_i.h"

#ifdef CYGWIN
#define FAR
#include "windows.h" // main win32api include for Cygwin
#include "sql.h"
#include "sqlext.h"
#include "odbc_string.h"
#else
#ifdef WIN_NT
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#endif

#include <stdio.h>

#include <stdlib.h>
#include <assert.h>

#include "cinterf.h"
#include "deref.h"

#include "error_xsb.h"
#include "export.h"
#include "register.h"
#include "ptoc_tag_xsb_i.h"
#include "io_builtins_xsb.h"
#include "flags_xsb.h"
#include "flag_defs_xsb.h"
#include "loader_xsb.h"
#include "memory_xsb.h"
#include "heap_xsb.h"
//#include "error_xsb.h"
#include "varstring_xsb.h"
#include "thread_xsb.h"

#define MAXCURSORNUM                    200
#define MAXVARSTRLEN                    65000
#define MAXI(a,b)                       ((a)>(b)?(a):(b))

extern xsbBool unify(CTXTdecltypec Cell, Cell);

static Psc     nullFctPsc = NULL;
/* static int      numberOfCursors = 0; */
static SQLLEN      SQL_NTSval = SQL_NTS;
static SQLLEN      SQL_NULL_DATAval = SQL_NULL_DATA;

/* NOTE, THIS IS NOT THREAD-SAFE */
static HENV henv = NULL;
/*HDBC hdbc;*/

struct ODBC_Cursor {
  struct ODBC_Cursor *NCursor; /* Next ODBC_Cursor in cursor chain*/
  struct ODBC_Cursor *PCursor; /* Prev ODBC_Cursor in cursor chain*/
  HDBC hdbc;            /* connection handle for this cursor*/
  int driver_code;	/* our code for driver, to handle ODBC inconsistencies, set in FindFreeCursor */
			/* = 0 default, = 1 for MS Access */
  int Status;           /* status of the cursor*/
  UCHAR *Sql;           /* pointer to the sql statement*/
  HSTMT hstmt;          /* the statement handle*/
  int NumBindVars;      /* number of bind values*/
  UCHAR **BindList;     /* pointer to array of pointers to the bind values*/
  int *BindTypes;       /* types of the bind values, 0->int, 1->float, 2->char, 3->nullvalue(anytype)*/
  SQLLEN *BindLens; /* lengths of the bind values that are strings */
  SWORD NumCols;        /* number of columns selected*/
  SWORD *ColTypes;      /* pointer to array of column types*/
  SQLULEN *ColLen;       /* pointer to array of max column lengths*/
  SQLLEN *OutLen;       /* pointer to array of actual column lengths*/
  UCHAR **Data;         /* pointer to array of pointers to data*/
};

/* Number of Cursors per Connection */
struct NumberofCursors{
  HDBC hdbc;
  int CursorCount;
  struct NumberofCursors *NCurNum;
};

//The below variables are declared global for single-threaded XSB, and declaired
//in thread context for multi-threaded XSB.
#ifndef MULTI_THREAD
struct NumberofCursors *FCurNum; /* First in the list of Number of Cursors */
/* global cursor table*/
struct ODBC_Cursor *FCursor;  /* root of curser chain*/
struct ODBC_Cursor *LCursor;  /* tail of curser chain*/

#define MAX_BIND_VALS 30
char *term_string[MAX_BIND_VALS] = {0};
#endif

/* for debugging: just dumps memory... */
void print_hdbc(char *msg, HDBC hdbc) {
  Integer *ptr;
  int wds = 48;

  printf("\n%s\nhdbc: %p",msg,hdbc);
  for(ptr = (Integer *)(((Integer)hdbc / 32) * 32); ptr < (Integer *)hdbc+wds ; ptr++) {
    if (((Integer)ptr % 32) == 0) printf("\n%p: ",ptr);
    printf(" %"Intxfmt,*ptr);
  }
  printf("\n\n");
}

SWORD ODBCToXSBType(SWORD odbcType)
{
  switch (odbcType) {
  case SQL_TINYINT:
  case SQL_SMALLINT:
  case SQL_INTEGER:
    return SQL_C_SLONG;
  case SQL_DECIMAL:
  case SQL_NUMERIC:
  case SQL_REAL:
  case SQL_FLOAT:
  case SQL_DOUBLE:
    return SQL_C_FLOAT;
  case SQL_BINARY:
  case SQL_VARBINARY:
  case SQL_LONGVARBINARY:
    return SQL_C_BINARY;
  case SQL_DATE:
  case SQL_TIME:
  case SQL_TIMESTAMP:
  case SQL_CHAR:
  case SQL_VARCHAR:
  default:
    return SQL_C_CHAR;
  }
}

int GetInfoTypeType(int SQL_INFO_TYPE)
{	//-1: Unkown
	//0 : String
	//1 : SQLUSMALLINT
	//2 : SQLUINTEGER
	//3 : SQLUINTEGER bitmask
	//4 : SQLUINTEGER flag

	int type;
	switch(SQL_INFO_TYPE) {
		case SQL_ACCESSIBLE_PROCEDURES:
		case SQL_ACCESSIBLE_TABLES:
#if (ODBCVER >= 0x0300)
		case SQL_CATALOG_NAME:
        case SQL_COLLATION_SEQ:
#endif        
		case SQL_CATALOG_NAME_SEPARATOR:
		case SQL_CATALOG_TERM:
		case SQL_COLUMN_ALIAS:
		case SQL_DATA_SOURCE_NAME:
		case SQL_DATA_SOURCE_READ_ONLY:
		case SQL_DATABASE_NAME:
		case SQL_DBMS_NAME:
		case SQL_DBMS_VER:
		case SQL_DESCRIBE_PARAMETER:
		case SQL_DM_VER:
		case SQL_DRIVER_NAME:
		case SQL_DRIVER_ODBC_VER:
		case SQL_DRIVER_VER:
		case SQL_EXPRESSIONS_IN_ORDERBY:
		case SQL_IDENTIFIER_QUOTE_CHAR:
		case SQL_INTEGRITY:
		case SQL_KEYWORDS:
		case SQL_LIKE_ESCAPE_CLAUSE:
		case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
		case SQL_MULT_RESULT_SETS:
		case SQL_MULTIPLE_ACTIVE_TXN:
		case SQL_NEED_LONG_DATA_LEN:
		case SQL_ODBC_VER:
		case SQL_ORDER_BY_COLUMNS_IN_SELECT:
		case SQL_PROCEDURE_TERM:
		case SQL_PROCEDURES:
		case SQL_ROW_UPDATES:
		case SQL_SCHEMA_TERM:
		case SQL_SEARCH_PATTERN_ESCAPE:
		case SQL_SERVER_NAME:
		case SQL_SPECIAL_CHARACTERS:
		case SQL_TABLE_TERM:
		case SQL_USER_NAME:
		case SQL_XOPEN_CLI_YEAR:
			type = 0;
			break;
#if (ODBCVER >= 0x0300)
		case SQL_ACTIVE_ENVIRONMENTS:
#endif        
		case SQL_CATALOG_LOCATION:
		case SQL_CONCAT_NULL_BEHAVIOR:
		case SQL_CORRELATION_NAME:
		case SQL_CURSOR_COMMIT_BEHAVIOR:
		case SQL_CURSOR_ROLLBACK_BEHAVIOR:
		case SQL_FILE_USAGE:
		case SQL_GROUP_BY:
		case SQL_IDENTIFIER_CASE:
		case SQL_MAX_CATALOG_NAME_LEN:
		case SQL_MAX_COLUMN_NAME_LEN:
		case SQL_MAX_COLUMNS_IN_GROUP_BY:
		case SQL_MAX_COLUMNS_IN_INDEX:
		case SQL_MAX_COLUMNS_IN_ORDER_BY:
		case SQL_MAX_COLUMNS_IN_SELECT:
		case SQL_MAX_COLUMNS_IN_TABLE:
		case SQL_MAX_CONCURRENT_ACTIVITIES:
		case SQL_MAX_CURSOR_NAME_LEN:
		case SQL_MAX_DRIVER_CONNECTIONS:
		case SQL_MAX_IDENTIFIER_LEN:
		case SQL_MAX_PROCEDURE_NAME_LEN:
		case SQL_MAX_SCHEMA_NAME_LEN:
		case SQL_MAX_TABLE_NAME_LEN:
		case SQL_MAX_TABLES_IN_SELECT:
		case SQL_MAX_USER_NAME_LEN:
		case SQL_NON_NULLABLE_COLUMNS:
		case SQL_NULL_COLLATION:
		case SQL_QUOTED_IDENTIFIER_CASE:
		case SQL_TXN_CAPABLE:
			type = 1;
			break;
#if (ODBCVER >= 0x0300)
		case SQL_ASYNC_MODE:
		//case SQL_CURSOR_ROLLBACK_SQL_CURSOR_SENSITIVITY:
		case SQL_DDL_INDEX:
#endif        
		case SQL_DEFAULT_TXN_ISOLATION:
		case SQL_DRIVER_HDBC:
		case SQL_DRIVER_HENV:
		case SQL_DRIVER_HDESC:
		case SQL_DRIVER_HLIB:
		case SQL_DRIVER_HSTMT:
		case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS:
		case SQL_MAX_BINARY_LITERAL_LEN:
		case SQL_MAX_CHAR_LITERAL_LEN:
		case SQL_MAX_INDEX_SIZE:
		case SQL_MAX_ROW_SIZE:
		case SQL_MAX_STATEMENT_LEN:
		case SQL_ODBC_INTERFACE_CONFORMANCE:
		case SQL_SQL_CONFORMANCE:
			type = 2;
			break;

		case SQL_ALTER_TABLE:
		case SQL_BOOKMARK_PERSISTENCE:
		case SQL_CATALOG_USAGE:
		case SQL_CONVERT_BIGINT:
		case SQL_CONVERT_BINARY:
		case SQL_CONVERT_BIT:
		case SQL_CONVERT_CHAR:
#ifdef SQL_CONVERT_GUID
		case SQL_CONVERT_GUID:
#endif        
		case SQL_CONVERT_DATE:
		case SQL_CONVERT_DECIMAL:
		case SQL_CONVERT_DOUBLE:
		case SQL_CONVERT_FLOAT:
		case SQL_CONVERT_INTEGER:
#if (ODBCVER >= 0x0300)
		case SQL_AGGREGATE_FUNCTIONS:
		case SQL_ALTER_DOMAIN:
		case SQL_BATCH_ROW_COUNT:
		case SQL_BATCH_SUPPORT:
		case SQL_CONVERT_INTERVAL_YEAR_MONTH:
		case SQL_CONVERT_INTERVAL_DAY_TIME:
		case SQL_CREATE_ASSERTION:
		case SQL_CREATE_CHARACTER_SET:
		case SQL_CREATE_COLLATION:
		case SQL_CREATE_DOMAIN:
		case SQL_CREATE_SCHEMA:
		case SQL_CREATE_TABLE:
		case SQL_CREATE_TRANSLATION:
		case SQL_CREATE_VIEW:
		case SQL_DATETIME_LITERALS:
#endif
		case SQL_CONVERT_LONGVARBINARY:
		case SQL_CONVERT_LONGVARCHAR:
		case SQL_CONVERT_NUMERIC:
		case SQL_CONVERT_REAL:
		case SQL_CONVERT_SMALLINT:
		case SQL_CONVERT_TIME:
		case SQL_CONVERT_TIMESTAMP:
		case SQL_CONVERT_TINYINT:
		case SQL_CONVERT_VARBINARY:
		case SQL_CONVERT_VARCHAR:
		case SQL_CONVERT_FUNCTIONS:
		case SQL_DROP_ASSERTION:
		case SQL_DROP_CHARACTER_SET:
		case SQL_DROP_COLLATION:
		case SQL_DROP_DOMAIN:
		case SQL_DROP_SCHEMA:
		case SQL_DROP_TABLE:
		case SQL_DROP_TRANSLATION:
		case SQL_DROP_VIEW:
		case SQL_DYNAMIC_CURSOR_ATTRIBUTES1:
		case SQL_DYNAMIC_CURSOR_ATTRIBUTES2:
		case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1:
		case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2:
		case SQL_GETDATA_EXTENSIONS:
		case SQL_INDEX_KEYWORDS:
		case SQL_INFO_SCHEMA_VIEWS:
		case SQL_INSERT_STATEMENT:
		case SQL_KEYSET_CURSOR_ATTRIBUTES1:
		case SQL_KEYSET_CURSOR_ATTRIBUTES2:
		case SQL_NUMERIC_FUNCTIONS:
		case SQL_OJ_CAPABILITIES:
		case SQL_PARAM_ARRAY_ROW_COUNTS:
		case SQL_PARAM_ARRAY_SELECTS:
		case SQL_POS_OPERATIONS:
		case SQL_SCHEMA_USAGE:
		case SQL_SCROLL_OPTIONS:
		case SQL_SQL92_DATETIME_FUNCTIONS:
		case SQL_SQL92_FOREIGN_KEY_DELETE_RULE:
		case SQL_SQL92_FOREIGN_KEY_UPDATE_RULE:
		case SQL_SQL92_GRANT:
		case SQL_SQL92_NUMERIC_VALUE_FUNCTIONS:
		case SQL_SQL92_PREDICATES:
		case SQL_SQL92_RELATIONAL_JOIN_OPERATORS:
		case SQL_SQL92_REVOKE:
		case SQL_SQL92_ROW_VALUE_CONSTRUCTOR:
		case SQL_SQL92_STRING_FUNCTIONS:
		case SQL_SQL92_VALUE_EXPRESSIONS:
		case SQL_STANDARD_CLI_CONFORMANCE:
		case SQL_STATIC_CURSOR_ATTRIBUTES1:
		case SQL_STATIC_CURSOR_ATTRIBUTES2:
		case SQL_STRING_FUNCTIONS:
		case SQL_SUBQUERIES:
		case SQL_SYSTEM_FUNCTIONS:
		case SQL_TIMEDATE_ADD_INTERVALS:
		case SQL_TIMEDATE_DIFF_INTERVALS:
		case SQL_TIMEDATE_FUNCTIONS:
		case SQL_TXN_ISOLATION_OPTION:
		case SQL_UNION:
			type = 3;
			break;
		default:
			type= -1;
	}
	return type;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     PrintErrorMsg()*/
/*  PARAMETERS:*/
/*     struct ODBC_Cursor *c - pointer to a cursor, or NULL*/
/*  NOTES:*/
/*     PrintErrorMsg() prints out the error message that associates*/
/*     with the statement handler of cursor i.*/
/*-----------------------------------------------------------------------------*/
Cell PrintErrorMsg(CTXTdeclc struct ODBC_Cursor *cur)
{
  UCHAR FAR *szsqlstate;
  SDWORD FAR *pfnativeerror;
  UCHAR FAR *szerrormsg;
  SWORD cberrormsgmax;
  SWORD FAR *pcberrormsg;
  RETCODE rc;
  Cell term;
  int isnew;

  szsqlstate=(UCHAR FAR *)mem_alloc(sizeof(UCHAR FAR)*10,LEAK_SPACE);
  pfnativeerror=(SDWORD FAR *)mem_alloc(sizeof(SDWORD FAR),LEAK_SPACE);
  szerrormsg=(UCHAR FAR *)mem_alloc(sizeof(UCHAR FAR)*SQL_MAX_MESSAGE_LENGTH,LEAK_SPACE);
  pcberrormsg=(SWORD FAR *)mem_alloc(sizeof(SWORD FAR),LEAK_SPACE);
  cberrormsgmax=SQL_MAX_MESSAGE_LENGTH-1;
  if (cur != NULL)
    rc = SQLError(SQL_NULL_HENV, cur->hdbc, cur->hstmt, szsqlstate,
		  pfnativeerror, szerrormsg,cberrormsgmax,pcberrormsg);
  else
    rc = SQLError(SQL_NULL_HENV, NULL, SQL_NULL_HSTMT, szsqlstate,
		  pfnativeerror, szerrormsg,cberrormsgmax,pcberrormsg);
  if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO)) {
    term = makecs(hreg);
    bld_functor(hreg, pair_psc(insert("odbc_error",2,(Psc)flags[CURRENT_MODULE],&isnew)));
    bld_string(hreg+1,string_find((char *)szsqlstate,1)); 
    bld_string(hreg+2,string_find((char *)szerrormsg,1)); 
    hreg += 3;
  } else {
    term = makestring(string_find("Unknown ODBC Error",1));
  }
  mem_dealloc(szsqlstate,sizeof(UCHAR FAR)*10,LEAK_SPACE);
  mem_dealloc(pfnativeerror,sizeof(SDWORD FAR),LEAK_SPACE);
  mem_dealloc(szerrormsg,sizeof(UCHAR FAR)*SQL_MAX_MESSAGE_LENGTH,LEAK_SPACE);
  mem_dealloc(pcberrormsg,sizeof(SWORD FAR),LEAK_SPACE);
  return term;
}

/*------------------------------------------------------------------------------*/
/*  FUNCTION NAME:								*/
/*     GenErrorMsgBall()							*/
/*  PARAMETERS:									*/
/*     char * - error message to put into ball					*/
/*------------------------------------------------------------------------------*/
Cell GenErrorMsgBall(char *errmsg) {
  return makestring(string_find(errmsg,1));
}


void free_cur_bindlist(struct ODBC_Cursor *cur, int j) {
  if (cur->BindTypes[j] == 0) {
    mem_dealloc(cur->BindList[j],sizeof(Integer),ODBC_SPACE);
  } else if (cur->BindTypes[j] == 1) {
    mem_dealloc(cur->BindList[j],sizeof(Float),ODBC_SPACE);
  }
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     SetCursorClose()*/
/*  PARAMETER:*/
/*     struct ODBC_Cursor *cur - pointer to current cursor*/
/*  NOTES:*/
/*     free all the memory resource allocated for cursor cur*/
/*-----------------------------------------------------------------------------*/
void SetCursorClose(struct ODBC_Cursor *cur)
{
  int j;

  SQLFreeStmt(cur->hstmt, SQL_CLOSE);    /* free statement handler*/

  if (cur->NumBindVars) {                 /* free bind variable list*/
    for (j = 0; j < cur->NumBindVars; j++) {
      if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);
    }
    mem_dealloc(cur->BindList,sizeof(UCHAR *) * cur->NumBindVars,ODBC_SPACE);
    mem_dealloc(cur->BindTypes,sizeof(int) * cur->NumBindVars,ODBC_SPACE);
    mem_dealloc(cur->BindLens,sizeof(SQLLEN) * cur->NumBindVars,ODBC_SPACE);
  }

  if (cur->NumCols) {                  /* free the resulting row set*/
    for (j = 0; j < cur->NumCols; j++) {
      mem_dealloc(cur->Data[j],((unsigned) cur->ColLen[j]+1)*sizeof(UCHAR),ODBC_SPACE);
    }
    mem_dealloc(cur->ColTypes,sizeof(SWORD) * cur->NumCols,ODBC_SPACE);
    mem_dealloc(cur->ColLen,sizeof(SQLULEN) * cur->NumCols,ODBC_SPACE);
    mem_dealloc(cur->OutLen,sizeof(SQLLEN) * cur->NumCols,ODBC_SPACE);
    mem_dealloc(cur->Data,sizeof(UCHAR *) * cur->NumCols,ODBC_SPACE);
  }

  /* free memory for the sql statement associated w/ this cursor*/
  if (cur->Sql) {
    mem_dealloc(cur->Sql,strlen((char *)cur->Sql)+1,ODBC_SPACE);
  }
  /* initialize the variables.  set them to the right value*/
  cur->Sql = 0;
  cur->NumCols =
    cur->Status =
    cur->NumBindVars = 0;
}

XSB_StrDefine(odbc_err_msg);

/* taken from easysoft, web  */
char *detailed_error(char *msg1, char *msg2, SQLHANDLE handle, SQLSMALLINT type) {
    SQLSMALLINT i = 0;
    SQLINTEGER native;
    SQLCHAR state[ 7 ];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;
    char msg_segment[250];

    XSB_StrSet(&odbc_err_msg,msg1);
    XSB_StrAppend(&odbc_err_msg,msg2);
    XSB_StrAppend(&odbc_err_msg,": Diagnostics:");
    XSB_StrAppend(&odbc_err_msg,msg_segment);
    do {
      ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
			  sizeof(text), &len );
      if (SQL_SUCCEEDED(ret)) {
	snprintf(msg_segment,250,"::%s:%d:%ld:%s", state, i, (long)native, text);
        XSB_StrAppend(&odbc_err_msg,msg_segment);
      }
    } while( ret == SQL_SUCCESS );
    return odbc_err_msg.string;
  }

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCConnect()*/
/*  PARAMETERS:*/
/*     R1: 1*/
/*     R2: 0 -> call connect with server/user/pw */
/*         1 -> call driver_connect with full connection string */
/*     R3: Server or connection_string*/
/*     R4: User name or don't care*/
/*     R5: Password or don't care*/
/*     R6: var, for returned Connection ID, or 0 if error.*/
/*     R7: RetCode: 0 => Successful, otw error.*/
/*  NOTES:*/
/*     This function is called when a user wants to start a db session,*/
/*     assuming that she doesn't have one open.  It initializes system*/
/*     resources for the new session, including allocations of various things:*/
/*     environment handler, connection handler, statement handlers and then*/
/*     tries to connect to the database, either by server,userid,pw if R2=0,*/
/*     or thrugh a driver by using r3 as the connections tring, if R2=1. */
/*     If any of these allocations or connection fails, function returns a*/
/*     connection code 0.  Otherwise, the connection ID. */
/*-----------------------------------------------------------------------------*/
void ODBCConnect(CTXTdecl)
{
  UCHAR uid[128];
  UCHAR *server;
  UCHAR *pwd;
  UCHAR *connectIn;
  HDBC hdbc = NULL;
  RETCODE rc;
  int new;

  /* if we don't yet have an environment, allocate one.*/
  //locked to prevent two threads from fighting over who creates the env.
  SYS_MUTEX_LOCK( MUTEX_ODBC) ;
  if (!henv) {
    /* allocate environment handler*/
    rc = SQLAllocEnv(&henv);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      ctop_int(CTXTc 6, 0);
      unify(CTXTc reg_term(CTXTc 7), makestring(string_find("Environment allocation failed",1)));
      return;
    }
    /*    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC2,
		SQL_IS_UINTEGER);
    */

    LCursor = FCursor = NULL;
    FCurNum = NULL;
    if (nullFctPsc == NULL)
        nullFctPsc = pair_psc(insert("NULL",1,global_mod,&new));
  }
  SYS_MUTEX_UNLOCK( MUTEX_ODBC) ;

  /* allocate connection handler*/
  rc = SQLAllocConnect(henv, &hdbc);
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
    ctop_int(CTXTc 6, 0);
    unify(CTXTc reg_term(CTXTc 7), makestring(string_find("Connection Resources Allocation Failed",1)));
    return;
  }

  if (!ptoc_int(CTXTc 2)) {
    /* get server name, user id and password*/
    server = (UCHAR *)ptoc_string(CTXTc 3);
    strcpy((char *)uid, ptoc_string(CTXTc 4));
    pwd = (UCHAR *)ptoc_string(CTXTc 5);
    
    /* connect to database*/
    rc = SQLConnect(hdbc, server, SQL_NTS, uid, SQL_NTS, pwd, SQL_NTS);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      char *temp_msg;
      temp_msg = detailed_error("Connection to server failed: ",(char *)server,hdbc,SQL_HANDLE_DBC);
      SQLFreeConnect(hdbc);
      ctop_int(CTXTc 6, 0);
      unify(CTXTc reg_term(CTXTc 7), makestring(string_find(temp_msg,1)));
      return;
    }
  } else {
    /* connecting through driver using a connection string */
    connectIn = (UCHAR *)ptoc_longstring(CTXTc 3);
    rc = SQLDriverConnect(hdbc, NULL, connectIn, SQL_NTS, NULL, 0, NULL,SQL_DRIVER_NOPROMPT);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      char *temp_msg;
      temp_msg = detailed_error("Connection to driver failed: ",(char *)connectIn,hdbc,SQL_HANDLE_DBC);
      SQLFreeConnect(hdbc);
      ctop_int(CTXTc 6, 0);
      unify(CTXTc reg_term(CTXTc 7), makestring(string_find(temp_msg,1)));
      return;
    }
  }
  
  ctop_int(CTXTc 6, (UInteger)hdbc);
  ctop_int(CTXTc 7, 0);
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCDisconnect()    */
/*  PARAMETERS:*/
/*     R1: 2*/
/*     R2: hdbc if closing a particular connection*/
/*         0 if closing entire ODBC session.*/
/*     R3: var RetCode 0 if OK, != if error */
/*  NOTES:*/
/*     Disconnect us from the server and free all the system resources we*/
/*     allocated for the session - statement handlers, connection handler,*/
/*     environment handler and memory space. */
/*-----------------------------------------------------------------------------*/
void ODBCDisconnect(CTXTdecl)
{
  struct ODBC_Cursor *cur = FCursor;
  struct ODBC_Cursor *tcur;
  struct NumberofCursors *numi = FCurNum, *numj = FCurNum;
  HDBC hdbc = (HDBC)ptoc_int(CTXTc 2);
  RETCODE rc;
  int i;

  rc = SQLTransact(henv,hdbc,SQL_COMMIT);
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
    char *err_msg;
    err_msg = detailed_error("Error committing transaction","",hdbc,SQL_HANDLE_DBC);
    unify(CTXTc reg_term(CTXTc 3), GenErrorMsgBall(err_msg));
    return;
  }

  if (hdbc == NULL) {  /* close entire connection*/
    if (FCursor != NULL) {
      unify(CTXTc reg_term(CTXTc 3), GenErrorMsgBall("Must close all connections before shutting down"));
      return;
    }
    SQLFreeEnv(henv);
    ctop_int(CTXTc 3, 0);
    return;
  }

  /* only free cursors associated with this connection (hdbc)*/
  while((numj != NULL) && (numj->hdbc != hdbc)){
    if(numj != FCurNum) numi=numi->NCurNum;
    numj=numj->NCurNum;
  }

  if(numj != NULL){
    if(numj == FCurNum) FCurNum=numj->NCurNum;
    else numi->NCurNum=numj->NCurNum;
    mem_dealloc(numj,sizeof(struct NumberofCursors),ODBC_SPACE);
  }

  while (cur != NULL) {
    if (cur->hdbc == hdbc) {
      tcur = cur->NCursor;
      if (cur->Status != 0) {
	SetCursorClose(cur);
      }
      SQLFreeStmt(cur->hstmt,SQL_DROP);
      if (cur->PCursor) (cur->PCursor)->NCursor = cur->NCursor;
      else FCursor = cur->NCursor;
      if (cur->NCursor) (cur->NCursor)->PCursor = cur->PCursor;
      else LCursor = cur->PCursor;
      mem_dealloc(cur,sizeof(struct ODBC_Cursor),ODBC_SPACE);
      /*      (num->CursorCount)-- */
      cur = tcur;
    }
    else cur = cur->NCursor;
  }

  for (i=0; i<MAX_BIND_VALS; i++) {
    if (term_string[i]) {
      mem_dealloc(term_string[i],strlen(term_string[i])+1,ODBC_SPACE);
    }
  }

  SQLDisconnect(hdbc);
  SQLFreeConnect(hdbc);
  /*  SQLFreeEnv(henv);*/
  ctop_int(CTXTc 3, 0);
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     FindFreeCursor()*/
/*  PARAMETERS:*/
/*     R1: 7*/
/*     R2: Connection Handle*/
/*     R3: SQL Statement*/
/*     R4: var, in which ODBC_Cursor addr is returned*/
/*     R5: var, Retcode 0->OK, otw error*/
/*  NOTES:*/
/*     Find a free statement handler and return its index number into the*/
/*     global cursor table.  It gives priority to a closed cursor with same*/
/*     stantement number over ordinary closed cursors.  If there is no handler*/
/*     left,  function returns NULL. possible cursor status values are*/
/*     0 - never been used - no resource associated w/ the cursor */
/*     1 - used before but having been closed-the cursor has all the resource*/
/*     2 - reusing a used cursor w/ the same statement number, no resource*/
/*         needs to be allocated*/
/*     3 - using a cursor that has no resource - it needs to be allocated*/
/*-----------------------------------------------------------------------------*/
void FindFreeCursor(CTXTdecl)
{
  struct ODBC_Cursor *curi = FCursor, *curj = NULL, *curk = NULL;
  struct NumberofCursors *num = FCurNum;
  HDBC hdbc = (HDBC)ptoc_int(CTXTc 2);
  char *Sql_stmt = ptoc_longstring(CTXTc 3);
  RETCODE rc;
  char drname[25]; SQLSMALLINT drnamelen;

  /* search */
  while (curi != NULL) {
    if (curi->hdbc == hdbc) { /* only look at stmt handles for this connection */
      if (curi->Status == 0) curj = curi; /* cursor never been used*/
      else {
	if (curi->Status == 1) {    /* a closed cursor*/
	  /* same statement as this one, so grab and return it*/
	  if (!strcmp((char *)curi->Sql,Sql_stmt)) {
	    if (curi != FCursor) {
	      (curi->PCursor)->NCursor = curi->NCursor;
	      if (curi == LCursor) LCursor = curi->PCursor;
	      else (curi->NCursor)->PCursor = curi->PCursor;
	      FCursor->PCursor = curi;
	      curi->PCursor = NULL;
	      curi->NCursor = FCursor;
	      FCursor = curi;
	    }
	    curi->Status = 2;
	    ctop_int(CTXTc 4, (UInteger)curi);
	    /*printf("reuse cursor: %p\n",curi);*/
	    ctop_int(CTXTc 5, 0);
	    return;
	  } else {
	    curk = curi;                      /* otherwise just record it*/
	  }
	}
      }
    }
    curi = curi->NCursor;
  }

  /* done w/ the search; see what was found*/
  if (curj != NULL) {   /* give priority to an unused cursor*/
    curi = curj;
    /*printf("take unused cursor: %p\n",curi);*/
  }
  else {
    while((num != NULL) && (num->hdbc != hdbc)){
      num=num->NCurNum;
    }
    if(num == NULL){
      num = (struct NumberofCursors *)mem_alloc(sizeof(struct NumberofCursors),ODBC_SPACE);
      num->hdbc = hdbc;
      num->NCurNum=FCurNum;
      FCurNum=num;
      num->CursorCount=0;
    }

    if (num->CursorCount < MAXCURSORNUM) { /* allocate a new cursor if allowed*/
    /* problem here: should have numberOfCursors for each connection */
      curi = (struct ODBC_Cursor *)mem_calloc(sizeof(struct ODBC_Cursor),1,ODBC_SPACE);
    curi->PCursor = NULL;
    curi->NCursor = FCursor;
    if (FCursor == NULL) LCursor = curi;
    else FCursor->PCursor = curi;
    FCursor = curi;

    rc = SQLAllocStmt(hdbc,&(curi->hstmt));
    if (!((rc==SQL_SUCCESS) ||
	  (rc==SQL_SUCCESS_WITH_INFO))) {
      mem_dealloc(curi,sizeof(struct ODBC_Cursor),ODBC_SPACE);
      /*      numberOfCursors--; */
      ctop_int(CTXTc 4, 0);
      unify(CTXTc reg_term(CTXTc 5), 
	    GenErrorMsgBall(detailed_error("ERROR while trying to allocate ODBC statement","",hdbc,SQL_HANDLE_DBC)));
      return;
    }

    num->CursorCount++;

    /*printf("allocate a new cursor: %p\n",curi);*/
    }
    else if (curk == NULL) {  /* no cursor left*/
      ctop_int(CTXTc 4, 0);
      unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("No Cursors Left"));
      return;
    }
    else {                    /* steal a cursor*/
      curi = curk;
      SetCursorClose(curi);
      /*printf("steal a cursor: %p\n",curi);*/
    }
  }

  /* move to front of list.*/
  if (curi != FCursor) {
    (curi->PCursor)->NCursor = curi->NCursor;
    if (curi == LCursor) LCursor = curi->PCursor;
    else (curi->NCursor)->PCursor = curi->PCursor;
    FCursor->PCursor = curi;
    curi->PCursor = NULL;
    curi->NCursor = FCursor;
    FCursor = curi;
  }

  rc = SQLGetInfo(hdbc,SQL_DRIVER_NAME,drname,(SQLSMALLINT)25,&drnamelen);
  if (rc != SQL_SUCCESS) curi->driver_code = 0;
  else if (!strcmp(drname,"odbcjt32.dll")) curi->driver_code = 1;
  else curi->driver_code = 0;

  curi->hdbc = hdbc;
  curi->Sql = (UCHAR *)mem_alloc(strlen(Sql_stmt)+1,ODBC_SPACE);
  if (!curi->Sql) {
    ctop_int(CTXTc 4, 0);
    unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Not enough memory for SQL stmt in FindFreeCursor!"));
    return;
  }
  strcpy((char *)curi->Sql,Sql_stmt);
  curi->Status = 3;
  ctop_int(CTXTc 4, (UInteger)curi);
  ctop_int(CTXTc 5, 0);
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     SetBindVarNum() */
/*  PARAMETERS:*/
/*     R1: 3*/
/*     R2: ODBC_Cursor Address*/
/*     R3: Number of bind values*/
/*     R4: RetCode - 0=> OK, otw error */
/*  NOTES:*/
/*     set the number of bind variables.  Note that the memory to*/
/*     store their values is not allocated here since we don't know*/
/*     their type: no information on how much memory is needed.  If*/
/*     we're reusing an old statement handler we don't have to worry*/
/*     about these things.  All we need to do is to make sure that*/
/*     the statement is indeed the same statement w/ the same bind*/
/*     variable number.*/
/*-----------------------------------------------------------------------------*/
void SetBindVarNum(CTXTdecl)
{
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  int NumBindVars = (int)ptoc_int(CTXTc 3);

  if (cur->Status == 2) {
    if (cur->NumBindVars != NumBindVars) {
      unify(CTXTc reg_term(CTXTc 4), GenErrorMsgBall("Number of Bind values provided does not agree with query"));
      return;
    }
    ctop_int(CTXTc 4, 0);
    return;
  }

  cur->NumBindVars = NumBindVars;
  cur->BindList = mem_alloc(sizeof(UCHAR *) * NumBindVars,ODBC_SPACE);
  if (!cur->BindList) {
    unify(CTXTc reg_term(CTXTc 4), GenErrorMsgBall("Not enough memory for cur->BindList!"));
    return;
  }
  cur->BindTypes = mem_alloc(sizeof(int) * NumBindVars,ODBC_SPACE);
  if (!cur->BindTypes) {
    unify(CTXTc reg_term(CTXTc 4), GenErrorMsgBall("Not enough memory for cur->BindTypes!"));
    return;
  }
  cur->BindLens = mem_alloc(sizeof(SQLLEN) * NumBindVars,ODBC_SPACE);
  if (!cur->BindLens) {
    unify(CTXTc reg_term(CTXTc 4), GenErrorMsgBall("Not enough memory for cur->BindLens!"));
    return;
  }

  ctop_int(CTXTc 4, 0);
}

DllExport void call_conv write_canonical_term(CTXTdeclc Cell prologterm, int letterflag);
#define wcan_string tsgLBuff1

void string_to_char(Cell list, char **tmp_term_string) {
  Cell tlist, car;
  char *charptr;
  int len = 0;
  
  if (*tmp_term_string) {
    mem_dealloc(*tmp_term_string,strlen(*tmp_term_string)+1,ODBC_SPACE);
  }
  XSB_Deref(list);
  tlist = list;
  while (islist(tlist)) {
    len++;
    tlist = p2p_cdr(tlist);
    XSB_Deref(tlist);
  }
  *tmp_term_string = charptr = mem_alloc(len+1,ODBC_SPACE);
  while (islist(list)) {
    car = p2p_car(list);
    XSB_Deref(car);
    *(charptr++) = (int)int_val(car);
    list = p2p_cdr(list);
    XSB_Deref(list);
  }
  *charptr = '\0';
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     SetBindVal()   */
/*  PARAMETERS:*/
/*     R1: 6*/
/*     R2: ODBC_Cursor Addr*/
/*     R3: Bind variable index (starting from 0)*/
/*     R4: Value to give the bind variable*/
/*     R5: var, for returned status*/
/*  NOTES:*/
/*     set the bind variables' values. */
/*     allocate memory if it is needed(status == 3)*/
/*-----------------------------------------------------------------------------*/
void SetBindVal(CTXTdecl)
{
  RETCODE rc;
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  int j = (int)ptoc_int(CTXTc 3);
  Cell BindVal = ptoc_tag(CTXTc 4);
  char errmsg[200];

  if (!((j >= 0) && (j < cur->NumBindVars))){
    unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Abnormal argument in SetBindVal"));
    return;
  }

  /* if we're reusing an opened cursor w/ the statement number*/
  /* reallocate BindVar if type has changed (May not be such a good idea?)*/
  if (cur->Status == 2) {
    if (isointeger(BindVal)) {
      if (cur->BindTypes[j] != 0) {
	if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);  //((void *)cur->BindList[j]);
	cur->BindList[j] = (UCHAR *)mem_alloc(sizeof(int),ODBC_SPACE);
	cur->BindTypes[j] = 0;
	rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, (Integer *)(cur->BindList[j]), 0, NULL);
	if (rc != SQL_SUCCESS) {
	  unify(CTXTc reg_term(CTXTc 5),PrintErrorMsg(CTXTc cur));
	  SetCursorClose(cur);
	  return;
	}
      }
      *((Integer *)cur->BindList[j]) = oint_val(BindVal);
    } else if (isofloat(BindVal)) {
      if (cur->BindTypes[j] != 1) {
	/*printf("ODBC: Changing Type: flt to %d\n",cur->BindTypes[j]);*/
	if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);
	cur->BindList[j] = (UCHAR *)mem_alloc(sizeof(Float),ODBC_SPACE);
	cur->BindTypes[j] = 1;
	rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, (Float *)(cur->BindList[j]), 0, NULL);
	if (rc != SQL_SUCCESS) {
	  unify(CTXTc reg_term(CTXTc 5),PrintErrorMsg(CTXTc cur));
	  SetCursorClose(cur);
	  return;
	}
      }
      *((Float *)cur->BindList[j]) = ofloat_val(BindVal);
    } else if (isstring(BindVal)) {
      if (cur->BindTypes[j] != 2) {
	/*printf("ODBC: Changing Type: str to %d\n",cur->BindTypes[j]);*/
	if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);
	cur->BindTypes[j] = 2;
	/* SQLBindParameter will be done later in parse for char variables*/
      }
      cur->BindList[j] = (UCHAR *)string_val(BindVal);  // wont be freed since type=2
    } else if (isconstr(BindVal) && get_str_psc(BindVal) == nullFctPsc) {
      if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);
      cur->BindTypes[j] = 3;
      cur->BindList[j] = NULL;
    } else if (isconstr(BindVal) && get_arity(get_str_psc(BindVal))==1) {
      if (!strcmp(get_name(get_str_psc(BindVal)),"string")) {
	if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);
	string_to_char(p2p_arg(BindVal,1),&(term_string[j]));
	cur->BindTypes[j] = 2;
	cur->BindList[j] = (UCHAR *)term_string[j];
      } else {
	if (cur->BindTypes[j] < 2) free_cur_bindlist(cur,j);
	write_canonical_term(CTXTc p2p_arg(BindVal,1),1);
	if (term_string[j]) mem_dealloc(term_string[j],strlen(term_string[j])+1,ODBC_SPACE);
	term_string[j] = mem_alloc(wcan_string->length+1,ODBC_SPACE);
	strcpy(term_string[j],wcan_string->string);
	cur->BindTypes[j] = 2;
	cur->BindList[j] = (UCHAR *)term_string[j];
      }
    } else {
      if (!cur->BindTypes[j]) unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Bind variable cannot be free"));
      else {
	snprintf(errmsg,200,"Unknown bind variable type: %d", cur->BindTypes[j]);
	unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall(errmsg));
      }
      return;
    }
    ctop_int(CTXTc 5,0);
    return;
  }

  /* otherwise, memory needs to be allocated in this case*/
  if (isointeger(BindVal)) {
    cur->BindTypes[j] = 0;
    cur->BindList[j] = (UCHAR *)mem_alloc(sizeof(Integer),ODBC_SPACE);
    if (!cur->BindList[j]) {
      unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Not enough memory for an int in SetBindVal"));
      return;
    }
    *((Integer *)cur->BindList[j]) = (Integer)oint_val(BindVal);
  } else if (isofloat(BindVal)) {
    cur->BindTypes[j] = 1;
    cur->BindList[j] = (UCHAR *)mem_alloc(sizeof(float),ODBC_SPACE);
    if (!cur->BindList[j]) {
      unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Not enough memory for a float in SetBindVal"));
      return;
    }
    *((Float *)cur->BindList[j]) = (Float)ofloat_val(BindVal);
  } else if (isstring(BindVal)) {
    cur->BindTypes[j] = 2;
    cur->BindList[j] = (UCHAR *)string_val(BindVal);
    } else if (isconstr(BindVal) && get_str_psc(BindVal) == nullFctPsc) {
    cur->BindTypes[j] = 3;
    cur->BindList[j] = NULL;
  } else if (isconstr(BindVal) && get_arity(get_str_psc(BindVal))==1) {
    if (!strcmp(get_name(get_str_psc(BindVal)),"string")) {
	string_to_char(p2p_arg(BindVal,1),&(term_string[j]));
	cur->BindTypes[j] = 2;
	cur->BindList[j] = (UCHAR *)term_string[j];
    } else {
      write_canonical_term(CTXTc p2p_arg(BindVal,1),1);
      if (term_string[j]) mem_dealloc(term_string[j],strlen(term_string[j])+1,ODBC_SPACE);
      term_string[j] = mem_alloc(wcan_string->length+1,ODBC_SPACE);
      strcpy(term_string[j],wcan_string->string);
      cur->BindTypes[j] = 2;
      cur->BindList[j] = (UCHAR *)term_string[j];
    }
  } else {
    if (!cur->BindTypes[j]) unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Bind variable cannot be free"));
    else {
      snprintf(errmsg,200,"Unknown bind variable type: %d", cur->BindTypes[j]);
      unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall(errmsg));
    }
    return;
  }
  ctop_int(CTXTc 5,0);
  return;
}


/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     Parse()*/
/*  PARAMETERS:*/
/*     R1: 2*/
/*     R2: the SQL statement for the cursor*/
/*     R3: var, 0 or error*/
/*  NOTES:*/
/*     parse the sql statement and submit it to DBMS to execute.  if all these*/
/*     succeed, then prepare for resulting row fetching.  this includes*/
/*     determination of column number in the resulting rowset and the length*/
/*     of each column and memory allocation which is used to store each row.*/
/*-----------------------------------------------------------------------------*/
void Parse(CTXTdecl)
{
int xret;
int j;
struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
RETCODE rc;
  if (cur->Status == 2) { /* reusing opened cursor*/
    rc = SQLFreeStmt(cur->hstmt,SQL_CLOSE);
    if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
      unify(CTXTc reg_term(CTXTc 3), PrintErrorMsg(CTXTc cur));
      SetCursorClose(cur);
      return;
    }
    /* reset just char and null vars, since they store addr of chars, and set lengths*/
    for (j = 0; j < cur->NumBindVars; j++) {
      cur->BindLens[j] = 0;
      switch (cur->BindTypes[j])
		{
		case 2:
		  cur->BindLens[j] = (SQLINTEGER)strlen((char *)cur->BindList[j]);
		  if (cur->driver_code == 1)
		    rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0,(char *)cur->BindList[j], 0, &SQL_NTSval);
		  else rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0,(char *)cur->BindList[j], cur->BindLens[j], &cur->BindLens[j]);
		  break;
		case 3:
			rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0,NULL, 0, &SQL_NULL_DATAval);
			break;
		}
    }
  } else {
    if (SQL_SUCCESS != (rc = SQLPrepare(cur->hstmt, cur->Sql, SQL_NTS)))
		{
		unify(CTXTc reg_term(CTXTc 3),PrintErrorMsg(CTXTc cur));
		SetCursorClose(cur);
		return;
		}


    /* set the bind variables*/
    for (j = 0; j < cur->NumBindVars; j++) {
      switch (cur->BindTypes[j])
		{
		case 0:
			rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, (Integer *)(cur->BindList[j]), 0, NULL);
			break;
		case 1:
			rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, (Float *)cur->BindList[j], 0, NULL);
			break;
		case 2:
			/* we're sloppy here.  it's ok for us to use the default values*/
		  cur->BindLens[j] = strlen((char *)cur->BindList[j]);
		  if (cur->driver_code == 1)
		    rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0,(char *)cur->BindList[j], 0, &SQL_NTSval);
		  else rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0,(char *)cur->BindList[j], cur->BindLens[j], &cur->BindLens[j]);
		  break;
		case 3:
			rc = SQLBindParameter(cur->hstmt, (short)(j+1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0,NULL, 0, &SQL_NULL_DATAval);
			break;
		default:
			unify(CTXTc reg_term(CTXTc 3), GenErrorMsgBall("illegal BindVal"));
			return;
		}
      if (rc != SQL_SUCCESS)
		{
		  unify(CTXTc reg_term(CTXTc 3),PrintErrorMsg(CTXTc cur));
		  SetCursorClose(cur);
		  return;
		}
    }
  }
  /* submit it for execution*/
  xret = SQLExecute(cur->hstmt);
  if (xret != SQL_SUCCESS && xret != SQL_SUCCESS_WITH_INFO) {
    unify(CTXTc reg_term(CTXTc 3),PrintErrorMsg(CTXTc cur));
    SetCursorClose(cur);
    return;
  }
  ctop_int(CTXTc 3,0);
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCCommit()*/
/*  PARAMETERS:*/
/*     R1: 10*/
/*     R2: Connection Handle*/
/*     R3: var, returned status, 0 if successful*/
/*-----------------------------------------------------------------------------*/
void ODBCCommit(CTXTdecl)
{
  struct ODBC_Cursor *cur = FCursor;
  HDBC hdbc = (HDBC)ptoc_int(CTXTc 2);
  RETCODE rc;

  if (((rc=SQLTransact(henv,hdbc,SQL_COMMIT)) == SQL_SUCCESS) ||
      (rc == SQL_SUCCESS_WITH_INFO)) {
    /* close only those with right hdbc....*/
    while (cur != NULL) {
      if (cur->hdbc == hdbc && cur->Status != 0) SetCursorClose(cur);
      cur = cur->NCursor;
    }
    ctop_int(CTXTc 3,0);
  } else {
    unify(CTXTc reg_term(CTXTc 3),PrintErrorMsg(CTXTc NULL));
  }
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCRollback()*/
/*  PARAMETERS:*/
/*     R1: 11*/
/*     R2: Connection Handle*/
/*     R3: var, for returned status: 0 if successful*/
/*-----------------------------------------------------------------------------*/
void ODBCRollback(CTXTdecl)
{
  struct ODBC_Cursor *cur = FCursor;
  HDBC hdbc = (HDBC)ptoc_int(CTXTc 2);
  RETCODE rc;

  if (((rc=SQLTransact(henv,hdbc,SQL_ROLLBACK)) == SQL_SUCCESS) ||
      (rc == SQL_SUCCESS_WITH_INFO)) {
    /* only close those with right hdbc*/
    while (cur != NULL) {
      if (cur->hdbc == hdbc && cur->Status != 0) SetCursorClose(cur);
      cur = cur->NCursor;
    }
    ctop_int(CTXTc 3,0);
  } else
    unify(CTXTc reg_term(CTXTc 3), PrintErrorMsg(CTXTc NULL));
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCColumns()*/
/*  PARAMETERS:*/
/*     R1: 12*/
/*     R2: ODBC_Cursor Index*/
/*     R3: Table name*/
/*     R4: var for returned status: 0 if successful*/
/*-----------------------------------------------------------------------------*/
void ODBCColumns(CTXTdecl)
{
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  char tmpstr[255];
  char *str1, *str2, *str3;
  RETCODE rc;

  strcpy(tmpstr,ptoc_string(CTXTc 3));
  str1 = strtok(tmpstr,".");
  str2 = str3 = NULL;
  if (str1) str2 = strtok(NULL,".");
  if (str2) str3 = strtok(NULL,".");
  if (!str3) {
    if (!str2) {str3 = str1; str1 = NULL;}
    else {str3 = str2; str2 = str1; str1 = NULL;}
  }
  /*printf("str1 %s, str2 %s, str3 %s\n",str1,str2,str3);*/
  if (((rc=SQLColumns(cur->hstmt,
		      (UCHAR *)str1, SQL_NTS,
		      (UCHAR *)str2, SQL_NTS,
		      (UCHAR *)str3, SQL_NTS,
		      NULL,0)) == SQL_SUCCESS) ||
      (rc == SQL_SUCCESS_WITH_INFO)) {
    ctop_int(CTXTc 4,0);
  } else {
    unify(CTXTc reg_term(CTXTc 4),PrintErrorMsg(CTXTc cur));
    SetCursorClose(cur);
  }
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCTables()*/
/*  PARAMETERS:*/
/*     R1: 13*/
/*     R2: ODBC_Cursor*/
/*     R3: var, returned status: 0 if successful*/
/*-----------------------------------------------------------------------------*/
void ODBCTables(CTXTdecl)
{
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  RETCODE rc;

  if (cur->Status == 2) { /* reusing opened cursor*/
    rc = SQLFreeStmt(cur->hstmt,SQL_CLOSE);
    if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
      unify(CTXTc reg_term(CTXTc 3), PrintErrorMsg(CTXTc cur));
      SetCursorClose(cur);
      return;
    }
  }

  if (((rc=SQLTables(cur->hstmt,
		     NULL, 0,
		     NULL, 0,
		     NULL, 0,
		     NULL, 0)) == SQL_SUCCESS) ||
      (rc == SQL_SUCCESS_WITH_INFO)) {
    ctop_int(CTXTc 3,0);
  } else {
    unify(CTXTc reg_term(CTXTc 3),PrintErrorMsg(CTXTc cur));
    SetCursorClose(cur);
  }
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCUserTables()*/
/*  PARAMETERS:*/
/*     R1: 14*/
/*     R2: ODBC_Cursor*/
/*     R3: var, for returned status: 0 if successful*/
/*-----------------------------------------------------------------------------*/
void ODBCUserTables(CTXTdecl)
{
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  UWORD TablePrivilegeExists;
  RETCODE rc;

  /* since some ODBC drivers don't implement the function SQLTablePrivileges*/
  /* we check it first*/
  SQLGetFunctions(cur->hdbc,SQL_API_SQLTABLEPRIVILEGES,&TablePrivilegeExists);
  if (!TablePrivilegeExists) {
    unify(CTXTc reg_term(CTXTc 3), 
	  GenErrorMsgBall("Privilege concept does not exist in this DBMS: you probably can access any of the existing tables"));
    return;
  }
  if (((rc=SQLTablePrivileges(cur->hstmt,
			      NULL, 0,
			      NULL, 0,
			      NULL, 0)) == SQL_SUCCESS) ||
      (rc == SQL_SUCCESS_WITH_INFO))
    ctop_int(CTXTc 3,0);
  else {
    unify(CTXTc reg_term(CTXTc 3),PrintErrorMsg(CTXTc cur));
    SetCursorClose(cur);
  }
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     DisplayColSize()*/
/*  PARAMETERS:*/
/*     SWORD coltype - column type which is a single word.*/
/*     UDWORD collen - column length which is returned by SQLDescribeCol*/
/*     UCHAR *colname - pointer to column name string*/
/*  RETURN VALUE:*/
/*     column length - the size of memory that is needed to store the column*/
/*     value for supported column types*/
/*     0             - otherwise*/
/*-----------------------------------------------------------------------------*/
UDWORD DisplayColSize(SWORD coltype, UDWORD collen, UCHAR *colname)
{
  switch (ODBCToXSBType(coltype)) {
  case SQL_C_SLONG:
    return sizeof(word *);
  case SQL_C_CHAR: {
    UDWORD tmp = MAXI(collen+1, (UDWORD)strlen((char *) colname));
    if (tmp < MAXVARSTRLEN) return tmp;
    else return MAXVARSTRLEN;
  }
  case SQL_C_BINARY: {
    return MAXVARSTRLEN;
  }
  case SQL_C_FLOAT:
    return sizeof(float *);
  default:
    fprintf(stderr,"Illegal ODBC Type: %d\n",coltype);
    return 0;
  }
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCDataSources()*/
/*  PARAMETERS:*/
/*     R1: 17 */
/*     R2: 1 - first call; 2 - subsequent calls */
/*     R3: var, returns DSN */
/*     R4: var, returns DS description */
/*     R5: var, returns status */
/*  NOTES:*/
/*-----------------------------------------------------------------------------*/
void ODBCDataSources(CTXTdecl)
{
  SQLCHAR DSN[SQL_MAX_DSN_LENGTH+1];
  SQLCHAR Description[SQL_MAX_DSN_LENGTH+1];
  RETCODE rc;
  int seq, new;
  SWORD dsn_size, descr_size;
  Cell op2 = ptoc_tag(CTXTc 3);
  Cell op3 = ptoc_tag(CTXTc 4);

  if (!henv) {
    //locked to prevent two threads from fighting over who creates the env
    SYS_MUTEX_LOCK( MUTEX_ODBC) ;
    /* allocate environment handler*/
    rc = SQLAllocEnv(&henv);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      unify(CTXTc reg_term(CTXTc 5), 
	    GenErrorMsgBall(detailed_error("Environment allocation failed","",henv,SQL_HANDLE_ENV)));
      return;
    }
    LCursor = FCursor = NULL;
    FCurNum = NULL;
    if (nullFctPsc == NULL)
        nullFctPsc = pair_psc(insert("NULL",1,global_mod,&new));
    SYS_MUTEX_UNLOCK( MUTEX_ODBC) ;
  }

  seq = (int)ptoc_int(CTXTc 2);
  if (seq == 1) {
    rc = SQLDataSources(henv,SQL_FETCH_FIRST,DSN,
			SQL_MAX_DSN_LENGTH,&dsn_size,
			Description,SQL_MAX_DSN_LENGTH,
			&descr_size);
    if (rc == SQL_NO_DATA_FOUND) {
      ctop_int(CTXTc 5,2);
      return;
    }
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      unify(CTXTc reg_term(CTXTc 5),
	    GenErrorMsgBall(detailed_error("Call to SQLDataSources failed","",henv,SQL_HANDLE_ENV)));
      return;
    }
  } else {
    rc = SQLDataSources(henv,SQL_FETCH_NEXT,DSN,
			SQL_MAX_DSN_LENGTH,&dsn_size,
			Description,SQL_MAX_DSN_LENGTH,
			&descr_size);
    if (rc == SQL_NO_DATA_FOUND) {
      ctop_int(CTXTc 5,2);
      return;
    }
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      unify(CTXTc reg_term(CTXTc 5),
	    GenErrorMsgBall(detailed_error("Call to SQLDataSources failed","",henv, SQL_HANDLE_ENV)));
      return;
    }
  }
  XSB_Deref(op2);
  if (isref(op2))
  {
    char * tempDSNstring= string_find((char *)DSN,1);
	  Cell cellStr = makestring(tempDSNstring);
    unify(CTXTc op2, cellStr);
  }
  else {
    unify(CTXTc reg_term(CTXTc 5),GenErrorMsgBall("[ODBCDataSources] Param 2 should be a free variable."));
    return;
  }
  XSB_Deref(op3);
  if (isref(op3))
  {
    unify(CTXTc op3, makestring(string_find((char *)Description,1)));
  }
  else {
    unify(CTXTc reg_term(CTXTc 5),GenErrorMsgBall("[ODBCDataSources] Param 3 should be a free variable."));
    return;
  }
  ctop_int(CTXTc 5,0);
  return;
}

// XXX TODO: Change implementation below
/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCDrivers()*/
/*  PARAMETERS:*/
/*     R1: 20 */
/*     R2: 1 - first call; 2 - subsequent calls */
/*     R3: var, returns Driver description */
/*     R4: var, returns Driver attributes  */
/*     R5: var, returns status */
/*  NOTES:*/
/*-----------------------------------------------------------------------------*/
#define SQL_MAX_ATTR_LENGTH 1024
#define SQL_MAX_DESC_LENGTH 1024
void ODBCDrivers(CTXTdecl)
{
  SQLCHAR Description[SQL_MAX_DESC_LENGTH];
  SQLCHAR Attributes[SQL_MAX_ATTR_LENGTH];
  RETCODE rc;
  int seq, new;
  SWORD descr_size, attr_size;
  Cell op2 = ptoc_tag(CTXTc 3);
  Cell op3 = ptoc_tag(CTXTc 4);

  if (!henv) {
    //locked to prevent two threads from fighting over who creates the env
    SYS_MUTEX_LOCK( MUTEX_ODBC) ;
    /* allocate environment handler*/
    rc = SQLAllocEnv(&henv);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      unify(CTXTc reg_term(CTXTc 5), GenErrorMsgBall("Environment allocation failed"));
      return;
    }
    LCursor = FCursor = NULL;
    FCurNum = NULL;
    if (nullFctPsc == NULL)
        nullFctPsc = pair_psc(insert("NULL",1,global_mod,&new));
    SYS_MUTEX_UNLOCK( MUTEX_ODBC) ;
  }

  seq = (int)ptoc_int(CTXTc 2);
  if (seq == 1) {
    rc = SQLDrivers(henv, SQL_FETCH_FIRST,
		    Description, SQL_MAX_DESC_LENGTH, &descr_size,
		    Attributes, SQL_MAX_ATTR_LENGTH, &attr_size);
    if (rc == SQL_NO_DATA_FOUND) {
      ctop_int(CTXTc 5,2);
      return;
    }
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      unify(CTXTc reg_term(CTXTc 5),GenErrorMsgBall("Call to SQLDrivers failed (seq=1)"));
      return;
    }
  } else {
    rc = SQLDrivers(henv, SQL_FETCH_NEXT,
		    Description, SQL_MAX_DESC_LENGTH, &descr_size,
		    Attributes, SQL_MAX_ATTR_LENGTH, &attr_size);
    if (rc == SQL_NO_DATA_FOUND) {
      ctop_int(CTXTc 5,2);
      return;
    }
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
      unify(CTXTc reg_term(CTXTc 5),GenErrorMsgBall("Call to SQLDrivers failed (seq != 1)"));
      return;
    }
  }
  XSB_Deref(op2);
  if (isref(op2)) {
    Cell cellStr = makestring(string_find((char *)Description,1));
    unify(CTXTc op2, cellStr);
  } else {
    unify(CTXTc reg_term(CTXTc 5),
	  GenErrorMsgBall("[ODBCDrivers] Param 2 should be a free variable."));
    return;
  }
  XSB_Deref(op3);
  if (isref(op3)) {
    char *current_attr = (char *)Attributes;
    int num_attrs = 0, isnew;
    CPtr prev_cons;
    Cell attr_list = makenil;  // to quiet compiler
    Psc eq_psc;

    /* First count the number of attribute-value pairs in the list */
    while (!(current_attr[0] == '\0' && current_attr[1] == '\0')) {
      if (current_attr[0] == '=') { 
	num_attrs++; 
      }
      current_attr++;
    }

    /* Make sure we have enough room */
    if (heap_local_overflow(5*num_attrs*sizeof(Cell))) {
      xsb_resource_error(CTXTc "No space for attributes", "odbc_drivers", 2);
    }
    
    /* Now split the pairs to create the list */
    current_attr = (char *)Attributes;
    prev_cons = NULL;
    eq_psc = pair_psc(insert("=",2,(Psc)flags[CURRENT_MODULE],&isnew));
    while (current_attr[0] != '\0') {
      char *attr;
      char *val;
      attr = current_attr;
      val = strchr(current_attr, '=');
      if (val) {
	val[0] = '\0';
	val++;
	current_attr = val + strlen(val) + 1;
	if (prev_cons) bld_list(prev_cons+1,hreg); else attr_list = makelist(hreg);

	bld_cs(hreg,hreg+2);
	bld_functor(hreg+2,eq_psc);
	bld_string(hreg+3,string_find(attr,1));
	bld_string(hreg+4,string_find(val,1));

	prev_cons = hreg;
	hreg += 5;
      } else {
	break;
      }
    }
    if (prev_cons) bld_nil(prev_cons+1); else attr_list = makenil;
    unify(CTXTc op3, attr_list);
  } else {
    unify(CTXTc reg_term(CTXTc 5),
	  GenErrorMsgBall("[ODBCDrivers] Param 3 should be a free variable."));
    return;
  }
  ctop_int(CTXTc 5,0);
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     DescribeSelect()*/
/*  PARAMETERS:*/
/*     R1: 15*/
/*     R2: Ptr to cursor record*/
/*     R3: var, for returned status:*/
/*         0 - the result row has at least one column and*/
/*         1 - something goes wrong, we can't retrieve column information, */
/*             memory allocation fails (if this happens program stops).*/
/*         2 - no column is affected*/
/*  NOTES:*/
/*     memory is also allocated for future data storage*/
/*-----------------------------------------------------------------------------*/
void ODBCDescribeSelect(CTXTdecl)
{
  int j;
  UCHAR colname[50];
  SWORD colnamelen;
  SWORD scale;
  SWORD nullable;
  SQLULEN collen;
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);

  cur->NumCols = 0;
  SQLNumResultCols(cur->hstmt, (SQLSMALLINT*)&(cur->NumCols));
  if (!(cur->NumCols)) {
    /* no columns are affected, set cursor status to unused */
    cur->Status = 1;
    ctop_int(CTXTc 3,2);
    return;
  }
  /* if we aren't reusing a closed statement handle, we need to get*/
  /* resulting rowset info and allocate memory for it*/
  if (cur->Status != 2) {
    cur->ColTypes =
      (SWORD *)mem_alloc(sizeof(SWORD) * cur->NumCols,ODBC_SPACE);
    if (!cur->ColTypes) {
      unify(CTXTc reg_term(CTXTc 3),GenErrorMsgBall("Not enough memory for ColTypes!"));
      return;
    }

    cur->Data =
      (UCHAR **)mem_alloc(sizeof(UCHAR *) * cur->NumCols,ODBC_SPACE);
    if (!cur->Data) {
      unify(CTXTc reg_term(CTXTc 3),GenErrorMsgBall("Not enough memory for Data!"));
      return;
    }

    cur->OutLen =
      (SQLLEN *)mem_alloc(sizeof(SQLLEN) * cur->NumCols,ODBC_SPACE);
    if (!cur->OutLen) {
      unify(CTXTc reg_term(CTXTc 3),GenErrorMsgBall("Not enough memory for OutLen!"));
      return;
    }

    cur->ColLen =
      (SQLULEN *)mem_alloc(sizeof(SQLULEN) * cur->NumCols,ODBC_SPACE);
    if (!cur->ColLen) {
      unify(CTXTc reg_term(CTXTc 3),GenErrorMsgBall("Not enough memory for ColLen!"));
      return;
    }

    for (j = 0; j < cur->NumCols; j++) {
      SQLDescribeCol(cur->hstmt, (short)(j+1), (UCHAR FAR*)colname,
		     sizeof(colname), &colnamelen,
		     &(cur->ColTypes[j]),
		     &collen, &scale, &nullable);
      /* SQLServer returns this wierd type for a system table, treat it as varchar?*/
      if (cur->ColTypes[j] == -9) cur->ColTypes[j] = SQL_VARCHAR;
      colnamelen = (colnamelen > 49) ? 49 : colnamelen;
      colname[colnamelen] = '\0';
      cur->ColLen[j] = DisplayColSize(cur->ColTypes[j],(UDWORD)collen,colname);
      if (!(cur->ColLen[j])) {
	/* let SetCursorClose function correctly free all the memory allocated*/
	/* for Data storage: cur->Data[j]'s*/
	cur->NumCols = j; /* set so close frees memory allocated thus far*/
	SetCursorClose(cur);
	/*	return(1);*/
	unify(CTXTc reg_term(CTXTc 3),GenErrorMsgBall("Error in column lengths"));
	return;
      }
      cur->Data[j] =
	(UCHAR *) mem_alloc(sizeof(UCHAR)*((unsigned) cur->ColLen[j]+1),ODBC_SPACE);
      if (!cur->Data[j]) {
	char errmsg[200];
	snprintf(errmsg,200,"Not enough memory for Data[%d]!",j);
	unify(CTXTc reg_term(CTXTc 3),GenErrorMsgBall(errmsg));
	return;
      }
    }
  }
  /* bind them*/
  for (j = 0; j < cur->NumCols; j++) {
    SQLBindCol(cur->hstmt, (short)(j+1),
	       ODBCToXSBType(cur->ColTypes[j]), cur->Data[j],
	       //	       cur->ColLen[j], (SDWORD FAR *)(&(cur->OutLen[j])));
	       cur->ColLen[j], (&(cur->OutLen[j])));
  }
  /*  return 0;*/
  ctop_int(CTXTc 3,0);
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     FetchNextRow()*/
/*  PARAMETERS:*/
/*     R1: 4*/
/*     R2: ODBC_Cursor Address*/
/*     R3: var, for returned status*/
/*         0 => successful, a row is read and available*/
/*         1 => successful, no row avaliable*/
/*         2 => unsuccessful, an error occurred in fetching.*/
/*  NOTES:*/
/*     fetch next row of result rowset.*/
/*-----------------------------------------------------------------------------*/
void FetchNextRow(CTXTdecl)
{
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  RETCODE rc;

  if (cur->Status == 0) {
    ctop_int(CTXTc 3,2);
    return;
  }

  rc = SQLFetch(cur->hstmt);

  if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
    ctop_int(CTXTc 3,0);
  else if (rc == SQL_NO_DATA_FOUND){
    cur->Status = 1; /* done w/fetching. set cursor status to unused */
    ctop_int(CTXTc 3,1);
  }
  else {
    SetCursorClose(cur);         /* error occured in fetching*/
    ctop_int(CTXTc 3,2);
  }
  return;
}

/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCConnectOption() */
/*  PARAMETERS:*/
/*     R1: 16*/
/*     R2: Connection Handle*/
/*     R3: 0 -> get; 1 -> set*/
/*     R4: Option indicator (int)*/
/*     R5: if R2=0 -> variable that attr val is returned in*/
/*         if R2=1 -> value used to set attr*/
/*     R6: Return Code*/
/*  NOTES:*/
/*     either gets value of a connection option, or sets it.*/
/*-----------------------------------------------------------------------------*/
void ODBCConnectOption(CTXTdecl)
{
  HDBC hdbc = (HDBC)ptoc_int(CTXTc 2);
  int set = (int)ptoc_int(CTXTc 3);
  Integer value = 0;
  RETCODE rc;

  if (set) {
    rc = SQLSetConnectOption(hdbc,(UWORD)ptoc_int(CTXTc 4),(UDWORD)ptoc_int(CTXTc 5));
  } else {
    rc = SQLGetConnectOption(hdbc,(UWORD)ptoc_int(CTXTc 4),(PTR)&value);
    ctop_int(CTXTc 5,value);
  }
  if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
    ctop_int(CTXTc 6,0);
  else unify(CTXTc reg_term(CTXTc 6),PrintErrorMsg(CTXTc NULL));
}

//extern xsbBool glstack_realloc(CTXTc int,int);

Cell build_codes_list(CTXTdeclc byte *charptr) {
  size_t len = strlen((char *)charptr);

  if (len == 0) {
    return makenil;
  } else {
    CPtr this_term;
    check_glstack_overflow(4,pcreg,2*sizeof(Cell)*len);
    this_term = hreg;
    /* char_to_codepoint increments charptr */
    cell(hreg) = makeint(char_to_codepoint(CURRENT_CHARSET,&charptr));
    hreg += 2;
    while (*charptr != 0) {
      cell(hreg-1) = makelist(hreg);
      cell(hreg) = makeint(char_to_codepoint(CURRENT_CHARSET,&charptr));
      hreg += 2;
    }
    cell(hreg-1) = makenil;
    return makelist(this_term);
  }
}
/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     GetColumn() */
/*  PARAMETERS:*/
/*     R1: 5*/
/*     R2: ODBC_Cursor*/
/*     R3: Column Index*/
/*     R4: Column Value returned*/
/*     R5: Return Code*/
/*  NOTES:*/
/*     get the indicated column.*/
/*-----------------------------------------------------------------------------*/
int GetColumn(CTXTdecl)
{
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  int ColCurNum = (int)ptoc_int(CTXTc 3);
  Cell op = ptoc_tag(CTXTc 4);
  SQLULEN len;
  if (ColCurNum < 0 || ColCurNum >= cur->NumCols) {
    /* no more columns in the result row*/
    ctop_int(CTXTc 5,1);
    return TRUE;
  }
  ctop_int(CTXTc 5,0);

  /* get the data*/
  if (cur->OutLen[ColCurNum] == SQL_NULL_DATA) {
    /* column value is NULL*/
    Cell nullterm = makecs(hreg);
    new_heap_functor(hreg,nullFctPsc);
    new_heap_free(hreg);
    if (isconstr(op) && get_arity(get_str_psc(op)) == 1) 
      /* for "string" and "term"... */
      return unify(CTXTc get_str_arg(op,1),nullterm);
    else return unify(CTXTc op,nullterm);
  }

  /* convert the string to either integer, float or string*/
  /* according to the column type and pass it back to XSB*/
  switch (ODBCToXSBType(cur->ColTypes[ColCurNum])) {
  case SQL_C_CHAR:
    /* convert the column string to a C string */
    len = ((cur->ColLen[ColCurNum] < (unsigned)cur->OutLen[ColCurNum])?
	   cur->ColLen[ColCurNum]:(unsigned)cur->OutLen[ColCurNum]);
    *(cur->Data[ColCurNum]+len) = '\0';

    /* compare strings here, so don't intern strings unnecessarily*/
    XSB_Deref(op);
    if (isref(op)) {
      return unify(CTXTc op, makestring(string_find((char *)cur->Data[ColCurNum],1)));
    }
    if (isconstr(op) && get_arity(get_str_psc(op)) == 1) {
      if (!strcmp(get_name(get_str_psc(op)),"string")) {
	return unify(CTXTc get_str_arg(ptoc_tag(CTXTc 4),1),  /* op might have moved! */
		     build_codes_list(CTXTc cur->Data[ColCurNum]));
      } else if (!strcmp(get_name(get_str_psc(op)),"term")) {
	STRFILE strfile;
	
	strfile.strcnt = strlen((char *)cur->Data[ColCurNum]);
	if (strfile.strcnt >= MAXVARSTRLEN-1)
	  xsb_warn(CTXTc "[ODBC] Likely overflow of data in column of PROLOG_TERM type\n");
	strfile.strptr = strfile.strbase = cur->Data[ColCurNum];
	iostrs[0] = &strfile;
	read_canonical_term(CTXTc iostrdecode(0),2); /* terminating '.'? */
	return TRUE;
      } else if (!strcmp(get_name(get_str_psc(op)),"NULL")) {
	return FALSE;
      } else {
	xsb_warn(CTXTc "unrecognized return argument type; return failed");
	return FALSE;
      }
    }
    if (!isstring(op)) return FALSE;
    if (strcmp(string_val(op),(char *)cur->Data[ColCurNum])) return FALSE;
    return TRUE;
  case SQL_C_BINARY:
    /* convert the column string to a C string */
    len = ((cur->ColLen[ColCurNum] < (unsigned)cur->OutLen[ColCurNum])?
	   cur->ColLen[ColCurNum]:(SQLULEN)cur->OutLen[ColCurNum]);
    *(cur->Data[ColCurNum]+len) = '\0';

    /* compare strings here, so don't intern strings unnecessarily*/
    XSB_Deref(op);
    if (isref(op))
      return unify(CTXTc op, makestring(string_find((char *)cur->Data[ColCurNum],1)));
    if (isconstr(op) && get_arity(get_str_psc(op)) == 1) {
      if (!strcmp(get_name(get_str_psc(op)),"string")) {
	return unify(CTXTc get_str_arg(ptoc_tag(CTXTc 4),1),  /* op might have moved! */
		     build_codes_list(CTXTc cur->Data[ColCurNum]));
      } else {
	STRFILE strfile;
	
	strfile.strcnt = strlen((char *)cur->Data[ColCurNum]);
	strfile.strptr = strfile.strbase = cur->Data[ColCurNum];
	iostrs[0] = &strfile;
	read_canonical_term(CTXTc iostrdecode(0),2); /* terminating '.'? */
	return TRUE;
      }
    }
    if (!isstring(op)) return FALSE;
    if (strcmp(string_val(op),(char *)cur->Data[ColCurNum])) return FALSE;
    return TRUE;
  case SQL_C_SLONG:
    {
      Cell h;
      Integer intvalue;
      intvalue = (Integer)(*(int *)(cur->Data[ColCurNum]));  // int is 32 bit in unix
      bld_oint(&h,intvalue);
      return unify(CTXTc op, h);
    }
  case SQL_C_FLOAT:
#ifndef FAST_FLOATS
    {
      Cell h;
      bld_boxedfloat(CTXTc &h,*(float *)(cur->Data[ColCurNum]));
      return unify(CTXTc op, h);
    }
#else
    return unify(CTXTc op,makefloat(*(float *)(cur->Data[ColCurNum])));
#endif
  }
  return FALSE;
}
/*-----------------------------------------------------------------------------*/
/*  FUNCTION NAME:*/
/*     ODBCGetInfo() */
/*  PARAMETERS:*/
/*     R1: 18*/
/*     R2: Connection Handle*/
/*     R3: InfoType*/
/*     R4: Info*/
/*     R5: Return Code*/
/*  NOTES:*/
/*     Returns meta data information about the connection. Type of information */
/* 	   available is detailed in ODBC API under the SQLGetInfo Function. The	   */
/*	   information returned can be of type string or integer.  Return Code     */
/*	   values 0 means success, 1 is error getting the info, -1 is unkown       */
/*	   information type,-2 SQLGetInfo is not supported by the connection.      */
/*																			   */
/*-----------------------------------------------------------------------------*/
void ODBCGetInfo(CTXTdecl)
{
  HDBC hdbc 	  = (HDBC) ptoc_int(CTXTc 2);
  //  SQLRETURN sqlrc = SQL_SUCCESS;

  SQLCHAR strValue[50];
  SQLUSMALLINT  supported;
  SQLRETURN retcode;
  SQLINTEGER nValue;
  SQLSMALLINT pcbValue;

  short int InfoType = (int)ptoc_int(CTXTc 3);
  short int InfoTypeType = GetInfoTypeType(InfoType);

  /* check to see if SQLGetInfo() is supported */
  /*  sqlrc =*/ SQLGetFunctions(hdbc, SQL_API_SQLGETINFO, &supported);

  if (supported == SQL_TRUE)
  {
	  switch(InfoTypeType) {
	  case 0:
		retcode = SQLGetInfo(hdbc, InfoType, strValue, 50, &pcbValue);
		if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
  	    {
			//printf("string type:%d:%s:%d\n",InfoType,strValue,pcbValue);
	      ctop_string(CTXTc 4,(char *)strValue);
			ctop_int(CTXTc 5,0);
  		} else {
		  ctop_int(CTXTc 5,1);
		}
		break;
	  case 1:
	  case 2:
	  case 3:
	  case 4:
	  	retcode = SQLGetInfo(hdbc, InfoType, &nValue, 0, &pcbValue);
		if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
  	    {
			//printf("int type:%d:%d:%d\n",InfoType,nValue,pcbValue);
			ctop_int(CTXTc 4,nValue);
			ctop_int(CTXTc 5,0);
  		} else {
			ctop_int(CTXTc 5,1);
		}
		break;
	  case -1:
	  default:
	  	ctop_int(CTXTc 5, -1);
		break;
	  }
  } else {
	  ctop_int(CTXTc 5,-2);
  }
}
/*------------------------------------------------------------------------------*/
/*  FUNCTION NAME:								*/
/*     ODBCRowCount()								*/
/*  PARAMETERS:									*/
/*     R1: 19									*/
/*     R2: ODBC_Cursor								*/
/*     R3: returned count							*/
/*     R4: Return Code								*/
/*  NOTES:									*/
/*     Returns the row-count value returned by the ODBC call to SQLRowCount.	*/
/*     This value is the count of rows affected by the immediately preceding	*/
/*     executed UPDATE, INSERT, or DELETE statement.				*/
/*     Return Code is 0 for success, 1 error, ...				*/
/*------------------------------------------------------------------------------*/
void ODBCRowCount(CTXTdecl) {
  struct ODBC_Cursor *cur = (struct ODBC_Cursor *)ptoc_int(CTXTc 2);
  SQLLEN count;
  RETCODE rc;

  rc = SQLRowCount(cur->hstmt, &count);
  if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
    ctop_int(CTXTc 3, 0);
    unify(CTXTc reg_term(CTXTc 4), PrintErrorMsg(CTXTc cur));
    return;
  }

  ctop_int(CTXTc 3,count);
  ctop_int(CTXTc 4,0);
  return;
}
