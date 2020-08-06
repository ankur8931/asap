/*
** File: packages/curl/cc/error.h
** Author: Aneesh Ali
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2010
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
*/

/*****************************************************************************
** This file defines the constants, macros and enumerations useful in error
** handling.
*****************************************************************************/

#ifndef H_ERROR_INCLUDED
#define H_ERROR_INCLUDED
#include <stdarg.h>

typedef enum
{ ERR_ERRNO,				/* , int */
					/* ENOMEM */
					/* EACCES --> file, action */
					/* ENOENT --> file */
  ERR_TYPE,				/* char *expected, term_t actual */
  ERR_DOMAIN,				/* char *expected, term_t actual */
  ERR_EXISTENCE,			/* char *expected, term_t actual */

  ERR_FAIL,				/* term_t goal */

  ERR_LIMIT,				/* char *limit, long max */
  ERR_MISC				/* char *fmt, ... */
} plerrorid;

int		sgml2pl_error(plerrorid, ...);

#endif /*H_ERROR_INCLUDED*/
