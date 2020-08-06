/* File:      libwww_parse_rdf.c
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
** $Id: libwww_parse_rdf.c,v 1.8 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include "libwww_util.h"
#include "libwww_parse.h"
#include "libwww_parse_rdf.h"


/* BOOL, PRIVATE, PUBLIC, etc., are defined in a Libwww header */


void set_rdf_conversions()
{
  /* Must delete old converter and create new. Apparently something in libwww
     releases the atoms used in thes converters, which causes it to crash 
     in HTStreamStack() on the second call to rdfparse. */
  HTPresentation_deleteAll(RDF_converter);
  RDF_converter = HTList_new();

  HTConversion_add(RDF_converter,"*/*", "www/debug",
		   HTBlackHoleConverter, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"message/rfc822", "*/*",
		   HTMIMEConvert, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"message/x-rfc822-foot", "*/*",
		   HTMIMEFooter, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"message/x-rfc822-head", "*/*",
		   HTMIMEHeader, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"message/x-rfc822-cont", "*/*",
		   HTMIMEContinue, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"message/x-rfc822-upgrade","*/*",
		   HTMIMEUpgrade, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"message/x-rfc822-partial", "*/*",
		   HTMIMEPartial, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"multipart/*", "*/*",
		   HTBoundary, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"text/x-http", "*/*",
		   HTTPStatus_new, 1.0, 0.0, 0.0);
  /* www/rdf is invented for servers that don't recognize RDF */
  HTConversion_add(RDF_converter,"text/html", "www/rdf",
		   HTRDFToTriples, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"text/xml", "www/rdf",
		   HTRDFToTriples, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"text/plain", "www/rdf",
		   HTRDFToTriples, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter,"www/present", "www/rdf",
		   HTRDFToTriples, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter, "text/rdf", "*/*", 
		   HTRDFToTriples, 1.0, 0.0, 0.0);
  HTConversion_add(RDF_converter, "application/rdf", "*/*",
		   HTRDFToTriples, 1.0, 0.0, 0.0);
}


void libwww_newRDF_parserHandler (HTStream *		me,
				  HTRequest *		request,
				  HTFormat 		target_format,
				  HTStream *		target_stream,
				  HTRDF *    	        rdfparser,
				  void *         	context)
{
  if (rdfparser) {
    
    /* Create userdata and pass rdfparser there */
    USERDATA *userdata = rdf_create_userData(rdfparser,request,target_stream);
    
    /* Register the triple callback */
    HTRDF_registerNewTripleCallback (rdfparser,
				     rdf_new_triple_handler,
				     userdata);
  }
}


/* context here is of type USERDATA */
PRIVATE void rdf_new_triple_handler (HTRDF *rdfp, HTTriple *t, void *context)
{
  USERDATA *userdata = (USERDATA *)context;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In rdf_new_triple_handler(%s)", RequestID(userdata->request)));
#endif
  /* create a new triple */
  if (rdfp && t) {
    prolog_term ptriple = extern_p2p_car(userdata->parsed_term_tail);

    extern_c2p_functor("rdftriple",3,ptriple);
    if (HTTriple_predicate(t))
      extern_c2p_string(HTTriple_predicate(t), extern_p2p_arg(ptriple,1));
    else
      extern_c2p_string("rdfunknown", extern_p2p_arg(ptriple,1));
    if (HTTriple_subject(t))
      extern_c2p_string(HTTriple_subject(t), extern_p2p_arg(ptriple,2));
    else
      extern_c2p_string("rdfunknown", extern_p2p_arg(ptriple,2));
    if (HTTriple_object(t))
      extern_c2p_string(HTTriple_object(t), extern_p2p_arg(ptriple,3));
    else
      extern_c2p_string("rdfunknown", extern_p2p_arg(ptriple,3));

#ifdef LIBWWW_DEBUG_VERBOSE
    print_prolog_term(userdata->parsed_term_tail, "Current result tail");
#endif

    userdata->parsed_term_tail = extern_p2p_cdr(userdata->parsed_term_tail);
    extern_c2p_list(userdata->parsed_term_tail);
  }
}


PRIVATE USERDATA *rdf_create_userData(HTRDF     *parser,
				      HTRequest *request,
				      HTStream  *target_stream)
{
  USERDATA *me = NULL;
#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Start rdf_create_userData: Request %s", RequestID(request)));
#endif
  if (parser) {
    /* make sure that MIME type is appropriate for RDF */
    if (!verifyMIMEformat(request, RDFPARSE)) {
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
      HT_OUTOFMEM("libwww_parse_rdf");
    me->delete_method = rdf_delete_userData;
    me->parser = parser;
    me->request = request;
    me->target = target_stream;
    me->parsed_term = extern_p2p_new();
    extern_c2p_list(me->parsed_term);
    me->parsed_term_tail = me->parsed_term;
  }
  
#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***End rdf_create_userData: Request %s", RequestID(request)));
#endif

  /* Hook up userdata to the request context */
  ((REQUEST_CONTEXT *)HTRequest_context(request))->userdata = (void *)me;

  return me;
}



PRIVATE void rdf_delete_userData(void *userdata)
{
  prolog_term parsed_result, status_term;
  USERDATA *me = (USERDATA *)userdata;
  HTRequest *request = me->request;

  if (request) {
    parsed_result =
      ((REQUEST_CONTEXT *)HTRequest_context(request))->request_result;
    status_term =
      ((REQUEST_CONTEXT *)HTRequest_context(request))->status_term;
  }
  else return;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In rdf_delete_userData(%s)", RequestID(request)));
#endif

#ifdef LIBWWW_DEBUG_VERBOSE
  print_prolog_term(me->parsed_term, "Current parse value");
#endif

  /* terminate the parsed prolog terms list */
  extern_c2p_nil(me->parsed_term_tail);

  /* pass the result to the outside world */
  if (is_var(me->parsed_term))
    extern_p2p_unify(parsed_result, me->parsed_term);
  else
    xsb_abort("[LIBWWW_REQUEST] Request %s: Arg 4 (Result) must be unbound variable",
	      RequestID(request));

  HT_FREE(me);

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Request %s: freed the USERDATA object", RequestID(request)));
#endif

  return;
}

