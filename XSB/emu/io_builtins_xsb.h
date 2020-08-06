/* File:      io_builtins_xsb.h
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: io_builtins_xsb.h,v 1.23 2013/01/04 14:56:22 dwarren Exp $
** 
*/

#include "io_defs_xsb.h"
#include "token_xsb.h"

#define ERROR_ON_EOF         0 
#define EOF_CODE_ON_EOF   1
#define RESET_ON_EOF          2

typedef struct  {
  FILE *file_ptr;
  char *file_name;
  char io_mode;
  int stream_type;
  int reposition;
  int eof_action;
  int charset;
#ifdef MULTI_THREAD
  pthread_mutex_t stream_mutex;
  int stream_mutex_owner;
#endif  
} stream_record;

extern stream_record open_files[];      /* Table of file pointers for open files */

#define OPENFILES_MUTEX(i) &(open_files[i].stream_mutex) 
#define OPENFILES_MUTEX_OWNER(i) (open_files[i].stream_mutex_owner) 

#ifdef MULTI_THREAD
#define CHECK_IOS_OWNER(index)\
  {\
	if ( iostrs[iostrdecode(index)]->owner != xsb_thread_id )\
		xsb_error( "trying to access other threads iostrs entry" );\
  }
#define XSB_STREAM_LOCK(index) { \
  if (index >= 0) \
  {	pthread_mutex_lock(OPENFILES_MUTEX(index)); \
	OPENFILES_MUTEX_OWNER(index) = xsb_thread_id; \
  }\
  else\
  {	CHECK_IOS_OWNER(index);\
  }\
}
#define XSB_STREAM_UNLOCK(index) { \
  if (index >= 0) \
  {	pthread_mutex_unlock(OPENFILES_MUTEX(index)); \
	OPENFILES_MUTEX_OWNER(index) = -1; \
  }\
}
#else
#define CHECK_IOS_OWNER(index)
#define XSB_STREAM_LOCK(index) 
#define XSB_STREAM_UNLOCK(index) 
#endif

extern void strclose( int ) ;

extern int xsb_intern_fileptr(CTXTdeclc FILE *file,char *c,char *c2,char *c3, int charset);
extern int xsb_intern_file(CTXTdeclc char *c1,char *c2,int *i,char *strmode,int opennew);

extern void write_quotedname(FILE *fptr, int charset, char *string);
extern int double_quotes(char *string, char *new_string);
extern xsbBool quotes_are_needed(char *string);

Integer read_canonical_term(CTXTdeclc int, int);

void print_term_canonical(CTXTdeclc FILE *, int, Cell, int);

extern int get_more_chunk(CTXTdecl);
extern void findall_copy_to_heap(CTXTdeclc Cell, CPtr, CPtr *);
extern int findall_init_c(CTXTdecl);
extern void findall_free(CTXTdeclc int);

extern int char_to_codepoint(int, byte **s_ptr);

int unset_fileptr(FILE *stream);

