/* File:      libwww_parse_html.h
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
** $Id: libwww_parse_html.h,v 1.8 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


typedef HText  USERDATA;
struct _HText {
  DELETE_USERDATA *    	  delete_method;
  int 	      	      	  status;    	   /* this is used to carry status into
					      delete_userData */
  HTRequest *		  request;
  HTParentAnchor * 	  node_anchor; 	   /* not used */
  HTStream *		  target;
  SGML_dtd *		  dtd;
  int	 	     	  suppress_is_default; /* whether we begin parsing by
						  suppressing tags */
  prolog_term	     	  parsed_term;      /* actual result of the parse */
  prolog_term	     	  parsed_term_tail; /* auxil variable */
  int   		  stackptr;
  int	       	          stacksize;	    /* current size of stack */
  struct stack_node {
    int	       	   element_number;    /* which element this is  */
    SGMLContent    element_type;      /* SGML_EMPTY, PCDATA_SPECIAL, normal */
    int	       	   suppress;   	      /* whether this element is in the
					 suppressed region */
    prolog_term	   elt_term;	      /* here we build elements */
    prolog_term    content_list_tail; /* auxil var to help build elements */
  } 	    	    	  *stack;     /* keeps nested elements */
};


/* function declarations */

PRIVATE inline HTTag *special_find_tag(USERDATA *htext, int element_number);

PRIVATE USERDATA *html_create_userData( HTRequest         *request,
					HTParentAnchor    *anchor,
					HTStream          *output_stream);
PRIVATE void html_delete_userData(void *me);

PRIVATE int find_matching_elt(USERDATA *htext, int elt_number);

PRIVATE void html_push_element (USERDATA        *htext,
				int             element_number,
				const BOOL      *present,
				const char     **value);
PRIVATE void html_pop_element(USERDATA *htext);
PRIVATE void html_push_suppressed_element(USERDATA *htext, int element_number);
PRIVATE void html_pop_suppressed_element(USERDATA *htext);
PRIVATE void collect_html_attributes ( prolog_term  elt_term,
				  HTTag        *tag,
				  const BOOL   *present,
				  const char  **value);


PRIVATE void html_addText (USERDATA *htext, const char *textbuf, int len);
PRIVATE void html_beginElement(USERDATA  	*htext,
			       int	        element_number,
			       const BOOL       *present,
			       const char       **value);
PRIVATE void html_endElement(USERDATA *htext, int element_number);

