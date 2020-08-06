/*  $Id: sgmlutil.h,v 1.5 2011-08-06 05:54:39 kifer Exp $

    Modified from SWI-Prolog

    Author:        Jan Wielemaker
    E-mail:        jan@swi.psy.uva.nl
    WWW:           http://www.swi-prolog.org
    Copyright (C): 1985-2002, University of Amsterdam

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef DTD_UTIL_H_INCLUDED
#define DTD_UTIL_H_INCLUDED
#include "sgmldefs.h"

#include <sys/types.h>
#ifdef _WINDOWS		      /* get size_t */
#include <malloc.h>
#endif

#include "xsb_config.h"
#include "context.h"

typedef struct 
{ int allocated;
  int size;
  ichar *data;
} icharbuf;

typedef struct 
{ int allocated;
  Integer size;
  ochar *data;
} ocharbuf;

int		istrlen(const ichar *s);
ichar *         istrdup(const ichar *s);
ichar *		istrcpy(ichar *d, const ichar *s);
ichar *		istrupper(ichar *s);
ichar *		istrlower(ichar *s);
int             istrprefix(const ichar *pref, const ichar *s);
int             istreq(const ichar *s1, const ichar *s2);
int             istrcaseeq(const ichar *s1, const ichar *s2);
int		istrncaseeq(const ichar *s1, const ichar *s2, size_t len);
int             istrhash(const ichar *t, int tsize);
int             istrcasehash(const ichar *t, int tsize);
ichar *		istrchr(const ichar *s, int c);
int		istrtol(const ichar *s, long *val);
void *		sgml_malloc(size_t size);
void *		sgml_calloc(size_t n, size_t size);
void		sgml_free(void *mem);
void *		sgml_realloc(void *old, size_t size);
void		sgml_nomem(void);

int check_for_error_socket();

#define add_icharbuf(buf, chr) \
	do { if ( buf->size < buf->allocated ) \
	       buf->data[buf->size++] = chr; \
	     else \
	       __add_icharbuf(buf, chr); \
	   } while(0)
#define add_ocharbuf(buf, chr) \
	do { if ( buf->size < buf->allocated ) \
	       buf->data[buf->size++] = chr; \
	     else \
	       __add_ocharbuf(buf, chr); \
	   } while(0)

icharbuf *	new_icharbuf(void);
void		free_icharbuf(icharbuf *buf);
void		__add_icharbuf(icharbuf *buf, int chr);
void		del_icharbuf(icharbuf *buf);
void		terminate_icharbuf(icharbuf *buf);
void		empty_icharbuf(icharbuf *buf);

int		ostrlen(const ochar *s);
ochar *         ostrdup(const ochar *s);

ocharbuf *	new_ocharbuf(void);
void		free_ocharbuf(ocharbuf *buf);
void		__add_ocharbuf(ocharbuf *buf, int chr);
void		del_ocharbuf(ocharbuf *buf);
void		terminate_ocharbuf(ocharbuf *buf);
void		empty_ocharbuf(ocharbuf *buf);

const char *	str_summary(const char *s, int len);
char *		str2ring(const char *in);
char *		ringallo(size_t);
ichar *		load_sgml_file_to_charp(const char *file, int normalise_rsre,
					size_t *len);

#if defined(USE_STRING_FUNCTIONS) && !defined(UTIL_H_IMPLEMENTATION)

#define istrlen(s1) strlen((char const *)(s1))
#define istreq(s1,s2) (strcmp((char const *)(s1),(char const *)(s2))==0)

#endif

#endif /*DTD_UTIL_H_INCLUDED*/
