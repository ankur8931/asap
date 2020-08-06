/* File: mysql_driver.c
** Author: Saikat Mukherjee, Hui Wan
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
** This is the driver for connecting to a MySQL database.
** This is invoked from the middle_layer module in emu.
*/

#include "../../cc/nodeprecate.h"

#include "xsb_config.h"

#ifdef WIN_NT
#include <windows.h>
#endif

#ifdef WIN_NT
#define XSB_DLL
#endif

#include "mysql_driver_defs.h"

static void driverMySQL_error(MYSQL* mysql);
static int driverMySQL_getXSBType(MYSQL_FIELD* field);
static struct xsb_data** driverMySQL_getNextRow(struct driverMySQL_queryInfo* query);
static struct xsb_data** driverMySQL_prepNextRow(struct driverMySQL_preparedresultset* rs);

void freeQueryInfo(struct driverMySQL_queryInfo* query);
void freeConnection(struct driverMySQL_connectionInfo* connection);
//void freeResult(struct xsb_data** result, int numOfElements);
void freeResultset(struct driverMySQL_preparedresultset* rs);
void freeBind(MYSQL_BIND* bind, int num);

struct driverMySQL_connectionInfo* mysqlHandles[MAX_CONNECTIONS];
struct driverMySQL_queryInfo* mysqlQueries[MAX_QUERIES];
struct driverMySQL_preparedresultset* prepQueries[MAX_QUERIES];
int numHandles, numQueries;
int numPrepQueries;

static const char* errorMesg;


DllExport int call_conv driverMySQL_initialise()
{
  numHandles = 0;
  numQueries = 0;
  numPrepQueries = 0;
  errorMesg = NULL;

  return TRUE;
}


int driverMySQL_connect(struct xsb_connectionHandle* handle)
{
  struct driverMySQL_connectionInfo* mysqlHandle;
  MYSQL* mysql = mysql_init( NULL );
  if ( mysql == NULL )
    {
      errorMesg = "mysql_init() failed\n";	
      return FAILURE;
    }
	
  if (!mysql_real_connect(mysql, handle->server, handle->user, handle->password, handle->database, 0, NULL, 0))
    {
      driverMySQL_error(mysql);
      free(mysql);
      mysql = NULL;
      return FAILURE; 
    }
	
  mysqlHandle = (struct driverMySQL_connectionInfo *)malloc(sizeof(struct driverMySQL_connectionInfo));
  mysqlHandle->handle = (char *)malloc((strlen(handle->handle) + 1) * sizeof(char));
  strcpy(mysqlHandle->handle, handle->handle);
  mysqlHandle->mysql = mysql;
  mysqlHandles[numHandles++] = mysqlHandle;

  return SUCCESS;
}


int driverMySQL_disconnect(struct xsb_connectionHandle* handle)
{
  int i, j;

  for (i = 0 ; i < numHandles ; i++)
    {
      if (!strcmp(handle->handle, mysqlHandles[i]->handle))
	{
	  mysql_close(mysqlHandles[i]->mysql);
	  freeConnection(mysqlHandles[i]);
	  for (j = i + 1 ; j < numHandles ; j++)
	    mysqlHandles[j-1] = mysqlHandles[j];
	  numHandles--;
	  break;
	}
    }

  return SUCCESS;
}


struct xsb_data** driverMySQL_query(struct xsb_queryHandle* handle)
{
  struct driverMySQL_connectionInfo* connection;
  struct driverMySQL_queryInfo* query;
  MYSQL_RES* resultSet;
  int i,n;

  query = NULL;
  connection = NULL;
  resultSet = NULL;
  if (handle->state == QUERY_RETRIEVE)
    {
      for (i = 0 ; i < numQueries ; i++)
	{
	  if (!strcmp(mysqlQueries[i]->handle, handle->handle))
	    {
	      query = mysqlQueries[i];
	      break;
	    }
	}
    }
  else if (handle->state == QUERY_BEGIN)
    {
      for (i = 0 ; i < numHandles ; i++)
	{
	  if (!strcmp(mysqlHandles[i]->handle, handle->connHandle->handle))
	    {
	      connection = mysqlHandles[i];
	      break;
	    }
	}
      query = (struct driverMySQL_queryInfo *)malloc(sizeof(struct driverMySQL_queryInfo));
      query->handle = (char *)malloc((strlen(handle->handle) + 1) * sizeof(char));
      strcpy(query->handle, handle->handle);
      query->query = (char *)malloc((strlen(handle->query) + 1) * sizeof(char));
      strcpy(query->query, handle->query);
      query->connection = connection;
      query->resultSet = NULL;

      if (mysql_query(query->connection->mysql, query->query))
	{
	  driverMySQL_error(query->connection->mysql);
	  freeQueryInfo(query);
	  return NULL;	
	}
      else
	{
	  if ((resultSet = mysql_store_result(query->connection->mysql)) == NULL)
	    {
	      freeQueryInfo(query);
	      return NULL;
	    }
	  query->resultSet = resultSet;
	  mysqlQueries[numQueries++] = query;
	  handle->state = QUERY_RETRIEVE;
	  n = mysql_num_fields(resultSet);
	  handle->numResultCols = n;
	  query->returnFields = n;
	}
    }

  return driverMySQL_getNextRow(query);
}


static struct xsb_data** driverMySQL_getNextRow(struct driverMySQL_queryInfo* query)
{
  struct xsb_data** result;
  MYSQL_ROW row;
  int numFields;
  int i, j;
  char** p_temp=NULL;

  result = NULL;
  if ((row = mysql_fetch_row(query->resultSet)) == NULL)
    {
      if (mysql_errno(query->connection->mysql))
	driverMySQL_error(query->connection->mysql);
      else
	{
	  for (i = 0 ; i < numQueries ; i++)
	    {
	      if (!strcmp(mysqlQueries[i]->handle, query->handle))
		{
		  freeQueryInfo(query);
		  for (j = i + 1 ; j < numQueries ; j++)
		    mysqlQueries[j-1] = mysqlQueries[j];
		  numQueries--;
		}
	    }
	}
      return NULL;
    }

  numFields = query->returnFields;
  result = (struct xsb_data **)malloc(numFields * sizeof(struct xsb_data *));
  for (i = 0 ; i < numFields ; i++) {
      result[i] = (struct xsb_data *)malloc(sizeof(struct xsb_data));
      result[i]->val = (union xsb_value *)malloc(sizeof(union xsb_value));
      result[i]->type = driverMySQL_getXSBType(mysql_fetch_field_direct(query->resultSet, i));

      if (row[i] == NULL)
	result[i]->type = NULL_VALUE_TYPE;

      switch (result[i]->type) {
	case INT_TYPE:
	  result[i]->val->i_val = strtol(row[i],p_temp,10);
	  break;

	case FLOAT_TYPE:
	  result[i]->val->f_val = strtod(row[i],p_temp);
	  break;

	case STRING_TYPE:
	  result[i]->val->str_val = (char *)malloc((strlen(row[i])+1) * sizeof(char));
	  strcpy(result[i]->val->str_val, (char *)row[i]);
	  break;

	case NULL_VALUE_TYPE:
	  break;
	}
    }

  return result;
}


/***** PREPARED STATEMENT FUNCTIONALITY (for MySQL 5.1 version) *****/


int driverMySQL_prepareStatement(struct xsb_queryHandle* handle)
{
  struct driverMySQL_preparedresultset* rs;
  MYSQL* mysql;
  MYSQL_STMT* stmt;
  MYSQL_RES* res;
  int i;
  char* sqlQuery;
  int numResultCols;
  MYSQL_FIELD* field;

  sqlQuery = (char *)malloc((strlen(handle->query) + 1) * sizeof(char));
  strcpy(sqlQuery, handle->query);
	
  mysql = NULL;	

  for (i = 0 ; i < numHandles ; i++)
    {
      if (!strcmp(mysqlHandles[i]->handle, handle->connHandle->handle))
	{
	  mysql = mysqlHandles[i]->mysql;
	  break;
	}
    }

  if ((stmt = mysql_stmt_init(mysql)) == NULL)
    {
      errorMesg = "mysql_stmt_init() failed\n";
      //errorMesg = mysql_stmt_error(stmt);
      free(sqlQuery);
      sqlQuery = NULL;
      return FAILURE;		
    }
  if ( mysql_stmt_prepare(stmt, sqlQuery, strlen(sqlQuery)))
    {
      errorMesg = mysql_stmt_error(stmt);
      free(sqlQuery);
      sqlQuery = NULL;
      return FAILURE;		
    }

  rs = (struct driverMySQL_preparedresultset *)malloc(sizeof(struct driverMySQL_preparedresultset));
  rs->statement = stmt;

  res = mysql_stmt_result_metadata(stmt);

  numResultCols = 0;
  if (res == NULL)
    {
      if (mysql_errno(mysql))
	{
	  errorMesg = mysql_stmt_error(stmt);
	  free(sqlQuery);
	  sqlQuery = NULL;
	  free(rs);
	  rs = NULL;
	  return FAILURE;
	}
    }
  else numResultCols = mysql_num_fields(res);

  rs->returnFields = numResultCols;

  handle->numParams = mysql_stmt_param_count(stmt);
  handle->numResultCols = rs->returnFields;
  handle->state = QUERY_BEGIN;
	
  rs->handle = handle;
  rs->bindResult = NULL;
	
  rs->metaInfo = (struct xsb_data **)malloc(rs->returnFields * sizeof(struct xsb_data *));
  for (i = 0 ; i < rs->returnFields ; i++)
    {
      rs->metaInfo[i] = (struct xsb_data *)malloc(sizeof(struct xsb_data));
      field = mysql_fetch_field_direct(res, i);
      rs->metaInfo[i]->type = driverMySQL_getXSBType(field);
      rs->metaInfo[i]->length = field->length;
    }
  prepQueries[numPrepQueries++] = rs;

  free(sqlQuery);
  sqlQuery = NULL;

  return rs->handle->numParams;
}

struct xsb_data** driverMySQL_execPrepareStmt(struct xsb_data** bindValues, struct xsb_queryHandle* handle)
{
  struct driverMySQL_preparedresultset* rs;
  int i, numOfParams;
  MYSQL_BIND *bind, *bindResult;
	
  int* intTemp;
  double* doubleTemp;
  unsigned long* lengthTemp;
  char* charTemp;

  rs = NULL;

  for (i = 0 ; i < numPrepQueries; i++)
    {
      if (!strcmp(prepQueries[i]->handle->handle, handle->handle))
	{
	  rs = prepQueries[i];
	  break;
	}
    }

  if (rs == NULL)
    {
      errorMesg = "no specified query handle exists";
      return NULL;
    }
  if (handle->state == QUERY_RETRIEVE)
    {
      return driverMySQL_prepNextRow(rs);		
    }
	
  numOfParams = rs->handle->numParams;
  bind = (MYSQL_BIND *)calloc( numOfParams, sizeof(MYSQL_BIND));
  memset(bind, 0, sizeof(*bind));
  for (i = 0 ; i < numOfParams ; i++)
    {
      if (bindValues[i]->type == INT_TYPE)
	{			
	  bind[i].buffer_type = MYSQL_TYPE_LONG;
	  intTemp = (int*)malloc (sizeof(int));
	  *intTemp = bindValues[i]->val->i_val;
	  bind[i].buffer = intTemp;
	  bind[i].is_null = calloc(1,sizeof(my_bool));
	}
      else if (bindValues[i]->type == FLOAT_TYPE)
	{		        
	  bind[i].buffer_type = MYSQL_TYPE_DOUBLE;
	  doubleTemp = (double*)malloc (sizeof(double));
	  *doubleTemp = bindValues[i]->val->f_val;
	  bind[i].buffer = doubleTemp;
	  bind[i].is_null = calloc(1,sizeof(my_bool)) ;  			
	}
      else if (bindValues[i]->type == STRING_TYPE)
	{
	  bind[i].buffer_type = MYSQL_TYPE_STRING;
	  lengthTemp = (unsigned long*)malloc (sizeof(unsigned long));
	  *lengthTemp = strlen(bindValues[i]->val->str_val);
	  bind[i].length = lengthTemp;
	  bind[i].buffer_length = strlen(bindValues[i]->val->str_val);
	  bind[i].is_null = calloc(1,sizeof(my_bool)) ;    
	  charTemp = (char*)malloc((strlen(bindValues[i]->val->str_val)+1) * sizeof(char));
	  strcpy( charTemp, bindValues[i]->val->str_val);
	  bind[i].buffer = charTemp;
	}
    }
  if (mysql_stmt_bind_param(rs->statement, bind))
    {
      errorMesg = mysql_stmt_error(rs->statement);
      freeBind(bind,numOfParams);
      return NULL;
    }
  if (mysql_stmt_execute(rs->statement))
    {
      errorMesg = mysql_stmt_error(rs->statement);
      freeBind(bind,numOfParams);
      return NULL;
    }

  if (rs->returnFields == 0)
    {
      freeBind(bind,numOfParams);
      return NULL;
    }

  bindResult = (MYSQL_BIND *)malloc(rs->returnFields * sizeof(MYSQL_BIND));
  memset(bindResult, 0, sizeof(*bindResult));
  for (i = 0 ; i < rs->returnFields ; i++)
    {
      switch (rs->metaInfo[i]->type)
	{
	case INT_TYPE:
	  bindResult[i].buffer_type = MYSQL_TYPE_LONG;
	  bindResult[i].buffer = malloc( sizeof(int) );
	  bindResult[i].is_null = calloc(1,sizeof(my_bool)) ;
	  bindResult[i].length = malloc( sizeof(unsigned long) );
	  bindResult[i].error = calloc(1,sizeof(my_bool)) ; 
	  break;

	case FLOAT_TYPE:
	  bindResult[i].buffer_type = MYSQL_TYPE_DOUBLE;
	  bindResult[i].buffer = malloc( sizeof(double) );
	  bindResult[i].is_null = calloc(1,sizeof(my_bool));
	  bindResult[i].length = malloc( sizeof(unsigned long) ) ;
	  bindResult[i].error = calloc(1,sizeof(my_bool)) ;
	  break;
			
	case STRING_TYPE:
	  bindResult[i].buffer_type = MYSQL_TYPE_VAR_STRING;
	  bindResult[i].buffer_length = rs->metaInfo[i]->length+1;
	  bindResult[i].buffer = malloc((rs->metaInfo[i]->length+1) * sizeof(char));
	  bindResult[i].is_null = calloc(1,sizeof(my_bool)) ;
	  bindResult[i].length = malloc( sizeof(unsigned long) );
	  *(bindResult[i].length) =  rs->metaInfo[i]->length+1;
	  bindResult[i].error = calloc(1,sizeof(my_bool)) ;
	  break;
	}
    }
	
  if (mysql_stmt_bind_result(rs->statement, bindResult))
    {
      errorMesg = mysql_stmt_error(rs->statement);
      mysql_stmt_free_result(rs->statement);
      mysql_stmt_reset(rs->statement);
      freeBind(bind,numOfParams);
      freeBind(bindResult,rs->returnFields);
      return NULL;
    }

  if(rs->bindResult!=NULL)
    freeBind(rs->bindResult,rs->returnFields);

  rs->bindResult = bindResult;

  handle->state = QUERY_RETRIEVE;

  freeBind(bind,numOfParams);

  return driverMySQL_prepNextRow(rs);
}

struct xsb_data** driverMySQL_prepNextRow(struct driverMySQL_preparedresultset* rs)
{

  struct xsb_data ** result = NULL;
  MYSQL_BIND *bindResult = rs->bindResult;
  int i;

  int flag;
  flag = mysql_stmt_fetch(rs->statement);
  if (flag == MYSQL_NO_DATA)
    {
      mysql_stmt_free_result(rs->statement);
      mysql_stmt_reset(rs->statement);
      return NULL;
    }

  result = (struct xsb_data **)malloc(rs->returnFields * sizeof(struct xsb_data *));
  for ( i = 0 ; i < rs->returnFields ; i++){

    result[i] = (struct xsb_data *)malloc(sizeof(struct xsb_data));
    result[i]->type = rs->metaInfo[i]->type;
    result[i]->length = rs->metaInfo[i]->length;
    result[i]->val = (union xsb_value *)malloc(sizeof(union xsb_value));
    result[i]->val->str_val = NULL;

    switch (result[i]->type){
    case INT_TYPE:
      result[i]->val->i_val = *( (int*) bindResult[i].buffer );
      break;
    case FLOAT_TYPE:
      result[i]->val->f_val = *( (double*) bindResult[i].buffer );
      break;
    case STRING_TYPE:
      result[i]->val->str_val = (char *)malloc((strlen((char *)(bindResult[i].buffer))+1) * sizeof(char));
      strcpy( result[i]->val->str_val, bindResult[i].buffer );
      break;
    }
  }


  return result;
}

int driverMySQL_closeStatement(struct xsb_queryHandle* handle)
{
  struct driverMySQL_preparedresultset* rs;
  int i, j;

  for (i = 0 ; i < numPrepQueries ; i++) {
    if (!strcmp(prepQueries[i]->handle->handle, handle->handle)) {

      rs = prepQueries[i];

      for (j = i + 1 ; j < numPrepQueries ; j++)
	prepQueries[j-1] = prepQueries[j];

      freeResultset(rs);
      numPrepQueries--;
      break;

    }
  }

  return SUCCESS;
}

/***** END OF PREPARED STATEMENT FUNCTIONALITY ***** */


char* driverMySQL_errorMesg()
{
  char* temp;
  if (errorMesg != NULL)
    {
      temp = (char *)malloc((strlen(errorMesg) + 1) * sizeof(char));
      strcpy(temp, errorMesg);
      errorMesg = NULL;
      return temp;
    }
  return NULL;
}

static void driverMySQL_error(MYSQL* mysql)
{
  errorMesg = mysql_error(mysql);
}

static int driverMySQL_getXSBType(MYSQL_FIELD* field)
{
  int type;

  type = 0;
  switch (field->type)
    {
    case FIELD_TYPE_TINY:
    case FIELD_TYPE_SHORT:
    case FIELD_TYPE_LONG:
    case FIELD_TYPE_INT24:
    case FIELD_TYPE_LONGLONG:
      type = INT_TYPE;
      break;
		
    case FIELD_TYPE_DECIMAL:
    case FIELD_TYPE_FLOAT:
    case FIELD_TYPE_DOUBLE:
      type = FLOAT_TYPE;
      break;

    case FIELD_TYPE_STRING:
    case FIELD_TYPE_DATE:
    case FIELD_TYPE_TIMESTAMP:
    case FIELD_TYPE_TIME:
    case FIELD_TYPE_DATETIME:
    case FIELD_TYPE_YEAR:
    case FIELD_TYPE_NEWDATE:
    case FIELD_TYPE_ENUM:
    case FIELD_TYPE_SET:
    case FIELD_TYPE_TINY_BLOB:
    case FIELD_TYPE_MEDIUM_BLOB:
    case FIELD_TYPE_LONG_BLOB:
    case FIELD_TYPE_BLOB:
    case FIELD_TYPE_VAR_STRING:
    case FIELD_TYPE_NULL:
    default:
      type = STRING_TYPE;
      break;
    }

  return type;
}

void driverMySQL_freeResult(struct xsb_data** result, int numOfElements)
{
  int i;
  if (result != NULL) {
    for (i=0; i<numOfElements; i++) {
      if(result[i]!=NULL){
	if(result[i]->val != NULL){
	  if(result[i]->type == STRING_TYPE && result[i]->val->str_val != NULL )
	    { 
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


DllExport int call_conv driverMySQL_register(void)
{
  union functionPtrs* funcConnect;
  union functionPtrs* funcDisconnect;
  union functionPtrs* funcQuery;
  union functionPtrs* funcErrorMesg;
  union functionPtrs* funcPrepare;
  union functionPtrs* funcExecPrepare;
  union functionPtrs* funcCloseStmt;
  union functionPtrs* funcFreeResult;

  registerXSBDriver("mysql", NUMBER_OF_MYSQL_DRIVER_FUNCTIONS);

  funcConnect = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcConnect->connectDriver = driverMySQL_connect;
  registerXSBFunction("mysql", CONNECT, funcConnect);

  funcDisconnect = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcDisconnect->disconnectDriver = driverMySQL_disconnect;
  registerXSBFunction("mysql", DISCONNECT, funcDisconnect);

  funcQuery = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcQuery->queryDriver = driverMySQL_query;
  registerXSBFunction("mysql", QUERY, funcQuery);

  funcPrepare = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcPrepare->prepareStmtDriver = driverMySQL_prepareStatement;
  registerXSBFunction("mysql", PREPARE, funcPrepare);

  funcExecPrepare = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcExecPrepare->executeStmtDriver = driverMySQL_execPrepareStmt;
  registerXSBFunction("mysql", EXEC_PREPARE, funcExecPrepare);

  funcCloseStmt = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcCloseStmt->closeStmtDriver = driverMySQL_closeStatement;
  registerXSBFunction("mysql", CLOSE_STMT, funcCloseStmt);
	
  funcErrorMesg = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcErrorMesg->errorMesgDriver = driverMySQL_errorMesg;
  registerXSBFunction("mysql", ERROR_MESG, funcErrorMesg);

  funcFreeResult = (union functionPtrs *)malloc(sizeof(union functionPtrs));
  funcFreeResult->freeResultDriver = driverMySQL_freeResult;
  registerXSBFunction("mysql", FREE_RESULT, funcFreeResult);

  return TRUE;
}

void freeQueryInfo(struct driverMySQL_queryInfo* query)
{
  if (query == NULL)
    return;


  if (query->handle != NULL)
    { free(query->handle);
      query->handle = NULL;
    }

  if (query->query != NULL)
    { free(query->query);
      query->query = NULL;
    }

  if (query->resultSet != NULL)
    mysql_free_result(query->resultSet);
  //  if (query->resultSet != NULL)
  //free(query->resultSet);
  
  free(query);
  query = NULL;
  return;
}

void freeConnection(struct driverMySQL_connectionInfo* connection)
{
  if (connection == NULL)
    return;

  if (connection->handle!= NULL)
    { free(connection->handle);
      connection->handle = NULL;
    }
  free(connection);
  connection = NULL;
  return;
}

void freeMetaInfo(struct xsb_data** metaInfo, int num)
{
  int i;
  if (metaInfo != NULL) {
    for (i=0; i<num; i++) {
      if(metaInfo[i]!=NULL)
	{ free(metaInfo[i]);
	  metaInfo[i] = NULL;
	}
    }
    free(metaInfo);
    metaInfo = NULL;
  }
  return;
}

void freeResultset(struct driverMySQL_preparedresultset* rs)
{

  if (rs == NULL)
    return;

  freeMetaInfo(rs->metaInfo,rs->returnFields);
  freeBind(rs->bindResult,rs->returnFields);

  if ( rs->statement != NULL )
    {
      mysql_stmt_free_result(rs->statement); 
      mysql_stmt_close(rs->statement);
    }

  free(rs);
  rs = NULL;
  return;
}

void freeBind(MYSQL_BIND* bind, int num)
{

  int i;

  if (bind != NULL) {
    for (i=0; i<num; i++) {
      if ( bind[i].buffer != NULL ) free(bind[i].buffer);
      if ( bind[i].is_null != NULL ) free(bind[i].is_null);
      if ( bind[i].length != NULL ) free(bind[i].length);
      if ( bind[i].error != NULL ) free(bind[i].error);
    }
    free(bind);
    bind = NULL;
  }
  return;

}
