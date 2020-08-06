/* File:      libwww_parse_xml.c
** Author(s): kifer, Yang Yang
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
** $Id: libwww_parse_xml.c,v 1.18 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include "libwww_util.h"
#include "libwww_parse.h"
#include "libwww_parse_xml.h"


/* BOOL, PRIVATE, PUBLIC, etc., are defined in a Libwww header */

/* ------------------------------------------------------------------------- */
/*			     HTXML STREAM HANDLERS			     */
/* ------------------------------------------------------------------------- */

PRIVATE void HTXML_setHandlers (XML_Parser me)
{
  XML_SetElementHandler(me, xml_beginElement, xml_endElement);
  XML_SetCharacterDataHandler(me, xml_addText);
  XML_SetProcessingInstructionHandler(me, xml_processingInstruction);
  XML_SetUnparsedEntityDeclHandler(me, xml_unparsedEntityDecl);
  XML_SetNotationDeclHandler(me, xml_notationDecl);
  XML_SetExternalEntityRefHandler(me, xml_externalEntityRef);
  XML_SetUnknownEncodingHandler(me, xml_unknownEncoding, NULL);

  /* This exists only in expat 1.1. This version doesn't prohibit expansion of
     internal entities. Commented until expat 1.1 is included in libwww

     XML_SetDefaultHandlerExpand(me, xml_default);
  */
}

void HTXML_newInstance (HTStream *		me,
			HTRequest *		request,
			HTFormat 		target_format,
			HTStream *		target_stream,
			XML_Parser              xmlparser,
			void * 			context)
{
  USERDATA *userdata = xml_create_userData(xmlparser, request, target_stream);
  XML_SetUserData(xmlparser, (void *) userdata);
  if (me && xmlparser) HTXML_setHandlers(xmlparser);
}



/* This is the callback that captures start tag events */
PRIVATE void xml_beginElement(void  *userdata, /* where we build everything */
			      const XML_Char *tag, /* tag */
			      const XML_Char **attributes)
{
  USERDATA *userdata_obj = (USERDATA *) userdata;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_beginElement(%s): stackptr=%d tag=%s suppress=%d choose=%d",
	     RequestID(userdata_obj->request),
	     userdata_obj->stackptr, tag,
	     IS_SUPPRESSED_TAG((HKEY)(char *)tag, userdata_obj->request),
	     IS_SELECTED_TAG((HKEY)(char *)tag, userdata_obj->request)
	      ));
#endif

  if (IS_STRIPPED_TAG((HKEY)(char *)tag, userdata_obj->request)) return;

  if ((suppressing(userdata_obj)
       && !IS_SELECTED_TAG((HKEY)(char *)tag, userdata_obj->request))
      || (parsing(userdata_obj)
	  && IS_SUPPRESSED_TAG((HKEY)(char *)tag, userdata_obj->request))) {
    xml_push_suppressed_element(userdata_obj, tag);
    return;
  }

  /* parsing or suppressing & found a selected tag */
  if ((parsing(userdata_obj)
       && !IS_SUPPRESSED_TAG((HKEY)(char *)tag, userdata_obj->request))
      || (suppressing(userdata_obj) 
	  && IS_SELECTED_TAG((HKEY)(char *)tag, userdata_obj->request))) {
    xml_push_element(userdata_obj,tag,attributes);
    return;
  }
}


/* The callback for the end-tag event */
PRIVATE void xml_endElement (void *userdata, const XML_Char *tag)
{
  USERDATA *userdata_obj = (USERDATA *) userdata;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_endElement(%s): stackptr=%d, tag=%s",
	     RequestID(userdata_obj->request),
	     userdata_obj->stackptr, tag
	      ));
#endif

  if (IS_STRIPPED_TAG((HKEY)(char *)tag, userdata_obj->request)) return;

  /* Expat does checking for tag mismatches, so we don't have to */
  if (parsing(userdata_obj))
    xml_pop_element(userdata_obj);
  else
    xml_pop_suppressed_element(userdata_obj);

#ifdef LIBWWW_DEBUG_VERBOSE
  if (userdata_obj->stackptr >= 0) {
    if (!STACK_TOP(userdata_obj).suppress)
      print_prolog_term(STACK_TOP(userdata_obj).elt_term, "elt_term");
  }
#endif

  return;
}



/* The callback to capture text events */
PRIVATE void xml_addText (void	         *userdata,
			  const XML_Char *textbuf,
			  int	     	 len)
{
  USERDATA *userdata_obj = (USERDATA *) userdata;
  static XSB_StrDefine(pcdata_buf);
  int shift = 0;
  REQUEST_CONTEXT *context =
    (REQUEST_CONTEXT *)HTRequest_context(userdata_obj->request);

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_addText (%s)", RequestID(userdata_obj->request)));
#endif

  if (IS_STRIPPED_TAG((HKEY)"pcdata", userdata_obj->request)) return;
  if (suppressing(userdata_obj)) return;

  /* strip useless newlines */
  if (strncmp(textbuf,"\n", len) == 0) return;

  if (!xml_push_element(userdata_obj, "pcdata", NULL))
    return;

  /* copy textbuf (which isn't null-terminated) into a variable length str */
  XSB_StrEnsureSize(&pcdata_buf, len+1);
  strncpy(pcdata_buf.string, textbuf, len);
  pcdata_buf.length = len;
  XSB_StrNullTerminate(&pcdata_buf);

  /* if string starts with a newline, skip the newline */
  if (strncmp(textbuf,"\n", strlen("\n")) == 0)
    shift = strlen("\n");

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In addText: pcdata=%s", pcdata_buf.string+shift));
#endif

  /* put the text string into the elt term and then pop it */
  if (context->convert2list)
    extern_c2p_chars(pcdata_buf.string+shift,
		     4,
		     extern_p2p_arg(STACK_TOP(userdata_obj).elt_term,3));
  else
    extern_c2p_string(pcdata_buf.string+shift,
	       extern_p2p_arg(STACK_TOP(userdata_obj).elt_term,3));

  xml_pop_element(userdata_obj);
  return;
}


/* Collect tag's attributes and make them into a list of the form
   [attval(attr,val), ...]; bind it to Arg 2 of ELT_TERM */
PRIVATE void collect_xml_attributes (prolog_term     elt_term,
				     const XML_Char  **attrs)
{
  static XSB_StrDefine(attrname);
  prolog_term
    prop_list = extern_p2p_arg(elt_term,2),
    prop_list_tail = prop_list,
    prop_list_head;

  extern_c2p_list(prop_list_tail);

  while (attrs && *attrs) {
    XSB_StrEnsureSize(&attrname, strlen((char *)*attrs));
    strcpy_lower(attrname.string, (char *)*attrs);
    
#ifdef LIBWWW_DEBUG_VERBOSE
    xsb_dbgmsg((LOG_DEBUG,"***attr=%s", attrname.string));
#endif
    prop_list_head = extern_p2p_car(prop_list_tail);
    extern_c2p_functor("attval",2,prop_list_head);
    extern_c2p_string(attrname.string, extern_p2p_arg(prop_list_head,1));
    /* get value */
    attrs++;
    /* if *attrs=NULL, then it is an error: expat will stop */
    if (*attrs)
      extern_c2p_string((char *)*attrs, extern_p2p_arg(prop_list_head, 2));
    
    prop_list_tail = extern_p2p_cdr(prop_list_tail);
    extern_c2p_list(prop_list_tail);
    attrs++;
  }
  
  /* Terminate the property list */
  extern_c2p_nil(prop_list_tail);
  return;
}


/* push element onto USERDATA->stack */
PRIVATE int xml_push_element (USERDATA    *userdata,
			       const XML_Char  *tag,
			       const XML_Char  **attrs)
{
  static XSB_StrDefine(lower_tagname);
  prolog_term location;

  /*   If tag is not valid */
  if (tag == NULL) return TRUE;

  if (userdata->stackptr < 0)
    location = userdata->parsed_term_tail;
  else 
    location = STACK_TOP(userdata).content_list_tail;

  userdata->stackptr++;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_push_element(%s): stackptr=%d tag=%s",
	      RequestID(userdata->request), userdata->stackptr, tag));
#endif

  CHECK_STACK_OVERFLOW(userdata);

  /* wire the new elt into where it should be in the content list */
  STACK_TOP(userdata).elt_term = extern_p2p_car(location);

  STACK_TOP(userdata).tag = (XML_Char *)tag; /* cast to discard const
						declaration */
  STACK_TOP(userdata).suppress = FALSE;

  /* lowercase the tag */
  XSB_StrEnsureSize(&lower_tagname, strlen(tag)+1);
  strcpy_lower(lower_tagname.string, tag);

  /* normal tags look like elt(tagname, attrlist, contentlist);
     pcdata tags are: elt(pcdata,[],text); */
  if (XSB_StrCmp(&lower_tagname, "pcdata")==0)
    extern_c2p_functor("elt",3,STACK_TOP(userdata).elt_term);
  else /* normal elt */
    extern_c2p_functor("elt",3,STACK_TOP(userdata).elt_term);

  extern_c2p_string(lower_tagname.string, extern_p2p_arg(STACK_TOP(userdata).elt_term, 1));
  collect_xml_attributes(STACK_TOP(userdata).elt_term, attrs);
  
#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***elt_name=%s", lower_tagname.string));
  print_prolog_term(STACK_TOP(userdata).elt_term, "elt_term");
#endif

  /* normal element */
  if (XSB_StrCmp(&lower_tagname, "pcdata")!=0) {
    STACK_TOP(userdata).content_list_tail =
      extern_p2p_arg(STACK_TOP(userdata).elt_term,3);
    extern_c2p_list(STACK_TOP(userdata).content_list_tail);
  }
  return TRUE;
}


/* When done with an elt, close its contents list and pop the stack */
PRIVATE void xml_pop_element(USERDATA *userdata)
{
#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In xml_pop_element(%s): stackptr=%d, elt_name=%s",
	     RequestID(userdata->request),
	     userdata->stackptr,
	      STACK_TOP(userdata).tag));
#endif
  /* close the property list, for normal elements */
  if (strcasecmp(STACK_TOP(userdata).tag, "pcdata")!=0) {
    extern_c2p_nil(STACK_TOP(userdata).content_list_tail);
  }

  /* insert new list cell into the tail and change content_list_tail to point
     to the new tail */
  if (userdata->stackptr > 0) {
    STACK_PREV(userdata).content_list_tail =
      extern_p2p_cdr(STACK_PREV(userdata).content_list_tail);
    extern_c2p_list(STACK_PREV(userdata).content_list_tail);
  } else {
    userdata->parsed_term_tail = extern_p2p_cdr(userdata->parsed_term_tail);
    extern_c2p_list(userdata->parsed_term_tail);
  }

  userdata->stackptr--;

#ifdef LIBWWW_DEBUG_VERBOSE
  if (userdata->stackptr >= 0)
    print_prolog_term(STACK_TOP(userdata).content_list_tail,
		      "content_list_tail");
  else
    print_prolog_term(userdata->parsed_term_tail, "parsed_term_tail");
#endif

  return;
}


/* Push tag, but keep only the tag info; don't convert to prolog term */
PRIVATE void xml_push_suppressed_element(USERDATA   *userdata,
					 const XML_Char *tag)
{
  /* non-empty tag */
  userdata->stackptr++; /* advance ptr, but don't push tag */

  STACK_TOP(userdata).tag = (XML_Char *)tag; /* cast to discard const
						declaration */
  STACK_TOP(userdata).suppress = TRUE;

  /* passing content list tail through suppressed elements */
  if (userdata->stackptr == 0)
    STACK_TOP(userdata).content_list_tail = userdata->parsed_term_tail;
  else 
    STACK_TOP(userdata).content_list_tail =
      STACK_PREV(userdata).content_list_tail;

  return;
}


PRIVATE void xml_pop_suppressed_element(USERDATA *userdata)
{
  /* chain the list tails back through the sequence of suppressed tags */
  if (userdata->stackptr > 0) {
    STACK_PREV(userdata).content_list_tail = STACK_TOP(userdata).content_list_tail;
  } else {
    userdata->parsed_term_tail = STACK_TOP(userdata).content_list_tail;
  }

  userdata->stackptr--;

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In xml_pop_suppressed_element(%s): stackptr=%d",
	      RequestID(userdata->request), userdata->stackptr));
  if (userdata->stackptr >= 0)
    print_prolog_term(STACK_TOP(userdata).content_list_tail, "content_list_tail");
  else
    print_prolog_term(userdata->parsed_term_tail, "parsed_term_tail");
#endif

  return;
}


PRIVATE USERDATA *xml_create_userData(XML_Parser parser,
				      HTRequest *request,
				      HTStream  *target_stream)
{
  USERDATA *me = NULL;
#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Start xml_create_userData: Request %s", RequestID(request)));
#endif
  if (parser) {
    /* make sure that MIME type is appropriate for XML */
    if (!verifyMIMEformat(request, XMLPARSE)) {
      /*
	HTStream * input = HTRequest_inputStream(request);
	(*input->isa->abort)(input, NULL);
	HTRequest_setInputStream(request,NULL);
	HTRequest_kill(request);
	return NULL;
      */
      xsb_abort("[LIBWWW_REQUEST] Bug: Request type/MIME type mismatch");
    }
    if ((me = (USERDATA *) HT_CALLOC(1, sizeof(USERDATA))) == NULL)
      HT_OUTOFMEM("libwww_parse_xml");
    me->delete_method = xml_delete_userData;
    me->parser = parser;
    me->request = request;
    me->target = target_stream;
    me->suppress_is_default = 
      ((REQUEST_CONTEXT *)HTRequest_context(request))->suppress_is_default;
    me->parsed_term = extern_p2p_new();
    extern_c2p_list(me->parsed_term);
    me->parsed_term_tail = me->parsed_term;
    SETUP_STACK(me);
  }

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***End xml_create_userData: Request %s", RequestID(request)));
#endif

  /* Hook up userdata to the request context */
  ((REQUEST_CONTEXT *)HTRequest_context(request))->userdata = (void *)me;

  return me;
}


PRIVATE void xml_delete_userData(void *userdata)
{
  prolog_term parsed_result, status_term;
  USERDATA *me = (USERDATA *)userdata;
  HTRequest *request = me->request;

  if (me->request) {
    parsed_result =
      ((REQUEST_CONTEXT *)HTRequest_context(request))->request_result;
    status_term =
      ((REQUEST_CONTEXT *)HTRequest_context(request))->status_term;
  } else return;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_delete_userData(%s): stackptr=%d",
	      RequestID(request), me->stackptr));
#endif

  /* if the status code says the doc was loaded fine, but stackptr is != -1,
     it means the doc is ill-formed */
  if (me->stackptr >= 0 && (me->status == HT_LOADED)) {
    extern_c2p_int(WWW_DOC_SYNTAX,status_term);
  }

  /* terminate the parsed prolog terms list */
  extern_c2p_nil(me->parsed_term_tail);

  /* pass the result to the outside world */
  if (is_var(me->parsed_term))
    extern_p2p_unify(parsed_result, me->parsed_term);
  else
    xsb_abort("[LIBWWW_REQUEST] Request %s: Arg 4 (Result) must be unbound variable",
	      RequestID(request));

  if (me->target) FREE_TARGET(me);
  if (me->stack) HT_FREE(me->stack);
  HT_FREE(me);

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Request %s: freed the USERDATA object", RequestID(request)));
#endif

  return;
}


/* Unused handlers, which might get used later in the development */

PRIVATE void xml_processingInstruction (void * userData,
					const XML_Char * target,
					const XML_Char * data)
{
  return;
}

/* 
** This is called for a declaration of an unparsed (NDATA)
** entity.  The base argument is whatever was set by XML_SetBase.
** The entityName, systemId and notationName arguments will never be null.
** The other arguments may be.
*/
PRIVATE void xml_unparsedEntityDecl (void * userData,
				     const XML_Char * entityName,
				     const XML_Char * base,
				     const XML_Char * systemId,
				     const XML_Char * publicId,
				     const XML_Char * notationName)
{
  return;
}

/* 
** This is called for a declaration of notation.
** The base argument is whatever was set by XML_SetBase.
** The notationName will never be null.  The other arguments can be.
*/
PRIVATE void xml_notationDecl (void * userData,
			       const XML_Char * notationName,
			       const XML_Char * base,
			       const XML_Char * systemId,
			       const XML_Char * publicId)
{
  return;
}

/* 
** This is called for a reference to an external parsed general entity.  The
** referenced entity is not automatically parsed.  The application can parse it
** immediately or later using XML_ExternalEntityParserCreate.  The parser
** argument is the parser parsing the entity containing the reference; it can
** be passed as the parser argument to XML_ExternalEntityParserCreate.  The
** systemId argument is the system identifier as specified in the entity
** declaration; it will not be null.  The base argument is the system
** identifier that should be used as the base for resolving systemId if
** systemId was relative; this is set by XML_SetBase; it may be null.  The
** publicId argument is the public identifier as specified in the entity
** declaration, or null if none was specified; the whitespace in the public
** identifier will have been normalized as required by the XML spec.  The
** openEntityNames argument is a space-separated list of the names of the
** entities that are open for the parse of this entity (including the name of
** the referenced entity); this can be passed as the openEntityNames argument
** to XML_ExternalEntityParserCreate; openEntityNames is valid only until the
** handler returns, so if the referenced entity is to be parsed later, it must
** be copied.  The handler should return 0 if processing should not continue
** because of a fatal error in the handling of the external entity.  In this
** case the calling parser will return an XML_ERROR_EXTERNAL_ENTITY_HANDLING
** error.  Note that unlike other handlers the first argument is the parser,
** not userData.  */
PRIVATE int xml_externalEntityRef (XML_Parser     parser,
				   const XML_Char *openEntityNames,
				   const XML_Char *base,
				   const XML_Char *systemId,
				   const XML_Char *publicId)
{
  XML_Parser extParser =
    XML_ExternalEntityParserCreate(parser, openEntityNames, 0);
  HTAnchor  *anchor = NULL;
  HTRequest *request = HTRequest_new();
  char      *uri;
  USERDATA  *userdata = XML_GetUserData(parser);
  HTRequest *parent_request = userdata->request;
  char      *cwd = HTGetCurrentDirectoryURL();
  USERDATA *subuserdata;

  uri = HTParse((char *)systemId, cwd, PARSE_ALL);
  anchor = HTAnchor_findAddress(uri);

  HTRequest_setOutputFormat(request, WWW_SOURCE);
  set_subrequest_context(parent_request,request,xml_push_dummy(userdata));
  setup_termination_filter(request,xml_entity_termination_handler);
  subuserdata = xml_create_userData(extParser, request, NULL);
  XML_SetUserData(extParser, (void *) subuserdata);
  total_number_of_requests++;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_externalEntityRef(%s): uri=%s", RequestID(request), uri));
#endif

  /* libwww breaks when a local file request is issued concurrently with an
     existing request to another local file. So, we ignore such subrequests. */
  if ((strncmp(uri,"file:/",6) != 0) || !HTRequest_preemptive(parent_request)) {
    if (strncmp(uri,"file:/",6) == 0)
      HTRequest_setPreemptive(request,YES);
    HTLoadAnchor(anchor,request);
  } else {
    HTRequest_setAnchor(request,anchor);
    xml_entity_termination_handler(request,NULL,NULL,WWW_EXTERNAL_ENTITY);
  }

  HT_FREE(uri);
  HT_FREE(cwd);
  return TRUE;
}


/* 
** This is called for an encoding that is unknown to the parser.
** The encodingHandlerData argument is that which was passed as the
** second argument to XML_SetUnknownEncodingHandler.
** The name argument gives the name of the encoding as specified in
** the encoding declaration.
** If the callback can provide information about the encoding,
** it must fill in the XML_Encoding structure, and return 1.
** Otherwise it must return 0.
** If info does not describe a suitable encoding,
** then the parser will return an XML_UNKNOWN_ENCODING error.
*/
PRIVATE int xml_unknownEncoding (void 	        *encodingHandlerData,
				 const XML_Char *name,
				 XML_Encoding   *info)
{
  return 0;
}

/* Default is commented out so that expat will parse entities.
PRIVATE void xml_default (void * userData, const XML_Char * str, int len)
{
  XSB_StrDefine(unparsed);

  XSB_StrEnsureSize(&unparsed, len+1);
  strncpy(unparsed.string, str, len);
  unparsed.length = len;
  XSB_StrNullTerminate(&unparsed);
#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_default: Request: %s: Unparsed: %s",
	     RequestID(((USERDATA *)userData)->request), unparsed.string));
#endif

  return;
}
*/



/* Pushes an open prolog term onto the stack and return that term */
PRIVATE prolog_term xml_push_dummy(USERDATA    *userdata)
{
  prolog_term location;

  if (userdata->stackptr < 0)
    location = userdata->parsed_term_tail;
  else 
    location = STACK_TOP(userdata).content_list_tail;

  userdata->stackptr++;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In xml_push_dummy(%s): stackptr=%d",
	      RequestID(userdata->request), userdata->stackptr));
#endif

  CHECK_STACK_OVERFLOW(userdata);

  /* wire the new elt into where it should be in the content list */
  STACK_TOP(userdata).elt_term = location;
  STACK_TOP(userdata).tag = "extentity";

  /* insert new list cell into the tail and change content_list_tail to point
     to the new tail */
  if (userdata->stackptr > 0) {
    STACK_PREV(userdata).content_list_tail =
      extern_p2p_cdr(STACK_PREV(userdata).content_list_tail);
    extern_c2p_list(STACK_PREV(userdata).content_list_tail);
  } else {
    userdata->parsed_term_tail = extern_p2p_cdr(userdata->parsed_term_tail);
    extern_c2p_list(userdata->parsed_term_tail);
  }

  userdata->stackptr--;

#ifdef LIBWWW_DEBUG_VERBOSE
  if (userdata->stackptr >= 0)
    print_prolog_term(STACK_TOP(userdata).content_list_tail,
		      "content_list_tail");
  else
    print_prolog_term(userdata->parsed_term_tail, "parsed_term_tail");
#endif

  return location;
}


int xml_entity_termination_handler(HTRequest   *request,
				   HTResponse  *response,
				   void        *param,
				   int 	       status)
{
  char *ext_entity_expansion=NULL;
  REQUEST_CONTEXT *context = (REQUEST_CONTEXT *)HTRequest_context(request);
  USERDATA *userdata = context->userdata;
  XML_Parser extParser = userdata->parser;

  /* the following conditions are handled by standard libwww filters */
  if (context->retry && AUTH_OR_REDIRECTION(status))
    return HT_OK; /* this causes other filters to be used */

  if (status==HT_LOADED) {
    ext_entity_expansion = HTChunk_toCString(context->result_chunk);
#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In xml_entity_termination_handler(%s): entity=%s", 
		RequestID(request), ext_entity_expansion));
#endif
    XML_Parse(extParser,ext_entity_expansion,strlen(ext_entity_expansion),1);
    HT_FREE(ext_entity_expansion);
  } else {
    prolog_term request_result = extern_p2p_car(context->request_result);
    char *uri = HTAnchor_address((HTAnchor *)HTRequest_anchor(request));
#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In xml_entity_termination_handler(%s): request failed",
		RequestID(request)));
#endif
    extern_c2p_functor("unexpanded_entity",2,request_result);
    extern_c2p_string(uri,extern_p2p_arg(request_result,1));
    extern_c2p_int(status,extern_p2p_arg(request_result,2));
  }

  report_asynch_subrequest_status(request, status);

  XML_ParserFree(extParser);
  if (userdata)
    (((USERDATA *)userdata)->delete_method)(userdata);

  if (total_number_of_requests > 0)
    total_number_of_requests--;
  /* when the last request is done, stop the event loop */
  if ((total_number_of_requests == 0) && event_loop_runnung) {
    HTEventList_stopLoop();
    event_loop_runnung = FALSE;
#ifdef LIBWWW_DEBUG
    xsb_dbgmsg((LOG_DEBUG,"***In xml_entity_termination_handler: event loop halted, status=%d, HTNetCount=%d",
		status, HTNet_count()));
#endif
  }

  return !HT_OK;
}


void set_xml_conversions()
{
  /* Must delete old converter and create new. Apparently something in libwww
     releases the atoms used in thes converters, which causes it to crash 
     in HTStreamStack() on the second call to xmlparse. */
  HTPresentation_deleteAll(XML_converter);
  XML_converter = HTList_new();

  HTConversion_add(XML_converter,"*/*", "www/debug",
		   HTBlackHoleConverter, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"message/rfc822", "*/*",
		   HTMIMEConvert, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"message/x-rfc822-foot", "*/*",
		   HTMIMEFooter, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"message/x-rfc822-head", "*/*",
		   HTMIMEHeader, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"message/x-rfc822-cont", "*/*",
		   HTMIMEContinue, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"message/x-rfc822-upgrade","*/*",
		   HTMIMEUpgrade, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"message/x-rfc822-partial", "*/*",
		   HTMIMEPartial, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"multipart/*", "*/*",
		   HTBoundary, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"text/x-http", "*/*",
		   HTTPStatus_new, 1.0, 0.0, 0.0);
  /* www/xml is invented for servers that don't recognize XML */
  HTConversion_add(XML_converter,"text/plain", "www/xml",
		   HTXML_new, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"text/html", "www/xml",
		   HTXML_new, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter,"www/present", "www/xml",
		   HTXML_new, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter, "text/xml", "*/*", 
		   HTXML_new, 1.0, 0.0, 0.0);
  HTConversion_add(XML_converter, "application/xml", "*/*",
		   HTXML_new, 1.0, 0.0, 0.0);
}


