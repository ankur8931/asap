
#include <stdio.h>

#include "xsb_config.h"
#include "basictypes.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "psc_xsb.h"
#include "tries.h"
#include "tab_structs.h"
#include "debug_xsb.h"
#include "register.h"

#ifdef DEBUG_VERBOSE
void my_print_completion_stack(CPtr leader_ptr)
{
  int i = 0;
  EPtr eptr;
  VariantSF subg;
  CPtr temp = openreg;

  fprintf(stddbg,"openreg -> ");
  while (temp < (leader_ptr + COMPLFRAMESIZE)) {
    if ((i % COMPLFRAMESIZE) == 0) {
      fprintf(stddbg,EOFR);	
      subg = compl_subgoal_ptr(temp);
      print_subg_header(subg);
      printf("ans_list_ptr %x ans_list_tail %x\n",
	     subg_ans_list_ptr(subg),
	     subg_ans_list_tail(subg));
    }
    fprintf(stddbg,"Completion Stack %p: %lx\t(%s)",
	    temp, *temp, compl_stk_frame_field[(i % COMPLFRAMESIZE)]);
    /* only for batched
    if ((i % COMPLFRAMESIZE) >= COMPLFRAMESIZE-2) {
      for (eptr = (EPtr)*temp; eptr != NULL; eptr = next_edge(eptr)) {
	fprintf(stddbg," --> %p", edge_to_node(eptr));
	}
    } */
    fprintf(stddbg,"\n");
    temp++; i++;
  }
  fprintf(stddbg, EOS);
}

void remove_unfounded_set(CPtr leader_ptr) {

  my_print_completion_stack((CPtr) leader_ptr);

}

#endif
