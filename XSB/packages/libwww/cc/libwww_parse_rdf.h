/* File:      libwww_parse_rdf.h
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
** $Id: libwww_parse_rdf.h,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


typedef struct RDF_userData USERDATA;
struct RDF_userData {
  DELETE_USERDATA *   	  delete_method;
  int 	      	      	  status;    	   /* this is used to carry status into
					      delete_userData */
  HTRDF *                 parser; 
  HTRequest *		  request;
  HTStream *		  target;
  prolog_term	     	  parsed_term;      /* actual result of the parse */
  prolog_term	     	  parsed_term_tail; /* auxil variable */
};

PRIVATE void rdf_new_triple_handler (HTRDF *rdfp, HTTriple *t, void *context);
PRIVATE void rdf_delete_userData(void *userdata);
PRIVATE USERDATA *rdf_create_userData(HTRDF 	*parser,
				      HTRequest *request,
				      HTStream  *target_stream);
