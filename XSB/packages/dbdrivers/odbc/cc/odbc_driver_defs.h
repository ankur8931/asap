/* File: odbc_driver.h
** Author: Saikat Mukherjee
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2002-2008
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
** This is the header file for the ODBC driver.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>

#include "cinterf.h"
#include "driver_manager_defs.h"


/****** data structures for metadata *****/

struct driverODBC_columnmeta
{
	SQLSMALLINT type;
	SQLULEN length;
};

struct driverODBC_meta
{
	SQLSMALLINT numCols;
	struct driverODBC_columnmeta** types;
};

/******* data structures for handles ******/

struct driverODBC_queryInfo
{
	char* query;
	char* handle;
	SQLHSTMT hstmt;
	struct driverODBC_meta* resultmeta;
	struct driverODBC_meta* parammeta;
};

struct driverODBC_connectionInfo
{
	SQLHDBC hdbc;
	char* handle;
};

/********* function declarations *********/

DllExport int call_conv driverODBC_initialise();
int driverODBC_connect(struct xsb_connectionHandle* handle);
int driverODBC_disconnect(struct xsb_connectionHandle* handle);
struct xsb_data** driverODBC_query(struct xsb_queryHandle* handle);
int driverODBC_prepareStatement(struct xsb_queryHandle* qHandle);
struct xsb_data** driverODBC_execPrepareStatement(struct xsb_data** param, struct xsb_queryHandle* handle);
int driverODBC_closeStatement(struct xsb_queryHandle* handle);
char* driverODBC_errorMesg();
void driverODBC_freeResult();
DllExport int call_conv driverODBC_register();


DllExport extern int call_conv registerXSBDriver(char* dr, int num);
DllExport extern int call_conv registerXSBFunction(char* dr, int type, union functionPtrs* func);

#define NUMBER_OF_ODBC_DRIVER_FUNCTIONS  8

