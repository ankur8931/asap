/* File:      tr_code_xsb_i.h
** Author(s): Prasad Rao, Kostis Sagonas, Baoqiu Cui
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
** $Id: tr_code_xsb_i.h,v 1.38 2013-05-06 21:10:25 dwarren Exp $
** 
*/


#define opatom Atom(NodePtr)
#define opsucc ((byte *)(Child(NodePtr)))
#define opfail ((byte *)(Sibl(NodePtr)))

#define FIRST_HASH_NODE  -1
#define NO_MORE_IN_HASH  -2
#define HASH_IS_FREE	 -3
#define HASH_IS_NOT_FREE -4

/*----------------------------------------------------------------------*/

/* These are used only in instruction "hash_handle"
   ------------------------------------------------ */
/*
 *  Calculate the bucket number in which Subterm would be located,
 *  should it exist in the trie.
 */
#define hash_nonvar_subterm(Subterm, pBTHT, BucketNum) {	\
								\
   Cell symbol = 0;	/* eliminate compiler warning */	\
								\
   switch (cell_tag(Subterm)) {					\
   case XSB_STRING:						\
   case XSB_INT:						\
   case XSB_FLOAT:	      	  				\
     symbol = EncodeTrieConstant(Subterm);			\
     break;							\
   case XSB_LIST:						\
     symbol = EncodeTrieList(Subterm);				\
     break;							\
   case XSB_STRUCT:						\
     symbol = EncodeTrieFunctor(Subterm);			\
     break;							\
   default:							\
     fprintf(stderr,"Bad tag :Type %lu ",(long unsigned)cell_tag(Subterm)); \
     xsb_exit( "In instruction hash_handle");			\
     break;							\
   }								\
   BucketNum = TrieHash(symbol,BTHT_GetHashSeed(pBTHT));	\
 }

#define find_next_nonempty_bucket(pBTHT, pTable, BucketNum) {	\
   Integer TableSize = BTHT_NumBuckets(pBTHT);			\
								\
   while (TRUE) {						\
     BucketNum++;						\
     if (BucketNum >= TableSize) {				\
       BucketNum = NO_MORE_IN_HASH;				\
       break;							\
     }								\
     else if ( IsNonNULL(*(pTable + BucketNum)) )		\
       break;							\
   }								\
 }

/*----------------------------------------------------------------------*/

/*
 * Decide how to proceed from current node.  Used in variable-containing
 * nodes since it is unclear from the context (embedded instruction)
 * whether we are at a leaf node.  Only variables or constants can be
 * leaves of the trie, but constants have special instructions when they
 * appear as leaves.
 */
#define next_lpcreg {				\
   if ( IsLeafNode(NodePtr) )			\
     proceed_lpcreg				\
   else						\
     non_ftag_lpcreg;				\
 }

/*
 * Use when current node is known to be a leaf of the trie.  If we're in
 * an answer trie, then check for and handle conditional answers.
 */
#define proceed_lpcreg {			\
   if( IsInAnswerTrie(NodePtr) && delay_it )	\
     handle_conditional_answers;		\
   global_trieinstr_vars_num = trieinstr_vars_num;	\
   trieinstr_vars_num = -1;			\
   Last_Nod_Sav = NodePtr;			\
   lpcreg = cpreg;				\
   TRIE_R_UNLOCK();				\
 }

/*
 * Use when the current node is known NOT to be a leaf of the trie.
 * Continue by going to the child of the current node.
 */
#define non_ftag_lpcreg		lpcreg = opsucc

/*----------------------------------------------------------------------*/

#define restore_regs_and_vars(tbreg,offset)	\
    undo_bindings(tbreg);			\
    delayreg = cp_pdreg(tbreg);                 \
    restore_some_wamregs(tbreg, ereg);	        \
    restore_trie_registers(tbreg + offset) 

/*----------------------------------------------------------------------*/
/* Garbage collection strongly prefers tagged integers in CP stack...   */
/*       PLEASE PRESERVE THIS IVNARIANT --- Kostis & Bart !             */
/*----------------------------------------------------------------------*/

#define save_trie_registers(tbreg) {				\
  CPtr temp_arrayptr;						\
  int reg_count = 0, i;						\
								\
  i = trieinstr_vars_num;					\
  while (i >= 0) {						\
    *(--tbreg) = (Cell)trieinstr_vars[i];			\
    i--;								\
  }									\
  *(--tbreg) = makeint(trieinstr_vars_num);				\
  temp_arrayptr = trieinstr_unif_stkptr;				\
  while (temp_arrayptr >= trieinstr_unif_stk) {				\
    /* INV: temp_array_ptr + reg_count == trieinstr_unif_stkptr */	\
    *(--tbreg) = *temp_arrayptr;					\
    reg_count++;							\
    temp_arrayptr--;							\
  }									\
  (*--tbreg) = makeint(reg_count);					\
  }

/* The above macro is separated into 2 for use in the trie_root instruction */
#define save_trieinstr_vars(tbreg) {				\
    int  i;							\
								\
  i = trieinstr_vars_num;					\
  while (i >= 0) {						\
    *(--tbreg) = (Cell)trieinstr_vars[i];				\
    i--;								\
  }								\
  *(--tbreg) = makeint(trieinstr_vars_num);			\
}


#define save_trieinstr_unif_stk(tbreg) {			\
  CPtr temp_arrayptr;						\
  int reg_count = 0;						\
								\
  temp_arrayptr = trieinstr_unif_stkptr;				\
  while (temp_arrayptr >= trieinstr_unif_stk) {				\
 /* INV: temp_array_ptr + reg_count == trieinstr_unif_stkptr */	\
    *(--tbreg) = *temp_arrayptr;				\
    reg_count++;						\
    temp_arrayptr--;						\
  }								\
  (*--tbreg) = makeint(reg_count);				\
  }


#define restore_trie_registers(temp) {			\
    Integer i;						\
    CPtr treg = temp;					\
    i = cell(treg);					\
    i = int_val(i);					\
    while (i > 0) {					\
      trieinstr_unif_stkptr++;					\
      *trieinstr_unif_stkptr = *(++treg);			\
      i--;						\
    }							\
    i = *(++treg);					\
    trieinstr_vars_num = (int)int_val(i);		\
    for (i = 0; i <= trieinstr_vars_num; i++) {	\
      trieinstr_vars[i] = (CPtr)*(++treg);		\
    }							\
}

/*----------------------------------------------------------------------*/

#define unify_with_trie_numcon {					\
  XSB_Deref(*trieinstr_unif_stkptr);					       	\
  if (isref(*trieinstr_unif_stkptr)) {						\
    bind_ref((CPtr)*trieinstr_unif_stkptr, opatom);				 \
  }									\
  else if (isattv(*trieinstr_unif_stkptr)) {					\
    attv_dbgmsg(">>>> add_interrupt in unify_with_trie_numcon\n");	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(*trieinstr_unif_stkptr) + 1)), opatom);\
    bind_int_tagged((CPtr)dec_addr(*trieinstr_unif_stkptr), opatom);          		\
  }									\
  else {								\
    /*    if (isinternstr(opatom)) printf("lookup internstr %p\n",opatom);*/ \
   if (*trieinstr_unif_stkptr != opatom) {			       	\
      Fail1;								\
      XSB_Next_Instr();							\
    }									\
  }									\
}

#define unify_with_trie_str {					\
  Psc psc;							\
  int i, arity;							\
								\
  XSB_Deref(*trieinstr_unif_stkptr);			       		\
  psc = (Psc) cs_val(opatom);					\
  arity = (int) get_arity(psc);					\
  will_overflow_trieinstr_unif_stk(trieinstr_unif_stkptr + arity);		\
  if (isref(*trieinstr_unif_stkptr)) {					\
    bind_ref((CPtr) *trieinstr_unif_stkptr, makecs(hreg));		\
    trieinstr_unif_stkptr--;						\
    *(hreg++) = (Cell) psc;					\
    for (i = arity; i >= 1; i--) {				\
      *(trieinstr_unif_stkptr + i) = (Cell) hreg;			\
      new_heap_free(hreg);					\
    }								\
    trieinstr_unif_stkptr += arity;					\
    check_glstack_overflow(0,pcreg,0);				\
  }								\
  else if (isattv(*trieinstr_unif_stkptr)) {				\
    attv_dbgmsg(">>>> add_interrupt in unify_with_trie_str\n");	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(*trieinstr_unif_stkptr) + 1)), makecs(hreg+INT_REC_SIZE));	\
    bind_copy((CPtr)dec_addr(*trieinstr_unif_stkptr), makecs(hreg));        \
    trieinstr_unif_stkptr--;						\
    *(hreg++) = (Cell) psc;					\
    for (i = arity; i >= 1; i--) {				\
      *(trieinstr_unif_stkptr + i) = (Cell) hreg;			\
      new_heap_free(hreg);					\
    }								\
    trieinstr_unif_stkptr += arity;					\
    check_glstack_overflow(0,pcreg,0);				\
  }   								\
  else {							\
    CPtr temp = (CPtr)*trieinstr_unif_stkptr;				\
    if ((isconstr(temp)) && (psc == get_str_psc(temp))) {	\
      trieinstr_unif_stkptr--;						\
      temp = (CPtr)cs_val(temp);				\
      for (i = arity; i >= 1; i--) {				\
	*(trieinstr_unif_stkptr+i) = *(temp+arity-i+1);			\
      }								\
      trieinstr_unif_stkptr += arity;					\
    }								\
    else {							\
      Fail1;							\
      XSB_Next_Instr();						\
    }								\
  }								\
}

#define unify_with_trie_list {						\
  XSB_Deref(*trieinstr_unif_stkptr);    						\
  if (isref(*trieinstr_unif_stkptr)) {						\
    bind_ref((CPtr) *trieinstr_unif_stkptr, (Cell) makelist(hreg));		\
    *trieinstr_unif_stkptr = (Cell)(hreg+1);         /* head of list */		\
    will_overflow_trieinstr_unif_stk(trieinstr_unif_stkptr + 1);				\
    *(++trieinstr_unif_stkptr) = (Cell) hreg;        /* tail of list */		\
    new_heap_free(hreg);						\
    new_heap_free(hreg);						\
    check_glstack_overflow(0,pcreg,0);					\
  }									\
  else if (isattv(*trieinstr_unif_stkptr)) {					\
    attv_dbgmsg(">>>> add_interrupt in unify_with_trie_list\n");	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(*trieinstr_unif_stkptr) + 1)), makelist(hreg+INT_REC_SIZE));	\
    bind_copy((CPtr)dec_addr(*trieinstr_unif_stkptr), makelist(hreg));       \
    *trieinstr_unif_stkptr = (Cell)(hreg+1);         /* tail of list */		\
    will_overflow_trieinstr_unif_stk(trieinstr_unif_stkptr + 1);				\
    *(++trieinstr_unif_stkptr) = (Cell) hreg;        /* head of list */		\
    new_heap_free(hreg);						\
    new_heap_free(hreg);						\
    check_glstack_overflow(0,pcreg,0);					\
  }									\
  else {								\
    CPtr temp = (CPtr)*trieinstr_unif_stkptr;					\
    if (islist(temp)) {							\
      will_overflow_trieinstr_unif_stk(trieinstr_unif_stkptr + 1);			\
      *trieinstr_unif_stkptr++ = (Cell)(clref_val(temp)+1);			\
      *trieinstr_unif_stkptr = (Cell)(clref_val(temp));				\
    } else {								\
      Fail1;								\
      XSB_Next_Instr();							\
    }									\
  }									\
}

/*
 * In clp(Q,R), most (or all) of the attvs in the call are updated in the
 * answer.  So we have a set of *new* attvs in the answer trie.  This set
 * of new attvs will be copied into the answer trie when the *first* attv
 * in the call is copied into the answer trie (since most/all of the other
 * attvs are related to the first one).  The later occurrences of the
 * *other* attvs are encoded as `unify_with_trie_val', but we don't want
 * to trigger attv interrupts when we update the attvs in the call.
 */

#define unify_with_trie_val {						\
  Cell cell2deref;							\
  XSB_Deref(*trieinstr_unif_stkptr);    						\
  if (isref(*trieinstr_unif_stkptr)) {						\
    cell2deref = (Cell)trieinstr_vars[(int)int_val(opatom)];			\
    XSB_Deref(cell2deref);	       					\
    if (cell2deref != *trieinstr_unif_stkptr)					\
      bind_ref((CPtr) *trieinstr_unif_stkptr, cell2deref);			\
  }									\
  else if (isattv(*trieinstr_unif_stkptr)) {					\
    cell2deref = (Cell) trieinstr_vars[(int)int_val(opatom)];			\
    XSB_Deref(cell2deref);     						\
    if (*trieinstr_unif_stkptr != cell2deref) {				\
      /* Need to trigger attv interrupt */				\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(*trieinstr_unif_stkptr) + 1)), \
		  cell2deref);				\
    bind_ref((CPtr)dec_addr(*trieinstr_unif_stkptr), cell2deref);	\
    }									\
    else {								\
      attv_dbgmsg(">>>> keep old attr in unify_with_trie_val\n");	\
    }									\
  }									\
  else {								\
    op1 = (Cell)*trieinstr_unif_stkptr;						\
    op2 = (Cell) trieinstr_vars[(int)int_val(opatom)];			\
    if (unify(CTXTc op1,op2) == FALSE) {				\
      Fail1;								\
      XSB_Next_Instr();							\
    }									\
  }									\
  trieinstr_unif_stkptr--;							\
}

#define unify_with_variant_trie_val {					\
  Cell cell2deref;							\
  XSB_Deref(*trieinstr_unif_stkptr);					\
  if (isref(*trieinstr_unif_stkptr)) {					\
    cell2deref = (Cell)trieinstr_vars[(int)int_val(opatom)];		\
    XSB_Deref(cell2deref);	       					\
    if (cell2deref != *trieinstr_unif_stkptr)				\
      bind_ref((CPtr) *trieinstr_unif_stkptr, cell2deref);		\
  }									\
  else if (isattv(*trieinstr_unif_stkptr)) {				\
    cell2deref = (Cell) trieinstr_vars[(int)int_val(opatom)];		\
    XSB_Deref(cell2deref);     						\
    if (*trieinstr_unif_stkptr != cell2deref) {				\
      /* Specialization: do not need to trigger attv interrupt */	\
      bind_ref(clref_val(*trieinstr_unif_stkptr), cell2deref);		\
    }									\
    else {								\
      attv_dbgmsg(">>>> keep old attr in unify_with_trie_val\n");	\
    }									\
  }									\
  else {								\
    op1 = (Cell)*trieinstr_unif_stkptr;					\
    op2 = (Cell) trieinstr_vars[(int)int_val(opatom)];			\
    if (unify(CTXTc op1,op2) == FALSE) {		\
      Fail1;								\
      XSB_Next_Instr();							\
    }									\
  }									\
  trieinstr_unif_stkptr--;						\
}

/* TLS: 
   trieinstr_unif_stkptr is ptr to the call that we are unifying the answer with
*/
#define unify_with_trie_attv {						\
  XSB_Deref(*trieinstr_unif_stkptr);			       			\
  trieinstr_vars_num = (int)int_val(opatom) &0xffff;			\
  if (isref(*trieinstr_unif_stkptr)) {						\
    bind_ref((CPtr) *trieinstr_unif_stkptr, makeattv(hreg));			\
  }									\
  else if (isattv(*trieinstr_unif_stkptr)) {					\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(*trieinstr_unif_stkptr) + 1)),makeattv(hreg+INT_REC_SIZE));   \
    bind_ref((CPtr)dec_addr(*trieinstr_unif_stkptr), makeattv(hreg));	\
  }									\
  else {								\
    add_interrupt(CTXTc makeattv(hreg+INT_REC_SIZE+1), *trieinstr_unif_stkptr);	\
    bind_copy((hreg+INT_REC_SIZE),*trieinstr_unif_stkptr);			\
  }									\
  trieinstr_vars[trieinstr_vars_num] = (CPtr) *trieinstr_unif_stkptr;		\
  new_heap_free(hreg);							\
  *trieinstr_unif_stkptr = (Cell) hreg;						\
  new_heap_free(hreg);							\
  check_glstack_overflow(0,pcreg,0);					\
}

#ifdef UNDEFINED
#define unify_with_trie_attv {						\
  XSB_Deref(*trieinstr_unif_stkptr);			       			\
  trieinstr_vars_num = (int)int_val(opatom) &0xffff;			\
  if (isref(*trieinstr_unif_stkptr)) {						\
    bind_ref((CPtr) *trieinstr_unif_stkptr, makeattv(hreg));			\
  }									\
  else if (isattv(*trieinstr_unif_stkptr)) {					\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(*trieinstr_unif_stkptr) + 1)),makeattv(hreg+INT_REC_SIZE));   \
    bind_ref((CPtr)dec_addr(*trieinstr_unif_stkptr), makeattv(hreg));	\
  }									\
  else {								\
    printf("%p >>>> add_interrupt in unify_with_trie_attv\n",*trieinstr_unif_stkptr);		\
    makeattv((hreg+INT_REC_SIZE));					\
    add_interrupt(CTXTc makeattv(hreg+INT_REC_SIZE+1), *trieinstr_unif_stkptr);	\
    bind_copy((hreg+INT_REC_SIZE),*trieinstr_unif_stkptr);			\
  }									\
  trieinstr_vars[trieinstr_vars_num] = *trieinstr_unif_stkptr;			\
  new_heap_free(hreg);							\
  *trieinstr_unif_stkptr = (Cell) hreg;						\
  new_heap_free(hreg);							\
  check_glstack_overflow(0,pcreg,0);					\
}
#endif

/*----------------------------------------------------------------------*/
