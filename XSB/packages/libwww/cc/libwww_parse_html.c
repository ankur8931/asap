/* File:      libwww_parse_html.c
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
** $Id: libwww_parse_html.c,v 1.15 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include "libwww_util.h"
#include "libwww_parse.h"
#include "libwww_parse_html.h"


/* BOOL, PRIVATE, PUBLIC, etc., are defined in a Libwww header */

/* This is the callback that captures start tag events */
PRIVATE void html_beginElement(USERDATA	*htext, /* where we build everything */
			       int	element_number, /* internal tag # */
			       /* bitmap: tells which tag attrs are present */
			       const BOOL *present,
			       /* array of values for the attributes
				  specified by the "present" bitmap */ 
			       const char **value)
{
#ifdef LIBWWW_DEBUG
  HTTag *tag = SGML_findTag(htext->dtd, element_number);
  xsb_dbgmsg((LOG_DEBUG,"***In html_beginElement(%s): stackptr=%d tag=%s suppress=%d choose=%d",
	     RequestID(htext->request),
	     htext->stackptr, HTTag_name(tag),
	     IS_SUPPRESSED_TAG((HKEY)element_number, htext->request),
	     IS_SELECTED_TAG((HKEY)element_number, htext->request)
	      ));
#endif

  if (IS_STRIPPED_TAG((HKEY)element_number, htext->request)) return;

  if ((suppressing(htext) && !IS_SELECTED_TAG((HKEY)element_number, htext->request))
      || (parsing(htext) && IS_SUPPRESSED_TAG((HKEY)element_number, htext->request))) {
    html_push_suppressed_element(htext, element_number);
    return;
  }

  /* parsing or suppressing & found a selected tag */
  if ((parsing(htext) && !IS_SUPPRESSED_TAG((HKEY)element_number,htext->request))
      || (suppressing(htext) 
	  && IS_SELECTED_TAG((HKEY)element_number, htext->request))) {
    html_push_element(htext,element_number,present,value);
    return;
  }
}


/* The callback for the end-tag event */
PRIVATE void html_endElement (USERDATA *htext, int element_number)
{
  int i, match;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In html_endElement(%s): stackptr=%d",
	      RequestID(htext->request), htext->stackptr));
#endif

  if (IS_STRIPPED_TAG((HKEY)element_number, htext->request)) return;

  match = find_matching_elt(htext, element_number);
  /* the closing tag is probably out of place */
  if (match < 0) return;

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***match=%d", match));
#endif

  for (i=htext->stackptr; i>=match; i--)
    if (parsing(htext))
      html_pop_element(htext);
    else
      html_pop_suppressed_element(htext);

#ifdef LIBWWW_DEBUG_VERBOSE
  if (htext->stackptr >= 0) {
    if (!STACK_TOP(htext).suppress)
      print_prolog_term(STACK_TOP(htext).elt_term, "elt_term");
  }
#endif

  return;
}



/* The callback to capture text events */
PRIVATE void html_addText (USERDATA *htext, const char *textbuf, int len)
{
  static XSB_StrDefine(pcdata_buf);
  int shift = 0;
  REQUEST_CONTEXT *context =
    (REQUEST_CONTEXT *)HTRequest_context(htext->request);

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In html_addText: Request %s", RequestID(htext->request)));
#endif

  if (IS_STRIPPED_TAG((HKEY)PCDATA_SPECIAL, htext->request)) return;
  if (suppressing(htext)) return;


  /* strip useless newlines */
  if (strncmp(textbuf,"\n", len) == 0) return;

  html_push_element(htext, PCDATA_SPECIAL, NULL, NULL);

  /* copy textbuf (which isn't null-terminated) into a variable length str */
  XSB_StrEnsureSize(&pcdata_buf, len+1);
  strncpy(pcdata_buf.string, textbuf, len);
  pcdata_buf.length = len;
  XSB_StrNullTerminate(&pcdata_buf);

  /* if string starts with a newline, skip the newline */
  if (strncmp(textbuf,"\n", strlen("\n")) == 0)
    shift = strlen("\n");

  /* put the text string into the elt term and then pop it */
  if (context->convert2list)
    extern_c2p_chars(pcdata_buf.string+shift,
		     4,
		     extern_p2p_arg(STACK_TOP(htext).elt_term,3));
  else 
    extern_c2p_string(pcdata_buf.string+shift, extern_p2p_arg(STACK_TOP(htext).elt_term,3));

  html_pop_element(htext);
  return;
}


/* Collect tag's attributes and make them into a list of the form
   [attval(attr,val), ...]; bind it to Arg 2 of ELT_TERM */
PRIVATE void collect_html_attributes ( prolog_term  elt_term,
				       HTTag        *tag,
				       const BOOL   *present,
				       const char  **value)
{
  int tag_attributes_number = HTTag_attributes(tag);
  static XSB_StrDefine(attrname);
  int cnt;
  prolog_term
    prop_list = extern_p2p_arg(elt_term,2),
    prop_list_tail = prop_list,
    prop_list_head;

  extern_c2p_list(prop_list_tail);

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In collect_html_attributes: tag_attributes_number=%d",
	      tag_attributes_number));
#endif

  for (cnt=0; cnt<tag_attributes_number; cnt++) {
    if (present[cnt]) {
      XSB_StrEnsureSize(&attrname, strlen(HTTag_attributeName(tag, cnt)));
      strcpy_lower(attrname.string, HTTag_attributeName(tag, cnt));
      
#ifdef LIBWWW_DEBUG_VERBOSE
      xsb_dbgmsg((LOG_DEBUG,"***attr=%s, val=%s ",
		  attrname.string, (char *)value[cnt]));
#endif
      prop_list_head = extern_p2p_car(prop_list_tail);
      extern_c2p_functor("attval",2,prop_list_head);
      extern_c2p_string(attrname.string, extern_p2p_arg(prop_list_head,1));
      /* some attrs, like "checked", are boolean and have no value; in this
	 case we leave the value arg uninstantiated */
      if ((char *)value[cnt])
	extern_c2p_string((char *)value[cnt], extern_p2p_arg(prop_list_head, 2));
    
      prop_list_tail = extern_p2p_cdr(prop_list_tail);
      extern_c2p_list(prop_list_tail);
    }
  }

  /* Terminate the property list */
  extern_c2p_nil(prop_list_tail);
  return;
}


/* push element onto HTEXT->stack */
PRIVATE void html_push_element (USERDATA       *htext,
				int            element_number,
				const BOOL     *present,
				const char     **value)
{
  static XSB_StrDefine(tagname);
  HTTag *tag = special_find_tag(htext, element_number);
  prolog_term location;

  /*   If tag is not valid for HTML */
  if (tag == NULL) return;

  if (htext->stackptr < 0)
    location = htext->parsed_term_tail;
  else 
    location = STACK_TOP(htext).content_list_tail;

  htext->stackptr++;

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In html_push_element(%s): stackptr=%d",
	      RequestID(htext->request), htext->stackptr));
#endif

  CHECK_STACK_OVERFLOW(htext);

  /* wire the new elt into where it should be in the content list */
  STACK_TOP(htext).elt_term = extern_p2p_car(location);

  STACK_TOP(htext).element_number = element_number;
  STACK_TOP(htext).suppress = FALSE;

  /* normal tags look like elt(tagname, attrlist, contentlist);
     pcdata tags are: elt(pcdata,[],text);
     empty tags look like elt(tagname,attrlist,[]); */
  STACK_TOP(htext).element_type = HTTag_content(tag);
  extern_c2p_functor("elt",3,STACK_TOP(htext).elt_term);

  XSB_StrEnsureSize(&tagname, strlen(HTTag_name(tag)));
  strcpy_lower(tagname.string, HTTag_name(tag));
  extern_c2p_string(tagname.string, extern_p2p_arg(STACK_TOP(htext).elt_term, 1));
  collect_html_attributes(STACK_TOP(htext).elt_term, tag, present, value);
#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***elt_name=%s", HTTag_name(tag)));
  print_prolog_term(STACK_TOP(htext).elt_term, "elt_term");
#endif

  switch (STACK_TOP(htext).element_type) {
  case SGML_EMPTY:
    extern_c2p_nil(extern_p2p_arg(STACK_TOP(htext).elt_term,3));
    html_pop_element(htext);
    break;
  case PCDATA_SPECIAL:
    /* nothing to do: we pop this after htext is inserted in html_addText */
    break;
  default: /* normal elt */
    STACK_TOP(htext).content_list_tail = extern_p2p_arg(STACK_TOP(htext).elt_term,3);
    extern_c2p_list(STACK_TOP(htext).content_list_tail);
  }
}


/* When done with an elt, close its contents list and pop the stack */
PRIVATE void html_pop_element(USERDATA *htext)
{
#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In html_pop_element(%s): stackptr=%d, elt_name=%s",
	     RequestID(htext->request),
	     htext->stackptr,
	      HTTag_name(special_find_tag(htext, STACK_TOP(htext).element_number))));
#endif
  /* close the property list, for notmal elements */
  switch (STACK_TOP(htext).element_type) {
  case SGML_EMPTY: /* this case can't occur */
    break;
  case PCDATA_SPECIAL:
    break;
  default: /* normal element */
    extern_c2p_nil(STACK_TOP(htext).content_list_tail);
  }

  /* insert new list cell into the tail and change content_list_tail to point
     to the new tail */
  if (htext->stackptr > 0) {
    STACK_PREV(htext).content_list_tail =
      extern_p2p_cdr(STACK_PREV(htext).content_list_tail);
    extern_c2p_list(STACK_PREV(htext).content_list_tail);
  } else {
    htext->parsed_term_tail = extern_p2p_cdr(htext->parsed_term_tail);
    extern_c2p_list(htext->parsed_term_tail);
  }

  htext->stackptr--;

#ifdef LIBWWW_DEBUG_VERBOSE
  if (htext->stackptr >= 0)
    print_prolog_term(STACK_TOP(htext).content_list_tail, "content_list_tail");
  else
    print_prolog_term(htext->parsed_term_tail, "parsed_term_tail");
#endif

  return;
}


/* Push tag, but keep only the tag info; don't convert to prolog term */
PRIVATE void html_push_suppressed_element(USERDATA *htext, int element_number)
{
  /* if empty tag, then just return */
  if (SGML_findTagContents(htext->dtd, element_number) == SGML_EMPTY)
      return;
  /* non-empty tag */
  htext->stackptr++; /* advance ptr, but don't push tag */

  STACK_TOP(htext).element_number = element_number;
  STACK_TOP(htext).suppress = TRUE;

  /* passing content list tail through suppressed elements */
  if (htext->stackptr == 0)
    STACK_TOP(htext).content_list_tail = htext->parsed_term_tail;
  else 
    STACK_TOP(htext).content_list_tail = STACK_PREV(htext).content_list_tail;

  return;
}


PRIVATE void html_pop_suppressed_element(USERDATA *htext)
{
  /* chain the list tails back through the sequence of suppressed tags */
  if (htext->stackptr > 0) {
    STACK_PREV(htext).content_list_tail = STACK_TOP(htext).content_list_tail;
  } else {
    htext->parsed_term_tail = STACK_TOP(htext).content_list_tail;
  }

  htext->stackptr--;

#ifdef LIBWWW_DEBUG_VERBOSE
  xsb_dbgmsg((LOG_DEBUG,"***In html_pop_suppressed_element(%s): stackptr=%d",
	      RequestID(htext->request), htext->stackptr));
  if (htext->stackptr >= 0)
    print_prolog_term(STACK_TOP(htext).content_list_tail, "content_list_tail");
  else
    print_prolog_term(htext->parsed_term_tail, "parsed_term_tail");
#endif

  return;
}

/* search the stack to see if there is a matching element */
PRIVATE int find_matching_elt(USERDATA *htext, int elt_number)
{
  int i;
  for (i=htext->stackptr; i>=0; i--) {
#ifdef LIBWWW_DEBUG_VERBOSE
    xsb_dbgmsg((LOG_DEBUG,"***In find_matching_elt"));
    xsb_dbgmsg((LOG_DEBUG,"***i=%d htext->stack[i].element_number=%d(%s) elt_number=%d(%s)",
	       i,
	       htext->stack[i].element_number, 
	       SGML_findTagName(htext->dtd, htext->stack[i].element_number),
	       elt_number,
		SGML_findTagName(htext->dtd, elt_number)));
#endif
    if (htext->stack[i].element_number == elt_number)
      return i;
  }
  return -1;
}


PRIVATE inline HTTag *special_find_tag(USERDATA *htext, int element_number)
{
  static HTTag pcdata_tag = {"pcdata", NULL, 0, PCDATA_SPECIAL};
  if (element_number == PCDATA_SPECIAL)
    return &pcdata_tag;
  return SGML_findTag(htext->dtd, element_number);
}


/* USERDATA creation and deletion callbacks */
USERDATA *html_create_userData( HTRequest *             request,
				HTParentAnchor *        anchor,
				HTStream *              output_stream)
{
  USERDATA *me = NULL;

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***Start html_create_userData(%s):", RequestID(request)));
#endif
  if (request) {
    /* make sure that MIME type is appropriate for HTML */
    if (!verifyMIMEformat(request, HTMLPARSE)) {
      /* The following causes segfault, so we xsb_abort instead 
	 HTStream * input = HTRequest_inputStream(request);
	 HTRequest_kill(request);
	 (*input->isa->abort)(input, NULL);
	 return NULL;
      */
      xsb_abort("[LIBWWW_REQUEST] Bug: Request type/MIME type mismatch");
    }
    if ((me = (USERDATA *) HT_CALLOC(1, sizeof(USERDATA))) == NULL)
      HT_OUTOFMEM("libwww_parse_html");
    me->delete_method = html_delete_userData;
    me->request = request;
    me->node_anchor =  anchor;
    me->target = output_stream;
    me->dtd = HTML_dtd();
    me->suppress_is_default = 
      ((REQUEST_CONTEXT *)HTRequest_context(request))->suppress_is_default;
    me->parsed_term = extern_p2p_new();
    extern_c2p_list(me->parsed_term);
    me->parsed_term_tail = me->parsed_term;
    SETUP_STACK(me);
  }

#ifdef LIBWWW_DEBUG
  xsb_dbgmsg((LOG_DEBUG,"***In html_create_userData(%s):", RequestID(request)));
#endif

  /* Hook up userdata to the request context */
  ((REQUEST_CONTEXT *)HTRequest_context(request))->userdata = (void *)me;
  return me;
}


PRIVATE void html_delete_userData(void *userdata)
{
  int i;
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
  xsb_dbgmsg((LOG_DEBUG,"***In html_delete_userData(%s): stackptr=%d",
	      RequestID(request), me->stackptr));
#endif

  /* close open tags on stack */
  for (i=me->stackptr; i>=0; i--)
    if (parsing(me))
      html_pop_element(me);
    else
      html_pop_suppressed_element(me);

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
  xsb_dbgmsg((LOG_DEBUG,"***Request %s: freed the USERDATA obj", RequestID(request)));
#endif

  return;
}


void html_register_callbacks()
{
  /* register callback for begin/end element events */
  HText_registerElementCallback(html_beginElement, html_endElement);
  /* register callback for text chunks */
  HText_registerTextCallback(html_addText);
  /* register callbacks to create and delete the HText (USERDATA)
     objects. These are objects where we build parsed terms */
  HText_registerCDCallback(html_create_userData, 
			   (HText_delete *)html_delete_userData);
  return;
}


void set_html_conversions()
{
  /* Must delete old converter and create new. Apparently something in libwww
     releases the atoms used in thes converters, which causes it to crash 
     in HTStreamStack() on the second call to rdfparse. */
  HTPresentation_deleteAll(HTML_converter);
  HTML_converter = HTList_new();

  HTConversion_add(HTML_converter,"*/*", "www/debug",
		   HTBlackHoleConverter, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"message/rfc822", "*/*",
		   HTMIMEConvert, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"message/x-rfc822-foot", "*/*",
		   HTMIMEFooter, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"message/x-rfc822-head", "*/*",
		   HTMIMEHeader, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"message/x-rfc822-cont", "*/*",
		   HTMIMEContinue, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"message/x-rfc822-upgrade","*/*",
		   HTMIMEUpgrade, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"message/x-rfc822-partial", "*/*",
		   HTMIMEPartial, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"multipart/*", "*/*",
		   HTBoundary, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"text/x-http", "*/*",
		   HTTPStatus_new, 1.0, 0.0, 0.0);
  /* www/html is invented by us to force html conversion */
  HTConversion_add(HTML_converter,"text/html", "www/html",
		   HTMLPresent, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"text/plain", "www/html",
		   HTMLPresent, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"www/present", "www/html",
		   HTMLPresent, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"text/xml", "www/html",
		   HTMLPresent, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter,"text/rdf", "www/html",
		   HTMLPresent, 1.0, 0.0, 0.0);
  HTConversion_add(HTML_converter, "application/html", "*/*",
		   HTMLPresent, 1.0, 0.0, 0.0);
}


