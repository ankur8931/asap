/* File:      wfs_xsb_i.h
** Author(s): Kostis Sagonas
** Documentation added by TLS 02/02
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
** $Id: wfs_xsb_i.h,v 1.25 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


/* special debug includes */
#include "debugs/debug_delay.h"

/*----------------------------------------------------------------------*/

#define unvisit(leader) \
    for (csf = leader; csf >= (ComplStackFrame)openreg; csf--) \
      compl_visited(csf) = FALSE

/*----------------------------------------------------------------------*/

#define edge_alloc_chunk_size	(1024 * sizeof(struct ascc_edge))

static char *edge_space_chunk_ptr = NULL;

static EPtr free_edges = NULL, free_edge_space = NULL, top_edge_space = NULL;

void abolish_edge_space(void)
{
    char *t;
 
    while (edge_space_chunk_ptr) {
      t = *(char **)edge_space_chunk_ptr;
      mem_dealloc(edge_space_chunk_ptr,edge_alloc_chunk_size+sizeof(Cell),TABLE_SPACE);
      edge_space_chunk_ptr = t;
    }
    free_edges = free_edge_space = top_edge_space = NULL;
}

static EPtr alloc_more_edge_space(void)
{
    char *t;

    if ((t = (char *)mem_alloc(edge_alloc_chunk_size+sizeof(Cell),TABLE_SPACE)) == NULL)
      xsb_abort("No space to allocate more edges for SCC detection");
    *(char **)t = edge_space_chunk_ptr;
    edge_space_chunk_ptr = t;
    free_edge_space = (EPtr)(t+sizeof(Cell));
    top_edge_space = (EPtr)(t+edge_alloc_chunk_size+sizeof(Cell));
    return free_edge_space++;
}

#define Free_Edge(theEdge) \
    next_edge(theEdge) = free_edges; \
    free_edges = theEdge

#define New_Edge(theEdge,FromEptr,ToNode) \
    if (free_edges) {\
      theEdge = free_edges; \
      free_edges = next_edge(free_edges); \
    } else { \
      if (free_edge_space < top_edge_space) { \
        theEdge = free_edge_space++; \
      } else { \
        theEdge = alloc_more_edge_space(); \
      } \
    } \
    edge_to_node(theEdge) = (ComplStackFrame) ToNode; \
    next_edge(theEdge) = FromEptr; \
    FromEptr = theEdge

/* Add a dependency graph edge, and its transpose as long as the
   parent node csf2 (DG "from" node / DGT "to" node) is not completed
   and lies within the ASCC */

#define add_ascc_edges(subgoal,cs_frame,leader_cs_frame)	\
    csf2 = (ComplStackFrame)subg_compl_stack_ptr(subgoal);	\
    if (leader_cs_frame >= csf2  && !is_completed(subgoal)) {	\
      New_Edge(eptr, compl_DGT_edges(cs_frame), csf2);		\
      New_Edge(eptr, compl_DG_edges(csf2), cs_frame);		\
    }

static void reclaim_edge_space(ComplStackFrame csf_ptr)
{
    EPtr e, eptr;

    e = compl_DG_edges(csf_ptr);
    while (e != NULL) {
      eptr = e;
      e = next_edge(e);
      Free_Edge(eptr);
    }
    compl_DG_edges(csf_ptr) = NULL;
    e = compl_DGT_edges(csf_ptr);
    while (e != NULL) {
      eptr = e;
      e = next_edge(e);
      Free_Edge(eptr);
    }
    compl_DGT_edges(csf_ptr) = NULL;
}

/*----------------------------------------------------------------------*/
/* These macros abstract the fact that SLG-WAM and CHAT construct the	*/
/* subgoal dependency graph by looking at different data structures.	*/
/*----------------------------------------------------------------------*/

#define first_consumer(SUBG,CSF) (subg_pos_cons(SUBG))
#define ptcp_of_gen(SUBG,CSF)	 ((VariantSF)(tcp_ptcp(subg_cp_ptr(SUBG))))
#define ptcp_of_cons(CONS_CP)	 ((VariantSF)(nlcp_ptcp(CONS_CP)))
#define ptcp_of_csusp(CSUSP_CP)	 ((VariantSF)(csf_ptcp(CSUSP_CP)))

/*----------------------------------------------------------------------*/

static inline void construct_dep_graph(CTXTdeclc ComplStackFrame leader_compl_frame)
{
    EPtr eptr;
    CPtr asf, nsf;
    VariantSF p, source_subg;
    ComplStackFrame csf1, csf2;

    csf1 = leader_compl_frame;
    while (csf1 >= (ComplStackFrame)openreg) {
      source_subg = compl_subgoal_ptr(csf1);
      if (!is_completed(source_subg)) {
	/*--- find the edge of the DepGraph due to the generator node ---*/
	if ((p = ptcp_of_gen(source_subg,csf1)) != NULL) {
	  add_ascc_edges(p, csf1, leader_compl_frame);
	}
	/*--- find the edges of the DepGraph due to the consumers ---*/
	for (asf = first_consumer(source_subg,csf1);
	     asf != NULL; asf = nlcp_prevlookup(asf)) {
	  if ((p = ptcp_of_cons(asf)) != NULL) {
	    add_ascc_edges(p, csf1, leader_compl_frame);
	  }
	}
	/*--- find the edges of the DepGraph due to the suspended nodes ---*/
	for (nsf = subg_compl_susp_ptr(source_subg);
	     nsf != NULL; nsf = csf_prevcsf(nsf)) {
	  if ((p = ptcp_of_csusp(nsf)) != NULL) {
	    add_ascc_edges(p, csf1, leader_compl_frame);
	  }
	}
      }
      csf1 = (ComplStackFrame)next_compl_frame(csf1);
    }
#ifdef VERBOSE_COMPLETION
    xsb_dbgmsg((LOG_DEBUG,"! Constructed the edges of the DepGraph"));
#endif
}

/*----------------------------------------------------------------------*/
/* The following function handles negation for Batched Scheduling.	*/
/*----------------------------------------------------------------------*/

static void batched_compute_wfs(CTXTdeclc CPtr leader_compl_frame, 
				CPtr leader_breg, 
				VariantSF leader_subg)
{  
  CPtr ComplStkFrame; /* CopyFrame */
  xsbBool sccs_needed;
  VariantSF curr_subg;
  CPtr cont_breg = leader_breg;

/*----------------------------------------------------------------------*/
  /* Perform a check whether exact completion is needed (sccs_needed).
     For subgoals that are already marked as (early) completed,
     preclude their completion suspension frames from ever being
     secheduled and the memory occupied by their chat areas is
     properly reclaimed. 
  */

  sccs_needed = FALSE;
  ComplStkFrame = leader_compl_frame;
  while (ComplStkFrame >= openreg) {
    curr_subg = compl_subgoal_ptr(ComplStkFrame);
    if (is_completed(curr_subg)) {
      subg_compl_susp_ptr(curr_subg) = NULL;
    } else {
      if (subg_compl_susp_ptr(curr_subg) != NULL) sccs_needed = TRUE;
    }
    ComplStkFrame = next_compl_frame(ComplStkFrame);
  }

  /**********************************************************************/
  /* NOTE: many of the following assume that leader_compl_frame
   * remains unchanged */
/*----------------------------------------------------------------------*/
  /* The general algorithm for exact SCC check is defined in the 1998
     SLGWAM TOPLAS paper. Below we will 

     1) Construct a dependency graph (DG) + its transpose (DGT) via
     construct_dep_graph((ComplStackFrame)leader_compl_frame).  Nodes
     of this graph are formed via cells in compretion stack frames
     compl_DG_edges, and compl_DGT_edges.
     
     2) Traverse that dependency graph to find an independent SCC,
     $SCC_1$ This is the combined purpose of DFS_DGT(), unvisit() and
     find_independent_scc() (see the commentary on DFS_DGT in
     scc_xsb.c).  At the end of this process, subgoals in the
     independent SCC have compl_visited marked as TRUE

     3) Check the independent SCC to see whether there is a negative
     loop --- non_lrd_stratified is TRUE if there is such a loop.
     This is done directly in this function, for each subgoal $S$ in
     the independent SCC, the completion_suspension frames if $S$ are
     traversed to see whether there is a ptcp (root subgoal) pointer
     to some subgoal $S'$ in the independet SCC.  If so,
     non_lrd_stratified is set to true, and the $S$ is marked as
     delayed.  
  */

  if (sccs_needed) {
    xsbBool found;
    CPtr nsf;
    CPtr CopyFrame;
    ComplStackFrame csf, max_finish_csf;
    xsbBool non_lrd_stratified;

#ifdef VERBOSE_COMPLETION
    xsb_dbgmsg((LOG_DEBUG,"\t===> SCC detection is needed...(%d subgoals in ASCC)...",
	       (int)((leader_compl_frame-openreg)/COMPLFRAMESIZE+1)));
    print_completion_stack();
#endif

    construct_dep_graph(CTXTc (ComplStackFrame)leader_compl_frame);
    dbg_print_completion_stack(LOG_COMPLETION);

    max_finish_csf = DFS_DGT(CTXTc (ComplStackFrame)leader_compl_frame);
    xsb_dbgmsg((LOG_COMPLETION, 
	       "! MAX FINISH_SUBGOAL AT COMPL STACK: %p",max_finish_csf));
    /* mark as not visited all subgoals in the completion stack
     * below leader_compl_frame */
    unvisit((ComplStackFrame)leader_compl_frame);

    /* mark as visited all subgoals in the same SCC as max_finish_csf 
     * by traversing the SDG */
    find_independent_scc(max_finish_csf);
    
    dbg_print_completion_stack(LOG_COMPLETION);

    /* Perform a LRD stratification check of the program-query pair	*/
    /* and classify the completion suspensions as stratified or not.	*/
    ComplStkFrame = leader_compl_frame;
    non_lrd_stratified = FALSE;
    
    while (ComplStkFrame >= openreg) {
      CPtr    susp_csf;
      VariantSF susp_subgoal;

      curr_subg = compl_subgoal_ptr(ComplStkFrame);
      if (!is_completed(curr_subg)) {
	if (compl_visited(ComplStkFrame) != FALSE) {
	  curr_subg = compl_subgoal_ptr(ComplStkFrame);  
	  for (nsf = subg_compl_susp_ptr(curr_subg);
	       nsf != NULL; nsf = csf_prevcsf(nsf)) {
	    if ((susp_subgoal = ptcp_of_csusp(nsf)) != NULL) {
	      susp_csf = subg_compl_stack_ptr(susp_subgoal);      
	      if (!is_completed(susp_subgoal) && 
		  susp_csf <= leader_compl_frame &&
		  compl_visited(susp_csf) != FALSE) {
		/*--- The suspended subgoal is in the completable SCC ---*/
		mark_delayed(ComplStkFrame, susp_csf, nsf);
		non_lrd_stratified = TRUE;
		xsb_dbgmsg((LOG_DELAY, "\t   Subgoal "));
		dbg_print_subgoal(LOG_DELAY,stddbg,(VariantSF)susp_subgoal);
		xsb_dbgmsg((LOG_DELAY, " depends negatively on subgoal "));
		dbg_print_subgoal(LOG_DELAY, stddbg, curr_subg);
		xsb_dbgmsg((LOG_DELAY, "\n"));
	      } /*  no completed susp_subg */
	    }
	  } /* for each nsf */
	} /* not visited */
      } /* skip completed subgoals in the completion stack */
      ComplStkFrame = next_compl_frame(ComplStkFrame);
    } /* while */
  
/* #define SERIOUS_DEBUGGING_NEEDED	*/
#ifdef SERIOUS_DEBUGGING_NEEDED
    print_completion_stack();
#endif

/*----------------------------------------------------------------------*/
    /* 
       In the case where there is no loop through negation in the
       independent SCC (non_lrd_stratified == FALSE, then the SLGWAM
       needs to find the continuation, the youngest subgoal of the
       ASCC that will not be completed.  

       If the independent SCC has a loop through negation, the
       continuation pointer will be the leader breg, otherwise, it
       will be the breg of the youngest subgoal not in the independent
       ASCC (I dont understand this, according to my thinking it might
       as well be the leader, as we will have to do SCC checks, etc
       again.

       One trick that is used is that compl_visited can now be set to
       DELAYED (-1), FALSE (0) or true (1).  The continuation will be
       set to the breg of the first subgoal whose compl_visited value
       is neither FALSE or delayed.  

       While I have not verified this, my understanding is that
       starting with openreg, there will be a contiguous segment of
       compl stack frames that are TRUE, followed by a csf that is
       either FALSE or DELAYED.  Apparently it should be DELAYED only
       if the independent SCC contains aloop through negation.
       Therefore, I dont see why there is a <= FALSE below --
       according to my theory (which could be wrong) it should ==
       FALSE, since non_lrd_stratified == FALSE.
    */

    if (non_lrd_stratified == FALSE) {
      found = FALSE;
      for (ComplStkFrame = openreg;
	   !found && ComplStkFrame <= leader_compl_frame;
	   ComplStkFrame = prev_compl_frame(ComplStkFrame)) {
	if (compl_visited(ComplStkFrame) <= FALSE) {
	  cont_breg = subg_cp_ptr(compl_subgoal_ptr(ComplStkFrame));
	  breg = cont_breg;
	  xsb_dbgmsg((LOG_COMPLETION,
		     "------ Setting TBreg to %p...", cont_breg));
	  found = TRUE;
	}
      }
       if (!found) {	/* case in which the ASCC contains only one SCC */
			/* and all subgoals will be completed (no delay) */
	cont_breg = tcp_prevbreg(leader_breg);
      }
    } else {	/* the chosen SCC has a loop through negation */
      for (ComplStkFrame = openreg; ComplStkFrame <= leader_compl_frame;
	   ComplStkFrame = prev_compl_frame(ComplStkFrame)) {
	if (compl_visited(ComplStkFrame) != FALSE)
	  compl_visited(ComplStkFrame) = DELAYED;
      }
      cont_breg = subg_cp_ptr(compl_subgoal_ptr(leader_compl_frame));
      breg = cont_breg;
      xsb_dbgmsg((LOG_COMPLETION, "------ Setting TBreg to %p...", cont_breg));
    }
    
/*----------------------------------------------------------------------*/
    /* 
       The next loop has two functions.  First, any subgoals that are
       completely evaluated (those that are visited but not delayed)
       are marked as completed.  In addition, breg will be set to
       point to the cP for the the youngest subgoal that has been
       visited (delayed or not).  The failure continuation of this CP
       in turn will be the continuation obtained above.  

       One point to make is that reclaim_incomplete_table_structs()
       Clears up some table structures for subsumptive tabling, and
       presumably the answer list for variant tabling.  It also frees
       consumer choice points in CHAT.  In either case, frozen local
       stack, heap, (and trail & cp space in the SLGWAM) will only be
       reclaimed upon completion of the leader.       
    */

 {
   for (ComplStkFrame = leader_compl_frame; ComplStkFrame >= openreg;
	ComplStkFrame = next_compl_frame(ComplStkFrame)) {
     if (compl_visited(ComplStkFrame) != FALSE) {
       curr_subg = compl_subgoal_ptr(ComplStkFrame);
       if (compl_visited(ComplStkFrame) != DELAYED) {
#ifdef CONC_COMPL
      	  if( !is_completed(curr_subg) )
          {
        	mark_as_completed(curr_subg);
		if( IsSharedSF( curr_subg ) )
        		WakeDependentThreads(th, curr_subg);
     	  }
#else
          mark_as_completed(curr_subg);
#endif
	 reclaim_incomplete_table_structs(curr_subg);
	 if (neg_simplif_possible(curr_subg)) {
	   simplify_neg_fails(CTXTc curr_subg);
	 }
       }
       
/*----------------------------------------------------------------------*/
       /* Now, its time to schedule the completion suspensions found
	  for the youngest subgoal in the independent SCC, as found in
	  the previous loop.  

	  There are a couple of anomalies in this procedure, for the
	  SLGWAM.  The first is that we have completion suspension as
	  "single-dispatch", that is only one completion suspension is
	  scheduled for each dependency check --- I thought that we
	  used to dispatch completion_suspension frames for all
	  subgoals in the independent SCC???  Second, there is a
	  completion_suspension CP that is created.
	  Completion_suspension CPs, as opposed to completion
	  suspension frames, are archaic remnants of single-stack
	  scheduling.  I believe their creation here is unnecessary.
	  In principle, we should be able to simply set the breg to
	  the first of the completion_suspension Frames.
       */

       if ((nsf = subg_compl_susp_ptr(curr_subg)) != NULL) {
	 CPtr min_breg;
	 
	 set_min(min_breg, breg, bfreg);
	 if (compl_visited(ComplStkFrame) != DELAYED) {
	   save_compl_susp_cp(min_breg, cont_breg, nsf);
	   breg = min_breg;
	   /*-- forget these completion suspensions --*/
	   subg_compl_susp_ptr(curr_subg) = NULL;
#ifdef VERBOSE_COMPLETION
	   xsb_dbgmsg((LOG_DEBUG,"------ Setting Breg to %p...", breg));
#endif
	 } else {	/* unsuspend only those suspensions that are delayed */
	   CPtr dnsf = NULL, ndnsf = NULL;
	   CPtr head_dnsf = NULL, head_ndnsf = NULL;
	   while (nsf != NULL) {	/* partition into two lists */
	     if (csf_neg_loop(nsf) == FALSE) {
	       if (ndnsf == NULL) head_ndnsf = nsf; 
	       else csf_prevcsf(ndnsf) = nsf;
	       ndnsf = nsf;
	       nsf = csf_prevcsf(nsf);
	       csf_prevcsf(ndnsf) = NULL;
	     } else {
	       if (dnsf == NULL) head_dnsf = nsf; 
	       else csf_prevcsf(dnsf) = nsf;
	       dnsf = nsf;
	       nsf = csf_prevcsf(nsf);
	       csf_prevcsf(dnsf) = NULL;
	     }
	   }
	   if (head_dnsf != NULL) {
	     save_compl_susp_cp(min_breg, cont_breg, head_dnsf);
	     breg = min_breg;
	   }
	   subg_compl_susp_ptr(curr_subg) = head_ndnsf;
	 }
	 cont_breg = breg; /* So that other Compl_Susp_CPs can be saved. */
       }
     }
   }
 }
    xsb_dbgmsg((LOG_COMPLETION, "------ Completed the chosen SCC..."));
/*----------------------------------------------------------------------*/
    /* Finally, compact the Completion Stack (and reclaim edge 
       space for the dependency graphs).
       
       Might be useful to only copy the frame if CopyFrame is not
       == the ComplStkFrame
    */

    ComplStkFrame = CopyFrame = leader_compl_frame; 
    while (ComplStkFrame >= openreg) {
      curr_subg = compl_subgoal_ptr(ComplStkFrame);
      reclaim_edge_space((ComplStackFrame)ComplStkFrame);
      if (!is_completed(curr_subg)) {
	subg_compl_stack_ptr(curr_subg) = CopyFrame;
	/* the macro below also updates CopyFrame */
	compact_completion_frame(CopyFrame, ComplStkFrame, curr_subg);
      } else { /* this may be done 2x! */
	reclaim_incomplete_table_structs(curr_subg);
      }
      ComplStkFrame = next_compl_frame(ComplStkFrame);
    }
    openreg = prev_compl_frame(CopyFrame);

/*----------------------------------------------------------------------*/

    /* 
       This next loop chains some of the TCPs, but does not form what
       I would think of as a scheduling chain -- it simply removes any
       pointers to completed subgoals whose space has been reclaimed.

       The line

    tcp_prevbreg(subg_cp_ptr(compl_subgoal_ptr(leader_compl_frame))) = 
      tcp_prevbreg(subg_cp_ptr(leader_subg));

      looks anomalous at first, but the leader compl_frame may have
      been compacted away, thus its continuation must be preserved for
      whoever is the new leader.

      However, I dont believe that the "for" loop is necessary, as it
      doesn't matter what the tcp_breg is for choice points that are
      not leaders (and if they are promoted to leaders, their
      tcp_prevbreg value will be adjusted anyway.
     */

    tcp_prevbreg(subg_cp_ptr(compl_subgoal_ptr(leader_compl_frame))) = 
      tcp_prevbreg(subg_cp_ptr(leader_subg));
    for (ComplStkFrame = next_compl_frame(leader_compl_frame);
	 ComplStkFrame >= openreg;
	 ComplStkFrame = next_compl_frame(ComplStkFrame)){
      tcp_prevbreg(subg_cp_ptr(compl_subgoal_ptr(ComplStkFrame))) = 
	subg_cp_ptr(compl_subgoal_ptr(prev_compl_frame(ComplStkFrame)));
    }
  } /* if sccs_needed */
  else { /* sccs not needed */
    ComplStkFrame = leader_compl_frame;
    while (ComplStkFrame >= openreg) {
      curr_subg = compl_subgoal_ptr(ComplStkFrame);
#ifdef CONC_COMPL
      if( !is_completed(curr_subg) )
      {
        mark_as_completed(curr_subg);
	if( IsSharedSF( curr_subg ) )
        	WakeDependentThreads(th, curr_subg);
      }
#else
      mark_as_completed(curr_subg);
#endif
      if (neg_simplif_possible(curr_subg)) {
	simplify_neg_fails(CTXTc curr_subg);
      }
      ComplStkFrame = next_compl_frame(ComplStkFrame);
    }
    
    ComplStkFrame = leader_compl_frame;
    while (ComplStkFrame >= openreg) {
      curr_subg = compl_subgoal_ptr(ComplStkFrame);
      reclaim_incomplete_table_structs(curr_subg);
      ComplStkFrame = next_compl_frame(ComplStkFrame);
    }
    /* point openreg to first empty space */
    openreg = prev_compl_frame(leader_compl_frame);	
  }
  
  xsb_dbgmsg((LOG_COMPLETION, "------ Completed an ASCC..."));
} /* compute_wfs() */

/*----------------------------------------------------------------------*/
