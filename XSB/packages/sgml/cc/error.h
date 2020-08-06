/*****************************************************************************
 *                             error.h
 * This file defines the constants, macros and enumerations useful in error
 * handling.
 *
 ***************************************************************************/

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
