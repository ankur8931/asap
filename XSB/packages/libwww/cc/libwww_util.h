/* File:      libwww_util.h
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
** $Id: libwww_util.h,v 1.15 2010-08-19 15:03:39 spyrosh Exp $
** 
*/



#include "WWWLib.h"
#include "WWWHTTP.h"
#include "WWWInit.h"
#include "HTAABrow.h"
#include "WWWApp.h"
#include "WWWXML.h"
#include "HTUtils.h"
#include "HTTPReq.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "basictypes.h"
#include "basicdefs.h"
#include "auxlry.h"
#include "xsb_config.h"
#include "wind2unix.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "cinterf.h"
#include "varstring_xsb.h"

/* XSB_LIBWWW_PACKAGE is used in http_errors.h */
#define XSB_LIBWWW_PACKAGE
#include "../prolog_includes/http_errors.h"


/* definitions and macros included in all files */

extern int total_number_of_requests;
extern int event_loop_runnung;
HTList *XML_converter=NULL, *RDF_converter=NULL, *HTML_converter=NULL;

/*
#define LIBWWW_DEBUG_VERBOSE
#define LIBWWW_DEBUG
#define LIBWWW_DEBUG_TERSE
#include "debug_xsb.h"
*/
#ifdef LIBWWW_DEBUG_VERBOSE
#define LIBWWW_DEBUG
#endif
#ifdef LIBWWW_DEBUG
#define LIBWWW_DEBUG_TERSE
#endif

/* special tag type that we use to wrap around text */
#define PCDATA_SPECIAL 	  -77


/* from HTTP.c */
#define FREE_TARGET(t)	(*(t->target->isa->_free))(t->target)

/* Must define this, since HTStream is just a name aliased to _HTStream */
struct _HTStream {
    const HTStreamClass *	isa;
};

enum request_type {FETCH, HTMLPARSE, XMLPARSE, RDFPARSE, HEADER};
typedef enum request_type REQUEST_TYPE;

union hkey {
  int intkey;
  char *strkey;
};
typedef union hkey HKEY;
struct hash_table {
  int 	       size;
  REQUEST_TYPE type;
  HKEY 	       *table;
};
typedef struct hash_table HASH_TABLE;

typedef struct auth AUTHENTICATION;
struct auth {
  char 	         *realm;
  char 	         *uid;   /* username */
  char 	         *pw;    /* password */
  AUTHENTICATION *next;  /* next authorization record (used for subrequests) */
};

/* used to pass the input info to request and get output info from request back
   to the Prolog side*/
struct request_context {
  int  request_id;
  int  subrequest_id;
  int  suppress_is_default;
  int  convert2list;    /* if convert pcdata to Prolog lists on exit */
  int  is_subrequest;  /* In XML parsing, we might need to go to a different
			  URI to fetch an external reference. This spawns a new
			  blocking subrequest with the same context. */
  int  statusOverride; /* If set, this status code should replace the one
			  returned by libwww */
  time_t last_modtime; /* page modtime */
  /* data structure where we build parsed terms, etc. */
  void *userdata;
  /* input */
  REQUEST_TYPE type;	    /* request type: html/xml parsing, fetching page */
  int  timeout;
  time_t user_modtime;      /* oldest modtime the user can tolerate */
  prolog_term formdata;
  AUTHENTICATION auth_info; /* list of name/pw pairs */
  int 	         retry;     /* whether to retry authentication */
  HTMethod   method;
  HASH_TABLE selected_tags_tbl;
  HASH_TABLE suppressed_tags_tbl;
  HASH_TABLE stripped_tags_tbl;
  /* output */
  prolog_term status_term;
  prolog_term result_params;  /* additional params returned in the result */
  prolog_term request_result; /* either the parse tree of a string containing
				 the HTML page */
  HTChunk     *result_chunk;  /* used only by the FETCH method. Here we get the
				 resulting page before converting it to
				 prolog_term */
};
typedef struct request_context REQUEST_CONTEXT;

typedef void DELETE_USERDATA(void *userdata);

/* like strcpy, but also converts to lowercase */
void strcpy_lower(char *to, const char *from);


int add_to_htable(HKEY item, HASH_TABLE *htable);
int is_in_htable(const HKEY item, HASH_TABLE *htable);


void print_prolog_term(prolog_term term, char *message);

void html_register_callbacks();
void HTXML_newInstance (HTStream *		me,
			HTRequest *		request,
			HTFormat 		target_format,
			HTStream *		target_stream,
			XML_Parser              xmlparser,
			void * 			context);
void libwww_newRDF_parserHandler (HTStream *		me,
				  HTRequest *		request,
				  HTFormat 		target_format,
				  HTStream *		target_stream,
				  HTRDF *    	        rdfparser,
				  void *         	context);
void add_result_param(prolog_term *result_param, 
		      char *functor, int cnt, ...);
void report_asynch_subrequest_status(HTRequest *request, int status);
void report_synch_subrequest_status(HTRequest *request, int status);
int verifyMIMEformat(HTRequest *request, REQUEST_TYPE type);
char *RequestID(HTRequest *request);

int xml_entity_termination_handler(HTRequest   *request,
				   HTResponse  *response,
				   void      *param,
				   int 	     status);
REQUEST_CONTEXT *set_subrequest_context(HTRequest *request,
					HTRequest *subrequest,
					prolog_term result_term);
void setup_termination_filter(HTRequest *request, HTNetAfter *filter);
void set_xml_conversions(void);
void set_rdf_conversions(void);
void set_html_conversions(void);

#define AUTH_OR_REDIRECTION(status) \
    ((status == HT_NO_ACCESS) || (status == HT_NO_PROXY_ACCESS) \
       || (status == HT_REAUTH) || (status == HT_PROXY_REAUTH) \
       || (status == HT_SEE_OTHER) || (status == HT_PERM_REDIRECT) \
       || (status == HT_FOUND) || (status == HT_TEMP_REDIRECT))
