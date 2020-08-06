/* File:      psc_xsb.h
** Author(s): Jiyang Xu, Terry Swift, Kostis Sagonas
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
** $Id: psc_xsb.h,v 1.53 2013-05-06 21:10:25 dwarren Exp $
** 
*/

#ifndef __PSC_XSB_H__
#define __PSC_XSB_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYMBOL_TABLE_DEFS

#define SYMBOL_TABLE_DEFS

/* The abstract module for the symbol table (PSC table) */

/*======================================================================*/
/* Type definitions: Psc						*/
/*======================================================================*/

/* PSC records are highly overloaded structures. See psc_defs.h for
   further documentation.

   env_byte: Two lowest-order bits of env byte indicate whether the
   symbol is visible by any module, local to a module, or unloaded.
   Bit 3 indicates whether the predicate is tabled for subsumption,
   bit 4 indicates whether the predicate is tabled for variance.
   (Bits 3 and 4 can both be on, indicating that the predicate is
   tabled, but whether it is variant or subsumptive has not yet been
   determined.)  Bit 5 indicates whether it has been determined that
   the predicate is thread-shared or thread-private.  Bit 6 indicates
   the predicate is shared among threads in the MT engine.  Thus, bit
   5 is meaningful only if bit 5 is also set.  
   Bits 7 and 8 are used for get_spy.

   [] \   symbol status 
   [] / 
   [] \   tabling type
   [] /
   [] \   shared/private
   [] / 
   [] \   spy (two bits used -- only one needed (I think)   
   [] /

   data: If the psc record indicates a predicate data indicates its
   module; otherwise it contains data, as used in conpsc-style
   functions.  (how about foreign?)

   ep/load_inst: If the psc record indicates a (loaded prolog)
   predicate name, then the ep is its entry point; otherwise if a
   module, its ep is the beginning of the chain of psc pairs for
   predicates in the module.  

   If the psc record indicates a loaded foreign function ep points to
   The call_forn instruction, and load_inst is a pointer to the
   function itself.

   If the psc record indicates an unloaded predicate/foreign function,
   the ep points to the load_pred instruction, and this_psc is its
   opcode.  The action of calling this instruction will be to load the
   predicate, set the ep to the entry point of the byte code, and then
   branch to the byte code.

*/

struct psc_rec {
  byte env;			/* 0&0x3 - visible; 1&0x3 - local; 2&0x3 - unloaded;  */
  				/* 0xc0, 2 bits for spy */
				/* 0x20 - shared, 0x10 for determined; 0x8 - tabled */
  //  byte incr;                    /* Only first 2 bits used: 1 incremental; 0 is non-incremental, 2: opaque; 4 for INTERNED */
  unsigned int incremental:2;
  unsigned int intern:1;
  unsigned int immutable:1;
  unsigned int unused:4;
  byte entry_type;		/* see psc_defs.h */
  byte arity; 
  char *nameptr;
  struct psc_rec *data;      /* psc of module, if pred in non-usermod module, otw filename pred loaded from */
  byte *ep;                     /* entry point (initted to next word) if pred; filename of pred loaded from if module */
  word load_inst;               /* byte-code load_pred, or jump, or call_forn */
  struct psc_rec *this_psc;     /* BC arg: entry-point or foreign entry point */
};

typedef struct psc_rec *Psc;

/* --- Pair    -------------------------------------------- */

struct psc_pair {
  Psc psc_ptr;
  struct psc_pair *next;
};

typedef struct psc_pair *Pair;

/* === env definition: (env) ======================================*/

/* Type definitions */
#include "psc_defs.h"
#include "incr_xsb_defs.h"
#include "export.h"


/*======================================================================*/
/* Interface macros (in the following "psc" is typed "Psc")		*/
/*======================================================================*/

#define  get_env(psc)		((psc)->env & T_ENV)
#define  get_type(psc)		((psc)->entry_type)
#define  get_spy(psc)		((psc)->env & T_SPY)
#define  get_tabled(psc)	((psc)->env & T_TABLED)
#define  get_subsumptive_tabled(psc)	((psc)->env & T_TABLED_SUB & ~T_TABLED_VAR)
#define  get_variant_tabled(psc)	((psc)->env & T_TABLED_VAR & ~T_TABLED_SUB)

#define  get_shared(psc)	((psc)->env & T_SHARED)
#define  get_private(psc)	((psc)->env & ~T_SHARED & T_SHARED_DET)

  //#define  get_incr(psc)           (((psc)->incr & T_INCR) == INCREMENTAL)  
#define  get_incr(psc)           ((psc)->incremental == INCREMENTAL)  
#define  get_opaque(psc)         ((psc)->incremental == OPAQUE)  
#define  get_nonincremental(psc) ((psc)->incremental == NONINCREMENTAL) 

  //#define  get_intern(psc)	 ((psc)->incr & T_INTERN)
#define  get_intern(psc)	 ((psc)->intern)
#define  get_immutable(psc)	 ((psc)->immutable)

#define  get_arity(psc)		((psc)->arity)
#define  get_ep(psc)		((psc)->ep)
#define  get_data(psc)		((psc)->data)
#define  get_name(psc)		((psc)->nameptr)
#define  get_mod_for_psc(psc)	(isstring(get_data(psc))?global_mod:get_data(psc))
#define  get_mod_name(psc)	get_name(get_mod_for_psc(psc))

#define  set_type(psc, type)	(psc)->entry_type = type
#define  set_env(psc, envir)	(psc)->env = ((psc)->env & ~T_ENV) | envir
#define  set_spy(psc, spy)	(psc)->env = ((psc)->env & ~T_SPY) | spy
#define  set_shared(psc, shar)	(psc)->env = ((psc)->env & ~T_SHARED) | shar
#define  set_tabled(psc, tab)	(psc)->env = ((psc)->env & ~T_TABLED) | tab

#define  set_incr(psc,val)      ((psc)->incremental = val)  /* incremental */

#define  set_intern(psc,val)    ((psc)->intern = val) /* val 0 or T_INTERN */
#define  set_immutable(psc,val)    ((psc)->immutable = val) 

#define  set_arity(psc, ari)	((psc)->arity = ari)
#define  set_length(psc, len)	((psc)->length = len)
#define  set_ep(psc, val)	do {(psc)->ep = val;     \
    				    cell_opcode(&((psc)->load_inst)) = jump; \
				    (psc)->this_psc = (void *)val;} while(0)
#define  set_data(psc, val)     ((psc)->data = val)
#define  set_name(psc, name)	((psc)->nameptr = name)

#define set_forn(psc, val) {                   \
    cell_opcode(get_ep(psc)) = call_forn;      \
    *(((byte **)get_ep(psc))+1) = (byte *)(val);	\
}

#define  pair_psc(pair)		((pair)->psc_ptr)
#define  pair_next(pair)	((pair)->next)

/*======================================================================*/
/* Interface routines							*/
/*======================================================================*/

extern Pair search_in_usermod(int, char *);
extern Pair insert_module(int, char *);
extern Pair insert(char *, byte, Psc, int *);

DllExport extern char* call_conv string_find(const char*, int);

/*======================================================================*/
/*  Special instance (0-arity interface functions)			*/
/*======================================================================*/

extern Psc global_mod;			/* PSC for "global" */
extern Psc true_psc;
extern Psc if_psc;
extern Psc list_psc;
extern Psc comma_psc;
extern Psc box_psc;
extern Psc tnot_psc;
extern Psc colon_psc;
extern Psc caret_psc;
extern Psc setof_psc;
extern Psc bagof_psc;
extern Psc ccall_mod_psc;
extern Psc c_callloop_psc;
extern Psc delay_psc;
extern Psc cond_psc;
extern Psc cut_psc;
extern Psc load_undef_psc;
extern Psc answer_completion_psc;
extern Psc cyclic_psc;
extern Psc visited_psc;
extern Psc dollar_var_psc;

extern char *nil_string;
extern char *true_string;
extern char *cut_string;
extern Pair list_pscPair;
extern char *list_dot_string;
extern char * cyclic_string;

extern int force_string_gc;

extern Psc ret_psc[];
extern Psc get_ret_psc(int);
inline static char *get_ret_string()	{ return (char *)ret_psc[0]; }

extern Psc get_intern_psc();

/* Can't use CTXTdeclc here because its included early in context.h */
#ifdef MULTI_THREAD
extern Pair link_sym(struct th_context *, Psc, Psc);
extern struct Table_Info_Frame *get_tip(struct th_context *, Psc);
extern void set_psc_ep_to_psc(struct th_context *, Psc, Psc);
#else
extern struct Table_Info_Frame *get_tip(Psc);
extern Pair link_sym(Psc, Psc);
extern void set_psc_ep_to_psc(Psc, Psc);
#endif

extern void print_symbol_table();
extern Psc get_psc_from_ep(void *);

extern void insert_cpred(char * ,int ,int (*)(void) );

/*======================================================================*/
/*  HiLog related macros.						*/
/*======================================================================*/

#define hilog_psc(psc)	\
		(((!strcmp(get_name(psc),"apply")) && (get_arity(psc) > 1)))
#define hilog_cs(term) /* to be used when known that term is a XSB_STRUCT */ \
		((hilog_psc(get_str_psc(term))))
#define hilog_term(term) \
		((cell_tag(term) == XSB_STRUCT) && (hilog_psc(get_str_psc(term))))

/*----------------------------------------------------------------------*/

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PSC_XSB_H__ */
