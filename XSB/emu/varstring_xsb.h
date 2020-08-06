/* File:      varstring.h
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
** $Id: varstring_xsb.h,v 1.14 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#ifndef VARSTRING_INCLUDED
#include "export.h"

struct varstr;
typedef struct varstr VarString;

struct varstr_ops {
  void (*set)(VarString*,char*);       /* copy a char* to VarString; increase
					  or shrink space, if needed        */ 
  void (*setv)(VarString*,VarString*); /* like set; 2nd arg is a VarString* */
  void (*append)(VarString*,char*);    /* append a char* to VarString;
					  increase space, if needed         */
  void (*prepend)(VarString*,char*);   /* like append, but prepend instead  */
  void (*appendv)(VarString*,VarString*);  /* append 2nd VarString to 1st   */
  void (*appendc)(VarString*,char);    /* append char to VarString   */
  void (*prependv)(VarString*,VarString*); /* prepend 2nd VarString to 1st  */
  int  (*compare)(VarString*,VarString*);  /* like strcmp for VarStrings    */
  int  (*strcmp)(VarString*,char*);        /* compare VarString to a char*  */

  /* append block of N chars; don't NULL-terminate */
  void (*appendblk)(VarString*,char*,int); 
  /* append block of N chars; don't NULL-terminate */
  void (*prependblk)(VarString*,char*,int);
  void (*null_terminate)(VarString*);  /* Null-terminate VarString           */
  void (*ensure_size)(VarString*,int); /* Make sure size is at least N       */
  void (*shrink)(VarString*,int);      /* 2nd arg becomes the increment.
					  Space shrinks to the minimum needed
					  to accommodate existing data       */
  void  (*destroy)(VarString*);        /* release the space, uninitialize    */
};



/* All attributes are read-only;
   it is not recommended to refer to private attrs */
struct varstr {
  /* PRIVATE */
  int   size;    	       /* size of the allocated chunk 	      	     */
  int   increment;    	       /* increment by which to incr string size     */
  
  /* PUBLIC */
  int   length;    	       /* memory currently allocated for the string  */
  char  *string;    	       /* memory currently allocated for the string  */

  struct varstr_ops *op;       /* structure that defines valid VarString ops */
};


extern DllExport void call_conv varstring_init(VarString *vstr);
extern DllExport void call_conv varstring_create(VarString **vstr);
extern DllExport struct varstr_ops VarStrOps;

/* calling sequence shortcuts; all expect a VarString pointer */
#define XSB_StrSet(vstr,str)           (vstr)->op->set(vstr,str)
#define XSB_StrSetV(vstr1,vstr2)       (vstr1)->op->setv(vstr1,vstr2)
#define XSB_StrAppend(vstr,str)        (vstr)->op->append(vstr,str)
#define XSB_StrPrepend(vstr,str)       (vstr)->op->prepend(vstr,str)
#define XSB_StrAppendV(vstr1,vstr2)    (vstr1)->op->appendv(vstr1,vstr2)
#define XSB_StrAppendC(vstr,code)      (vstr)->op->appendc(vstr,code)
#define XSB_StrPrependV(vstr1,vstr2)   (vstr)->op->prependv(vstr1,vstr2)
#define XSB_StrCompare(vstr1,vstr2)    (vstr1)->op->compare(vstr1,vstr2)
#define XSB_StrCmp(vstr,str)           (vstr)->op->strcmp(vstr,str)
#define XSB_StrAppendBlk(vstr,blk,sz)  (vstr)->op->appendblk(vstr,blk,sz)
#define XSB_StrPrependBlk(vstr,blk,sz) (vstr)->op->prependblk(vstr,blk,sz)
#define XSB_StrNullTerminate(vstr)     (vstr)->op->null_terminate(vstr)
#define XSB_StrEnsureSize(vstr,size)   (vstr)->op->ensure_size(vstr,size)
#define XSB_StrShrink(vstr,incr)       (vstr)->op->shrink(vstr,incr)
/* destruction is necessary for automatic VarString's */
#define XSB_StrDestroy(vstr)           (vstr)->op->destroy(vstr)


/* XSB_StrDefine doesn't work in a DLL under Windows for some reason.
   Can't resolve VarStrOps. So, then use XSB_StrCreate() and XSB_StrInit()
*/
#define XSB_StrDefine(vstr)          VarString vstr = {0,0,0,NULL,&VarStrOps}
/* Allocates space for VarString, assigns the pointer to vstr, then initializes
   the VarString */
#define XSB_StrCreate(vstr)    	     varstring_create(vstr)
/* Assumes vstr points to an uninitialized VarString. Initializes it. */
#define XSB_StrInit(vstr)    	     varstring_init(vstr)


#define VARSTRING_INCLUDED

#endif
