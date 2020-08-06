/* File:      dis.c
** Author(s): Warren, Swift, Xu, Sagonas
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
** $Id: dis.c,v 1.37 2013-05-06 21:10:24 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <string.h>

#include "auxlry.h"
#include "context.h"
#include "psc_xsb.h"
#include "hash_xsb.h"
#include "loader_xsb.h"
#include "cell_xsb.h"
#include "inst_xsb.h"
#include "builtin.h"
#include "memory_xsb.h"
#include "flags_xsb.h"
#include "tries.h"
#include "tab_structs.h"
#include "cell_xsb_i.h"

extern void print_term(FILE *, Cell, byte, long);

/* --------------- The following are working variables ----------------	*/

extern Cell builtin_table[BUILTIN_TBL_SZ][2];
extern TIFptr get_tip_or_tdisp(Psc);

/* Include these so that the Gnu C Compiler does not complain */
void dis_data(FILE *);
void dis_text(FILE *);
static void dis_data_sub(FILE *, Pair *, char *);

void dis(xsbBool distext)
{  
   dis_data(stdout);
   if (distext) dis_text(stdout);
   fflush(stdout);
   fclose(stdout); 
}

void dis_data(FILE *filedes)
{
	int i;
	Pair *temp_ptr;
	Psc psc_ptr;
	char *modname;

	temp_ptr = (Pair *)(&flags[MOD_LIST]);
	while(*temp_ptr) {
	   psc_ptr = (*temp_ptr)->psc_ptr;
	   modname = get_name(psc_ptr);
	   if (get_type(psc_ptr))	/* 00000100 */
	     fprintf(filedes, "module('%s',loaded).\n",modname);
	   else 
	     fprintf(filedes, "module('%s', unloaded).\n", modname);
	   if (strcmp(modname,"global")==0)
	   	for(i=0; i < (int)symbol_table.size; i++) {
		  if ( symbol_table.table[i] ) {
/* 		    fprintf(filedes, "... ... BUCKET NO. %d\n", i); */
		    dis_data_sub(filedes, (Pair *)(symbol_table.table + i),modname);
		  }
		}
	   else if (strcmp(modname,"usermod")==0) 
	     fprintf(filedes, "equiv(usermod,global).\n");
	   else 
	     dis_data_sub(filedes, (Pair *)&get_data(psc_ptr),modname);
	   fprintf(filedes, "\n");
	   temp_ptr = &((*temp_ptr)->next);
	}
}

static void dis_data_sub(FILE *filedes, Pair *chain_ptr, char* modname)
{
   Psc temp;

   while (*chain_ptr) {
	temp = (*chain_ptr)->psc_ptr;
	fprintf(filedes,"entry('%s',",modname);
	fprintf(filedes, "%p,", temp);
	fflush(filedes);
	fprintf(filedes, "'%s'", get_name(temp));
	fprintf(filedes, "/%d,", get_arity(temp));
	switch(get_type(temp)) {
	    case T_PRED: fprintf(filedes, "'PRED',"); break;
	    case T_DYNA: fprintf(filedes, "'DYNA',"); break;
	    case T_ORDI: fprintf(filedes, "'ORDI',"); break;
	      //	    case T_FILE: fprintf(filedes, "'FILE',"); break;
	    case T_MODU: fprintf(filedes, "'MODU',"); break;
	    case T_FORN: fprintf(filedes, "'FORN',"); break;
	    case T_UDEF: fprintf(filedes, "'UDEF',"); break;
	    default:	 fprintf(filedes, "\'????\',"); break;
	}
	switch(get_env(temp)) {
	    case T_VISIBLE:  fprintf(filedes, "'VISIBLE',"); break;
	    case T_HIDDEN:   fprintf(filedes, "'HIDDEN',"); break;
	    case T_UNLOADED: fprintf(filedes, "'UNLOADED',"); break;
	    default:	     fprintf(filedes, "error_env,"); break;
	}
	// TLS: should T_DYNA be checked, also???
	if (get_type(temp) == T_PRED) {
	  if (get_tip_or_tdisp(temp) == NULL) 
	    fprintf(filedes, "'UNTABLED',"); 
	  else 
	    fprintf(filedes, "'TABLED',");
	} else
	  fprintf(filedes, "'n/a',");
	fprintf(filedes, "%p).\n", get_ep(temp));  /* dsw ???? */
	chain_ptr = &((*chain_ptr)->next);
   } /* while */
}

Integer inst_cnt = 0;

CPtr print_inst(FILE *fd, CPtr inst_ptr)
{
    Cell instr ;
    CPtr loc_pcreg ;
    int i,a;
    Psc psc;

    loc_pcreg = (CPtr) inst_ptr;
    inst_cnt++;
    fprintf(fd,"inst("),
      //      fprintf(fd,"%lld, %lld, ", inst_cnt, (Integer)loc_pcreg);
      // TLS: 15/09 -- changed to print out address as %p so that it accords with ptrs.  did not check on 32-bit
      //      fprintf(fd,"%" Intfmt ", %" Intfmt " ", inst_cnt, (Integer)loc_pcreg);
    fprintf(fd,"%" Intfmt ", %p ", inst_cnt, loc_pcreg);
    instr = cell(loc_pcreg++) ;
/* We want the instruction string printed out below.  
 * Someday we should ANSI-fy it. 
 */
    fprintf(fd, "%s",(char *)inst_table[cell_opcode(&instr)][0]);
    a = 1 ; /* current operand */
    for (i=1; i<=4; i++) {
	switch (inst_table[cell_opcode(&instr)][i]) {
	 case A:
	   if (cell_opcode(&instr) == (byte) builtin) {
	     a++;
	     fprintf(fd, ", '%d'", cell_operand3(&instr));
	     fprintf(fd, ", '%s'", 
		     (char *)builtin_table[cell_operand3(&instr)][0]);
	   } else 
	     fprintf(fd, ", %d", cell_operandn(&instr,a++));
	   break;
	 case V:
	   fprintf(fd, ", %d", cell_operandn(&instr,a++));
	   break;
	 case R:
	   fprintf(fd, ", r%d", cell_operandn(&instr,a++));
	   break;
	 case T:
	   fprintf(fd, ", 0x%lx", (unsigned long) cell(loc_pcreg++));
	   break;
	 case P:
	   a++;
	   break;
	 case S:
	   if (cell_opcode(&instr) == (byte) call ||
	       cell_opcode(&instr) == (byte) xsb_execute) {
	     fprintf(fd, ", 0x%lx", (unsigned long) *loc_pcreg);
	     psc = (Psc) cell(loc_pcreg++);
	     fprintf(fd,", /('%s',%d)", get_name(psc), get_arity(psc));
	   }
	   else
	     fprintf(fd, ", 0x%lx", (unsigned long) cell(loc_pcreg++));
	   break;
	 case H:
	   fprintf(fd, ", ");
	   print_term(fd, cell(loc_pcreg++), 1, (long)flags[WRITE_DEPTH]);
	   break;
	 case C:
	 case L:
	 case G:
	   fprintf(fd, ", 0x%lx", (unsigned long) cell(loc_pcreg++));
	   break;
	 case I:
	 case N:
	   fprintf(fd, ", %ld", (unsigned long) cell(loc_pcreg++));
	   break;
	 case B:
	   fprintf(fd, ", %" Intfmt, (Integer) int_val(cell(loc_pcreg)));
	   loc_pcreg++;
	   break;
	 case F:
	   fprintf(fd, ", %f", ofloat_val(cell(loc_pcreg)));
	   loc_pcreg++;
	   break;
	 case PP:
	   a += 2;
	   break;
	 case PPP:
	   break;
	 case PPR:
	   fprintf(fd, ", r%d", cell_operand3(&instr));
	   break;
	 case RRR:
	   fprintf(fd, ", r%d", cell_operand1(&instr));
	   fprintf(fd, ", r%d", cell_operand2(&instr));
	   fprintf(fd, ", r%d", cell_operand3(&instr));
	   break;
	 case X:
	   break;
	 default:
	   break;
	}  /* switch */
	/*if (cell_opcode(&instr) == noop) loc_pcreg += 2 * *(loc_pcreg-1); */
	if (cell_opcode(&instr) == noop) loc_pcreg += cell_operand3(&instr)/2; /* ?!@% */
	else if (cell_opcode(&instr) == dynnoop) loc_pcreg += cell_operand3(&instr)/2; /* ?!@% */
    } /* for */
    fprintf(fd, ")");
    fflush(fd);
    return loc_pcreg;
} /* print_inst */


void dis_text(FILE * filedes)
{
   pseg   this_seg;
   pindex index_seg ;
   CPtr   endaddr, inst_addr2 ;
   int comma;

   fprintf(filedes, "\n/*text below\t\t*/\n\n");
   this_seg = (pseg) inst_begin_gl;
   while (this_seg) {		/* repeat for all text segment */
      fprintf(filedes, "segment([\n");
      endaddr = (CPtr) ((pb) seg_hdr(this_seg) + seg_size(this_seg)) ;
      inst_addr2 = seg_text(this_seg);
      comma = 0;
      while (inst_addr2<endaddr) {
	if (comma) 
	  fprintf(filedes,", \n");
	comma = 1;
	inst_addr2 = print_inst(filedes, inst_addr2);
      }
      index_seg = seg_index(this_seg);
      while (index_seg) {
	inst_addr2 = i_block(index_seg);
	endaddr = (CPtr)((pb)index_seg + i_size(index_seg));
	if (cell_opcode(i_block(index_seg)) == try ||
            cell_opcode(i_block(index_seg)) == tabletry ||
	    cell_opcode(i_block(index_seg)) == tabletrysingle) {	
	                                           /* is try/retry/trust */
	  while (inst_addr2<endaddr) {
	    if (comma) 
	      fprintf(filedes,", \n");
	    comma = 1;
	    inst_addr2 = print_inst(filedes, inst_addr2);
	  }
	} else {					/* is hash table */
	  if (comma) 
	    fprintf(filedes,", \n");
	  fprintf(filedes, "     hash_table([\n");
	  comma = 0;
	  while (inst_addr2<endaddr) {
	    if (comma) {
	      fprintf(filedes, ", \n");
	    }
	    comma = 1;
	    fprintf(filedes, 
		    "          hash_entry(0x%p,0x%lx)", 
		    inst_addr2, 
		    (unsigned long) cell(inst_addr2));
	    inst_addr2 ++;
	  }
	  fprintf(filedes, "])");
	}
	index_seg = i_next(index_seg);
      }
      fprintf(filedes, "]).\n");
      this_seg = seg_next(this_seg);
   }  
}

