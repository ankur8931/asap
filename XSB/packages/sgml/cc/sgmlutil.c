/*  $Id: sgmlutil.c,v 1.9 2013-05-06 21:10:26 dwarren Exp $

    Part of SWI-Prolog

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

#include "deprecation.h"

#define UTIL_H_IMPLEMENTATION
#include "sgmlutil.h"
#include <ctype.h>
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#define MAXSTRLEN 256

ichar *
istrdup(const ichar *s)
{ ichar *dup = malloc((istrlen(s)+1)*sizeof(ichar));
  ichar *d = dup;

  while(*s)
    *d++ = *s++;
  *d = 0;

  return dup;
}


ichar *
istrcpy(ichar *d, const ichar *s)
{ ichar *r = d;

  while(*s)
    *d++ = *s++;
  *d = 0;

  return r;
}


int
istrcaseeq(const ichar *s1, const ichar *s2)
{ ichar c;

  while ((c = *s1++) != '\0')
  { if (tolower(*(ichar const *)s2++) != tolower(c))
      return FALSE;
  }

  return *s2 == '\0';
}

/*
int
istreq(const ichar *s1, const ichar *s2)
{ while(*s1 && *s1 == *s2)
    s1++, s2++;
  
  if ( *s1 == 0 && *s2 == 0 )
    return TRUE;
  
  return FALSE;
}
*/
int
istrncaseeq(const ichar *s1, const ichar *s2, size_t len) {
  int templen = (int) len;
  while(--templen >= 0 && tolower(*s1) == tolower(*s2))
    s1++, s2++;
  
  if ( templen < 0 )
    return TRUE;
  
  return FALSE;
}


int
istrprefix(const ichar *pref, const ichar *s)
{ while(*pref && *pref == *s)
    pref++, s++;
  
  if ( *pref == 0 )
    return TRUE;
  
  return FALSE;
}


ichar *
istrchr(const ichar *s, int c)
{ for( ; *s; s++ )
  { if ( c == *s )
      return (ichar *)s;
  }

  return NULL;
}


ichar *
istrupper(ichar *s)
{ ichar *r = s;

  for( ; *s; s++)
    *s = toupper(*s);

  return r;
}


ichar *
istrlower(ichar *s)
{ ichar *r = s;

  for( ; *s; s++)
    *s = tolower(*s);

  return r;
}


int
istrhash(const ichar *t, int tsize)
{ unsigned int value = 0;
  unsigned int shift = 5;

  while(*t)
  { unsigned int c = *t++;
    
    c -= 'a';
    value ^= c << (shift & 0xf);
    shift ^= c;
  }

  value = value ^ (value >> 16);

  return value % tsize;
}


int
istrcasehash(const ichar *t, int tsize)
{ unsigned int value = 0;
  unsigned int shift = 5;

  while(*t)
  { unsigned int c = tolower(*t++);	/* case insensitive */
    
    c -= 'a';
    value ^= c << (shift & 0xf);
    shift ^= c;
  }

  value = value ^ (value >> 16);

  return value % tsize;
}


int
istrtol(const ichar *s, long *val)
{ long v;
  char *e;

  if ( *s )
  { v = strtol((const char *)s, &e, 10);
    if ( !e[0] && errno != ERANGE )
    { *val = v;
      return TRUE;
    }
  }

  return FALSE;
}



		 /*******************************
		 *    INPUT CHARACTER BUFFER	*
		 *******************************/

icharbuf *
new_icharbuf()
{ icharbuf *buf = sgml_malloc(sizeof(*buf));

  buf->allocated = 0;
  buf->size = 0;
  buf->data = NULL;

  return buf;
}


void
free_icharbuf(icharbuf *buf)
{ if ( buf->data )
    sgml_free(buf->data);

  sgml_free(buf);
}


void
__add_icharbuf(icharbuf *buf, int chr)
{ if ( buf->size == buf->allocated )
  { buf->allocated = (buf->allocated ? buf->allocated*2 : 128);

    if ( buf->data )
      buf->data = sgml_realloc(buf->data, buf->allocated);
    else
      buf->data = sgml_malloc(buf->allocated);
  }
  
  buf->data[buf->size++] = chr;
}


void
del_icharbuf(icharbuf *buf)
{ if ( buf->size > 0 )
    buf->size--;
}


void
terminate_icharbuf(icharbuf *buf)
{ add_icharbuf(buf, '\0');
  buf->size--;
}


void
empty_icharbuf(icharbuf *buf)
{ buf->size = 0;
}


		 /*******************************
		 *	OUTPUT CHARACTERS	*
		 *******************************/

int
ostrlen(const ochar *s)
{ int len =0;
  
  while(*s++)
    len++;

  return len;
}


ochar *
ostrdup(const ochar *s)
{ ochar *dup = sgml_malloc((ostrlen(s)+1)*sizeof(ochar));
  ochar *d = dup;

  while(*s)
    *d++ = *s++;
  *d = 0;

  return dup;
}


		 /*******************************
		 *    OUTPUT CHARACTER BUFFER	*
		 *******************************/

ocharbuf *
new_ocharbuf()
{ ocharbuf *buf = sgml_malloc(sizeof(*buf));

  buf->allocated = 0;
  buf->size = 0;
  buf->data = NULL;

  return buf;
}


void
free_ocharbuf(ocharbuf *buf)
{ if ( buf->data )
    sgml_free(buf->data);

  sgml_free(buf);
}


void
__add_ocharbuf(ocharbuf *buf, int chr)
{ if ( buf->size == buf->allocated )
  { buf->allocated = (buf->allocated ? buf->allocated*2 : 128);

    if ( buf->data )
      buf->data = sgml_realloc(buf->data, buf->allocated);
    else
      buf->data = sgml_malloc(buf->allocated);
  }
  
  buf->data[buf->size++] = chr;
}


void
del_ocharbuf(ocharbuf *buf)
{ if ( buf->size > 0 )
    buf->size--;
}


void
terminate_ocharbuf(ocharbuf *buf)
{ add_ocharbuf(buf, '\0');
  buf->size--;
}


void
empty_ocharbuf(ocharbuf *buf)
{ buf->size = 0;
}

		 /*******************************
		 *	   BUFFER RING		*
		 *******************************/

#define RINGSIZE 16
static char *ring[RINGSIZE];
static int  ringp;

char *
str2ring(const char *in)
{ char *copy = strdup(in);

  if ( ring[ringp] )
    sgml_free(ring[ringp]);
  ring[ringp++] = copy;
  if ( ringp == RINGSIZE )
    ringp = 0;

  if ( !copy )
    sgml_nomem();

  return copy;
}


char *ringallo(size_t size)
{ char *result = malloc(size);
    
  if ( ring[ringp] != 0 )
    sgml_free(ring[ringp]);
  ring[ringp++] = result;
  if ( ringp == RINGSIZE )
    ringp = 0;

  return result;
}


               /*******************************
               *              MISC            *
               *******************************/

char const *
str_summary(char const *s, int len)
{ char *buf;
  size_t l = strlen(s);

  if ( l < (size_t)len )
    return s;
  buf = ringallo(len + 10);
  strncpy(buf, s, len-5);
  strcpy(&buf[len-5], " ... ");
  strcpy(&buf[len], &s[l-5]);

  return buf;
}



		 /*******************************
		 *	      FILES		*
		 *******************************/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Load a file into memory. This would be so  easy if we didn't had to deal
with &#RE/&#RS handling that forces us to create the proper record start
and end.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef O_BINARY
#define O_BINARY 0
#endif

ichar *
load_sgml_file_to_charp(const char *file, int normalise_rsre, size_t *length)
{ int fd;

  if ( (fd = open(file, O_RDONLY|O_BINARY)) >= 0 )
  { struct stat buf;

    if ( fstat(fd, &buf) == 0 )
    { int len = buf.st_size;
      char *r = sgml_malloc(len+1);

      if ( r )
      { char *s = r;
	
	while(len>0) {
	  int n;

	  if ( (n=read(fd, s, len)) < 0 ) {
	    close(fd);			/* I/O error */
	    sgml_free(r);
	    return NULL;
	  } else if ( n == 0 )
	    break;
	  len -= n;
	  s += n;
	}

	len = (int) (s-r);
	*s = '\0';			/* ensure closing EOS */
	close(fd);

	if ( normalise_rsre )
	{ int nl;
	  int last_is_lf;

	  last_is_lf = (s > 0 && s[-1] == '\n');

	  for(s=r, nl=0; *s; s++) {
	    if ( *s == '\n' && s>r && s[-1] != '\r' )
	      nl++;
	  }

	  if ( nl > 0 ) {
	    char *r2 = sgml_malloc(len+nl+1);
	    char *t;

	    for(s=r, t=r2; *s; s++) {
	      if ( *s == '\n' ) {
		if ( s>r && s[-1] != '\r' ) {
		  *t++ = CARRIAGERETURN;
		  *t++ = LINEFEED;
		} else {
		  *t++ = *s;
		}
	      }
	    }
            len = (int) (t-r2);
	    *t = '\0';
	    sgml_free(r);
	    r = r2;
	  }
	  
	  if ( last_is_lf )
	    r[--len] = '\0';		/* delete last LINEFEED */
	}
	
	if ( length )
	  *length = len;
	
	return (ichar *)r;
      }
    }
  }
  
  return NULL;
}


		 /*******************************
		 *	     ALLOCATION		*
		 *******************************/

#ifdef _WINDOWS
#include <windows.h>
#endif

void
sgml_nomem()
{ fprintf(stderr, "SGML: Fatal: out of memory\n");

#ifdef _WINDOWS
   MessageBox(NULL, "SGML: Fatal: out of memory", "SGML", MB_OK|MB_TASKMODAL);
#endif

  exit(1);
}


void *
sgml_malloc(size_t size)
{ void *mem;

  if ( size == 0 )
    return NULL;

  if ( (mem = malloc(size)) ) {
    return mem;
  }

  sgml_nomem();
  return NULL;
}


void *
sgml_realloc(void *old, size_t size)
{ void *mem;

  if ( old )
    { if ( (mem = realloc(old, size)) ) {
      return mem;
      }
  } else
    { if ( (mem = malloc(size)) ) {
      return mem;
      }
  }

  sgml_nomem();
  return NULL;
}


void *
sgml_calloc(size_t n, size_t size)
{ void *mem;

  if ( (mem=malloc(size*n+1)) ) { // need +1 in window 7; otw crashes in free?
    memset(mem,0,size*n);
    return mem;
  }

  sgml_nomem();
  return NULL;
}


void
sgml_free(void *mem)
{ if ( mem ) {
    free(mem);
  }
}
