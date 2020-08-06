/* File:      dynamic_stack.h
** Author(s): Ernie Johnson
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
** $Id: dynamic_stack.h,v 1.9 2011-05-18 19:21:40 dwarren Exp $
** 
*/


#ifndef DYNAMIC_STACK

#define DYNAMIC_STACK



/*-------------------------------------------------------------------------*/

/* Dynamic Stack Structure
   ----------------------- */
typedef struct {
  struct {
    void *top;		   /* next available frame */
    void *base;		   /* stack bottom */
    void *ceiling;	   /* off-end pointer for determining fullness */
  } stack;
  struct {
    size_t frame;	   /* size of a frame */
    size_t stackinit;	   /* initial size of stack in number of frames */
    size_t stackcur;	   /* current size of stack in number of frames */
  } size;
  char *name;
} DynamicStack;

#define DynStk_Top(DS)		( (DS).stack.top )
#define DynStk_Base(DS)		( (DS).stack.base )
#define DynStk_Ceiling(DS)	( (DS).stack.ceiling )
#define DynStk_FrameSize(DS)	( (DS).size.frame )
#define DynStk_InitSize(DS)	( (DS).size.stackinit )
#define DynStk_CurSize(DS)	( (DS).size.stackcur )
#define DynStk_Name(DS)		( (DS).name )

#define DynStk_NumFrames(DS)	\
   (((char *)DynStk_Top(DS) - (char *)DynStk_Base(DS)) / DynStk_FrameSize(DS))


/* Top-of-Stack Manipulations
   -------------------------- */
#define DynStk_NextFrame(DS)	\
   (void *)((char *)DynStk_Top(DS) + DynStk_FrameSize(DS))
#define DynStk_PrevFrame(DS)	\
   (void *)((char *)DynStk_Top(DS) - DynStk_FrameSize(DS))


/* Stack Maintenance
   ----------------- */
extern void dsPrint(DynamicStack, char *);
extern void dsInit(DynamicStack *, size_t, size_t, char *);
extern void dsExpand(DynamicStack *, size_t);
extern void dsShrink(DynamicStack *);

#define DynStk_Init(DS,NumElements,FrameType,Desc)	\
   dsInit(DS,NumElements,sizeof(FrameType),Desc)

#define DynStk_ResetTOS(DS)	DynStk_Top(DS) = DynStk_Base(DS)
#define DynStk_IsEmpty(DS)	( DynStk_Top(DS) == DynStk_Base(DS) )
#define DynStk_IsFull(DS)	( DynStk_Top(DS) >= DynStk_Ceiling(DS) )

#define DynStk_WillOverflow(DS,NFrames)				\
   ( (char *)DynStk_Top(DS) + NFrames * DynStk_FrameSize(DS)	\
     > (char *)DynStk_Ceiling(DS) )


#define DynStk_ExpandIfFull(DS) {		\
   if ( DynStk_IsFull(DS) )			\
     dsExpand(&(DS),1);				\
 }

#define DynStk_ExpandIfOverflow(DS,N) {		\
   if ( DynStk_WillOverflow(DS,N) )		\
     dsExpand(&(DS),N);				\
 }


/*
 * In the following macros, "Frame" is assigned a pointer to either
 * the next or the current frame on the stack.
 */

/* Operations With Bounds Checking
   ------------------------------- */
#define DynStk_Push(DS,Frame) {			\
   DynStk_ExpandIfFull(DS);			\
   DynStk_BlindPush(DS,Frame);			\
 }

#define DynStk_Pop(DS,Frame) {			\
   if ( ! DynStk_IsEmpty(DS) )			\
     DynStk_BlindPop(DS,Frame)			\
   else						\
     Frame = NULL;				\
 }

#define DynStk_Peek(DS,Frame) {			\
   if ( ! DynStk_IsEmpty(DS) )			\
     DynStk_BlindPeek(DS,Frame);		\
   else						\
     Frame = NULL;				\
 }


/* Operations Without Bounds Checking
   ---------------------------------- */
#define DynStk_BlindPush(DS,Frame) {		\
   Frame = DynStk_Top(DS);			\
   DynStk_Top(DS) = DynStk_NextFrame(DS);	\
 }

#define DynStk_BlindPop(DS,Frame) {		\
   DynStk_Top(DS) = DynStk_PrevFrame(DS);	\
   Frame = DynStk_Top(DS);			\
 }

#define DynStk_BlindPeek(DS,Frame)		\
   Frame = DynStk_PrevFrame(DS)

/*-------------------------------------------------------------------------*/


#endif
