
static 
#if !defined(WIN_NT)
inline 
#endif
#ifdef CONC_COMPL
/* can't perform early completion for CONC_COMPL shared tables */
void perform_early_completion(CTXTdeclc VariantSF ProdSF,CPtr ProdCPF) {
    if( IsPrivateSF(ProdSF) )			
    {   if (tcp_pcreg(ProdCPF) != (byte *) &answer_return_inst) 
      	    tcp_pcreg(ProdCPF) = (byte *) &check_complete_inst; 
        mark_as_completed(ProdSF)				
    }
#else
void perform_early_completion(CTXTdeclc VariantSF ProdSF,CPtr ProdCPF) {
  //  printf("performing ec for ");print_subgoal(stddbg,ProdSF);printf("\n");
  if (tcp_pcreg(ProdCPF) != (byte *) &answer_return_inst) 	
    tcp_pcreg(ProdCPF) = (byte *) &check_complete_inst;   	
  mark_as_completed(ProdSF);					
  if (flags[CTRACE_CALLS] && !subg_forest_log_off(ProdSF))  { 
    sprint_subgoal(CTXTc forest_log_buffer_1,0,ProdSF);     
    fprintf(fview_ptr,"cmp(%s,ec,%d).\n",forest_log_buffer_1->fl_buffer,
	    ctrace_ctr++);
  }
  if ( flags[EC_REMOVE_SCC] && is_leader(ProdSF)) { 
    remove_incomplete_tries(CTXTc subg_compl_stack_ptr(ProdSF));
    subg_pos_cons(ProdSF) = 0;
    subg_compl_susp_ptr(ProdSF) = 0;  /* ec'd -- neg must fail */
  }
}
#endif
