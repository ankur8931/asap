/* File:      psc_xsb.c
** Author(s): Xu, Sagonas, Swift
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
** $Id: psc_xsb.c,v 1.61 2013-05-06 21:10:25 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <string.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "psc_xsb.h"
#include "tries.h"
#include "hash_xsb.h"
#include "tab_structs.h"
#include "loader_xsb.h"
#include "flags_xsb.h"
#include "sig_xsb.h"
#include "inst_xsb.h"
#include "memory_xsb.h"
#include "register.h"
#include "thread_xsb.h"
#include "cinterf.h"

extern Psc synint_proc(Psc, int);

/* === String Table manipulation ======================================	*/

/*
 * Looks up a string in the String Table.  If it is not found and the
 * insert flag is set, then inserts the string into the table.
 * If the string exists in the table, returns a pointer to the string
 * part of the corresponding table entry.
 *                      
 * String Table entries have the form:
 *           +--------------------------+
 *           | Ptr_to_Next | String ... |
 *           +--------------------------+
 */
/* TLS: use of NOERROR mutexes is ok (12/05) but if we put in error
   checking in mem_xxxoc() functions, we'll need to adjust these
   mutexes. */

extern size_t last_string_space_size;
extern size_t last_assert_space_size;
#define CHAR_PTR_SIZE  sizeof(char *)

DllExport char* call_conv string_find(const char *str, int insert) {

  char **ptr, *str0;
  char **sptr;
  UNUSED(sptr);

  //  printf("interning %s\n",str);
  SYS_MUTEX_LOCK_NOERROR( MUTEX_STRING ) ;
  sptr = ptr = (char **)string_table.table + hash(str, 0, string_table.size);
  while (*ptr) {
    str0 = *ptr + CHAR_PTR_SIZE;
    if (strcmp(str, str0) == 0)
      goto exit_string_find;
    ptr = (char **)(*ptr);
  }
  
  //  if (strcmp(str,"Batch: tuning_batch_2_0")==0) printf("string_find notfound tuningbat_2_0, %p\n",sptr);

  if (insert) {
    str0 = (char *)mem_alloc(CHAR_PTR_SIZE + strlen(str) + 1,STRING_SPACE);
    *ptr = str0;
    *(char **)str0 = NULL;
    str0 = str0 + CHAR_PTR_SIZE;
    strcpy(str0, str);
    string_table_increment_and_check_for_overflow;
    if ((pspacesize[STRING_SPACE] > 4*last_string_space_size) &&
	(pspacesize[ASSERT_SPACE] < 2*last_assert_space_size)) {
      force_string_gc = TRUE;
    }
  }
  else
    str0 = NULL ;

exit_string_find:
  SYS_MUTEX_UNLOCK_NOERROR( MUTEX_STRING ) ;
  //  if (strcmp(str,"Batch: tuning_batch_2_0")==0) printf("string_find tuningbat_2_0, %d, %p, %p, %p\n",insert,str,str0,sptr);
  return str0;
}

char *string_find_safe(char *str) {

  char *ptr, *str0;

  ptr = (char *)((Integer)(*(string_table.table + hash(str, 0, string_table.size))) & ~1);
  while (ptr) {
    str0 = ptr + CHAR_PTR_SIZE;
    if (strcmp(str, str0) == 0)
      return str0;
    ptr = (char *)(((Integer)(*(void **)ptr)) & ~1);
  }
  //  printf("string_find_safe: not found '%s'\n",str);
  return NULL;
}

/* === PSC and PSC-PAIR structure creation/initialization =============== */
void init_psc_ep_info(Psc psc) {
  set_type(psc, 0);
  psc->env = 0;
  set_incr(psc,0);
  set_intern(psc,0);
  set_data(psc, 0);
  set_ep(psc,(byte *)&(psc->load_inst));
  cell_opcode(&(psc->load_inst)) = load_pred;
  psc->this_psc = psc;
}

/*
 *  Create a PSC record and initialize its fields.
 */
static Psc make_psc_rec(char *name, char arity) {
  Psc temp;
  
  temp = (Psc)mem_alloc(sizeof(struct psc_rec),ATOM_SPACE);
  //  set_env(temp, 0);
  //  set_spy(temp, 0);
  //  set_shared(temp, 0);
  //  set_tabled(temp, 0);
  set_name(temp, string_find(name, 1));
  set_arity(temp, arity);
  init_psc_ep_info(temp);
  return temp;
}

void set_psc_ep_to_psc(CTXTdeclc Psc psc_to_set, Psc target_psc) {
  if (get_arity(psc_to_set) != get_arity(target_psc)) {
    xsb_abort("[IMPORT AS] Cannot import predicate as a predicate with a different arity: %s/%d\n",
	     get_name(psc_to_set),get_arity(psc_to_set));
  } else if (get_ep(psc_to_set) != (byte *)&(psc_to_set->load_inst) &&
	     get_ep(psc_to_set) != (byte *)&(target_psc->load_inst)) {
    xsb_warn(CTXTc "[IMPORT AS] Redefining entry to import-as predicate: %s/%d\n",
	    get_name(psc_to_set),get_arity(psc_to_set));
    set_ep(psc_to_set,(byte *)&(target_psc->load_inst));
  } else {
    set_ep(psc_to_set,(byte *)&(target_psc->load_inst));
  }
}


/*
 *  Create a PSC-PAIR record, set it to point to a PSC record, and place
 *  it at the head of a PSC-PAIR record chain.
 */
static Pair make_psc_pair(Psc psc_ptr, Pair *link_ptr) {

  Pair new_pair;
  
  new_pair = (Pair)mem_alloc(sizeof(struct psc_pair),ATOM_SPACE);
  //  printf("new_psc_pair %d, prev %d\n",(int)new_pair, (int)*link_ptr);
  pair_psc(new_pair) = psc_ptr;         /* set 1st to point to psc_rec */
  pair_next(new_pair) = *link_ptr;      /* set 2nd to old head */
  *link_ptr = new_pair;                 /* new symbol is in the head! */
  return new_pair;
}


/* === get_tip: get the TIP from a PSC record =========================	*/
extern CPtr dynpredep_to_prortb(CTXTdeclc void *pred_ep);

TIFptr *get_tip_or_tdisp(Psc temp)
{
    CPtr temp1 ;

    switch (get_type(temp)) {
      case T_DYNA:
      case T_PRED:
	temp1 = (CPtr)get_ep(temp);
	if (temp1 != 0) {
	  switch (*(pb)temp1) {
	    case tabletry:
	    case tabletrysinglenoanswers: /* incremental evaluation */
	    case tabletrysingle:
	      return (TIFptr *) (temp1+2) ;
	    case test_heap:
	      if (*(pb)(temp1+2) == tabletry ||
		  *(pb)(temp1+2) == tabletrysingle  ||
		  *(pb)(temp1+2) == tabletrysinglenoanswers  
		  )		
		return (TIFptr *) (temp1+4) ;
	      else return (TIFptr *)NULL;
	      break;
	    case switchon3bound:
	    case switchonbound:
	    case switchonterm:
	      if (  *(pb) (temp1+3) == tabletry 
	        ||  *(pb) (temp1+3) == tabletrysingle
		    ||  *(pb) (temp1+3) == tabletrysinglenoanswers
		    ) 
		return (TIFptr *) (temp1+5) ;
	      else return (TIFptr *) NULL;
	    default:
	      return (TIFptr *) NULL;
	  }
	}
	else return (TIFptr *) NULL;
      default: 
	return (TIFptr *) NULL;
    }
}

/* get_tip takes a psc record and returns the tip (or null).  If
   multithreaded, it must go through the dispatch table to get the
   tip. 

TLS: Added a few lines below to return NULL if the psc is non-tabled.
Calling routines can then report the appropriate error.  */

TIFptr get_tip(CTXTdeclc Psc psc) {
  TIFptr *tip = get_tip_or_tdisp(psc);
#ifndef MULTI_THREAD
  return tip?(*tip):NULL;
#else
  //  printf("get tip %s/%d tip %p\n",get_name(psc),get_arity(psc),tip);
  if (!tip) { /* get it out of dispatch table */
    CPtr temp1 = (CPtr) get_ep(psc);
    if ((get_type(psc) == T_DYNA) &&
	(*(pb)(temp1) ==  switchonthread)) {
      temp1 = dynpredep_to_prortb(CTXTc temp1);
      if (temp1 && ( (*(pb)temp1 == tabletrysingle) || (*(pb)temp1 == tabletrysinglenoanswers)))
	return *(TIFptr *)(temp1+2);
      else return (TIFptr) NULL;
    } else {
      if (get_tabled(psc)) {
	xsb_error("Internal Error in table dispatch\n");
      } else { return NULL; }
    }
  }
  if (TIF_EvalMethod(*tip) != DISPATCH_BLOCK) return *tip;
  /* *tip points to 3rd word in TDispBlk, so get addr of TDispBlk */
  { struct TDispBlk_t *tdispblk = (struct TDispBlk_t *) (*tip);
    TIFptr rtip = (TIFptr)((&(tdispblk->Thread0))[xsb_thread_entry]);
    if (!rtip) {
      rtip = New_TIF(CTXTc psc);
      (&(tdispblk->Thread0))[xsb_thread_entry] = rtip;
    }
    return rtip; 
  }
#endif
}

/* === is_globalmod: Is a global module ===============================	*/

static int is_globalmod(Psc mod_psc)
{
/* 
 * The modules considered global are the ones that have the value 1 in
 * their data field of the module's psc record.  The modules I
 * know that have this property are the modules "global" and "usermod".
 */
    if (mod_psc)
      return (((Cell)get_data(mod_psc) == USERMOD_PSC));
    /** dsw need a better check here!!?! **/
    else
      return 1;
}


/* === search: search in a given chain ================================	*/

/*
 *  Returns a pointer to the PSC-PAIR structure which points to the
 *  PSC record of the desired symbol.
 */
static Pair search(int arity, char *name, Pair *search_ptr)
{
    Psc psc_ptr;
    /*    Pair *init_search_ptr = search_ptr; */
    /*    Pair found_pair; */

    while (*search_ptr) {
      psc_ptr = (*search_ptr)->psc_ptr;
      if (strcmp(name, get_name(psc_ptr)) == 0
	  && arity == get_arity(psc_ptr) )
	return (*search_ptr);
      else 
	search_ptr  = &((*search_ptr)->next);
    }
    return NULL;
} /* search */

Pair search_in_usermod(int arity, char *name) {
  Pair *search_ptr;
  search_ptr = (Pair *)(symbol_table.table +
			hash(name, arity, symbol_table.size));
  return search(arity,name,search_ptr);
}

/* === insert0: search/insert to a given chain ========================	*/

static Pair insert0(char *name, byte arity, Pair *search_ptr, int *is_new)
{
    Pair pair;
    
    pair = search(arity, name, search_ptr);
    if (pair==NULL) {
      *is_new = 1;
      pair = make_psc_pair(make_psc_rec(name,arity), search_ptr);
    }
    else
      *is_new = 0;
    return pair;
} /* insert0 */


/* === insert: search/insert to a given module ========================	*/

Pair insert(char *name, byte arity, Psc mod_psc, int *is_new)
{
    Pair *search_ptr, temp;

    SYS_MUTEX_LOCK_NOERROR( MUTEX_SYMBOL ) ;

    if (is_globalmod(mod_psc)) {
      search_ptr = (Pair *)(symbol_table.table +
	           hash(name, arity, symbol_table.size));
      temp = insert0(name, arity, search_ptr, is_new);
      if (*is_new)
	symbol_table_increment_and_check_for_overflow;
    }
    else {
      search_ptr = (Pair *)&(get_data(mod_psc));
      temp = insert0(name, arity, search_ptr, is_new);
    }
    SYS_MUTEX_UNLOCK_NOERROR( MUTEX_SYMBOL ) ;
    return temp ;
} /* insert */


/* === insert_module: search for/insert a given module ================	*/

Pair insert_module(int type, char *name)
{
    Pair new_pair;
    int is_new;

    SYS_MUTEX_LOCK_NOERROR( MUTEX_SYMBOL ) ;
    new_pair = insert0(name, 0, (Pair *)&flags[MOD_LIST], &is_new);
    if (is_new) {
	set_type(new_pair->psc_ptr, type);
	new_pair->psc_ptr->env = 0;
	//	new_pair->psc_ptr->incr = 0;
	set_incr(new_pair->psc_ptr,0);
	set_intern(new_pair->psc_ptr,0);
	set_data(new_pair->psc_ptr,0);
	set_ep(new_pair->psc_ptr,0);
	new_pair->psc_ptr->this_psc = 0;
	set_immutable(new_pair->psc_ptr,0);
    } else {	/* set loading bit: T_MODU - loaded; 0 - unloaded */
      set_type(new_pair->psc_ptr, get_type(new_pair->psc_ptr) | type);
    }
    SYS_MUTEX_UNLOCK_NOERROR( MUTEX_SYMBOL ) ;
    return new_pair;
} /* insert_module */


/* === link_sym: link a symbol into a given module ==================== */

/*
 *  Given a PSC record 'psc' for a particular symbol, check to see if
 *  that symbol already exists in the module 'mod_psc'.
 *  Does NOT exist => insert it and return a ptr to its PSC-PAIR record.
 *  DOES exist => check if the found PSC record is the same as 'psc'.
 *    YES => return a ptr to its PSC-PAIR record.
 *     NO => replace the old PSC record with 'psc'; return a ptr to the
 *           PSC-PAIR record.
 */

Pair link_sym(CTXTdeclc Psc psc, Psc mod_psc)
{
    Pair *search_ptr, found_pair;
    char *name;
    byte arity, global_flag, type;

    SYS_MUTEX_LOCK_NOERROR( MUTEX_SYMBOL ) ;
    name = get_name(psc);
    arity = get_arity(psc);
    if ( (global_flag = is_globalmod(mod_psc)) ) {
      search_ptr = (Pair *)symbol_table.table +
	           hash(name, arity, symbol_table.size);
    } else
      search_ptr = (Pair *)&get_data(mod_psc);
    if ((found_pair = search(arity, name, search_ptr))) {
      if (pair_psc(found_pair) != psc) {
	/*
	 *  Invalidate the old name!! It is no longer accessible
	 *  through the global chain.
	 */
	type = get_type(pair_psc(found_pair));
	if ( type != T_ORDI ) {
	  char message[220], modmsg[200];
	  if (type == T_DYNA || type == T_PRED) {
	    Psc mod_psc;
	    mod_psc = (Psc) get_data(pair_psc(found_pair));
	    if (mod_psc == 0) snprintf(modmsg,200,"%s","usermod");
	    else if (isstring(mod_psc)) snprintf(modmsg,200,"usermod from file: %s",string_val(mod_psc));
	    else snprintf(modmsg,200,"module: %s",get_name(mod_psc));
	    snprintf(message,220,
		    "%s/%d (type %d) had been defined in %s",
		     name, arity, type, 
		     modmsg);
	  } else 
	    snprintf(message,220,
		    "%s/%d (type %d) had been defined in another module!",
		    name, arity, type);
	  xsb_warn(CTXTc message);
	}
	pair_psc(found_pair) = psc;
      }
    }
    else {
      found_pair = make_psc_pair(psc, search_ptr);
      if (global_flag)
	symbol_table_increment_and_check_for_overflow;
    }
    SYS_MUTEX_UNLOCK_NOERROR( MUTEX_SYMBOL ) ;
    return found_pair;
} /* link_sym */


/*
 * Get the PSC for ret/n.  If it already exists, just return it.  Or
 * create one and save it in ret_psc[n].
 */
Psc get_ret_psc(int n)
{
  Pair temp;
  int new_indicator;

  if (!ret_psc[n]) {
    temp = (Pair) insert("ret", (byte) n, global_mod, &new_indicator);
    ret_psc[n] = pair_psc(temp);
  }
  return ret_psc[n];
}


/*
 * Get the PSC for intern/1, a generic functor for storing in the roots
 * of interned tries.
 */
Psc get_intern_psc() {

  Pair intern_handle;
  int new_indicator;

  intern_handle = insert("intern", 1, global_mod, &new_indicator);
  return (pair_psc(intern_handle));
}


/* Used for PRISM port */
void insert_cpred(char * name,int arity,int (*pfunc)(void) ) {
    int dummy_flag;
    Psc psc;

    psc = insert(name,arity, global_mod, &dummy_flag)->psc_ptr;
    set_forn(psc,pfunc);
    set_type(psc,T_FORN);

}
