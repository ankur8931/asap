/*      std_pred_xsb_i.h
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
** $Id: std_pred_xsb_i.h,v 1.89 2013-05-06 21:10:25 dwarren Exp $
** 
*/


/*----------------------------------------*/
#include "xsb_config.h"
#include "builtin.h"
#include "sp_unify_xsb_i.h"
#include "cut_xsb.h"
/*----------------------------------------*/

static xsbBool atom_to_list(CTXTdeclc int call_type);
static xsbBool number_to_list(CTXTdeclc int call_type);


/* TLS 10/01 changed functor so that it did not core dump on 
   functor(X,1,2) */
inline static xsbBool functor_builtin(CTXTdecl)
{
  /* r1: ?term; r2: ?functor; r3: ?arity (int)	*/
  int  new_indicator, disp;
  Integer value;
  Psc psc;
  char *name;
  Cell functor, term, arity;
  Pair sym;

  term = ptoc_tag(CTXTc 1);
  if (isnonvar(term)) {
    if (isconstr(term) && !isboxedfloat(term) && !isboxedinteger(term)) {
      psc = get_str_psc(term);
      name = get_name(psc);
      arity = get_arity(psc);
      return (atom_unify(CTXTc makestring(name), ptoc_tag(CTXTc 2)) &&
	      int_unify(CTXTc makeint(arity), ptoc_tag(CTXTc 3)));
    } else if (islist(term))
      return (atom_unify(CTXTc makestring(list_dot_string), ptoc_tag(CTXTc 2)) &&
	      int_unify(CTXTc makeint(2), ptoc_tag(CTXTc 3)));
    else return (unify(CTXTc term, ptoc_tag(CTXTc 2)) &&
		 int_unify(CTXTc makeint(0), ptoc_tag(CTXTc 3)));
  } else {	/* term is a variable */
    functor = ptoc_tag(CTXTc 2);
    if (isstring(functor) || isointeger(functor) || isofloat(functor) ||
	(isconstr(term) && get_arity(get_str_psc(term)) == 0)) {
      arity = ptoc_tag(CTXTc 3);
      /* tls: added !isnumber conjunct */
      if (arity_integer(arity) && !xsb_isnumber(functor)) {
	value = int_val(arity);
	if (value == 0) return unify(CTXTc functor, term);
	else {
	  if (value == 2 && isstring(functor) 
	      && string_val(functor) == list_dot_string) {
	    /* term is a variable and I bind it to a list. */
	    bind_list((CPtr)term, hreg);
	    new_heap_free(hreg);
	    new_heap_free(hreg);
	  } else { 
	    /* functor always creates a psc in the current module */
	    sym = (Pair)insert(string_val(functor), (char)value, 
			       (Psc)flags[CURRENT_MODULE],
			       &new_indicator);
	    sreg = hreg;
	    hreg += value+1;	/* need (arity+1) new cells */
	    bind_cs((CPtr)term, sreg);
	    new_heap_functor(sreg, sym->psc_ptr);
	    for (disp=0; disp<value; disp++) {
	      new_heap_free(sreg);
	    }
	  }
	  return TRUE;	/* always succeed! */
	}
	/* TLS rearranged order of the two elses below */
      } else {
	  if (xsb_isnumber(functor))
	    return (unify(CTXTc term, functor) && 
		    int_unify(CTXTc makeint(0), arity));
	  else {
	    if (isnonvar(arity)) {
	      if (isinteger(arity)) {
			if (int_val(arity) >= 0)
		  		xsb_representation_error(CTXTc "max_arity",
		  					 makestring(string_find("Arity of term",1)),"functor/3",3);
			else xsb_domain_error(CTXTc "not_less_than_zero",arity,"functor/3",3);
	      }
	      else xsb_type_error(CTXTc "integer",arity,"functor/3",3); 
	    }
	  else xsb_instantiation_error(CTXTc "functor/3", 3);
	  }
      }
    }
      else {
      if (isnonvar(functor))
	xsb_type_error(CTXTc "atomic",functor,"functor/3",2); 
      else xsb_instantiation_error(CTXTc "functor/3",3);
      }
  }
  return TRUE;
}

/* TLS 12/08 replaced what had been a fail if arg 2 was not a compound
   term with a type error, and what had been a fail if arg 1 was less
   than 0 with a domain error.  Both of these behaviors are specified
   in ISO */
inline static xsbBool arg_builtin(CTXTdecl)
{
  /* r1: +index (int); r2: +term; r3: ?arg (term) */
  Cell index;
  Cell term;
  int disp;

  index = ptoc_tag(CTXTc 1);
  if (isinteger(index)) {
    if ((disp = (int)int_val(index)) > 0) {
      term = ptoc_tag(CTXTc 2);
      if (isnonvar(term)) {
	if (isconstr(term) && !isboxedinteger(term) && !isboxedfloat(term)) {
	  if (disp <= (int)get_arity(get_str_psc(term))) {
	    return unify(CTXTc get_str_arg(term,disp), //(Cell)(clref_val(term)+disp),
			 ptoc_tag(CTXTc 3));
	  } 
	  else return FALSE;	/* fail */
	} else if (islist(term) && (disp==1 || disp==2)) {
	  return unify(CTXTc get_str_arg(term,disp-1), //(Cell)(clref_val(term)+disp-1),
		       ptoc_tag(CTXTc 3));
	  //	} else return FALSE;	/* fail */
	} else xsb_type_error(CTXTc "compound",term,"arg/3",2);
      } else xsb_instantiation_error(CTXTc "arg/3",2);
      //    } else return FALSE;	/* fail */
    } else {
      if (disp == 0) return FALSE;
      else xsb_domain_error(CTXTc "not_less_than_zero",index,"arg/3",2);
    }
  } else {
    if (isnonvar(index)) xsb_type_error(CTXTc "integer",index,"arg/3",1); 
    else xsb_instantiation_error(CTXTc "arg/3",1);
  }
  return TRUE;
}

/* determine whether to give type or instantiation error */
int not_univ_list_type(Cell list) {
  Cell lhead;
  if (isref(list)) return FALSE;
  lhead = get_list_head(list);
  XSB_Deref(lhead);
  if (isatomic(lhead)) return FALSE;
  return TRUE;
}

inline static xsbBool univ_builtin(CTXTdecl)
{
  /* r1: ?term; r2: ?list	*/
  int i, arity;
  int  new_indicator;
  char *name;
  Cell list, olist, new_list, term, chead, ctail, new_term;
  Pair sym;

  term = ptoc_tag(CTXTc 1);
  olist = list = ptoc_tag(CTXTc 2);
  if (isnonvar(term)) {	/* Usage is deconstruction of terms */
    if (!isref(list) && !islist(list)) {
      if (not_univ_list_type(list)){
	xsb_type_error(CTXTc "list",list,"=../2",2);  /* f(a) =.. 3. */
      } else xsb_instantiation_error(CTXTc "=../2",2);
    }
    new_list = makelist(hreg);
    if (isstring(term) || isointeger(term)) { follow(hreg) = term; hreg += 2; }
    else if (isconstr(term)) {
      arity = (get_arity(get_str_psc(term)));
      follow(hreg) = makestring(get_name(get_str_psc(term)));
      hreg += 2;
      for (i = 1 ; i <= arity ; i++) {
	follow(hreg-1) = makelist(hreg);
	follow(hreg) = get_str_arg(term,i); 
	hreg += 2;
      }
    } else { /* term is list */
      follow(hreg) = makestring(list_dot_string);
      follow(hreg+1) = makelist(hreg+2);
      follow(hreg+2) = get_list_head(term);
      follow(hreg+3) = makelist(hreg+4);
      follow(hreg+4) = get_list_tail(term);
      hreg += 6;
    }
    follow(hreg-1) = makenil;
    return unify(CTXTc list, new_list);
  } else { /* usage is construction; term is known to be a variable */
    if (islist(list)) {
      chead = get_list_head(list);
      XSB_Deref(chead);
      ctail = get_list_tail(list);
      XSB_Deref(ctail);
      if (isatom(chead)) {
	if (isnil(ctail)) {	/* atom construction */
	  new_term = chead;
	} else {
	  xsbBool list_construction = FALSE;
	  name = string_val(chead);
	  if (!strcmp(name, ".")) { /* check for list construction */
	    list = ctail;
	    if (islist(list)) {
	      list = get_list_tail(list); XSB_Deref(list);
	      if (islist(list)) {
		list = get_list_tail(list); XSB_Deref(list);
		if (isnil(list)) list_construction = TRUE;
	      }
	    }
	  }
	  if (list_construction) { /* no errors can occur */
	    new_term = makelist(hreg);
	    list = ctail;
	    bld_copy(hreg, get_list_head(list));
	    list = get_list_tail(list);
	    XSB_Deref(list);
	    bld_copy(hreg+1, get_list_head(list)); 
	    hreg += 2;
	  } else { /* compound term construction */
	    new_term = makecs(hreg); /* leave psc to be filled in later */
	    for (arity = 0, list = ctail; ;
		 arity++, list = get_list_tail(list)) {
	      XSB_Deref(list);
	      if (!islist(list)) break;
	      bld_copy(hreg+arity+1, get_list_head(list));
	    }
	    if (isnil(list) && arity <= MAX_ARITY) {
	      /* '=..'/2 always creates a psc in the current * module */
	      sym = (Pair)insert(name, (char)arity,
				 (Psc)flags[CURRENT_MODULE],
				 &new_indicator);
	      new_heap_functor(hreg, sym->psc_ptr);
	      hreg += arity+1;
	    } else {
	      /* leave hreg unchanged */
	      if (arity > MAX_ARITY)
		xsb_representation_error(CTXTc "max_arity",
					 makestring(string_find("Length of list",1)),"=../2",2);
	      else {
		if (isref(list)) xsb_instantiation_error(CTXTc "=../2",2);
		else xsb_type_error(CTXTc "list",olist,"=../2",2);  /* X =.. [foo|Y]. */
	      }
	    }
	  }
	} 
	return unify(CTXTc term,new_term);
      }
      if (isnil(ctail) && (isofloat(chead) || isointeger(chead))) { /* list=[num] */
	  bind_copy((CPtr)term, chead);	 /* term<-num  */
	  return TRUE;	/* succeed */
      } else { /* error, which one? */
	if (isref(chead)) xsb_instantiation_error(CTXTc "=../2",2);
	else if (!isnil(ctail) || (isofloat(chead) || isointeger(chead))) {
	  xsb_type_error(CTXTc "atom",chead,"=../2",2);
	} else xsb_type_error(CTXTc "atomic",chead,"=../2",2);
      }
    } /* List is a list */
    if (isref(list)) xsb_instantiation_error(CTXTc "=../2",2);
    if (isnil(list)) xsb_domain_error(CTXTc "non_empty_list",list,"=../2",2);
    else xsb_type_error(CTXTc "list",list,"=../2",2);  /* X =.. a */
  } /* usage is construction; term is known to be a variable */
  return TRUE;
}


inline static xsbBool hilog_arg(CTXTdecl)
{
  /* r1: +index (int); r2: +term; r3: ?arg (term) */
  Cell index, term;
  size_t disp;

  index = ptoc_tag(CTXTc 1);
  if (isinteger(index)) {
    if ((disp = int_val(index)) > 0) {
      term = ptoc_tag(CTXTc 2);
      if (isnonvar(term)) {
	if (isconstr(term)) {
	  if (hilog_cs(term)) disp++;
	  if (disp <= (int)get_arity(get_str_psc(term))) {
	    return unify(CTXTc get_str_arg(term,disp), //(Cell)(clref_val(term)+disp),
			 ptoc_tag(CTXTc 3));
	  } return FALSE;		/* fail */
	} else if (islist(term) && (disp==1 || disp==2)) {
	return unify(CTXTc get_str_arg(term,disp-1), //(Cell)(clref_val(term)+disp-1),
		       ptoc_tag(CTXTc 3));
	} else return FALSE;	/* fail */
      } else xsb_instantiation_error(CTXTc "hilog_arg/3",2);
    } else return FALSE;	/* fail */
  } else {
    if (isnonvar(index))
      xsb_type_error(CTXTc "integer",index,"hilog_arg/3",1);
    else xsb_instantiation_error(CTXTc "hilog_arg/3", 1);
  }
  return TRUE;
}

#define INITIAL_NAMELEN 256

inline static xsbBool atom_to_list(CTXTdeclc int call_type)
{
  /* r1: ?term; r2: ?character list	*/
  //  size_t i; unused?
  size_t len;
  Integer c;
  byte *atomname, *atomnamelast;
  byte *atomnameaddr = NULL;
  int atomnamelen;
  char tmpstr[5], *tmpstr_interned;  /* 2 -> 5, nfz */
  Cell heap_addr, term, term2;
  Cell list, new_list;
  CPtr top = 0;
  char *call_name = (call_type == ATOM_CODES ? "atom_codes/2" : "atom_chars/2");
  char *elt_type = (call_type == ATOM_CODES ? "character_code" : "character");

  term = ptoc_tag(CTXTc 1);
  XSB_Deref(term);
  list = ptoc_tag(CTXTc 2);
  if (!isnonvar(term)) {	/* use is: CODES/CHARS --> ATOM */
    atomnameaddr = (byte *)mem_alloc(INITIAL_NAMELEN,LEAK_SPACE);
    atomnamelen = INITIAL_NAMELEN;
    //    printf("Allocated namebuf: %p, %d\n",atomnameaddr,atomnamelen);
    atomname = atomnameaddr;
    atomnamelast = atomnameaddr + (atomnamelen - 1);
    term2 = list;	/* DON'T use heap for temp storage */
    do {
      XSB_Deref(term2);
      if (isnil(term2)) {
	*atomname = '\0';  /* remove ++ nfz */
	break;
      }
      if (islist(term2)) {
	heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
	if (((call_type==ATOM_CODES) && !isinteger(heap_addr))
	    || ((call_type==ATOM_CHARS) && !isstring(heap_addr))) {
	  if (isnonvar(heap_addr)) {
	    xsb_type_error(CTXTc elt_type,heap_addr,call_name,2); 
	  }
	  else xsb_instantiation_error(CTXTc call_name,2);
	  mem_dealloc(atomnameaddr,atomnamelen,LEAK_SPACE);
	  return FALSE;	/* fail */
	}
	if (isointeger(heap_addr))
	  c = (int)oint_val(heap_addr);
	else { /* ATOM CHARS */
	  byte *chptr;                           /* nfz */
	  chptr = (byte *) string_val(heap_addr);         /* nfz */
	  c = utf8_char_to_codepoint(&chptr);    /* nfz */
	}

	//	if (c < 0) {   /*  || c > 255 nfz */
	if (c < 0 || (c > 255 && CURRENT_CHARSET == LATIN_1)) {
	  mem_dealloc(atomnameaddr,atomnamelen,LEAK_SPACE);
	  xsb_representation_error(CTXTc "character_code",
				   makestring(string_find("(Non-LATIN_1 Character)",1)),
				   call_name,2);
	  return FALSE;	/* keep compiler happy */
	}
	if (atomname+3 >= atomnamelast) { /* nfz */
	  Integer diff;
	  if (is_cyclic(CTXTc term2)) {
	    mem_dealloc(atomnameaddr,atomnamelen,LEAK_SPACE);
	    xsb_type_error(CTXTc "list",makestring("infinite list(?)"),call_name,2);
	  }
	  diff = atomname - atomnameaddr;
	  atomnameaddr = (byte *)mem_realloc(atomnameaddr,atomnamelen,(atomnamelen << 1),LEAK_SPACE);
	  atomname = atomnameaddr + diff;
	  atomnamelen = atomnamelen << 1;
	  atomnamelast = atomnameaddr + (atomnamelen - 1);
	}
	atomname = utf8_codepoint_to_str((int)c, atomname); /* nfz */
	term2 = get_list_tail(term2);
      } else {
	mem_dealloc(atomnameaddr,atomnamelen,LEAK_SPACE);
	if (isref(term2)) xsb_instantiation_error(CTXTc call_name,2);
	else xsb_type_error(CTXTc "list",term2,call_name,2);  /* atom_chars(X,[1]) */
	return FALSE;	/* fail */
      }
    } while (1);
    bind_string((CPtr)(term), (char *)string_find((char *)atomnameaddr, 1));
		
    mem_dealloc(atomnameaddr,atomnamelen,LEAK_SPACE);
    return TRUE;
  } else {	/* use is: ATOM --> CODES/CHARS */
    if (isatom(term)) {
      atomname = (byte *)string_val(term);
      len = strlen((char *)atomname);
      if (len == 0) {
	if (!isnonvar(list)) {
	  bind_nil((CPtr)(list)); 
	  return TRUE;
	}
	else { 
	  return isnil(list);
	}
      } else {
	/* check that there is enough space on the heap! */  
	check_glstack_overflow(2, pcreg, 2*len*sizeof(Cell)) ;
	list = ptoc_tag(CTXTc 2);   /* in case it changed */

	new_list = makelist(hreg);
	//	for (i = 0; i < len; i++) {              /* nfz */
	while (*atomname != '\0'){                       /* nfz */
 	  if (call_type==ATOM_CODES){
	    int code;
	    code = utf8_char_to_codepoint(&atomname); /* nfz */
	    follow(hreg++) = makeint(code);               /* nfz */ 
	  }
	  else {
	    int k;
	    byte *atomname0 = atomname;                   /* nfz */ 
	    utf8_char_to_codepoint(&atomname);            /* nfz */	    
	    k = 0;               
	    while (atomname0<atomname){
	      tmpstr[k] = *atomname0++;
	      k++;
	    }
	    tmpstr[k]='\0';
	    tmpstr_interned=string_find(tmpstr,k);        /* nfz */	    
	    follow(hreg++) = makestring(tmpstr_interned);
	  }
	  top = hreg++;
	  follow(top) = makelist(hreg);
	}
	follow(top) = makenil;
	return unify(CTXTc list, new_list);
      } 
    } else xsb_type_error(CTXTc "atom",term,call_name,1);  /* atom_codes(1,F) */
  }
  return TRUE;
}

#define MAXNUMCHARLEN 256

char *cvt_float_to_str(CTXTdeclc Float);

inline static xsbBool number_to_list(CTXTdeclc int call_type)
{
  int i, tmpval;
  Integer c;
  char tmpstr[2], *tmpstr_interned;
  char str[MAXNUMCHARLEN];	
  int StringLoc = 0;
  Cell heap_addr, term, term2;
  Cell list;
  CPtr new_list;
  char hack_char;	
  char *call_name =
    (call_type == NUMBER_CODES ?
     "number_codes/2" : (call_type == NUMBER_DIGITS?
		       "number_digits/2" : "number_chars/2"));
  char *elt_type =
    (call_type == NUMBER_CODES ?
     "integer" : (call_type == NUMBER_DIGITS? "digit" : "digit atom"));


  term = ptoc_tag(CTXTc 1);
  XSB_Deref(term);
  list = ptoc_tag(CTXTc 2);
  XSB_Deref(list);
  if ((islist(list) && ground(list)) || isnil(list)) { /* use is: CHARS/CODES --> NUMBER */
    term2 = list;
    do {
      XSB_Deref(term2);
      if (isnil(term2)) {
	str[StringLoc++] = '\0';
	break;
      }
      if (islist(term2)) {
	heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
	if (((call_type==NUMBER_CODES) && (!isinteger(heap_addr)))
	    || ((call_type==NUMBER_CHARS) && (!isstring(heap_addr) || isnil(heap_addr)))
	    || ((call_type==NUMBER_DIGITS)
		&& !isstring(heap_addr)
		&& !isinteger(heap_addr))) {
	  xsb_type_error(CTXTc elt_type,list,call_name,2); /* number_chars(X,[a]) */
	}
	if (call_type==NUMBER_CODES)
	  c = int_val(heap_addr);
	else if ((call_type==NUMBER_DIGITS) && (isinteger(heap_addr))) {
	  tmpval = (int)int_val(heap_addr);
	  if ((tmpval < 0) || (tmpval > 9)) {
	    xsb_type_error(CTXTc elt_type,list,call_name,2); /* number_chars(X,[11]) */
	  }
	  c = ('0' + int_val(heap_addr));
	} else if (isstring(heap_addr))
	  c = *string_val(heap_addr);
	else {
	    xsb_type_error(CTXTc "integer, digit, or atom",list,call_name,2); /* number_chars(X,[a]) */
	}

	if (c < 0 || c > 255) {
	  //	  xsb_representation_error(CTXTc "character_code",heap_addr,call_name,2);
	  xsb_representation_error(CTXTc "character_code",
				   makestring(string_find("(Non-LATIN_1 Character)",1)),
				   call_name,2);
	}
	if (StringLoc > 200) xsb_type_error(CTXTc "list",makestring("infinite list(?)"),call_name,2);
	str[StringLoc++] = (char)c;
	term2 = get_list_tail(term2);
      } else {
	if (isref(term2))
	  xsb_instantiation_error(CTXTc call_name,2);
	else
	  xsb_type_error(CTXTc "list",term2,call_name,2);
	return FALSE;	/* fail */
      }
    } while (1);

    if (sscanf(str, "%" Intfmt "%c", &c, &hack_char) == 1) {
      if (isointeger(term)) {
	if (oint_val(term) != c) return FALSE;
      } else if (isref(term)) {bind_oint((CPtr)(term), c);}
      else return FALSE;
    } else {
      Float float_temp;
      //TODO: Refactor the below few lines of code once the "Floats are always double?" 
      //situation is resolved.
#ifndef FAST_FLOATS
      if (sscanf(str, "%lf%c", &float_temp, &hack_char) == 1)
#else
      if (sscanf(str, "%f%c", &float_temp, &hack_char) == 1)
#endif
	{
	  if (isofloat(term)) {
	    if (ofloat_val(term) != float_temp) return FALSE;
	  } else if (isref(term)) {bind_boxedfloat((CPtr)(term), float_temp);}
	  else return FALSE;
	}
      // TLS: changed to fail to syntax error to conform w. ISO.  
      else xsb_syntax_error_non_compile(CTXTc list,call_name,2);
      //            else return FALSE;	/* fail */
    }
  } else {	/* use is: NUMBER --> CHARS/CODES/DIGITS */
    if (isref(term)) {
      if (list_unifiable(CTXTc list)) xsb_instantiation_error(CTXTc call_name,1);
      else xsb_type_error(CTXTc "list",list,"number_codes/2",2); //fix pred name
    } else if (isointeger(term)) {
      sprintf(str, "%" Intfmt, oint_val(term));
    } else if (isofloat(term)) {
      strncpy(str,cvt_float_to_str(CTXTc ofloat_val(term)),MAXNUMCHARLEN);
    } else xsb_type_error(CTXTc "number",term,call_name,1);
    new_list = hreg;
    for (i=0; str[i] != '\0'; i++) {
      if (call_type==NUMBER_CODES)
	follow(hreg) = makeint((unsigned char)str[i]);
      else if (call_type==NUMBER_CHARS) {
	tmpstr[0] = str[i];
	tmpstr[1] = '\0';
	tmpstr_interned=string_find(tmpstr,1);
	follow(hreg) = makestring(tmpstr_interned);
      } else { /* NUMBER_DIGITS */
	tmpval = str[i] - '0';
	if (0 <= tmpval && tmpval < 10)
	  follow(hreg) = makeint((unsigned char)str[i] - '0');
	else {
	  tmpstr[0] = str[i];
	  tmpstr[1] = '\0';
	  tmpstr_interned=string_find(tmpstr,1);
	  follow(hreg) = makestring(tmpstr_interned);
	}
      }
      hreg += 2;
      follow(hreg-1) = makelist(hreg);
    } follow(hreg-1) = makenil;
    if (isref(list)) {bind_list((CPtr)list,new_list);}
    else return unify(CTXTc makelist(new_list),list);
    //else xsb_type_error(CTXTc "list",term,call_name,2);
  }
  return TRUE;
}

/*************************************************************************************
 SORTING

 TLS: Changed sorting functions so that they just use area from the
 stack in the case of short lists and so avoid a malloc.  For
 applications that use sorting heavily, this greately improves
 scalability for the MT engine, and should improve speed a little for
 single threade applications as well.
*************************************************************************************/

#define SHORTLISTLEN 1024

#ifdef MULTI_THREAD

/* Define own qsort routine when multithreading because it has to pass
   the thread ID to the compare routine when comparing terms.  The
   standard system qsort routine, which is used when single threading,
   does not support such an extra parameter.
*/

typedef int (*compfptr)(CTXTdeclc const void *, const void *) ;

#define INSERT_SORT	8

#define QSORT_SWAP(a,b) { Cell t = *a ; *a = *b ; *b = t ; }

#define QSORT_LESS(A,B) ((*qsort_cmp)((CTXT),(A),(B)) < 0)

void qsort0(CTXTdeclc compfptr qsort_cmp, CPtr low, CPtr high )
{
/* low is address of lowest element on array */
/* high is address of rightmost element on array */

	if ( high - low >= INSERT_SORT )
	{
		Cell pivot ;
		CPtr l, r ;
		CPtr mid = low + ( high - low ) / 2 ;

		if ( QSORT_LESS(mid,low) )
			QSORT_SWAP( mid, low )
		if ( QSORT_LESS(high,mid) )
		{	QSORT_SWAP( high, mid )
			if ( QSORT_LESS(mid,low) )
				QSORT_SWAP( mid, low ) 
		}
		pivot = *mid ;

		l = low + 1 ;
		r = high - 1 ;
		do
		{	while( QSORT_LESS(l, &pivot) ) l++ ;
			while( QSORT_LESS(&pivot, r) ) r-- ;

			if( l < r )
			{	QSORT_SWAP( l, r )
				l++; r--;
			}
			else if( l == r )
			{	l++; r--;
			}
		} while( l <= r ) ;
		qsort0(CTXTc qsort_cmp, low, r) ;
		qsort0(CTXTc qsort_cmp, l, high) ;
	}
	else if( low < high )		/* insertion sort for small lists */
	{	CPtr p, min = low, r ;
		
		/* set a sentinel to speed up insert sort main loop */
		for( p = low + 1 ; p <= high ; p++ )
			if( QSORT_LESS( p, min ) )
				min = p ;
		if( low != min )
			QSORT_SWAP( low, min ) ;

		for( r = low + 2 ; r <= high ; r++ )
		{	Cell new_el = *r ;

			for( p = r ; QSORT_LESS( &new_el, p-1 ) ; p-- )
				*p = *(p-1) ;
			*p = new_el ;
		}
	}
}

/* This function (and some others) avoid a */

void mt_qsort(th_context *th, CPtr v, int len, unsigned int sz, compfptr comp)
{
	qsort0( th, comp, v, v + len - 1 ) ;
}
#endif /* MULTI_THREAD */

inline static xsbBool sort(CTXTdecl)
{
  /* r1: +list of terms; r2: ?sorted list of terms */
  int i, len;
  Cell *cell_tbl;
  Cell heap_addr, term, term2;
  Cell list, new_list;
  Cell cell_tbl_array[SHORTLISTLEN];

  list = ptoc_tag(CTXTc 1);
  term2 = list; len = 0;
  do {
    XSB_Deref(term2);
    if (isnil(term2)) break;
    if (islist(term2)) {
      len++; term2 = get_list_tail(term2);
    } else {
      if (isref(term2)) xsb_instantiation_error(CTXTc "sort/2",1);
      else xsb_type_error(CTXTc "list",list,"sort/2",1);
    }
  } while(1);
  check_glstack_overflow(3, pcreg, (2*len)*sizeof(Cell)) ;
  list = ptoc_tag(CTXTc 1); /* reset in case moved */
  if (len > 0) {
    term2 = list;
    if (len > SHORTLISTLEN) cell_tbl = (Cell *)mem_alloc((len * sizeof(Cell)),LEAK_SPACE);
    else cell_tbl = &cell_tbl_array[0];
    if (!cell_tbl)
      xsb_resource_error(CTXTc "memory", "sort",2);
    for (i=0 ; i < len ; ++i) {
      XSB_Deref(term2);	/* Necessary for correctness.	*/
      heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
      cell_tbl[i] = heap_addr;
      term2 = get_list_tail(term2);
    }
#ifndef MULTI_THREAD
    qsort(cell_tbl, len, sizeof(Cell), compare);
#else
    mt_qsort(CTXTc cell_tbl, len, sizeof(Cell), compare);
#endif
    new_list = makelist(hreg);
    follow(hreg) = cell_tbl[0];
    hreg += 2;
    follow(hreg-1) = makelist(hreg);
    for (i=1 ; i < len ; i++) {
      if (compare(CTXTc (void*)cell_tbl[i], (void*)cell_tbl[i-1])) {
	follow(hreg) = cell_tbl[i];
	hreg += 2;
	follow(hreg-1) = makelist(hreg);
      }
    } follow(hreg-1) = makenil;
    if (len > SHORTLISTLEN) mem_dealloc(cell_tbl,len * sizeof(Cell),LEAK_SPACE);
    term = ptoc_tag(CTXTc 2);
    return unify(CTXTc new_list, term);
  }
  term = ptoc_tag(CTXTc 2);
  return unify(CTXTc list, term);
}

inline static xsbBool keysort(CTXTdecl)
{
  /* r1: +list of terms of the form Key-Value;	*/
  /* r2: ?sorted list of terms			*/
  int i, len;
  Cell heap_addr, term, term2;
  Cell list, new_list;
  Cell *cell_tbl;
  Cell cell_tbl_array[SHORTLISTLEN];

  list = ptoc_tag(CTXTc 1);
  term2 = list; len = 0;
  do {
    XSB_Deref(term2);
    if (isnil(term2)) break;
    if (islist(term2)) {
      heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
      if (isconstr(heap_addr) && 
	  get_arity(get_str_psc(heap_addr)) == 2 &&
	  !strcmp(get_name(get_str_psc(heap_addr)), "-")) {
	len++; term2 = get_list_tail(term2);
      } else {
	xsb_type_error(CTXTc "pair of the form Key-Value", (Cell)NULL,"keysort/2",1);
      }
    } else {
      if (isref(term2)) xsb_instantiation_error(CTXTc "keysort/2",1);
      else xsb_type_error(CTXTc "list",list,"keysort/2",1);
      return FALSE;	/* fail */
    }
  } while(1);
  check_glstack_overflow(3, pcreg, (2*len)*sizeof(Cell)) ;
  list = ptoc_tag(CTXTc 1);  /* reset in case moved */
  term = ptoc_tag(CTXTc 2);
  if (len > 0) {
    term2 = list;
    if (len > SHORTLISTLEN) cell_tbl = (Cell *)mem_alloc((len * sizeof(Cell)),LEAK_SPACE);
    else cell_tbl = &cell_tbl_array[0];
    if (!cell_tbl)
      xsb_resource_error(CTXTc "memory","keysort",2);
    for (i=0 ; i < len ; ++i) {
      XSB_Deref(term2);	/* Necessary for correctness.	*/
      heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
      cell_tbl[i] = heap_addr;
      term2 = get_list_tail(term2);
    }
#ifndef MULTI_THREAD
    qsort(cell_tbl, len, sizeof(Cell), key_compare);
#else
    mt_qsort(CTXTc cell_tbl, len, sizeof(Cell), key_compare);
#endif
    new_list = makelist(hreg);
    for (i=0 ; i < len ; i++) {
      follow(hreg) = cell_tbl[i];
      hreg += 2;
      follow(hreg-1) = makelist(hreg);
    } follow(hreg-1) = makenil;
    if (len > SHORTLISTLEN) mem_dealloc(cell_tbl,len * sizeof(Cell),LEAK_SPACE);
    return unify(CTXTc new_list, term);
  }
  return unify(CTXTc list, term);
}

#ifndef MULTI_THREAD
struct sort_par_spec par_spec;
#endif

int par_key_compare(CTXTdeclc const void * t1, const void * t2) {
  Integer ipar, cmp, ind1, ind2;
  Cell term1 = (Cell) t1 ;
  Cell term2 = (Cell) t2 ;

  XSB_Deref(term1);		/* term1 is not in register! */
  XSB_Deref(term2);		/* term2 is not in register! */
  if (par_spec.sort_num_pars > 0) {
    ipar = 0;
    while (ipar < par_spec.sort_num_pars) {
      ind1 = ind2 = par_spec.sort_par_ind[ipar];
      if (islist(term1)) ind1--;
      if (islist(term2)) ind2--;
      cmp = compare(CTXTc (void*)get_str_arg(term1,ind1),
		    (void*)get_str_arg(term2,ind2));
      if (cmp) {
	if (par_spec.sort_par_dir[ipar]) return (int)cmp;
	else return (int)-cmp;
      } else ipar++;
    }
    return 0;
  } else if (par_spec.sort_num_pars == 0) {
    return compare(CTXTc (void*)term1, (void*)term2);
  } else
    return -compare(CTXTc (void*)term1, (void*)term2);
}

inline static xsbBool parsort(CTXTdecl)
{
  /* r1: +list of terms;				*/
  /* r2: +list of sort indicators: asc(I) or desc(I)	*/
  /* r3: 1 if eliminate dupls, 0 if not			*/
  /* r4: ?sorted list of terms				*/
  int i, len;
  int max_ind = 0, elim_dupls;
  Cell heap_addr, term, term2, tmp_ind;
  Cell list, new_list;
  Cell *cell_tbl;
  char ermsg[50];

  elim_dupls = (int)ptoc_int(CTXTc 3);

  list = ptoc_tag(CTXTc 2);
  term2 = list; par_spec.sort_num_pars = 0;

  XSB_Deref(term2);
  if (isstring(term2) && !strcmp(string_val(term2),"asc")) par_spec.sort_num_pars = 0;
  else if (isstring(term2) && !strcmp(string_val(term2),"desc")) par_spec.sort_num_pars = -1;
  else
    while (TRUE) {
      if (isnil(term2)) break;
      if (islist(term2)) {
	heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
	if (isconstr(heap_addr) && 
	    get_arity(get_str_psc(heap_addr)) == 1 &&
	    !strcmp(get_name(get_str_psc(heap_addr)),"asc")) {
	  par_spec.sort_par_dir[par_spec.sort_num_pars] = 1;
	} else if (isconstr(heap_addr) && 
		   get_arity(get_str_psc(heap_addr)) == 1 &&
		   !strcmp(get_name(get_str_psc(heap_addr)),"desc")) {
	  par_spec.sort_par_dir[par_spec.sort_num_pars] = 0;
	} else xsb_type_error(CTXTc "asc/1 or desc/1 term",heap_addr,"parsort/4",2);
	tmp_ind = get_list_tail(heap_addr); XSB_Deref(tmp_ind);
	if (!isinteger(tmp_ind)) xsb_type_error(CTXTc "integer arg for asc/1 or desc/1",tmp_ind,"parsort/4",2);
	i = (int)int_val(tmp_ind);
	/* TLS: Should be range below */
	if (i < 1 || i > 255) xsb_domain_error(CTXTc "arity_sized_integer",tmp_ind,
					       "parsort/4",2) ;
	par_spec.sort_par_ind[par_spec.sort_num_pars] = i;
	if (i > max_ind) max_ind = i;
	par_spec.sort_num_pars++;
	term2 = get_list_tail(term2);
	XSB_Deref(term2);
      } else xsb_type_error(CTXTc "list",list,"parsort/4",2);
    }
      
  list = ptoc_tag(CTXTc 1);
  term2 = list; len = 0;
  do {
    XSB_Deref(term2);
    if (isnil(term2)) break;
    if (islist(term2)) {
      heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
      if (par_spec.sort_num_pars <= 0 || 
	  (isconstr(heap_addr) && (get_arity(get_str_psc(heap_addr)) >= max_ind)) ||
	  (islist(heap_addr) && max_ind <=2)) {
	len++; term2 = get_list_tail(term2);
      } else {
	sprintf(ermsg,"Term with arity at least %d", max_ind);
	xsb_domain_error(CTXTc ermsg,(Cell) heap_addr,"parsort/4",1) ;	
	return FALSE;	/* fail */
      }
    } else {
      if (isref(term2)) xsb_instantiation_error(CTXTc "parsort/4",1);
      else xsb_type_error(CTXTc "list",list,"parsort/4",1);
      return FALSE;	/* fail */
    }
  } while(1);

  check_glstack_overflow(4, pcreg, (2*len)*sizeof(Cell)) ;
  list = ptoc_tag(CTXTc 1);  /* reset in case moved */
  term = ptoc_tag(CTXTc 4);
  if (len > 0) {
    term2 = list;
    cell_tbl = (Cell *)mem_alloc(len * sizeof(Cell),LEAK_SPACE);
    if (!cell_tbl)
      xsb_resource_error(CTXTc "memory","parsort",4);
    for (i=0 ; i < len ; ++i) {
      XSB_Deref(term2);	/* Necessary for correctness.	*/
      heap_addr = get_list_head(term2); XSB_Deref(heap_addr);
      cell_tbl[i] = heap_addr;
      term2 = get_list_tail(term2);
    }
#ifndef MULTI_THREAD
    qsort(cell_tbl, len, sizeof(Cell), par_key_compare);
#else
    mt_qsort(CTXTc cell_tbl, len, sizeof(Cell), par_key_compare);
#endif
    new_list = makelist(hreg);
    if (elim_dupls) {
      follow(hreg) = cell_tbl[0];
      hreg += 2;
      follow(hreg-1) = makelist(hreg);
      for (i=1 ; i < len ; i++) {
	if (compare(CTXTc (void*)cell_tbl[i], (void*)cell_tbl[i-1])) {
	  follow(hreg) = cell_tbl[i];
	  hreg += 2;
	  follow(hreg-1) = makelist(hreg);
	}
      } 
    } else {
      for (i=0 ; i < len ; i++) {
	follow(hreg) = cell_tbl[i];
	hreg += 2;
	follow(hreg-1) = makelist(hreg);
      } 
    }
    follow(hreg-1) = makenil;
    mem_dealloc(cell_tbl,len * sizeof(Cell),LEAK_SPACE);
    return unify(CTXTc new_list, term);
  }
  return unify(CTXTc list, term);
}

xsbBool unify_with_occurs_check(CTXTdeclc Cell Term1, Cell Term2) { 
  //  printf("  Term2 %x, cs_val %x\n",Term2,cs_val(Term2));

 rec_unify_with_occurs_check:
  XSB_Deref(Term1);
  XSB_Deref(Term2);
  switch (cell_tag(Term1)) {
  case XSB_ATTV: 
  case XSB_REF: 
  case XSB_REF1: {
    if (isnonvar(Term2)) {
      if (not_occurs_in(Term1,Term2))
	return unify(CTXTc Term1,Term2);
      else return FALSE;
    } else 
      return (int) unify(CTXTc Term1,Term2);
  }
  case XSB_INT:
  case XSB_STRING:
  case XSB_FLOAT: 
    return unify(CTXTc Term1,Term2);

  case XSB_LIST: {
    switch (cell_tag(Term2)) {
    case XSB_ATTV: 
    case XSB_REF: 
    case XSB_REF1: 
      if (not_occurs_in(Term2,Term1))
	return unify(CTXTc Term1,Term2);
      else return FALSE;
    case XSB_LIST: {
      if (unify_with_occurs_check(CTXTc get_list_head(Term1), 
				  get_list_head(Term2) )) {
	Term1 = get_list_tail(Term1);
	Term2 = get_list_tail(Term2);
	goto rec_unify_with_occurs_check;
      } else return FALSE;
    }
    default: 
      return FALSE;
    }
  }

  case XSB_STRUCT: {
    switch (cell_tag(Term2)) {
    case XSB_ATTV: 
    case XSB_REF: 
    case XSB_REF1: 
      if (not_occurs_in(Term2,Term1))
	return unify(CTXTc Term1,Term2);
      else return FALSE;
    case XSB_STRUCT: {
      int i;
      int arity;
      if (get_str_psc(Term1) == get_str_psc(Term2)) {
	arity = get_arity(get_str_psc(Term1)); 
	if (arity == 0) return TRUE;
	for (i = 1; i < arity; i++) {
	  if (!unify_with_occurs_check(CTXTc get_str_arg(Term1,i), 
				       get_str_arg(Term2,i)))
	    return FALSE;
	}
	Term1 = get_str_arg(Term1,arity);
	Term2 = get_str_arg(Term2,arity);
	goto rec_unify_with_occurs_check;
      } else return FALSE;
    }
    default: 
      return FALSE;
    }
  }
    }
  return TRUE;  /* hush, little compiler */
  }

/*
int term_depth(CTXTdeclc Cell Term1,int indepth) { 

  XSB_Deref(Term1);
  switch (cell_tag(Term1)) {
    //  case XSB_ATTV: 
  case XSB_REF: 
  case XSB_REF1: 
  case XSB_INT:
  case XSB_STRING:
  case XSB_FLOAT: 
    return indepth+1;

  case XSB_LIST: {
    int car_depth = 0;
    int cdr_depth = 0;
    
    car_depth = term_depth(CTXTc (Cell) clref_val(Term1), indepth+1);
    cdr_depth = term_depth(CTXTc (Cell) (clref_val(Term1) + 1),indepth+1);
    return car_depth > cdr_depth ? car_depth : cdr_depth;
  }

  case XSB_STRUCT: {
    int max_depth = 0;
    int arity = get_arity(get_str_psc(Term1)); 
    int arg_depth, i;
    for (i = 1; i <= arity; i++) {
      arg_depth = term_depth(CTXTc (Cell) (clref_val(Term1) + i),indepth+1);
      if (arg_depth > max_depth) max_depth = arg_depth;
    }
    return max_depth;
  }
  }
    return 0;
}
*/

/**************************************************** */
#define TERM_TRAVERSAL_STACK_INIT 100000

typedef struct TermTraversalFrame{
  byte arg_num;
  byte arity;
  Cell parent;
} Term_Traversal_Frame ;

typedef Term_Traversal_Frame *TTFptr;
//      printf("reallocing to %d %d\n",term_traversal_stack_size, term_traversal_stack_size*sizeof(Term_Traversal_Frame)); 

#define push_term(Term) { 						\
    if (  ++term_traversal_stack_top == term_traversal_stack_size) {			\
      TTFptr old_term_traversal_stack = term_traversal_stack;				\
      term_traversal_stack = mem_realloc(old_term_traversal_stack,				\
			       term_traversal_stack_size*sizeof(Term_Traversal_Frame), \
			       2*term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE); \
      term_traversal_stack_size = 2*term_traversal_stack_size;				\
    }									\
    if (cell_tag(Term) == XSB_STRUCT) {					\
      term_traversal_stack[term_traversal_stack_top].arity =  (byte) get_arity(get_str_psc(Term)); \
      term_traversal_stack[term_traversal_stack_top].arg_num =  1;				\
      term_traversal_stack[term_traversal_stack_top].parent =  Term;			\
    } else {						/* list */	\
      term_traversal_stack[term_traversal_stack_top].arity =  1;				\
      term_traversal_stack[term_traversal_stack_top].arg_num =  0;				\
      term_traversal_stack[term_traversal_stack_top].parent =  Term;			\
    }									\
  }

#define pop_term(Term) {			\
    term_traversal_stack_top--;				\
    Term = term_traversal_stack[term_traversal_stack_top].parent;	\
  }

/*
int term_traversal_template(CTXTdeclc Cell Term) { 

  int term_traversal_stack_top = -1;
  int term_traversal_stack_size = TERM_TRAVERSAL_STACK_INIT;

  TTFptr term_traversal_stack = (TTFptr) mem_alloc(term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);

  XSB_Deref(Term);

  if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) return 1;
	
  push_term(Term);   

  while (term_traversal_stack_top >= 0) {

    if (term_traversal_stack[term_traversal_stack_top].arg_num > term_traversal_stack[term_traversal_stack_top].arity) {
      pop_term(Term);
    }
    else {
      Term = (Cell) (clref_val(term_traversal_stack[term_traversal_stack_top].parent) + term_traversal_stack[term_traversal_stack_top].arg_num);
      term_traversal_stack[term_traversal_stack_top].arg_num++;
      XSB_Deref(Term);
      if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) {	
      }
      else {
	push_term(Term);   
      }
    }
  }
  mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
  return 
}
*/

/**************************************************** */

/* Did not want to use recursion to avoid problems with C-stack for
   large terms (esp. in MT engine). */

/* Did not want to use recursion to avoid problems with C-stack for
   large terms (esp. in MT engine). */
int term_depth(CTXTdeclc Cell Term) { 

  int maxsofar = 0;
  int cur_depth = 0;
  int term_traversal_stack_top = 0;
  int term_traversal_stack_size = TERM_TRAVERSAL_STACK_INIT;

  TTFptr term_traversal_stack = (TTFptr) mem_alloc(term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);

  XSB_Deref(Term);

  if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT)  {
    mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
    return 1;
  }
	
  push_term(Term);   cur_depth++;		

  while (term_traversal_stack_top > 0) {

    if (term_traversal_stack[term_traversal_stack_top].arg_num > term_traversal_stack[term_traversal_stack_top].arity) {
      pop_term(Term);cur_depth--;
    }
    else {
      Term = get_str_arg(term_traversal_stack[term_traversal_stack_top].parent, term_traversal_stack[term_traversal_stack_top].arg_num);
      term_traversal_stack[term_traversal_stack_top].arg_num++;
      XSB_Deref(Term);
      if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) {	
	if (cur_depth +1 > maxsofar) maxsofar = cur_depth+1;
      }
      else {
	push_term(Term);   cur_depth++;		
      }
    }
  }
  mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
  return maxsofar;
}
/**************************************************** */
/* int term_sizew(CTXTdeclc Cell Term) { */
/* size = #functors + #non-compounds + Size of attributes for any attributed vars*/

int term_sizew(CTXTdeclc Cell Term) { 

  int cur_size= 0;
  int term_traversal_stack_top = 0;
  int term_traversal_stack_size = TERM_TRAVERSAL_STACK_INIT;

  TTFptr term_traversal_stack = (TTFptr) mem_alloc(term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);

  XSB_Deref(Term);

/*  if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT && !is_attv(Term)) return 1;*/
  if (!(is_attv(Term) || is_functor(Term) || is_list(Term))) {
     mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
     return 1; /*TODO: WGW why not set output arg and return TRUE? */
  }

  if (is_attv(Term)) {
     push_term(cell((CPtr)dec_addr(Term) + 1)) ;  
     cur_size++;
  } else { 
      push_term(Term);
     };   
  cur_size++;  /* add the current functor or attributed variable to the count	*/	

  while (term_traversal_stack_top > 0) {
    //    printf("term_traversal_stack_top %d\n");
    if (term_traversal_stack[term_traversal_stack_top].arg_num > term_traversal_stack[term_traversal_stack_top].arity) {
      pop_term(Term);
    }
    else {
      Term = get_str_arg(term_traversal_stack[term_traversal_stack_top].parent, term_traversal_stack[term_traversal_stack_top].arg_num);
      term_traversal_stack[term_traversal_stack_top].arg_num++;
      XSB_Deref(Term);
      if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT && !is_attv(Term)) {	  /* add 1 for a non compound */ /*and not XSB_ATTR*/
	cur_size++;
      }
      else {
       if (is_attv(Term)) {
          push_term(cell((CPtr)dec_addr(Term) + 1)) ; 
          cur_size++;
       } else { 
           push_term(Term);
          };   
       cur_size++;  /* add the current fuctor to the count	*/	
      }
    }
  }
  mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
  return cur_size;
}

/*
term_size(X,SX),put_attr(X,wgw,a),term_size(X,Sal),put_attr(X,wgw2,b),term_size(X,Sb).

*/
 

/***************************************************** */
/* TODO: dealloc if return FALSE?. */
xsbBool term_size_limit(CTXTdecl) { 
  /* WGW check 2nd arg is integer, call it LIMIT*/
/* 
  if (!(is_int(Limit)) xsb_instantiation_error(CTXTc "term_size_limit",2,2,XSB_INT);
*/

  int cur_size= 0;
  int term_traversal_stack_top = -1;
  int term_traversal_stack_size = TERM_TRAVERSAL_STACK_INIT;
  Cell Term;
  Integer Limit;
  TTFptr term_traversal_stack;
  Term = ptoc_tag(CTXTc 1);
  Limit = (Integer)ptoc_int(CTXTc 2);

  term_traversal_stack = (TTFptr) mem_alloc(term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);

  XSB_Deref(Term);

 if (!(is_attv(Term) || is_functor(Term) || is_list(Term)) && Limit > 0) {
   goto truereturn ; /*TODO: WGW why not set output arg and return TRUE? */
 } 

 if (!(is_attv(Term) || is_functor(Term) || is_list(Term)) && Limit < 2) {
   goto falsereturn ; /*TODO: WGW why not set output arg and return TRUE? */
 } 

 if (is_attv(Term)) {
     push_term(cell((CPtr)dec_addr(Term) + 1)) ;  
     cur_size++;
     if (cur_size > Limit) goto falsereturn;
  } else { 
      push_term(Term);
     };   
  cur_size++;  /* add the current functor or attributed variable to the count	*/	
  if (cur_size > Limit) goto falsereturn;

  while (term_traversal_stack_top >= 0) {

    if (term_traversal_stack[term_traversal_stack_top].arg_num > term_traversal_stack[term_traversal_stack_top].arity) {
      pop_term(Term);
    }
    else {
      Term = get_str_arg(term_traversal_stack[term_traversal_stack_top].parent, term_traversal_stack[term_traversal_stack_top].arg_num);
      term_traversal_stack[term_traversal_stack_top].arg_num++;
      XSB_Deref(Term);
      if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT && !is_attv(Term)) {	  /* add 1 for a non compound */ /*and not XSB_ATTR*/
	cur_size++;
        if (cur_size > Limit) goto falsereturn;
      }
      else {
       if (is_attv(Term)) {
          push_term(cell((CPtr)dec_addr(Term) + 1)) ; 
          cur_size++;
          if (cur_size > Limit) goto falsereturn;
       } else { 
           push_term(Term);
          };   
       cur_size++;  /* add the current fuctor to the count	*/	
       if (cur_size > Limit) goto falsereturn;
      }
    }
  }
//printf("*********** %d \n",cur_size);
if (cur_size > Limit) goto falsereturn;
truereturn:  
//  printf("***** truereturn \n");
  mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
  return TRUE;
falsereturn: 
//  printf("***** falsereturn \n");
  mem_dealloc(term_traversal_stack,term_traversal_stack_size*sizeof(Term_Traversal_Frame),OTHER_SPACE);
  return FALSE;
}
 
/**************************************************** */

#ifndef MULTI_THREAD
CTptr cycle_trail = 0;
int cycle_trail_size = 0;
  int cycle_trail_top = -1;
#endif

int is_cyclic(CTXTdeclc Cell Term) { 
  Cell visited_string;

  XSB_Deref(Term);
  if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) return FALSE;
  if (isinternstr(Term)) return FALSE;

  if (cycle_trail == (CTptr) 0){
    cycle_trail_size = TERM_TRAVERSAL_STACK_INIT;
    cycle_trail = (CTptr) mem_alloc(cycle_trail_size*sizeof(Cycle_Trail_Frame),OTHER_SPACE);
    }
  visited_string = makestring(string_find("_$visited",1));

  push_cycle_trail(Term);	
  get_list_head(Term) = visited_string;

  while (cycle_trail_top >= 0) {

    if (cycle_trail[cycle_trail_top].arg_num > cycle_trail[cycle_trail_top].arity) {
      pop_cycle_trail(Term);	
    }
    else {
      if (cycle_trail[cycle_trail_top].arg_num == 0) {
	Term = cycle_trail[cycle_trail_top].value;
      }
      else {
	//		printf("examining struct %p %d\n",clref_val(cycle_trail[cycle_trail_top].parent),cycle_trail[cycle_trail_top].arg_num);
	Term = get_str_arg(cycle_trail[cycle_trail_top].parent, cycle_trail[cycle_trail_top].arg_num);
      }
      cycle_trail[cycle_trail_top].arg_num++;
      //      printf("Term1 before %x\n",Term);
      XSB_Deref(Term);
      //      printf("Term1 after %x\n",Term);
      if ((cell_tag(Term) == XSB_LIST || cell_tag(Term) == XSB_STRUCT) && !isinternstr(Term)) {	
	if (get_list_head(Term) == visited_string) {
	  unwind_cycle_trail;
	  return TRUE;
	}
	else {
	  push_cycle_trail(Term);	
	  get_list_head(Term) = visited_string;
	}
      }
    }
  }
  //  mem_dealloc(cycle_trail,cycle_trail_size*sizeof(Cycle_Trail_Frame),OTHER_SPACE);
  return FALSE;
}

// visit term, fail if encounter a var
int ground_cyc(CTXTdeclc Cell Term, int cycle_action) { 
  Cell visited_string;

  XSB_Deref(Term);
  if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) {
    if (!isnonvar(Term) || is_attv(Term) ) { return FALSE; } else { return TRUE; }
  }

  if (cycle_trail == (CTptr) 0) {
    cycle_trail_size = TERM_TRAVERSAL_STACK_INIT;
    cycle_trail = (CTptr) mem_alloc(cycle_trail_size*sizeof(Cycle_Trail_Frame),OTHER_SPACE);
  }
  visited_string = makestring(string_find("_$visited",1));

  push_cycle_trail(Term);	
  get_list_head(Term) = visited_string;

  while (cycle_trail_top >= 0) {

    if (cycle_trail[cycle_trail_top].arg_num > cycle_trail[cycle_trail_top].arity) {
      pop_cycle_trail(Term);	
    }
    else {
      if (cycle_trail[cycle_trail_top].arg_num == 0) {
	Term = cycle_trail[cycle_trail_top].value;
      }
      else {
	 //printf("examining struct %p %d\n",clref_val(cycle_trail[cycle_trail_top].parent),cycle_trail[cycle_trail_top].arg_num);
	Term = get_str_arg(cycle_trail[cycle_trail_top].parent, cycle_trail[cycle_trail_top].arg_num);
      }
      cycle_trail[cycle_trail_top].arg_num++;
          // printf("Term1 before %p\n",Term);
      XSB_Deref(Term);
           //printf("Term1 after %p\n",Term);
      if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) {
        if (!isnonvar(Term) || is_attv(Term) ) { 
                unwind_cycle_trail;
                return FALSE; 
        }   
       }
       else {
	// printf("*clref_val(TERM) %d\n",*clref_val(Term));
	if (get_list_head(Term) == visited_string) {
           // printf("unwind_cycle_trail\n");
	  unwind_cycle_trail;
	  if (cycle_action == CYCLIC_SUCCEED) 
	    return TRUE;
	  else return FALSE;
	}
	else {
           // printf("push_cycle_trail\n");
	  push_cycle_trail(Term);	
           // printf("assign *clref_val(Term)\n");
	  get_list_head(Term) = visited_string;
	}
       }
     }
    }
  //  mem_dealloc(cycle_trail,cycle_trail_size*sizeof(Cycle_Trail_Frame),OTHER_SPACE);
  return TRUE;
}



/* a new function, not yet used, intended to implement \= without a choicepoint */
xsbBool unifiable(CTXTdeclc Cell Term1, Cell Term2) {
  CPtr *start_trreg = trreg;
  xsbBool unifies = unify(CTXTc Term1,Term2);
  while (trreg != start_trreg) {
    untrail2(trreg, ((Cell)trail_variable(trreg)));
    trreg = trail_parent(trreg);
  }
  return unifies;
}
