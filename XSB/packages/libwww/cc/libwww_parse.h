/* File:      libwww_parse.h
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
** $Id: libwww_parse.h,v 1.5 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

/* included macros for XML and HTML parsing only */


#define PARSE_STACK_INCREMENT  50


#define STACK_TOP(htext)        htext->stack[htext->stackptr]
#define STACK_PREV(htext)       htext->stack[htext->stackptr-1]

#define suppressing(htext) \
     	     	     ( (htext->stackptr < 0 && htext->suppress_is_default) \
      	      	       ||(htext->stackptr >= 0 && STACK_TOP(htext).suppress))
#define parsing(htext)  (!suppressing(htext))


#define IS_SELECTED_TAG(element, request) \
    is_in_htable(element, \
    	    	 &(((REQUEST_CONTEXT *)HTRequest_context(request))->selected_tags_tbl))
#define IS_SUPPRESSED_TAG(element, request) \
    is_in_htable(element, \
    	    	 &(((REQUEST_CONTEXT *)HTRequest_context(request))->suppressed_tags_tbl))
#define IS_STRIPPED_TAG(element, request) \
    is_in_htable(element, \
    	    	 &(((REQUEST_CONTEXT *)HTRequest_context(request))->stripped_tags_tbl))

#define CHECK_STACK_OVERFLOW(userdata) \
    if (userdata->stackptr >= userdata->stacksize) { \
       userdata->stack =  \
         realloc(userdata->stack, \
	         (userdata->stacksize + PARSE_STACK_INCREMENT) \
	         * sizeof(struct stack_node)); \
       userdata->stacksize += PARSE_STACK_INCREMENT; \
    }

#define SETUP_STACK(userdata) \
    userdata->stackptr = -1; \
    userdata->stack = \
     	 malloc(PARSE_STACK_INCREMENT * sizeof(struct stack_node)); \
    userdata->stacksize = PARSE_STACK_INCREMENT;
