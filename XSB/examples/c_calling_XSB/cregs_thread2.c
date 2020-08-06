/* File:      cvarstring.c
** Author(s): Swift
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
** $Id: cregs_thread2.c,v 1.3 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

/*   Simple example file showing how to call XSB from C without varstrings  
 *   To make this file, see the instructions in ./README */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/* cinterf.h is necessary for the XSB API, as well as the path manipulation routines*/
#include "cinterf.h"
extern char *xsb_executable_full_path(char *);
extern char *strip_names_from_path(char*, int);

/* context.h is necessary for the type of a thread context. */
#include "context.h"
#include "thread_xsb.h"

pthread_mutex_t user_p_ready_mut;
pthread_mutex_t user_r_ready_mut;

/*******************************************************************
Succeeding commands
*********************************************************************/
void *command_ps(void * arg) {
  int rc;
  th_context *p_th;
  p_th = (th_context *)arg;
  pthread_mutex_lock(&user_p_ready_mut); 

  c2p_functor(p_th, "test_p",0,reg_term(p_th,1));
  if ((rc = xsb_command(p_th)) == XSB_ERROR)
    fprintf(stderr,"Command error p: %s/%s\n",xsb_get_error_type(p_th),xsb_get_error_message(p_th));

  pthread_mutex_unlock(&user_p_ready_mut); 
  return NULL;
}

void *command_rs(void * arg) {
  int rc;
  th_context *r_th;
  r_th = (th_context *)arg;
  pthread_mutex_lock(&user_r_ready_mut); 
  
  c2p_functor(r_th, "test_r",0,reg_term(r_th,1));
  if ((rc = xsb_command(r_th)) == XSB_ERROR)
    fprintf(stderr,"++Command Error r: %s/%s\n",
	    xsb_get_error_type(r_th),xsb_get_error_message(r_th));

  pthread_mutex_unlock(&user_r_ready_mut); 
  return NULL;
}

/*******************************************************************
Failing commands
*********************************************************************/
void *command_ps_fail(void * arg) {
  int rc;
  th_context *p_th;
  pthread_mutex_lock(&user_p_ready_mut); 
  p_th = (th_context *)arg;
  
  c2p_functor(p_th, "test_p_fail",0,reg_term(p_th,1));
  if ((rc = xsb_command(p_th)) == XSB_ERROR)
    fprintf(stderr,"Command error p: %s/%s\n",
	    xsb_get_error_type(p_th),xsb_get_error_message(p_th));
  else if (rc == XSB_FAILURE)
    fprintf(stderr,"p_fail failed properly\n");

  pthread_mutex_unlock(&user_p_ready_mut); 
  return NULL;
}

void *command_rs_fail(void * arg) {
  int rc;
  th_context *r_th;
  pthread_mutex_lock(&user_r_ready_mut); 
  r_th = (th_context *)arg;
  
  c2p_functor(r_th, "test_r_fail",0,reg_term(r_th,1));
  if ((rc = xsb_command(r_th)) == XSB_ERROR)
   fprintf(stderr,"++Command Error r: %s/%s\n",
	   xsb_get_error_type(r_th),xsb_get_error_message(r_th));
  else if (rc == XSB_FAILURE)
    fprintf(stderr,"r_fail failed properly\n");
  pthread_mutex_unlock(&user_r_ready_mut); 
 return NULL;
}

/*******************************************************************
Error-throwing commands
*********************************************************************/
void *command_ps_err(void * arg) {
  int rc;
  th_context *p_th;
  pthread_mutex_lock(&user_p_ready_mut); 
  p_th = (th_context *)arg;
  
  c2p_functor(p_th, "test_p_err",0,reg_term(p_th,1));
  if ((rc = xsb_command(p_th)) == XSB_ERROR)
    fprintf(stderr,"Planned command error p: %s/%s\n",
	    xsb_get_error_type(p_th),xsb_get_error_message(p_th));
  else if (rc  == XSB_FAILURE)
    fprintf(stderr,"++p_err failed improperly\n");
  pthread_mutex_unlock(&user_p_ready_mut); 
  return NULL;
}

void *command_rs_err(void * arg) {
  int rc;
  th_context *r_th;
  pthread_mutex_lock(&user_r_ready_mut); 
  r_th = (th_context *)arg;
  
  c2p_functor(r_th, "test_r_err",0,reg_term(r_th,1));
  if ((rc = xsb_command(r_th)) == XSB_ERROR)
   fprintf(stderr,"Planned command error r: %s/%s\n",
	   xsb_get_error_type(r_th),xsb_get_error_message(r_th));
  else if (rc == XSB_FAILURE)
    fprintf(stderr,"++r_err failed improperly\n");
  pthread_mutex_unlock(&user_r_ready_mut); 
 return NULL;
}

/*******************************************************************
Successful queries
*********************************************************************/
void *query_ps(void * arg) {
  int rc;
  th_context *p_th;
  pthread_mutex_lock(&user_p_ready_mut); 
  p_th = (th_context *)arg;
  
  c2p_functor(p_th, "pregs",3,reg_term(p_th, 1));
  rc = xsb_query(p_th);
  while (rc == XSB_SUCCESS) {
    printf("Answer: pregs(%s,%s,%s)\n",
	   (p2c_string(p2p_arg(reg_term(p_th, 2),1))),
	   (p2c_string(p2p_arg(reg_term(p_th, 2),2))),
	   (p2c_string(p2p_arg(reg_term(p_th, 2),3))));
    rc = xsb_next(p_th);
  }

 if (rc == XSB_ERROR) 
   fprintf(stderr,"++Query Error p: %s/%s\n",
	   xsb_get_error_type(p_th),xsb_get_error_message(p_th));

  pthread_mutex_unlock(&user_p_ready_mut); 
  
 return NULL;
}

void *query_rs(void * arg) {
  int rc;
  th_context *r_th;
  pthread_mutex_lock(&user_r_ready_mut); 
  r_th = (th_context *)arg;

  c2p_functor(r_th, "rregs",3,reg_term(r_th, 1));
  rc = xsb_query(r_th);
  while (rc == XSB_SUCCESS) {
    printf("Answer: rregs(%d,%d,%d)\n",
	   (p2c_int(p2p_arg(reg_term(r_th, 2),1))),
	   (p2c_int(p2p_arg(reg_term(r_th, 2),2))),
	   (p2c_int(p2p_arg(reg_term(r_th, 2),3))));
    rc = xsb_next(r_th);
  }

 if (rc == XSB_ERROR) 
   fprintf(stderr,"++Query Error r: %s/%s\n",xsb_get_error_type(r_th),xsb_get_error_message(r_th));

 pthread_mutex_unlock(&user_r_ready_mut); 
 return NULL;
}

/*******************************************************************
Closed queries
*********************************************************************/
void *close_query_ps(void * arg) {
  int rc;
  th_context *p_th;
  pthread_mutex_lock(&user_p_ready_mut); 
  p_th = (th_context *)arg;
  
  c2p_functor(p_th, "pregs",3,reg_term(p_th, 1));
  rc = xsb_query(p_th);
  if (rc == XSB_ERROR) 
    fprintf(stderr,"++Closed query Error p: %s/%s\n",
	    xsb_get_error_type(p_th),xsb_get_error_message(p_th));
  printf("Closed return p(%s,%s,%s)\n",
	   (p2c_string(p2p_arg(reg_term(p_th, 2),1))),
	   (p2c_string(p2p_arg(reg_term(p_th, 2),2))),
	   (p2c_string(p2p_arg(reg_term(p_th, 2),3))));
  rc = xsb_close_query(p_th);
  if (rc == XSB_FAILURE) 
    fprintf(stderr,"++Failure on closed query (p)!\n");

 pthread_mutex_unlock(&user_p_ready_mut); 
 return NULL;
}

void *close_query_rs(void * arg) {
  int rc;
  th_context *r_th;
  pthread_mutex_lock(&user_r_ready_mut); 
  r_th = (th_context *)arg;

  c2p_functor(r_th, "rregs",3,reg_term(r_th, 1));
  rc = xsb_query(r_th);
  if (rc == XSB_ERROR) 
   fprintf(stderr,"++Closed query Error r: %s/%s\n",
	   xsb_get_error_type(r_th),xsb_get_error_message(r_th));
  printf("Closed return r(%d,%d,%d)\n",
	 (p2c_int(p2p_arg(reg_term(r_th, 2),1))),
	 (p2c_int(p2p_arg(reg_term(r_th, 2),2))),
	 (p2c_int(p2p_arg(reg_term(r_th, 2),3))));
  rc = xsb_close_query(r_th);
  if (rc == XSB_FAILURE) 
    fprintf(stderr,"++Failure on closed query (r)!\n");

 pthread_mutex_unlock(&user_r_ready_mut); 
 return NULL;
}

/*******************************************************************
Error-throwing queries
*********************************************************************/
void *query_ps_err(void * arg) {
  int rc;
  th_context *p_th;
  pthread_mutex_lock(&user_p_ready_mut); 
  p_th = (th_context *)arg;
  
  c2p_functor(p_th, "pregs_err",3,reg_term(p_th, 1));
  rc = xsb_query(p_th);
  while (rc == XSB_SUCCESS) {
    printf("Pre-err return p(%s,%s,%s)\n",
	   (p2c_string(p2p_arg(reg_term(p_th, 2),1))),
	   (p2c_string(p2p_arg(reg_term(p_th, 2),2))),
	   (p2c_string(p2p_arg(reg_term(p_th, 2),3))));
    rc = xsb_next(p_th);
  }

 if (rc == XSB_ERROR) 
   fprintf(stderr,"Planned query Error p: %s/%s\n",
	   xsb_get_error_type(p_th),xsb_get_error_message(p_th));
  pthread_mutex_unlock(&user_p_ready_mut); 
 return NULL;
}

void *query_rs_err(void * arg) {
  int rc;
  th_context *r_th;
  pthread_mutex_lock(&user_r_ready_mut); 
  r_th = (th_context *)arg;

  c2p_functor(r_th, "rregs_err",3,reg_term(r_th, 1));
  rc = xsb_query(r_th);
  while (rc == XSB_SUCCESS) {
    printf("Pre-err return r(%d,%d,%d)\n",
	 (p2c_int(p2p_arg(reg_term(r_th, 2),1))),
	 (p2c_int(p2p_arg(reg_term(r_th, 2),2))),
	 (p2c_int(p2p_arg(reg_term(r_th, 2),3))));
    rc = xsb_next(r_th);
  }

 if (rc == XSB_ERROR) 
   fprintf(stderr,"Planned query Error r: %s/%s\n",
	   xsb_get_error_type(r_th),xsb_get_error_message(r_th));
  pthread_mutex_unlock(&user_r_ready_mut); 
 return NULL;
}

int main(int argc, char *argv[])
{ 

  static th_context *main_th;
  static th_context *r_th;
  static th_context *q_th;
  pthread_t qthread_id,rthread_id;
  int qstatus, rstatus, rc;
  void *qreturn; void *rreturn;

  char init_string[MAXPATHLEN];

  pthread_mutex_init( &user_p_ready_mut, NULL ) ;
  pthread_mutex_init( &user_r_ready_mut, NULL ) ;

  /* xsb_init_string relies on the calling program to pass the absolute or relative
     path name of the XSB installation directory. We assume that the current
     program is sitting in the directory ../examples/c_calling_xsb/
     To get installation directory, we strip 3 file names from the path. */

  strcpy(init_string,strip_names_from_path(xsb_executable_full_path(argv[0]),3));

  if (xsb_init_string(init_string)) {
    fprintf(stderr,"%s initializing XSB: %s\n",xsb_get_error_type(xsb_get_main_thread()),
	    xsb_get_error_message(xsb_get_main_thread()));
    exit(XSB_ERROR);
  }

  main_th = xsb_get_main_thread();

  /* Create command to consult a file: edb.P, and send it. */
  if (xsb_command_string(main_th, "consult('edb.P').") == XSB_ERROR)
    fprintf(stderr,"++Error consulting edb.P: %s/%s\n",xsb_get_error_type(main_th),
	    xsb_get_error_message(main_th));

  xsb_ccall_thread_create(main_th,&r_th);
  xsb_ccall_thread_create(main_th,&q_th);

  /*
  while (!(r_th->_xsb_ready))
    usleep(1000);

  while (!(q_th->_xsb_ready))
    usleep(1000);
  */

  pthread_create(&rthread_id,NULL,command_rs,r_th);
  pthread_create(&qthread_id,NULL,command_ps,q_th);
  pthread_create(&rthread_id,NULL,command_rs,r_th);
  pthread_create(&qthread_id,NULL,command_ps,q_th);

  rstatus = pthread_join(rthread_id,&rreturn);
  if (rstatus != 0) fprintf(stderr,"R join returns status %d\n",rstatus);
  qstatus = pthread_join(qthread_id,&qreturn);
  if (qstatus != 0) fprintf(stderr,"Q join returns status %d\n",qstatus);

  pthread_create(&rthread_id,NULL,query_rs,r_th);
  pthread_create(&qthread_id,NULL,query_ps,q_th);
  pthread_create(&rthread_id,NULL,query_rs,r_th);
  pthread_create(&qthread_id,NULL,query_ps,q_th);

  rstatus = pthread_join(rthread_id,&rreturn);
  if (rstatus != 0) fprintf(stderr,"R join returns status %d\n",rstatus);
  qstatus = pthread_join(qthread_id,&qreturn);
  if (qstatus != 0) fprintf(stderr,"Q join returns status %d\n",qstatus);

  printf("/******************************************/\n");

  pthread_create(&rthread_id,NULL,command_rs_fail,r_th);
  pthread_create(&qthread_id,NULL,command_ps_fail,q_th);
  pthread_create(&rthread_id,NULL,command_rs_fail,r_th);
  pthread_create(&qthread_id,NULL,command_ps_fail,q_th);

  rstatus = pthread_join(rthread_id,&rreturn);
  if (rstatus != 0) fprintf(stderr,"R join returns status %d\n",rstatus);
  qstatus = pthread_join(qthread_id,&qreturn);
  if (qstatus != 0) fprintf(stderr,"Q join returns status %d\n",qstatus);

  pthread_create(&rthread_id,NULL,close_query_rs,r_th);
  pthread_create(&qthread_id,NULL,close_query_ps,q_th);
  pthread_create(&rthread_id,NULL,close_query_rs,r_th);
  pthread_create(&qthread_id,NULL,close_query_ps,q_th);

  rstatus = pthread_join(rthread_id,&rreturn);
  if (rstatus != 0) fprintf(stderr,"R join returns status %d\n",rstatus);
  qstatus = pthread_join(qthread_id,&qreturn);
  if (qstatus != 0) fprintf(stderr,"Q join returns status %d\n",qstatus);

  printf("/******************************************/\n");

  pthread_create(&rthread_id,NULL,command_rs_err,r_th);
  pthread_create(&qthread_id,NULL,command_ps_err,q_th);
  pthread_create(&rthread_id,NULL,command_rs_err,r_th);
  pthread_create(&qthread_id,NULL,command_ps_err,q_th);

  rstatus = pthread_join(rthread_id,&rreturn);
  if (rstatus != 0) fprintf(stderr,"R join returns status %d\n",rstatus);
  qstatus = pthread_join(qthread_id,&qreturn);
  if (qstatus != 0) fprintf(stderr,"Q join returns status %d\n",qstatus);

  pthread_create(&rthread_id,NULL,query_rs_err,r_th);
  pthread_create(&qthread_id,NULL,query_ps_err,q_th);
  pthread_create(&rthread_id,NULL,query_rs_err,r_th);
  pthread_create(&qthread_id,NULL,query_ps_err,q_th);

  rstatus = pthread_join(rthread_id,&rreturn);
  if (rstatus != 0) fprintf(stderr,"R join returns status %d\n",rstatus);
  qstatus = pthread_join(qthread_id,&qreturn);
  if (qstatus != 0) fprintf(stderr,"Q join returns status %d\n",qstatus);

  if ((rc = xsb_command_string(r_th,"test_r.")) != XSB_SUCCESS)
    fprintf(stderr,"error on test_r: rc: %d\n",rc);

  if (xsb_command_string(r_th,"import thread_exit/0 from thread.") == XSB_ERROR)
    fprintf(stderr,"++Error exiting r: %s/%s\n",xsb_get_error_type(r_th),
	    xsb_get_error_message(r_th));

  xsb_kill_thread(r_th);
  //xsb_command_string(r_th,"thread_exit.");
 /*
 if (xsb_command_string(r_th,"thread_exit.") == XSB_ERROR)
    fprintf(stderr,"++Error exiting: %s/%s\n",xsb_get_error_type(r_th),
	    xsb_get_error_message(r_th));
 */
 xsb_close(xsb_get_main_thread());      /* Close connection */

 return(0);
}


/*******************************************************************
Low-level commands
*********************************************************************/

void *ll_command_ps(void * arg) {
  th_context *p_th;
  int rc = 0;
  p_th = (th_context *)arg;
  
  c2p_functor(p_th,"ll_test_p",0,reg_term(p_th,1));

  if ((rc = xsb_command(p_th)) == XSB_ERROR) 
   fprintf(stderr,"LowLevel command error p: %s/%s\n",xsb_get_error_type(p_th),xsb_get_error_message(p_th));
  else if (rc == XSB_FAILURE) printf("llp failed\n");
  else if (rc == XSB_SUCCESS) printf("llp succeeded\n");
  return NULL;
}

void *ll_command_rs(void * arg) {
  th_context *r_th;
  r_th = (th_context *)arg;
  
  c2p_functor(r_th,"ll_test_r",0,reg_term(r_th,1));

  if (xsb_command(r_th) == XSB_ERROR) 
   fprintf(stderr,"LowLevel command error r: %s/%s\n",xsb_get_error_type(r_th),xsb_get_error_message(r_th));

  return NULL;
}

