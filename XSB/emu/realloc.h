/************************************************************************/
/*									*/
/* XSB System								*/
/* Copyright (C) SUNY at Stony Brook, 1995				*/
/*									*/
/* Everyone is granted permission to copy, modify and redistribute XSB, */
/* but only under the conditions described in the XSB Licence Agreement.*/
/* A copy of this licence is supposed to have been given to you along	*/
/* with XSB so you can know your rights and responsibilities.		*/
/* It should be in a file named LICENCE.				*/
/* Among other things, this notice must be preserved on all copies.	*/
/*									*/
/************************************************************************/

/*======================================================================
  File			:  realloc.h
  Author(s)		:  Bart Demoen
  Last modification	:  13 October, 1998

  For changes look at   :  13-10-1998
========================================================================*/

/*----------------------------------------------------------------------*/
/*                                                                      */
/* certain macros that once appeared in heap.c are moved to this file   */
/* since reallocation is no longer centralised                          */
/* reallocation is done in heap.c and cat.c concerning heap and ls      */
/* similar for printing stuff                                           */
/*----------------------------------------------------------------------*/

#define realloc_ref(cell_ptr, cell_val)                         \
{       if (heap_bot <= cell_val)                               \
  {   if (cell_val <= heap_top)  /* <= because of heaptop in CP 13-10-1998 */     \
                *cell_ptr = cell_val + heap_offset ;            \
            else if (cell_val <= ls_bot)                        \
                 *cell_ptr = cell_val + local_offset ;          \
}       }

#define realloc_ref_pre_image(cell_ptr, cell_val)                     \
{       if (heap_bot <= cell_val)                                     \
      /* <= because of heaptop in CP 13-10-1998 */                    \
  {   if (cell_val <= heap_top)                                       \
                *cell_ptr = (CPtr) ((Cell)(cell_val + heap_offset)   \
                            | PRE_IMAGE_MARK) ;                       \
            else if (cell_val <= ls_bot)                              \
                 *cell_ptr = (CPtr) ((Cell) (cell_val + local_offset) \
                            | PRE_IMAGE_MARK) ;                       \
} }

#define reallocate_heap_or_ls_pointer(cell_ptr) 		\
    cell_val = (Cell)*cell_ptr ; 				\
    switch (cell_tag(cell_val)) { 				\
    case XSB_REF:    	     	     	     	     	\
    case XSB_REF1 : 					\
      realloc_ref(cell_ptr,(CPtr)cell_val);            	\
      break ; /* end case XSB_FREE or XSB_REF */ 			\
    case XSB_STRUCT : 						\
      if (heap_bot<=(clref_val(cell_val)) && (clref_val(cell_val))<heap_top)\
	  *cell_ptr = (CPtr)makecs((Cell)(clref_val(cell_val)+heap_offset)) ; \
      break ; 						\
    case XSB_LIST : 						\
      if (heap_bot<=(clref_val(cell_val)) && (clref_val(cell_val))<heap_top)\
          *cell_ptr = (CPtr)makelist((Cell)(clref_val(cell_val)+heap_offset));\
      break ; 						\
    case XSB_ATTV:							\
      if (heap_bot<=(clref_val(cell_val)) && (clref_val(cell_val))<heap_top)\
          *cell_ptr = (CPtr)makeattv((Cell)(clref_val(cell_val)+heap_offset));\
      break ; 						\
    default : /* no need to reallocate */ 			\
      break ; 						\
    }


#define FROM_NOWHERE 0
#define FROM_LS 1
#define FROM_CP 2
#define FROM_TR 3
#define FROM_AREG 4
#define FROM_HEAP 5
#define FROM_COMPL 6

#define TO_NOWHERE 0
#define TO_LS 1
#define TO_CP 2
#define TO_TR 3
#define TO_AREG 4
#define TO_HEAP 5
#define TO_COMPL 6


/*------------------------- end of file realloc.h --------------------------*/
