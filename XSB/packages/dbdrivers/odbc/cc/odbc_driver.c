/* File: odbc_driver.c
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
** This is the ODBC driver for connecting to a database.
*/

#include "../../cc/nodeprecate.h"

#include "xsb_config.h"

#ifdef WIN_NT
#include <windows.h>
#endif

#ifdef WIN_NT
#define XSB_DLL
#endif

#ifdef BITS64
#define OUR_C_INT SQL_C_SBIGINT
#else
#define OUR_C_INT SQL_C_SLONG
#endif

#include "odbc_driver_defs.h"

static int driverODBC_getXSBType(SQLSMALLINT dataType);
static void driverODBC_error(SQLSMALLINT handleType, SQLHANDLE handle);
static struct xsb_data** driverODBC_getNextRow(struct driverODBC_queryInfo* query, int direct);
void freeQueryInfo(struct driverODBC_queryInfo* query);
void freeResult(struct xsb_data** result, int numOfElements);
//void freePcbValues(SQLINTEGER** pcbValues, int numOfElements);
void freePcbValues(SQLLEN** pcbValues, int numOfElements);
    
struct driverODBC_connectionInfo* odbcHandles[MAX_CONNECTIONS];
struct driverODBC_queryInfo* odbcQueries[MAX_QUERIES];
int numHandles, numQueries;
static SQLCHAR* errorMesg;
SQLHENV henv;

DllExport int call_conv driverODBC_initialise()
{
  numHandles = numQueries = 0;
  errorMesg = NULL;

  return TRUE;
}


int driverODBC_connect(struct xsb_connectionHandle* handle)
{
  struct driverODBC_connectionInfo* odbcHandle;
  SQLRETURN val;

  odbcHandle = (struct driverODBC_connectionInfo *)malloc(sizeof(struct driverODBC_connectionInfo));

  val = SQLAllocEnv(&henv);
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    errorMesg = (SQLCHAR *) "HENV allocation error\n";
    return FAILURE;
  }

  val = SQLAllocConnect(henv, &(odbcHandle->hdbc));
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_ENV, henv);
    free(odbcHandle);
    return FAILURE;
  }
	
  val = (SQLRETURN) SQLConnect(odbcHandle->hdbc,
			       (SQLCHAR *) handle->dsn,
			       SQL_NTS, (SQLCHAR *) handle->user,
			       SQL_NTS, (SQLCHAR *) handle->password,
			       SQL_NTS);
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_DBC, (SQLCHAR *) odbcHandle->hdbc);
    val = SQLFreeHandle(SQL_HANDLE_DBC, odbcHandle->hdbc);
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO)
      driverODBC_error(SQL_HANDLE_DBC, odbcHandle->hdbc);
    free(odbcHandle);
    return FAILURE;
  }
	
  odbcHandle->handle =
    (char *)malloc((strlen(handle->handle) + 1) * sizeof(char));
  strcpy(odbcHandle->handle, handle->handle);
  odbcHandles[numHandles++] = odbcHandle;
  return SUCCESS;
}


int driverODBC_disconnect(struct xsb_connectionHandle* handle)
{
  SQLRETURN val;
  int i, j;

  for (i = 0 ; i < numHandles ; i++) {
    if (!strcmp(odbcHandles[i]->handle, handle->handle)) {
      val = SQLDisconnect(odbcHandles[i]->hdbc);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_DBC, odbcHandles[i]->hdbc);
	return FAILURE;
      }
	
      val = SQLFreeHandle(SQL_HANDLE_DBC, odbcHandles[i]->hdbc);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_DBC, odbcHandles[i]->hdbc);
	return FAILURE;
      }
			
      free(odbcHandles[i]->handle);			
      free(odbcHandles[i]);
			
      for (j = i + 1 ; j < numHandles ; j++)
	odbcHandles[j-1] = odbcHandles[j];
      odbcHandles[numHandles-1] = NULL;
      numHandles--;
      break;
    }
  }
  return SUCCESS;
}


struct xsb_data** driverODBC_query(struct xsb_queryHandle* handle)
{
  struct driverODBC_queryInfo* query;
  SQLHDBC hdbc;
  SQLRETURN val;
  int i;

  query = NULL;
  hdbc = NULL;
  if (handle->state == QUERY_RETRIEVE) {
    for (i = 0 ; i < numQueries ; i++) {
      if (!strcmp(odbcQueries[i]->handle, handle->handle)) {
	query = odbcQueries[i];
	break;
      }
    }
  }
  else if (handle->state == QUERY_BEGIN) {
    for (i = 0 ; i < numHandles ; i++) {
      if (!strcmp(odbcHandles[i]->handle, handle->connHandle->handle)) {
	hdbc = odbcHandles[i]->hdbc;
	break;
      }
    }
	  
    query = (struct driverODBC_queryInfo *)malloc(sizeof(struct driverODBC_queryInfo));
    query->handle = (char *)malloc((strlen(handle->handle) + 1) * sizeof(char));
    strcpy(query->handle, handle->handle);
    query->query = (char *)malloc((strlen(handle->query) + 1) * sizeof(char));
    strcpy(query->query, handle->query);
    query->resultmeta = NULL;
    query->parammeta = NULL;

    val = SQLAllocStmt(hdbc, &(query->hstmt));
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_DBC, hdbc);
      freeQueryInfo(query);
      return NULL;
    }
		  
    val = SQLExecDirect(query->hstmt, (SQLCHAR *) handle->query, SQL_NTS);
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
      freeQueryInfo(query); 
      return NULL;
    }

    query->resultmeta = (struct driverODBC_meta *)malloc(sizeof(struct driverODBC_meta));
    query->resultmeta->types = NULL;
    val = SQLNumResultCols(query->hstmt, (SQLSMALLINT *)(&(query->resultmeta->numCols)));
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
      freeQueryInfo(query); 
      return NULL;
    }
    handle->numResultCols = query->resultmeta->numCols;
    if (query->resultmeta->numCols == 0) {
      odbcQueries[numQueries++] = query; 
      return NULL;
    }

    query->resultmeta->types = (struct driverODBC_columnmeta **)malloc(query->resultmeta->numCols * sizeof(struct driverODBC_columnmeta *));
    for (i = 0 ; i < query->resultmeta->numCols ; i++) 
      query->resultmeta->types[i] = NULL;
    for (i = 0 ; i < query->resultmeta->numCols ; i++) {
      query->resultmeta->types[i] = (struct driverODBC_columnmeta *)malloc(sizeof(struct driverODBC_columnmeta));
      val = SQLColAttribute(query->hstmt, (SQLUSMALLINT) (i + 1), SQL_COLUMN_TYPE, NULL, 0, NULL, (SQLPOINTER) &(query->resultmeta->types[i]->type));
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
	freeQueryInfo(query); 
	return NULL;
      }
      val = SQLColAttribute(query->hstmt, (SQLUSMALLINT) (i + 1), SQL_COLUMN_LENGTH, NULL, 0, NULL, (SQLPOINTER) &(query->resultmeta->types[i]->length));
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
	freeQueryInfo(query); 
	return NULL;
      }
    }
	  
    handle->state = QUERY_RETRIEVE;
    odbcQueries[numQueries++] = query;
  }
  return driverODBC_getNextRow(query, 1);
}


static struct xsb_data** driverODBC_getNextRow(struct driverODBC_queryInfo* query, int direct)
{
  struct xsb_data** result;
  //  SQLINTEGER** pcbValues;
  SQLLEN** pcbValues;
  SQLRETURN val;
  int i;

  result = (struct xsb_data **)malloc(query->resultmeta->numCols * sizeof(struct xsb_data *));
  //  pcbValues = (SQLINTEGER **)malloc(query->resultmeta->numCols * sizeof(SQLINTEGER *));
  pcbValues = (SQLLEN **)malloc(query->resultmeta->numCols * sizeof(SQLINTEGER *));
  for (i = 0; i<query->resultmeta->numCols; i++){ 
    result[i] = NULL;
    pcbValues[i] = NULL;
  }
  for (i = 0 ; i < query->resultmeta->numCols ; i++) {
    result[i] = (struct xsb_data *)malloc(sizeof(struct xsb_data));
    result[i]->val = (union xsb_value *)malloc(sizeof(union xsb_value));
    //    pcbValues[i] = (SQLINTEGER *)malloc(sizeof(SQLINTEGER));
    pcbValues[i] = (SQLLEN *)malloc(sizeof(SQLINTEGER));
    if (driverODBC_getXSBType(query->resultmeta->types[i]->type) == STRING_TYPE) {
      result[i]->type = STRING_TYPE;
      result[i]->length = query->resultmeta->types[i]->length + 1;
      result[i]->val->str_val = (char *)malloc(result[i]->length * sizeof(char));
      val = SQLBindCol(query->hstmt, (SQLUSMALLINT) (i+1), SQL_C_CHAR, (SQLPOINTER *)result[i]->val->str_val, (SQLINTEGER)result[i]->length, pcbValues[i]);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
        freeResult(result,query->resultmeta->numCols);
        freePcbValues(pcbValues,query->resultmeta->numCols);
	return NULL;
      }
    }
    else if (driverODBC_getXSBType(query->resultmeta->types[i]->type) == INT_TYPE) {
      result[i]->type = INT_TYPE;
      val = SQLBindCol(query->hstmt, (SQLUSMALLINT) (i+1), OUR_C_INT, (SQLPOINTER *)&result[i]->val->i_val, 0, pcbValues[i]);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
        freeResult(result,query->resultmeta->numCols);
        freePcbValues(pcbValues,query->resultmeta->numCols);
        return NULL;
      }
    }
    else if (driverODBC_getXSBType(query->resultmeta->types[i]->type) == FLOAT_TYPE) {
      result[i]->type = FLOAT_TYPE;
      val = SQLBindCol(query->hstmt, (SQLUSMALLINT) (i+1), SQL_C_DOUBLE, (SQLPOINTER *)&result[i]->val->f_val, 0, pcbValues[i]);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
        freeResult(result,query->resultmeta->numCols);
        freePcbValues(pcbValues,query->resultmeta->numCols);
	return NULL;
      }
    }
  }

  val = SQLFetch(query->hstmt);

  if (val == SQL_SUCCESS || val == SQL_SUCCESS_WITH_INFO) {
    for (i = 0 ; i < query->resultmeta->numCols ; i++) {
      if (*(pcbValues[i]) == SQL_NULL_DATA) {
	result[i]->val = NULL;
	result[i]->type = NULL_VALUE_TYPE;
      }
    }
  }

  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    if (direct != 1) {
      val = SQLFreeStmt(query->hstmt, SQL_CLOSE);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO)
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
    }
    freeResult(result,query->resultmeta->numCols);
    freePcbValues(pcbValues,query->resultmeta->numCols);
    return NULL;
  }

  freePcbValues(pcbValues,query->resultmeta->numCols); 
  return result;
}


int driverODBC_prepareStatement(struct xsb_queryHandle* qHandle)
{
  struct driverODBC_queryInfo* query;
  SQLRETURN val;
  SQLHDBC hdbc;
  int i;
	
  hdbc = NULL;

  query = (struct driverODBC_queryInfo *)malloc(sizeof(struct driverODBC_queryInfo));
  query->handle = (char *)malloc((strlen(qHandle->handle) + 1) * sizeof(char));
  strcpy(query->handle, qHandle->handle);
  query->query = (char *)malloc((strlen(qHandle->query) + 1) * sizeof(char));
  strcpy(query->query, qHandle->query);
  query->resultmeta = NULL; 
  query->parammeta = NULL;

  for (i = 0 ; i < numHandles ; i++) {
    if (!strcmp(qHandle->connHandle->handle, odbcHandles[i]->handle)) {
      hdbc = odbcHandles[i]->hdbc;
      break;
    }
  }
	
  val = SQLAllocStmt(hdbc, &(query->hstmt));
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_DBC, hdbc);
    freeQueryInfo(query);
    return FAILURE;
  }

  val = SQLPrepare(query->hstmt, (SQLCHAR *) query->query, SQL_NTS);
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
    freeQueryInfo(query);
    return FAILURE;
  }

  query->parammeta = (struct driverODBC_meta *)malloc(sizeof(struct driverODBC_meta));
  val = SQLNumParams(query->hstmt, &(query->parammeta->numCols));
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
    freeQueryInfo(query);
    return FAILURE;
  }

  query->parammeta->types = (struct driverODBC_columnmeta **)malloc(query->parammeta->numCols * sizeof(struct driverODBC_columnmeta));
  for (i = 0 ; i < query->parammeta->numCols ; i++) 
    query->parammeta->types[i] = NULL;
  for (i = 0 ; i < query->parammeta->numCols ; i++) {
    query->parammeta->types[i] = (struct driverODBC_columnmeta *)malloc(sizeof(struct driverODBC_columnmeta));
    val = SQLDescribeParam(query->hstmt, (SQLUSMALLINT) (i + 1), &(query->parammeta->types[i]->type), &(query->parammeta->types[i]->length), NULL, NULL);
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
      freeQueryInfo(query);
      return FAILURE;
    }
  }
	
  odbcQueries[numQueries++] = query;
  return query->parammeta->numCols;
}


struct xsb_data** driverODBC_execPrepareStatement(struct xsb_data** param, struct xsb_queryHandle* handle)
{
  struct driverODBC_queryInfo* query;
  SQLRETURN val;
  int i;

  val = 0;
  query = NULL; 
  for (i = 0 ; i < numQueries ; i++) {
    if (!strcmp(odbcQueries[i]->handle, handle->handle)) {
      query = odbcQueries[i];
      break;
    }
  }
  if (handle->state == QUERY_RETRIEVE)
    return driverODBC_getNextRow(query, 0);
  
  for (i = 0 ; i < query->parammeta->numCols ; i++) {
    if (param[i]->type == STRING_TYPE) 
      val = SQLBindParameter(query->hstmt, (SQLUSMALLINT)(i + 1), SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(param[i]->val->str_val) + 1, 0, (SQLPOINTER)param[i]->val->str_val, 0, NULL);
    else if (param[i]->type == INT_TYPE)
      val = SQLBindParameter(query->hstmt, (SQLUSMALLINT)(i + 1), SQL_PARAM_INPUT, SQL_C_DEFAULT, SQL_INTEGER, 0, 0, (SQLPOINTER)&param[i]->val->i_val, 0, NULL);
    else if (param[i]->type == FLOAT_TYPE)	
      val = SQLBindParameter(query->hstmt, (SQLUSMALLINT) (i + 1), SQL_PARAM_INPUT, SQL_C_DEFAULT, SQL_DOUBLE, 0, 0, (SQLPOINTER)&param[i]->val->f_val, 0, NULL);
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
      return NULL; 
    }
  }
  
  val = SQLExecute(query->hstmt);
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
    return NULL;
  }

  query->resultmeta = (struct driverODBC_meta *)malloc(sizeof(struct driverODBC_meta));
  query->resultmeta->types = NULL; 
  val = SQLNumResultCols(query->hstmt, (SQLSMALLINT *)(&(query->resultmeta->numCols)));
  if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
    driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
    return NULL;
  }
  handle->numResultCols = query->resultmeta->numCols;
  if (query->resultmeta->numCols == 0) {
    free(query->resultmeta);
    query->resultmeta=NULL;
    return NULL; 
  }

  query->resultmeta->types = (struct driverODBC_columnmeta **)malloc(query->resultmeta->numCols * sizeof(struct driverODBC_columnmeta *));
  for (i = 0 ; i < query->resultmeta->numCols ; i++) 
    query->resultmeta->types[i] = NULL;
  for (i = 0 ; i < query->resultmeta->numCols ; i++) {
    query->resultmeta->types[i] = (struct driverODBC_columnmeta *)malloc(sizeof(struct driverODBC_columnmeta));
    val = SQLColAttribute(query->hstmt, (SQLUSMALLINT) (i + 1), SQL_COLUMN_TYPE, NULL, 0, NULL, (SQLPOINTER) &(query->resultmeta->types[i]->type));
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
      return NULL;
    }
    val = SQLColAttribute(query->hstmt, (SQLUSMALLINT) (i + 1), SQL_COLUMN_LENGTH, NULL, 0, NULL, (SQLPOINTER) &(query->resultmeta->types[i]->length));
    if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
      driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
      return NULL;
    }
  }

  handle->state = QUERY_RETRIEVE;
  return driverODBC_getNextRow(query, 0);
}


int driverODBC_closeStatement(struct xsb_queryHandle* handle)
{
  struct driverODBC_queryInfo* query;
  SQLRETURN val;
  int i, j;

  for (i = 0 ; i < numQueries ; i++) {
    if (!strcmp(odbcQueries[i]->handle, handle->handle)) {
      query = odbcQueries[i];
        
      val = SQLFreeHandle(SQL_HANDLE_STMT, query->hstmt);
      if (val != SQL_SUCCESS && val != SQL_SUCCESS_WITH_INFO) {
	driverODBC_error(SQL_HANDLE_STMT, query->hstmt);
	return FAILURE;
      }
      for (j = i + 1 ; j < numQueries ; j++)
	odbcQueries[j-1] = odbcQueries[j];
      freeQueryInfo(query); 
      numQueries--;
    }
  }
  return SUCCESS;
}


char* driverODBC_errorMesg()
{
  char* temp;
  if (errorMesg != NULL) {
    temp = (char *)malloc(SQL_MAX_MESSAGE_LENGTH * sizeof(char));
    strcpy(temp, (char *)errorMesg);
    free(errorMesg);
    errorMesg = NULL;
    return temp;
  }
  return NULL;
}


static int driverODBC_getXSBType(SQLSMALLINT dataType)
{
  int type;
  type = STRING_TYPE;
  switch (dataType) {
  case SQL_CHAR:
  case SQL_VARCHAR:
  case SQL_LONGVARCHAR:
    type = STRING_TYPE;
    break;

  case SQL_DECIMAL:
  case SQL_NUMERIC:
  case SQL_REAL:    
  case SQL_DOUBLE:
    type = FLOAT_TYPE;
    break;

  case SQL_SMALLINT:
  case SQL_INTEGER:
  case SQL_TINYINT:
  case SQL_BIGINT:
    type = INT_TYPE;
    break;
  }

  return type;
}


static void driverODBC_error(SQLSMALLINT handleType, SQLHANDLE handle)
{
  SQLCHAR* sqlState;
	
  if (errorMesg != NULL) {
    free(errorMesg);
    errorMesg = NULL;
  }

  errorMesg = (SQLCHAR *)malloc(SQL_MAX_MESSAGE_LENGTH * sizeof(SQLCHAR));
  sqlState = (SQLCHAR *)malloc((SQL_SQLSTATE_SIZE+1) * sizeof(SQLCHAR));
  SQLGetDiagRec(handleType, handle, 1, sqlState, NULL, errorMesg, SQL_MAX_MESSAGE_LENGTH - 1, NULL);
  free(sqlState);
}

void driverODBC_freeResult(struct xsb_data** result, int numOfElements)
{
  int i;
  if (result != NULL) {
    for (i=0; i<numOfElements; i++) {
      if(result[i]!=NULL){
	if(result[i]->val != NULL){
	  if(result[i]->type==STRING_TYPE && result[i]->val->str_val != NULL) { 
	    free(result[i]->val->str_val);
	    result[i]->val->str_val = NULL;
	  }

	  free(result[i]->val);
	  result[i]->val = NULL;
	}
	free(result[i]);
	result[i]= NULL;
      }
    }
    free(result);
    result = NULL;
  }
  return;
}


DllExport int call_conv driverODBC_register(void)
{
  union functionPtrs* funcConnect;
  union functionPtrs* funcDisconnect;
  union functionPtrs* funcQuery;
  union functionPtrs* funcPrepare;
  union functionPtrs* funcExecPrepare;
  union functionPtrs* funcCloseStmt;
  union functionPtrs* funcErrorMesg;
  union functionPtrs* funcFreeResult;

  registerXSBDriver("odbc", NUMBER_OF_ODBC_DRIVER_FUNCTIONS);

  funcConnect = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcConnect->connectDriver = driverODBC_connect;
  registerXSBFunction("odbc", CONNECT, funcConnect);

  funcDisconnect = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcDisconnect->disconnectDriver = driverODBC_disconnect;
  registerXSBFunction("odbc", DISCONNECT, funcDisconnect);

  funcQuery = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcQuery->queryDriver = driverODBC_query;
  registerXSBFunction("odbc", QUERY, funcQuery);

  funcPrepare = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcPrepare->prepareStmtDriver = driverODBC_prepareStatement;
  registerXSBFunction("odbc", PREPARE, funcPrepare);
	
  funcExecPrepare = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcExecPrepare->executeStmtDriver = driverODBC_execPrepareStatement;
  registerXSBFunction("odbc", EXEC_PREPARE, funcExecPrepare);

  funcCloseStmt = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcCloseStmt->closeStmtDriver = driverODBC_closeStatement;
  registerXSBFunction("odbc", CLOSE_STMT, funcCloseStmt);

  funcErrorMesg = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcErrorMesg->errorMesgDriver = driverODBC_errorMesg;
  registerXSBFunction("odbc", ERROR_MESG, funcErrorMesg);

  funcFreeResult = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcFreeResult->freeResultDriver = driverODBC_freeResult;
  registerXSBFunction("odbc", FREE_RESULT, funcFreeResult);

  return TRUE;
}


void freeQueryInfo(struct driverODBC_queryInfo* query)
{
  int i;
  if (query == NULL)
    return;

  if (query->query != NULL)
    free(query->query);
  if (query->handle != NULL)
    free(query->handle);
  if (query->resultmeta != NULL) {
    for (i = 0 ; i < query->resultmeta->numCols; i++)
      if (query->resultmeta->types[i] != NULL)
        free(query->resultmeta->types[i]);
    if (query->resultmeta->types != NULL)
      free(query->resultmeta->types);
    free(query->resultmeta);
  }
  if (query->parammeta != NULL) {
    for (i = 0; i<query->parammeta->numCols; i++)
      if (query->parammeta->types[i] != NULL)
        free(query->parammeta->types[i]);
    if (query->parammeta->types != NULL)
      free(query->parammeta->types);
    free(query->parammeta);
  }
  free(query);
  query = NULL;
  return;
}


void freeResult(struct xsb_data** result, int numOfElements)
{
  int i;
  if (result == NULL)
    return;
  for (i =0;i<numOfElements;i++){
    if (result[i] != NULL) {
      if (result[i]->type == STRING_TYPE)
        free(result[i]->val->str_val);
      free(result[i]->val);
      free(result[i]);
    }
  }
  free(result);
  result = NULL;
  return;
}


//void freePcbValues(SQLINTEGER** pcbValues, int numOfElements)
void freePcbValues(SQLLEN** pcbValues, int numOfElements)
{
  int i;
  if (pcbValues == NULL)
    return;
  for (i =0;i<numOfElements;i++){
    if(pcbValues[i]!=NULL)
      free(pcbValues[i]);
  }
  free(pcbValues);
  pcbValues =NULL;
  return;
}
