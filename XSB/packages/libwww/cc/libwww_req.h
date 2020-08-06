/* File:      libwww_req.h
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2000
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
** $Id: libwww_req.h,v 1.3 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

/* included defs for libwww_request.c only */
/* this file is named libwww_req.h instead of libwww_request.h to avoid
   conflict with libwww_request.H on windows, which is case-blind */



//PRIVATE int total_number_of_requests;
//PRIVATE int event_loop_runnung;
PRIVATE int timeout_value;

#define SELECTED_TAGS_TBL_SIZE	    29
#define SUPPRESSED_TAGS_TBL_SIZE    41
#define STRIPPED_TAGS_TBL_SIZE	    37

#define DEFAULT_TIMEOUT	    	  5000  /* 5 sec */


PRIVATE REQUEST_CONTEXT *set_request_context(HTRequest *request,
					     prolog_term req_term,
					     int req_id);
PRIVATE void free_request_context (REQUEST_CONTEXT *context);

PRIVATE int printer (const char * fmt, va_list pArgs);
PRIVATE int tracer (const char * fmt, va_list pArgs);
PRIVATE BOOL libwww_send_credentials(HTRequest * request, HTAlertOpcode op,
				     int msgnum, const char * dfault,
				     void * input, HTAlertPar * reply);
PRIVATE AUTHENTICATION *find_credentials(AUTHENTICATION *auth_info,char *realm);
PRIVATE void release_libwww_request(HTRequest *request);
PRIVATE char *extract_uri(prolog_term req_term, HTRequest *req, int req_id);
PRIVATE void get_request_params(prolog_term req_term, HTRequest *req);
PRIVATE HTAssocList *get_form_params(prolog_term form_params, int request_id);
PRIVATE REQUEST_TYPE get_request_type(prolog_term req_term, int request_id);

PRIVATE void free_htable(HASH_TABLE *htable);
PRIVATE void init_htable(HASH_TABLE *htable, int size, REQUEST_TYPE type);
PRIVATE void init_tag_table(prolog_term tag_list, HASH_TABLE *tag_tbl);


PRIVATE void setup_request_structure (prolog_term req_term, int req_id);
PRIVATE int request_termination_handler(HTRequest    *request,
					HTResponse   *response,
					void 	   *param,
					int          status);

PRIVATE int handle_dependent_termination(HTRequest   *request,
					 HTResponse  *response,
					 void	      *param,
					 int 	      status);
PRIVATE void libwww_abort_all(char *msg, ...);
PRIVATE void setup_callbacks(REQUEST_TYPE type);
PRIVATE void extract_request_headers(HTRequest *request);
PRIVATE int timer_cbf(HTTimer *timer, void *param, HTEventType type);

typedef struct userdata USERDATA;
struct userdata {
  DELETE_USERDATA *    	  delete_method;
  int 	      	      	  status;    	   /* this is used to carry status into
					      delete_userData */
};
