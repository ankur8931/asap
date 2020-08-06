/* File:      token_xsb.h
** Author(s): Kostis F. Sagonas, Jiyang Xu
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1999
** Copyright (C) ECRC, Germany, 1990
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
** $Id: token_xsb.h,v 1.15 2011-06-05 19:24:03 tswift Exp $
** 
*/

#ifndef _TOKEN_XSB_H_
#define _TOKEN_XSB_H_

#include "token_defs_xsb.h"
#include "flags_xsb.h"

#define strgetc(p) (--(p)->strcnt>=0? ((int)*(p)->strptr++): -1)
#define strpeekc(p) ((p)->strcnt>=0? ((int)*(p)->strptr): -1)

#define GetC(card,instr) (instr ? strgetc(instr) : getc(card))

#define PutCode(codepoint,charset,file) {	\
    int cp = codepoint;				\
    if (cp < 128) putc(cp,file);		\
    else {					\
      byte s[5], *ch_ptr, *ch_ptr0;		\
      ch_ptr0 = s;				\
      ch_ptr = codepoint_to_str(cp,charset,s);	\
      while (ch_ptr0 < ch_ptr)			\
	putc(*ch_ptr0++,file);			\
    }						\
  }

struct strbuf {
  Integer strcnt;
  byte *strptr;
  byte *strbase;
#ifdef MULTI_THREAD
  int owner;
#endif
};

#define STRFILE struct strbuf
#define MAXIOSTRS 5
extern STRFILE *iostrs[MAXIOSTRS];
#define iostrdecode(j) (-1-j)
#define strfileptr(desc) iostrs[iostrdecode(desc)]
#define InitStrLen	10000

#define io_port_to_fptrs(io_port,fptr,sfptr,charset)	\
    if ((io_port < 0) && (io_port >= -MAXIOSTRS)) {	\
      sfptr = strfileptr(io_port);			\
      fptr = NULL;					\
      charset = UTF_8;					\
    } else {						\
      sfptr = NULL;					\
      SET_FILEPTR(fptr, io_port);			\
      charset = charset(io_port);			\
    }

#define CURRENT_CHARSET (int)flags[CHARACTER_SET]

#ifndef MULTI_THREAD
extern struct xsb_token_t *token;
#endif
#include "context.h"
extern struct xsb_token_t *GetToken(CTXTdeclc int, int);

extern int intype(int);
extern int GetCode(int, FILE *, STRFILE *);
extern int GetCodeP(int);
extern void unGetC(int, FILE *, STRFILE *);
extern byte *codepoint_to_str(int, int, byte *);
extern byte *utf8_codepoint_to_str(int, byte *);
//extern int utf8_strgetc(STRFILE *, int );
extern int strungetc(STRFILE *);
extern int utf8_nchars(byte *);
extern int char_to_codepoint(int, byte **);
extern int utf8_char_to_codepoint(byte **);
extern void write_string_code(FILE *, int, byte *);

#endif /* _TOKEN_XSB_H_ */

/*======================================================================*/
/*======================================================================*/

