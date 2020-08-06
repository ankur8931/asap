/* File:      std_cases_xsb_i.h
** Author(s): Kostis F. Sagonas
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
** $Id: std_cases_xsb_i.h,v 1.37 2012-02-24 21:06:49 dwarren Exp $
** 
*/


  case IS_ATTV:	/* r1: ?term */
    printf("is_attv in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return isattv(ptoc_tag(CTXTc 1));

  case VAR:		/* r1: ?term */
    printf("var in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return isref(ptoc_tag(CTXTc 1));
    
  case NONVAR:	/* r1: ?term */
    printf("nonvar in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return isnonvar(ptoc_tag(CTXTc 1));
    
  case ATOM:		/* r1: ?term */
    printf("atom in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return isatom(ptoc_tag(CTXTc 1));
    
  case INTEGER:	/* r1: ?term */ {
      Integer tag = ptoc_tag(CTXTc 1);
    printf("integer in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return (isointeger(tag));
  }
    
  case REAL:		/* r1: ?term */
  {
    Cell term = ptoc_tag(CTXTc 1);
    printf("real in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return isofloat(term);
  }  
  case NUMBER:	/* r1: ?term */ {
      Cell tag = ptoc_tag(CTXTc 1);
      printf("number in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
      return (xsb_isnumber(tag) || isboxedinteger(tag) || isboxedfloat(tag));
  }
  case ATOMIC: {	/* r1: ?term */
    Cell term = ptoc_tag(CTXTc 1);
    printf("atomic in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return (isatomic(term) || isboxedinteger(term) || isboxedfloat(term));
  }

  case COMPOUND: {	/* r1: ?term */
    Cell term = ptoc_tag(CTXTc 1);
    printf("compound in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return (((isconstr(term) && get_arity(get_str_psc(term))) ||
	    (islist(term))) && !isboxedfloat(term) && !isboxedinteger(term));
  }

  case CALLABLE: {	/* r1: ?term */
    Cell term = ptoc_tag(CTXTc 1);
    printf("callable in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return ((isconstr(term) && !isboxed(term)) || isstring(term) || islist(term));
  }

  case IS_LIST:	/* r1: ?term */
    printf("is_list in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return is_proper_list(ptoc_tag(CTXTc 1));
    
  case IS_MOST_GENERAL_TERM: /* r1: ?term */
    printf("is_most_general_term in std_cases_xsb_i.h This should NOT occur! This code should now be unreachable\n");
    return is_most_general_term(ptoc_tag(CTXTc 1)); 

  case FUNCTOR:	/* r1: ?term; r2: ?functor; r3: ?arity (int)	*/
    return functor_builtin(CTXT);
    
  case ARG:	/* r1: +index (int); r2: +term; r3: ?arg (term) */
    return arg_builtin(CTXT);
    
  case UNIV:	/* r1: ?term; r2: ?list	*/
    return univ_builtin(CTXT);
    
  case HiLog_ARG:	/* r1: +index (int); r2: +term; r3: ?arg (term) */
    return hilog_arg(CTXT);

  case HiLog_UNIV:	/* r1: ?term; r2: ?list	*/
    break;
    
  case ATOM_CHARS:	/* r1: ?term; r2: ?character symbol list	*/
    return atom_to_list(CTXTc ATOM_CHARS);
  case ATOM_CODES:	/* r1: ?term; r2: ?character ascii code list	*/
    return atom_to_list(CTXTc ATOM_CODES);
    
  /* number_chars should be redefined to return digit-atoms */
  case NUMBER_CHARS:	/* r1: ?term; r2: ?character symbol list	*/
    return number_to_list(CTXTc NUMBER_CHARS);
  case NUMBER_CODES:	/* r1: ?term; r2: ?character code list	*/
    return number_to_list(CTXTc NUMBER_CODES);
  case NUMBER_DIGITS:	/* r1: ?term; r2: ?digit list	*/
    return number_to_list(CTXTc NUMBER_DIGITS);
    

   case GROUND_CYC: {
     return ground_cyc(CTXTc (Cell) (reg + 1),(int)ptoc_int(CTXTc 2));
     break;
   }

  case PUT: {	/* r1: +integer	*/
    Cell term = ptoc_tag(CTXTc 1);
    if (isointeger(term)) {
      putc((int)oint_val(term), fileptr(pflags[CURRENT_OUTPUT]));
    } else {
      if (isnonvar(term)) xsb_type_error(CTXTc "integer",term,"put/1",1);
      else xsb_instantiation_error(CTXTc "put/1",1);
    }
    break;
  }
  case TAB: {	/* r1: +integer	*/
    Cell term = ptoc_tag(CTXTc 1);
    if (isointeger(term)) {
      Integer  i;
      for (i=1; i<=oint_val(term); i++)
	putc(' ', fileptr(pflags[CURRENT_OUTPUT]));
    } else {
      if (isnonvar(term)) xsb_type_error(CTXTc "integer",term,"tab/1",1);
      else xsb_instantiation_error(CTXTc "tab/1",1);
    }
    break;
  }

   case IS_CYCLIC: {
     return is_cyclic(CTXTc (Cell) (reg + 1));
     break;
   }

   case TERM_SIZE: {
     ctop_int(CTXTc 2, term_sizew(CTXTc ptoc_tag(CTXTc 1)));
     break;
   }

   case TERM_SIZE_LIMIT: {
     return(term_size_limit(CTXT));
     break;
   }

   case TERM_DEPTH: {
     ctop_int(CTXTc 2, term_depth(CTXTc ptoc_tag(CTXTc 1)));
     break;
   }

  case SORT:		/* r1: +list of terms; r2: ?sorted list of terms */
  return sort(CTXT);
    
  case KEYSORT:	/* r1: +list of terms of the form Key-Value;	*/
    /* r2: ?sorted list of terms			*/
   return keysort(CTXT);

  case PARSORT:	/* r1: +list of terms of the form Key-Value;	*/
    /* r2: +list of sort paramater specs			*/
    /* r3: ?sorted list of terms			*/
   return parsort(CTXT);

   case IS_ACYCLIC: {
     return !(is_cyclic(CTXTc (Cell) (reg + 1)));
     break;
   }

