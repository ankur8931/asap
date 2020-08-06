/* File: driver_manager_defs.h
** Author: Saikat Mukherjee
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2002-2006
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
*/

/*
** Data structures, function declarations for the driver_manager
*/

#ifdef WIN_NT
#include <windows.h>
#endif

#ifdef WIN_NT
#define XSB_DLL
#endif

#define MAX_CONNECTIONS 200
#define MAX_DRIVERS 100
#define MAX_QUERIES 2000

#define CONNECT 0 
#define DISCONNECT 1
#define QUERY 2
#define PREPARE 3
#define EXEC_PREPARE 4
#define CLOSE_STMT 5
#define ERROR_MESG 6
#define FREE_RESULT 7

#define INT_TYPE 1
#define FLOAT_TYPE 2
#define STRING_TYPE 3
#define TERM_TYPE 4

#define SUCCESS 0
#define FAILURE -1

#define QUERY_BEGIN 0
#define QUERY_RETRIEVE 1

#define QUERY_SIZE 56000
#define ELEMENT_SIZE 1500

#define DB_INTERFACE_TERM_SYMBOL '\177'

// codes returned by bindReturnList
#define RESULT_EMPTY_BUT_REQUESTED 0
#define RESULT_NONEMPTY_OR_NOT_REQUESTED 1
#define TOO_MANY_RETURN_COLS 2
#define INVALID_RETURN_LIST 3
#define TOO_FEW_RETURN_COLS 4

#define UNKNOWN_DB_ERROR "unknown error from the database"

// type of return value when it is a NULL
#define NULL_VALUE_TYPE  -999

// **** the database value data structures ****

union xsb_value
{
  Integer i_val;
  double f_val;
  char* str_val;
};

struct xsb_data
{
  int type;
  size_t length;
  union xsb_value* val;
};

// **** the handle data structures ****

struct xsb_connectionHandle
{
  char* handle; //key
  char* server;
  char* user;
  char* database;
  char* password;
  char* dsn;
  char* driver;
};

struct xsb_queryHandle
{
  struct xsb_connectionHandle* connHandle;
  char* handle;      /* key */
  char* query;
  int state;
  int numParams;     /* number of parameters to prepare statement */
  int numResultCols; /* number of columns in the result */
};

// **** the driver data structures ****


struct driverFunction
{
  int functionType;
  union functionPtrs* functionName;	
};

struct driver
{
  char* driver;
  int numberFunctions;
  struct driverFunction** functions;
};

// **** the function ptrs and function declarations ****

union functionPtrs
{
  int (*connectDriver)(struct xsb_connectionHandle*);
  int (*disconnectDriver)(struct xsb_connectionHandle*);
  struct xsb_data** (*queryDriver)(struct xsb_queryHandle*);
  int (*prepareStmtDriver)(struct xsb_queryHandle*);
  struct xsb_data** (*executeStmtDriver)(struct xsb_data**, struct xsb_queryHandle*);
  int (*closeStmtDriver)(struct xsb_queryHandle*);
  char* (*errorMesgDriver)();
  void (*freeResultDriver)();
};

DllExport int call_conv registerXSBDriver(char* driver, int num);
DllExport int call_conv registerXSBFunction(char* dr, int type, union functionPtrs* func);



