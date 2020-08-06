/* File:      loader_xsb.h
** Author(s): David S. Warren, Jiyang Xu
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
** $Id: loader_xsb.h,v 1.14 2012-10-12 16:42:57 tswift Exp $
** 
*/

#include "context.h"
#include "basictypes.h"

//#define FOREIGN_NAMELEN   128
#define FOREIGN_NAMELEN   MAXFILENAME

#define SHORT_LDOPTIONLEN 254

/* Data structures holding the compiled code */

struct index_block {
  struct index_block *next ;
  size_t size ;
} ;
 
typedef struct index_block index_hdr, * pindex ;
 
#define SIZE_IDX_HDR (sizeof(index_hdr))
 
#define i_next(i)       ((i)->next)
#define i_size(i)       ((i)->size)
#define i_block(i)      ((CPtr)((i)+1))
 
struct text_segment {
  struct text_segment * next ;
  struct text_segment * prev ;
  pindex index ;
  size_t size ;
} ;
 
typedef struct text_segment text_seg, seg_hdr, *pseg ;
 
#define SIZE_SEG_HDR    (sizeof(seg_hdr))
 
#define seg_hdr(s)      ((s)-1)
#define seg_next(s)     (((s)-1)->next)
#define seg_prev(s)     (((s)-1)->prev)
#define seg_index(s)    (((s)-1)->index)
#define seg_size(s)     (((s)-1)->size)
#define seg_text(s)     ((CPtr)(s))
 
#define OBJ_WORD_SIZE           4
#define WORD_SIZE               ( sizeof(Cell) )
/* Zoom the object file to fit actual word size */
#define ZOOM_FACTOR             (WORD_SIZE / OBJ_WORD_SIZE)

/************************************************************************/
/*									*/
/* fix_bb: fixes the byte-backwards problem.  It is passed a pointer to	*/
/* a sequence of 4 bytes read in from a file as bytes. It then converts	*/
/* those bytes to represent a number.  This code works for any machine,	*/
/* and makes the byte-code machine independent.				*/
/*									*/
/************************************************************************/

#define fix_bb(lptr) (cell((CPtr)(lptr)) = \
		    (((((Cell)(*(pb)(lptr)) << 8  | (Cell)(*((pb)(lptr)+1)))\
           << 8) | (Cell)(*((pb)(lptr)+2))) << 8) | (Cell)(*((pb)(lptr)+3)) \
		     )

/* experimental */
#define fix_bb4(lptr) (*(unsigned int *)(lptr) = \
		       (((((unsigned int)(*(pb)(lptr)) \
			<< 8  | (unsigned int)(*((pb)(lptr)+1)))\
			<< 8) | (unsigned int)(*((pb)(lptr)+2)))\
			<< 8) | (unsigned int)(*((pb)(lptr)+3)) \
		       )

/* === functions used in other parts of the system ====================== */

extern byte *loader(CTXTdeclc char *, int);
extern void env_type_set(CTXTdeclc Psc, byte, byte, xsbBool);
extern void unload_seg(pseg);
extern unsigned int read_magic(FILE *);
