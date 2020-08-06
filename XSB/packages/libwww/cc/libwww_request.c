/* File:      libwww_request.c
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
** $Id: libwww_request.c,v 1.20 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include "libwww_util.h"
#include "libwww_req.h"
#include "deref.h"
#include "cinterf.h"

int total_number_of_requests = 0;
int event_loop_runnung = FALSE;


/* Calling sequence:
       libwww_request([req1,req2,...])

   Each req: functor(URL, REQUEST_Params, PARSED-Result, ERROR-Code)
   functor: htmlparse, xmlparse, rdfparse, fetch, header.
       	    The first two are requests to parse HTML/XML. Fetch means retrieve
       	    a page without parsing; header means retrieve header only.
	    All except "header" could be form fillouts, which return a page or
	    a parsed page.
   REQUEST_Params: [param, param, ...]
           Param: timeout(secs), if_modified_since(date-in-GMT-format),
	       	  authentication(realm,username,passwd),
		  formdata('attr-val-pair-list'),
		  selection(chosen-taglist,suppressed-taglist,stripped-taglist)
	          means:
	     	    parse only inside the tags on the chosen tag list. Stop
	     	    parsing if a suppressed tag is found. Resume if a chosen
	     	    tag is found, etc. 
		    Stripped tags are those that just get discarded.
		    selection(_,suppressed-tag-list,...) means: parse all tags
		    except those in the suppressed tags list.
		    f(chosen-tag-list,_,...) means: parse only inside the
		    chosen tags. 
*/
DllExport int call_conv do_libwww_request___(void)
{
  prolog_term request_term_list = reg_term(1), request_list_tail;
  int request_id=0;

  /* Create a new premptive client */
  /* note that some sites block user agents that aren't Netscape or IE.
     So we fool them!!! */
  HTProfile_newHTMLNoCacheClient("Mozilla", "6.0");

  /* We must enable alerts in order for authentication modules to call our own
     callback defined by HTAlert_add below. However, we delete all alerts other
     than those needed for authentication */ 
  HTAlert_setInteractive(YES);
  /* Note: we just register a function to send the credentials.
     We don't need to register authentication filters, because they are already
     registered by profile initialization */
  HTAlert_deleteOpcode(HT_A_PROGRESS); /* harmless, but useless */
  HTAlert_deleteOpcode(HT_A_MESSAGE);
  HTAlert_deleteOpcode(HT_A_CONFIRM);  /* the next 3 coredump, if allowed */
  HTAlert_deleteOpcode(HT_A_PROMPT);
  HTAlert_deleteOpcode(HT_A_USER_PW);
  /* register alert callbacks that supplies credentials */
  HTAlert_add(libwww_send_credentials,HT_A_USER_PW); /* usrname and password */
  HTAlert_add(libwww_send_credentials,HT_A_SECRET);  /* just the password    */

  HTPrint_setCallback(printer);
  HTTrace_setCallback(tracer);
#if 0
  HTSetTraceMessageMask("sob");
#endif

  /* use abort here, because this is a programmatic mistake */
  if (!is_list(request_term_list))
    libwww_abort_all("[LIBWWW_REQUEST] Argument must be a list of requests");

  request_list_tail = request_term_list;
  total_number_of_requests=0;
  event_loop_runnung = FALSE;
  timeout_value = -1;
  while (is_list(request_list_tail) && !is_nil(request_list_tail)) {
    request_id++;
    total_number_of_requests++;
    setup_request_structure(extern_p2p_car(request_list_tail), request_id);
    request_list_tail = extern_p2p_cdr(request_list_tail);
  }

  if (timeout_value <= 0)
    timeout_value = DEFAULT_TIMEOUT;

  /* start the event loop and begin to parse all requests in parallel */
  if (total_number_of_requests > 0) {
    /* periodic timer that kills the event loop, if it stays on due to a bug */
    HTTimer* timer = HTTimer_new(NULL, timer_cbf, NULL, 2*timeout_value, 1, 1);

#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In libwww_request: Starting event loop. Total requests=%d, timeout=%d",
		total_number_of_requests, timeout_value));
#endif

    HTTimer_dispatch(timer);

    event_loop_runnung = TRUE;
    HTEventList_newLoop();
    /* it is important to set this to false, because otherwise,
       HTEventList_stopLoop might set HTEndLoop and then HTEventList_newLoop()
       will always exit immediately. This is a libwww bug, it seems. */
    event_loop_runnung = FALSE;

    /* expiring remaining timers is VERY important in order to avoid them
       kicking in at the wrong moment and killing subsequent requests */
#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***Expiring timers"));
#endif
    HTTimer_expireAll();
    HTTimer_delete(timer);

#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In libwww_request: event loop ended: total outstanding requests=%d", total_number_of_requests));
#endif
  }
  
  /* free all registered callbacks and global preferences, so that this won't
     interfere with other applications */
  HTProfile_delete();
  return TRUE;
}


/* Sets up the libwww request structure for the request specified in
   PROLOG_REQ, including the request context, which contains the info about the
   return parameters. */
PRIVATE void setup_request_structure(prolog_term req_term, int request_id)
{
  int	      status;
  HTAnchor    *anchor = NULL;
  HTRequest   *request=NULL;
  HTAssocList *formdata=NULL;
  char 	      *uri = NULL;
  char 	      *cwd = HTGetCurrentDirectoryURL();
  REQUEST_CONTEXT *context;

  /* Create a new request and attach the context structure to it */
  request=HTRequest_new();
  context=set_request_context(request,req_term,request_id);
  setup_termination_filter(request, request_termination_handler);
  setup_callbacks(context->type);
  /* get URL */
  uri = extract_uri(req_term,request, request_id);
  /* get other params */
  get_request_params(req_term, request);

  /* we set the timer only once (libwww trouble otherwise);
     timer must be set before achor is loaded into request */
  if (timeout_value <= 0 && context->timeout > 0) {
    timeout_value = context->timeout;
    HTHost_setEventTimeout(timeout_value);
  }

  formdata = (context->formdata ?
	      get_form_params(context->formdata,request_id) : NULL);

  uri = HTParse(uri, cwd, PARSE_ALL);
  /* Create a new Anchor */
  anchor = HTAnchor_findAddress(uri);
  /* make requests to local files preemptive (synchronous)---a workaround 
     for a bug in Libwww */
  if (strncmp(uri,"file:/",6) == 0)
    HTRequest_setPreemptive(request,YES);

  /* check if the page has expired by first bringing the header */
  if ((context->type != HEADER) && (context->user_modtime > 0)) {
    HTRequest *header_req = HTRequest_new();
    context->is_subrequest = TRUE; /* should change to something better */
    context->subrequest_id++;
    setup_termination_filter(header_req,handle_dependent_termination);
    HTRequest_setPreemptive(header_req, YES);
    /* closing connection hangs libwww on concurrent requests
       HTRequest_addConnection(header_req, "close", "");
    */
    /* attach parent's context to this request */
    HTRequest_setContext(header_req, (void *)context);
    HTHeadAnchor(anchor,header_req);
    context->last_modtime = HTAnchor_lastModified((HTParentAnchor *)anchor);

    if (context->user_modtime > context->last_modtime) {
      /* cleanup the request and don't start it */
#ifdef LIBWWW_DEBUG
      xsb_dbgmsg((LOG_DEBUG,"***Request %s: Page older(%d) than if-modified-since time(%d)",
		 RequestID(request),
		  context->last_modtime, context->user_modtime));
#endif

      total_number_of_requests--;
      /* set result status */
      if (is_var(context->status_term)) {
	if (context->last_modtime <= 0)
	  extern_c2p_int(HT_ERROR, context->status_term);
	else
	  extern_c2p_int(WWW_EXPIRED_DOC, context->status_term);
      } else
	libwww_abort_all("[LIBWWW_REQUEST] Request %s: Arg 5 (Status) must be unbound variable",
			 RequestID(request));
      /* set the result params (header info); */
      extract_request_headers(header_req);
      /* terminate the result parameters list */
      extern_c2p_nil(context->result_params);

      release_libwww_request(request);
      HT_FREE(uri);
      return;
    }
  }

  /* Hook up anchor to our request */
  switch (context->type) {
  case HEADER:
    /* header-only requests seem to require preemptive mode (otherwise timer
       interrupts and crashes them */
    HTRequest_setPreemptive(request, YES);
    status = (YES == HTHeadAnchor(anchor,request));
    break;
  case FETCH:
    {
      HTStream *target;
      HTRequest_setOutputFormat(request, WWW_SOURCE);
      /* redirect stream to chunk */
      target = HTStreamToChunk(request, &(context->result_chunk), 0);
      HTRequest_setOutputStream(request, target);
      /* then do the same as in the case of parsing */
      goto LBLREQUEST;
    }
  case XMLPARSE:
    /* This sets stream conversion for the XML request.
       Needed to make sure libwww doesn't treat it as an HTML stream.
       If libwww incorrectly recognizes an XML request, it will start calling
       HTML parser's callbacks instead of XML callbacks. */
    set_xml_conversions();
    HTRequest_setConversion(request, XML_converter, YES);
    /* www/xml forces libwww to recognize this as an XML request even if the
       document was received from a server that dosn't recognize XML.
       We can't use text/xml here, because then libwww gives error when a
       document with text/xml is received. Dunno why */
    HTRequest_setOutputFormat(request, HTAtom_for("www/xml"));
    goto LBLREQUEST;
  case RDFPARSE:
    /* The conversion type www/rdf is invented in order to force RDF parsing on
       streams that have other MIME types. */
    set_rdf_conversions();
    HTRequest_setConversion(request, RDF_converter, YES);
    HTRequest_setOutputFormat(request, HTAtom_for("www/rdf"));
    goto LBLREQUEST;
  case HTMLPARSE:
    /* We invent conversion type www/html, which forces HTML parsing even if
       MIME type is different, say, text/xml. */
    set_html_conversions();
    HTRequest_setConversion(request, HTML_converter, YES);
    HTRequest_setOutputFormat(request, HTAtom_for("www/html"));
    goto LBLREQUEST;
  LBLREQUEST:
    if (formdata) {
      if (context->method == METHOD_GET)
	status = (YES == HTGetFormAnchor(formdata,anchor,request));
      else if (context->method == METHOD_POST)
	status = (NULL != HTPostFormAnchor(formdata,anchor,request));
    } else {
      /* not a form request */
      status = (YES==HTLoadAnchor(anchor, request));
    }
    break;
  default:
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: Invalid request type",
		     request_id);
  }

#ifdef LIBWWW_DEBUG_TERSE
  switch (context->type) {
  case HTMLPARSE:
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: request type: htmlparse", request_id));
    break;
  case XMLPARSE:
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: request type: xmlparse", request_id));
    break;
  case RDFPARSE:
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: request type: rdfparse", request_id));
    break;
  case HEADER:
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: request type: header", request_id));
    break;
  case FETCH:
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: request type: fetch", request_id));
    break;
  default:
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: request type: invalid", request_id));
  }
  if (formdata)
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: HTTP Method: %s, preemptive: %d",
	       request_id,
	       (context->method==METHOD_GET ? "FORM,GET" : "FORM,POST"),
		HTRequest_preemptive(request)));
  else
    xsb_dbgmsg((LOG_DEBUG,"***Request %d: HTTP Method: NON-FORM REQ, preemptive: %d",
		request_id, HTRequest_preemptive(request)));
#endif

  if (formdata) HTAssocList_delete(formdata);

  /* bad uri syntax */
  if (!status) {
#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In setup_request_structure: Request %d failed: bad uri",
		request_id));
#endif
    total_number_of_requests--;
    if (is_var(context->status_term))
      extern_c2p_int(WWW_URI_SYNTAX, context->status_term);
    else
      libwww_abort_all("[LIBWWW_REQUEST] Request %s: Arg 5 (Status) must be unbound variable",
		       RequestID(request));

    extern_c2p_nil(context->result_params);
    release_libwww_request(request);
  }
  HT_FREE(uri);
}


/* In XML parsing, we sometimes have to issue additional requests to go and
   fetch external entities. Here we handle termination of such subrequests.
   A subrequest is independent of its parent request, except that it inherits
   the parent's id and context. When a subrequest returns, it should NOT
   release the context. */
PRIVATE int handle_dependent_termination(HTRequest   *request,
					  HTResponse  *response,
					  void	      *param,
					  int 	      status)
{
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);
#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In handle_dependent_termination(%s): user_modtime=%d status=%d",
	      RequestID(request), context->user_modtime, status));
#endif

  /* the following conditions are handled by standard libwww filters */
  if (context->retry && AUTH_OR_REDIRECTION(status))
    return HT_OK; /* this causes other filters to be used */

  if (status != HT_LOADED)
    report_synch_subrequest_status(request, status);

  /* Note: this still preserves the anchor and the context; we use them after
     this call to extract last modified time from the header */
  HTRequest_clear(request);
  /* restore parent process' context */
  context->is_subrequest = FALSE;
  return !HT_OK;
}



PRIVATE void libwww_abort_all(char *msg, ...)
{
  va_list args;
  char buf[MAXBUFSIZE];

  HTNet_killAll();
  va_start(args, msg);
  vsprintf(buf, msg, args);
  xsb_abort(buf);
  va_end(args);
}


void report_synch_subrequest_status(HTRequest *request, int status)
{
  prolog_term uri_term=extern_p2p_new(), error_term=extern_p2p_new();
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);
  char *uri = HTAnchor_physical(HTRequest_anchor(request));
  extern_c2p_string(uri,uri_term);
  extern_c2p_int(status, error_term);
  add_result_param(&(context->result_params),
		   "subrequest",2,uri_term,error_term);
}


void report_asynch_subrequest_status(HTRequest *request, int status)
{
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);
  char *uri = HTAnchor_physical(HTRequest_anchor(request));

  extern_c2p_functor("subrequest",2,context->result_params);
  extern_c2p_string(uri,extern_p2p_arg(context->result_params,1));
  extern_c2p_int(status, extern_p2p_arg(context->result_params,2));
}


PRIVATE REQUEST_CONTEXT *set_request_context(HTRequest *request,
					     prolog_term req_term,
					     int request_id)
{
  REQUEST_CONTEXT *context;

  if ((context=(REQUEST_CONTEXT *)calloc(1,sizeof(REQUEST_CONTEXT))) == NULL)
    libwww_abort_all("[LIBWWW_REQUEST] Not enough memory");

  context->request_id = request_id;
  context->subrequest_id = 0;
  context->suppress_is_default = FALSE;
  context->convert2list = FALSE;
  context->statusOverride = 0;
  context->is_subrequest = FALSE;
  context->userdata = NULL;
  context->last_modtime = 0;
  context->timeout = DEFAULT_TIMEOUT;
  context->user_modtime = 0;
  context->formdata=0;
  context->auth_info.realm = "";
  context->auth_info.uid = "foo";
  context->auth_info.pw = "foo";
  context->retry = TRUE;
  context->method = METHOD_GET;
  context->selected_tags_tbl.table = NULL;
  context->suppressed_tags_tbl.table = NULL;
  context->stripped_tags_tbl.table = NULL;

  context->type = get_request_type(req_term, request_id);
  context->result_chunk = NULL;

  init_htable(&(context->selected_tags_tbl),
	      SELECTED_TAGS_TBL_SIZE,context->type);
  init_htable(&(context->suppressed_tags_tbl),
	      SUPPRESSED_TAGS_TBL_SIZE,context->type);
  init_htable(&(context->stripped_tags_tbl),
	      STRIPPED_TAGS_TBL_SIZE,context->type);
  /* output */
  context->result_params = extern_p2p_arg(req_term,3);
  if(!is_var(context->result_params))
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: Arg 3 (Result parameters) must be unbound variable", request_id);
  extern_c2p_list(context->result_params);

  context->request_result = extern_p2p_arg(req_term,4);
  if(!is_var(context->request_result))
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: Arg 4 (Result) must be unbound variable", request_id);

  context->status_term = extern_p2p_arg(req_term,5);
  if(!is_var(context->status_term))
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: Arg 5 (Status) must be unbound variable", request_id);

  /* attach context to the request */
  HTRequest_setContext(request, (void *) context);

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Request %d: context set", request_id));
#endif

  return context;
}


PRIVATE void free_request_context (REQUEST_CONTEXT *context)
{
  AUTHENTICATION *next_auth, *curr_auth;
  if (!context) return;
  free_htable(&(context->selected_tags_tbl));
  free_htable(&(context->suppressed_tags_tbl));
  free_htable(&(context->stripped_tags_tbl));
  /* Note: we don't need to free context->result_chunk, since HTChunk_toCString
     deleted the chunk object, and we freed the chunk data earlier. */
  /* release authentication info, unless it is a subrequest (because request
     and subrequest share authinfo) */
  if (!(context->is_subrequest)) {
    next_auth = context->auth_info.next;
    while (next_auth) {
      curr_auth = next_auth;
      next_auth=next_auth->next;
      free(curr_auth);
    }
  }
  free(context);
}


/* Copy FROM to TO and lowercase on the way; assume TO is large enough */
void strcpy_lower(char *to, const char *from)
{
  int i=0;
  if (from)
    while (from[i]) {
      to[i] = tolower(from[i]);
      i++;
    }
  to[i] = '\0';
}


void print_prolog_term(prolog_term term, char *message)
{ 
  static XSB_StrDefine(StrArgBuf);
  prolog_term term2 = extern_p2p_deref(term);
  XSB_StrSet(&StrArgBuf,"");
  print_pterm(term2, 1, &StrArgBuf); 
#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***%s = %s", message, StrArgBuf.string));
#endif
} 


/* these are for tracing */
PRIVATE int printer (const char * fmt, va_list pArgs)
{
    return (vfprintf(stdout, fmt, pArgs));
}

PRIVATE int tracer (const char * fmt, va_list pArgs)
{
    return (vfprintf(stderr, fmt, pArgs));
}


/* This procedure gets the credentials from the input parameters
   and passes them back through the REPLY argument.
   For this to happen, alerts must be enabled. */
BOOL libwww_send_credentials(HTRequest * request, HTAlertOpcode op,
			     int msgnum, const char * dfault, void * realm,
			     HTAlertPar * reply)
{
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);
  AUTHENTICATION *authinfo = &(context->auth_info);
  AUTHENTICATION *credentials;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In libwww_send_credentials: Request=%s, realm: '%s' msgnum=%d",
	      RequestID(request), realm, msgnum));
#endif

  /* don't authenticate on retry */
  context->retry = FALSE;

  credentials = find_credentials(authinfo,realm);
  if (credentials) {
    /* have authentication info */
    HTAlert_setReplyMessage(reply, credentials->uid);
    HTAlert_setReplySecret(reply, credentials->pw);
    return TRUE;
  }
  /* if no credentials supplied, send some phony stuff */
  HTAlert_setReplyMessage(reply, "foo");
  HTAlert_setReplySecret(reply, "foo");
  return TRUE;
}


PRIVATE AUTHENTICATION *find_credentials(AUTHENTICATION *auth_info,char *realm)
{
  AUTHENTICATION *credentials = auth_info;

  while (credentials) {
    if ((credentials->realm == NULL)
	|| (strcmp(credentials->realm, realm) == 0))
      return credentials;
    credentials = credentials->next;
  }
  return NULL;
}


PRIVATE char *extract_uri(prolog_term req_term, HTRequest *request,
			  int request_id)
{
  /*
  static  XSB_StrDefine(uristr);
  */
  int 	  urilen;
  char    *uri;
  prolog_term uri_term;

  uri_term=extern_p2p_arg(req_term,1);
  if (is_charlist(uri_term, &urilen)) {
    ((REQUEST_CONTEXT *)HTRequest_context(request))->convert2list=TRUE;
    /*
    extern_p2c_chars(uri_term, &uristr);
    uri = uristr.string;
    */
    extern_p2c_chars(uri_term, uri, urilen);
  } else if (is_string(uri_term))
    uri=string_val(uri_term);
  else {
    release_libwww_request(request);
    /* use abort here, because it is a programmatic mistake */
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: Arg 1 (URI) must be an atom or a string", request_id);
  }
  return uri;
}


PRIVATE void release_libwww_request(HTRequest *request)
{
  free_request_context((REQUEST_CONTEXT *) HTRequest_context(request));
  HTRequest_kill(request);
}


/* Extraction of individual parameters from the request_params argument */
PRIVATE void get_request_params(prolog_term req_term, HTRequest *request)
{
  prolog_term param, req_params=extern_p2p_arg(req_term,2);
  char *paramfunctor;
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);

  if (!is_list(req_params) && !is_var(req_params) && !is_nil(req_params))
    libwww_abort_all("[LIBWWW_REQUEST] Request %s: Arg 2 (Request params) must be a list or a variable",
		     RequestID(request));
  while(is_list(req_params) && !is_nil(req_params)) {
    param = extern_p2p_car(req_params);
    paramfunctor = extern_p2c_functor(param);

    switch (paramfunctor[0]) {
    case 't': case 'T': /* user-specified timeout */ 
      if (!is_int(extern_p2p_arg(param, 1)))
	libwww_abort_all("[LIBWWW_REQUEST] Request %s: Timeout parameter must be an integer",
			 RequestID(request));
      context->timeout = extern_p2c_int(extern_p2p_arg(param, 1)) * 1000;
      if (context->timeout <= 0)
	context->timeout = DEFAULT_TIMEOUT;
      break;
    case 'i': case 'I': /* if-modified-since */
      if (!is_string(extern_p2p_arg(param, 1)))
	libwww_abort_all("[LIBWWW_REQUEST] Request %s: If_modified_since parameter must be a string",
		  RequestID(request));
      context->user_modtime =
	(long)HTParseTime(string_val(extern_p2p_arg(param,1)), NULL, YES);
      break;
    case 'a': case 'A': {  /* authorization */
      prolog_term auth_head, auth_tail=extern_p2p_arg(param,1);
      AUTHENTICATION *auth_info = &(context->auth_info);
      
      do {
	if (is_list(auth_tail)) {
	  auth_head=extern_p2p_car(auth_tail);
	  auth_tail=extern_p2p_cdr(auth_tail);
	} else auth_head = auth_tail;

	if (is_string(extern_p2p_arg(auth_head, 1)))
	  auth_info->realm = extern_p2c_string(extern_p2p_arg(auth_head, 1));
	else auth_info->realm = NULL;
	if (is_string(extern_p2p_arg(auth_head, 2)))
	  auth_info->uid = extern_p2c_string(extern_p2p_arg(auth_head, 2));
	else auth_info->uid = NULL;
	/* passwd is always required in auth info */
	if (is_string(extern_p2p_arg(auth_head, 3)))
	  auth_info->pw = extern_p2c_string(extern_p2p_arg(auth_head, 3));
	else auth_info->pw = NULL;

	if (is_list(auth_tail) && !is_nil(auth_tail)) {
	  auth_info->next = (AUTHENTICATION *)calloc(1,sizeof(AUTHENTICATION));
	  auth_info = auth_info->next;
	} else break;

      } while (TRUE);
      auth_info->next=NULL;
      break;
    }
    case 'f': case 'F':  /* formdata: name/value pair list to fill out forms */
      context->formdata = extern_p2p_arg(param, 1);
      break;
    case 'm': case 'M': {  /* HTTP method: GET/POST/PUT */
      char *method = extern_p2c_string(extern_p2p_arg(param, 1));
      switch (method[1]) {
      case 'O': case 'o':
	context->method = METHOD_POST;
	break;
      case 'E': case 'e':
	context->method = METHOD_GET;
	break;
      case 'P': case 'p':
	context->method = METHOD_PUT;
	break;
      }
      break;
    }
    case 's': case 'S':  /* selection of tags to parse */
      /* tag selection: selection(chosen-list,suppressed-list,strip-list) */
      if (extern_p2c_arity(param)==3) {
	prolog_term
	  select_term=extern_p2p_arg(param,1),
	  suppressed_term=extern_p2p_arg(param,2),
	  strip_term=extern_p2p_arg(param,3);
	
	if (is_var(select_term))
	  context->suppress_is_default=FALSE;
	else if (is_list(select_term)) {
	  context->suppress_is_default=TRUE;
	  init_tag_table(select_term, &(context->selected_tags_tbl));
	} else
	  libwww_abort_all("[LIBWWW_REQUEST] Request %d: In Arg 2, selection(CHOOSE,_,_): CHOOSE must be a var or a list");

	if (is_list(suppressed_term)) {
	  init_tag_table(suppressed_term, &(context->suppressed_tags_tbl));
	} else if (!is_var(suppressed_term))
	  libwww_abort_all("[LIBWWW_REQUEST] Request %s: In Arg 2, selection(_,SUPPRESS,_): SUPPRESS must be a var or a list",
			   RequestID(request));
	
	if (is_list(strip_term)) {
	  init_tag_table(strip_term, &(context->stripped_tags_tbl));
	} else if (!is_var(strip_term))
	  libwww_abort_all("[LIBWWW_REQUEST] Request %s: In Arg 2, selection(_,_,STRIP): STRIP must be a var or a list",
			   RequestID(request));
      } else {
	libwww_abort_all("[LIBWWW_REQUEST] Request %s: In Arg 2, wrong number of arguments in selection parameter",
			 RequestID(request));
      }
      break;
    default:  /* ignore unknown params */
      break;
    }
    req_params = extern_p2p_cdr(req_params);
  } 
  return;
}



PRIVATE HTAssocList *get_form_params(prolog_term form_params, int request_id)
{
  HTAssocList *formfields=NULL;

  if (!is_list(form_params))
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: List of form parameters must not be empty",
		     request_id);
  
  while (!is_nil(form_params)) {
    prolog_term head;
    char *string;

    head = extern_p2p_car(form_params);
    if (is_string(head))
      string = extern_p2c_string(head);
    else
      libwww_abort_all("[LIBWWW_REQUEST] Request %d: Non-string in form parameter list",
		       request_id);

    form_params = extern_p2p_cdr(form_params);
		
    /* create a list to hold the form arguments */
    if (!formfields) formfields = HTAssocList_new();

    /* parse the content and add it to the association list */
    HTParseFormInput(formfields, string);
  }
  return formfields;
}


PRIVATE REQUEST_TYPE get_request_type(prolog_term req_term, int request_id)
{
  char *functor;
  if (!is_functor(req_term)) {
    libwww_abort_all("[LIBWWW_REQUEST] Request %d: Bad request syntax",
		     request_id);
  }
  functor = extern_p2c_functor(req_term);

  if (strncmp("fetch",functor,3)==0) return FETCH;
  if (strncmp("xmlparse",functor,3)==0) return XMLPARSE;
  if (strncmp("rdfparse",functor,3)==0) return RDFPARSE;
  if (strncmp("htmlparse",functor,3)==0) return HTMLPARSE;
  if (strncmp("header",functor,3)==0) return HEADER;
  libwww_abort_all("[LIBWWW_REQUEST] Request %d: Invalid request type: %s",
		   request_id, functor);
  return TRUE; /* just to pacify the compiler */
}


PRIVATE void init_htable(HASH_TABLE *htable, int size, REQUEST_TYPE type)
{
  int i;
  /* RDFPARSE, FETCH, and HEADER requests don't use the hash table */
  if ((type != XMLPARSE) && (type != HTMLPARSE)) {
    htable->table = NULL;
    return;
  }

  htable->type = type;
  htable->size = size;
  if ((htable->table=(HKEY *)calloc(size, sizeof(HKEY))) == NULL )
    libwww_abort_all("[LIBWWW_REQUEST] Not enough memory");
  for (i=0; i<size; i++)
    if (type == HTMLPARSE)
      htable->table[i].intkey = -1;
    else /* XML */
      htable->table[i].strkey = NULL;
}


PRIVATE void free_htable(HASH_TABLE *htable)
{
  if (!htable || !htable->table) return;
  if (htable->type == HTMLPARSE) {
    free(htable->table);
  } else if (htable->type == XMLPARSE) {
    int i;
    if (!htable) return;
    for (i=0; i < htable->size; i++)
      if (htable->table[i].strkey != NULL)
	free(htable->table[i].strkey);
    free(htable->table);
  } else return;
}


PRIVATE unsigned long key2int(HKEY s, REQUEST_TYPE type)
{
  if (type == HTMLPARSE)
    return s.intkey;
  else if (type == XMLPARSE) {
    unsigned long h = 0;
    while (*(s.strkey))
      h = (h << 5) + h + (unsigned char)*(s.strkey)++;
    return h;
  }
  return 0; /* this is just to make the compiler happy */
}


#define FREE_CELL(hkey,type) \
     (type==HTMLPARSE ? (hkey.intkey==-1) : (hkey.strkey==NULL))

int add_to_htable(HKEY item, HASH_TABLE *htable)
{
  int idx, i;
  if (!htable || !htable->table) return FALSE;

  idx = (int) (key2int(item, htable->type) % htable->size);
  i = idx;
  while (!FREE_CELL(htable->table[i], htable->type)) {
    i++;
    i = i % htable->size;
    if (i == idx) /* reached full circle */
      return FALSE;
  }
  /* found spot */
  if (htable->type==HTMLPARSE)
    htable->table[i].intkey = item.intkey;
  else {
    htable->table[i].strkey = (char *)malloc(strlen(item.strkey)+1);
    strcpy_lower(htable->table[i].strkey, item.strkey);
  }
  return TRUE;
}


#define HASH_CELL_EQUAL(cell,item,type) \
    (type==HTMLPARSE ? (cell.intkey==item.intkey) \
		     : (cell.strkey && strcasecmp(cell.strkey,item.strkey)==0))

/* hash table stuff; deals with HKEY's stored in a table */
int is_in_htable(const HKEY item, HASH_TABLE *htable)
{
  int idx, i;
  if (!htable || !htable->table) return FALSE;

  idx = (int) (key2int(item,htable->type) % htable->size);
  i = idx;
  while (!FREE_CELL(htable->table[i], htable->type)) {
    if (HASH_CELL_EQUAL(htable->table[i], item, htable->type)) {
      return TRUE;
    }
    i++;
    i = i % htable->size;
    if (i == idx) /* reached full circle */
      return FALSE;
  }
  return FALSE;
}


/* This is a per-request termination handler */
PRIVATE int request_termination_handler (HTRequest   *request,
					 HTResponse  *response,
					 void 	     *param,
					 int 	     status)
{
  REQUEST_CONTEXT *context = ((REQUEST_CONTEXT *)HTRequest_context(request));
  USERDATA *userdata = (USERDATA *)(context->userdata);

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Request %s: In request_termination_handler, status %d",
	      RequestID(request), status));
#endif

  /* the following conditions are handled by standard libwww filters */
  if (context->retry && AUTH_OR_REDIRECTION(status))
    return HT_OK; /* this causes other filters to be used */

  /* Redirection code is commented out. It is better handled by the standard
     Libwww redirection/proxy handling filters */
#if 0
  if (status == HT_TEMP_REDIRECT || status == HT_PERM_REDIRECT ||
	status == HT_FOUND || status == HT_SEE_OTHER) {
    HTAnchor *redirection = HTResponse_redirection(response);
    /* if loaded redirection successfully, then return: the request will be
       processed by the existing event loop; otherwise, drop down and terminate
       the request */ 
    if (YES==HTLoadAnchor(redirection,request))
      return !HT_OK;
  }
#endif

  if (total_number_of_requests > 0)
    total_number_of_requests--;
  /* when the last request is done, stop the event loop */
  if ((total_number_of_requests == 0) && event_loop_runnung) {
    HTEventList_stopLoop();
    event_loop_runnung = FALSE;
#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In request_termination_handler: event loop halted, status=%d, HTNetCount=%d",
		status, HTNet_count()));
#endif
  }

  status = (context->statusOverride ? context->statusOverride : status);
  if (userdata)
    userdata->status = status;
  /* we should have checked already that status is a var */
  if (is_var(context->status_term))
    extern_c2p_int(status, context->status_term);

  extract_request_headers(request);
  /* terminate the result parameters list */
  extern_c2p_nil(context->result_params);

  /* Clean Up */
  if (userdata) {
    (userdata->delete_method)(userdata);
  } else if (context->type == FETCH) {
    char *result_as_string = HTChunk_toCString(context->result_chunk);

    if (!is_var(context->request_result))
      libwww_abort_all("[LIBWWW_REQUEST] Request %s: Arg 4 (Result) must be unbound variable",
		       RequestID(request));

    if (result_as_string) {
      if (context->convert2list)
	extern_c2p_chars(result_as_string, 5, context->request_result);
      else extern_c2p_string(result_as_string, context->request_result);
    }
    /* Note: HTChunk_toCString frees the chunk, and here we free the chank
       data. Thus, the chunk is completely cleared out. */
    HT_FREE(result_as_string);
  }

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In request_termination_handler: Cleanup: request %s, status=%d remaining requests: %d",
	      RequestID(request), status, total_number_of_requests));
#endif

  release_libwww_request(request);
  /* when a filter returns something other than HT_OK, no other after filters
     will be called */
  return !HT_OK;
}


PRIVATE void setup_callbacks(REQUEST_TYPE type)
{
  switch (type) {
  case HTMLPARSE:
    html_register_callbacks();
    break;
  case XMLPARSE:
    /* Register our new XML Instance handler */
    HTXMLCallback_registerNew(HTXML_newInstance, NULL);
    break;
  case RDFPARSE:
    HTRDF_registerNewParserCallback(libwww_newRDF_parserHandler, NULL);
    break;
  case FETCH:
    break;
  case HEADER:
    break;
  }
}


PRIVATE void init_tag_table(prolog_term tag_list, HASH_TABLE *tag_tbl)
{
  prolog_term tail, head;
  int i=0;
  char *tagname;
  HKEY taghandle;
  if ((tag_tbl->type != XMLPARSE) && (tag_tbl->type != HTMLPARSE))
    return;
  /* Save tag numbers in the table */
  tail=tag_list;
  while (is_list(tail) && !is_nil(tail) && i < tag_tbl->size) {
    head= extern_p2p_car(tail);
    tail=extern_p2p_cdr(tail);
    tagname = string_val(head);
    if (tag_tbl->type == XMLPARSE)
      taghandle = (HKEY)tagname;
    else 
      taghandle = (HKEY)(strcasecmp(tagname,"pcdata")==0?
			 PCDATA_SPECIAL
			 : SGML_findElementNumber(HTML_dtd(), tagname));
    add_to_htable(taghandle, tag_tbl);
    i++;
  }
}


/* Add term to the result parameter list. FUNCTOR is the name of the functor to
   use for this parameter; CNT is how many args to pass. The rest must be
   prolog terms that represent what is to appear inside */
void add_result_param(prolog_term *result_param, 
		      char *functor, int cnt, ...)
{
  prolog_term listHead;
  int i;
  va_list ap;

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***Starting add_result_param"));
#endif

  XSB_Deref(*result_param);
  if (is_list(*result_param))
    listHead = extern_p2p_car(*result_param);
  else {
    print_prolog_term(*result_param, "In add_result_param: result_param");
    libwww_abort_all("[LIBWWW_REQUEST] Bug: result_param is not a list");
  }
  extern_c2p_functor(functor, cnt, listHead);
  va_start(ap,cnt);
  for (i=0; i<cnt; i++)
    extern_p2p_unify(va_arg(ap, prolog_term), extern_p2p_arg(listHead, i+1));
  va_end(ap);

#ifdef LIBWWW_DEBUG_VERBOSE
  print_prolog_term(listHead, "In add_result_param: listHead");
#endif
  *result_param = extern_p2p_cdr(*result_param);
  extern_c2p_list(*result_param);
}


/* returns an uninstantiated var, which is a placeholder in the list of result
   parameters. The list of params is moved one elt down */
PRIVATE prolog_term get_result_param_stub(prolog_term *result_param)
{
  prolog_term listHead;

  XSB_Deref(*result_param);
  if (is_list(*result_param))
    listHead = extern_p2p_car(*result_param);
  else {
    print_prolog_term(*result_param, "In get_result_param_stub: result_param");
    libwww_abort_all("[LIBWWW_REQUEST] Bug: result_param is not a list");
  }
  *result_param = extern_p2p_cdr(*result_param);
  extern_c2p_list(*result_param);
  return listHead;
}


/* extract request headers and add then to the result parameters kept in the
   request context */
PRIVATE void extract_request_headers(HTRequest *request)
{
  HTParentAnchor * anchor;
  HTAssocList * headers;
  prolog_term paramvalue_term=extern_p2p_new(), paramname_term=extern_p2p_new();
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);

  anchor = HTRequest_anchor(request);
  headers = HTAnchor_header(anchor);
  if (headers) {
    HTAssocList *cur = headers;
    HTAssoc *pres;
    char *paramname, *paramvalue;

    while ((pres = (HTAssoc *) HTAssocList_nextObject(cur))) {
      paramname = HTAssoc_name(pres);
      paramvalue = HTAssoc_value(pres);
      extern_c2p_string(paramvalue, paramvalue_term);
      extern_c2p_string(paramname, paramname_term);

      add_result_param(&(context->result_params),
		       "header",2,paramname_term,paramvalue_term);

      /* save the page last_modified time */
      if (HTStrCaseMatch("Last-Modified", paramname))
	context->last_modtime = (long)HTParseTime(paramvalue,NULL,YES);
    }
  }
}


PRIVATE int timer_cbf(HTTimer *timer, void *param, HTEventType type)
{
  return !HT_OK;
}


int verifyMIMEformat(HTRequest *request, REQUEST_TYPE type)
{
  if (((REQUEST_CONTEXT *)HTRequest_context(request))->type == type)
    return TRUE;
  /*
    xsb_warn("LIBWWW_REQUEST Bug: Request %s Request type/MIME type mismatch",
	     RequestID(request));
  */
  return FALSE;
}


char *RequestID(HTRequest *request)
{
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);
  static char idstr[200];

  if (!context) return "null";

  if (context->is_subrequest)
    sprintf(idstr, "%d.%d", context->request_id, context->subrequest_id);
  else
    sprintf(idstr, "%d", context->request_id);

  return idstr;
}


REQUEST_CONTEXT *set_subrequest_context(HTRequest *request,
					HTRequest *subrequest,
					prolog_term result_term)
{
  REQUEST_CONTEXT *parent_context =
    (REQUEST_CONTEXT *)HTRequest_context(request);
  REQUEST_CONTEXT *context;
  HTStream *target;

  if ((context=(REQUEST_CONTEXT *)calloc(1,sizeof(REQUEST_CONTEXT))) == NULL)
    libwww_abort_all("[LIBWWW_REQUEST] Not enough memory");

  context->request_id = parent_context->request_id;
  context->subrequest_id = ++(parent_context->subrequest_id);
  context->suppress_is_default = FALSE;
  context->convert2list = parent_context->convert2list;
  context->statusOverride = 0;
  context->is_subrequest = TRUE;
  context->userdata = NULL;
  context->last_modtime = 0;
  context->timeout = DEFAULT_TIMEOUT;
  context->user_modtime = 0;
  context->formdata=0;
  context->auth_info = parent_context->auth_info;
  context->retry = TRUE;
  context->method = METHOD_GET;
  /* for subrequests, hash tables remain uninitialized: we don't do tag
     exclusion for external entities and such */
  context->selected_tags_tbl.table = NULL;
  context->suppressed_tags_tbl.table = NULL;
  context->stripped_tags_tbl.table = NULL;

  context->type = parent_context->type;
  context->result_chunk = NULL;

  /* redirect stream to chunk */
  target = HTStreamToChunk(subrequest, &(context->result_chunk), 0);
  HTRequest_setOutputStream(subrequest, target);

  /* output */
  context->result_params =
    get_result_param_stub(&parent_context->result_params);
  //extern_c2p_list(context->result_params);

  context->request_result = result_term;

  context->status_term = extern_p2p_new();

  /* attach context to the request */
  HTRequest_setContext(subrequest, (void *) context);
  /* we handle only HT_LOADED, HT_ERROR, and HT_NO_DATA conditions.
     We let the others (redirection, authentication, proxy) to be handled by
     the standard Libwww filters. */

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Subrequest %s: context set", RequestID(subrequest)));
#endif

  return context;

}


/* This sets termination filter for a request.
   The idea is that the filter catches all except some standard conditions,
   like authentication or redirection responses. */
void setup_termination_filter(HTRequest *request, HTNetAfter *filter)
{
  HTRequest_addAfter(request,
		     filter,
		     NULL,
		     NULL,
		     HT_ALL,
		     HT_FILTER_LAST,
		     NO); /* don't override global filters! */
}
