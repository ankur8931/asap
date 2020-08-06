/* File:      call_graph_xsb.c
** Author(s): Diptikalyan Saha, C. R. Ramakrishnan, Swift
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
** 
** 
*/
 
#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Special debug includes */
/* Remove the includes that are unncessary !!*/
#include "hashtable.h"
#include "hashtable_itr.h"
#include "debugs/debug_tries.h"
#include "auxlry.h"
#include "context.h"   
#include "cell_xsb.h"   
#include "inst_xsb.h"
#include "psc_xsb.h"
#include "heap_xsb.h"
#include "flags_xsb.h"
#include "deref.h"
#include "memory_xsb.h" 
#include "register.h"
#include "binding.h"
#include "trie_internals.h"
#include "tab_structs.h"
#include "choice.h"
#include "subp.h"
#include "cinterf.h"
#include "error_xsb.h"
#include "tr_utils.h"
#include "hash_xsb.h" 
//#include "call_xsb_i.h"
#include "tst_aux.h"
#include "tst_utils.h"
#include "token_xsb.h"
#include "call_graph_xsb.h"
#include "thread_xsb.h"
#include "loader_xsb.h" /* for ZOOM_FACTOR, used in stack expansion */
#include "tries.h"
//#include "tr_utils.h"
#include "debug_xsb.h"

//#define INCR_DEBUG
//#define INCR_DEBUG1
//#define INCR_DEBUG2

extern int prolog_call0(CTXTdeclc Cell);
extern int prolog_code_call(CTXTdeclc Cell, int);

/* 
Terminology: outedge -- pointer to calls that depend on call
             inedge  -- pointer to calls on which call depends
*/

#define ISO_INCR_TABLING 1

/********************** STATISTICS *****************************/

// affected_gl drives create_call_list and eager updates
//calllistptr affected_gl=NULL;
calllistptr lazy_affected= NULL;

int incr_table_update_safe_gl = TRUE;

// derives return_changed_call_list which reports those calls changed in last update
//calllistptr changed_gl=NULL;

// used to compare new to previous tables.
callnodeptr old_call_gl=NULL;
// is this needed?  Can we use the root ptr from SF?
BTNptr old_answer_table_gl=NULL;

calllistptr assumption_list_gl=NULL;  /* required for abolishing incremental calls */ 

// current number of incremental subgoals / edges
int current_call_node_count_gl=0,current_call_edge_count_gl=0;
// total number of incremental subgoals in session.
int  total_call_node_count_gl=0;  
// not used much -- for statistics
int unchanged_call_gl=0;

// This array does not need to be resized -- just needs to be > max_arity
static Cell cell_array1[500];

/* These seem to be safely within the bounds of regular (non-small) structures) */
Structure_Manager smCallNode  =  SM_InitDecl(CALL_NODE,CALLNODE_PER_BLOCK,"CallNode");
Structure_Manager smCallList  =  SM_InitDecl(CALLLIST,CALLLIST_PER_BLOCK,"CallList");
Structure_Manager smCall2List =  SM_InitDecl(CALL2LIST,CALL2LIST_PER_BLOCK,"Call2List");

/* smKey is small -- need to to use SM_DeallocateSmallStruct */
Structure_Manager smKey	      =  SM_InitDecl(KEY,KEY_PER_BLOCK,"HashKey");

/* appears to be minimal size for regular (non-small) structure */
Structure_Manager smOutEdge   =  SM_InitDecl(OUTEDGE,OUTEDGE_PER_BLOCK,"Outedge");

DEFINE_HASHTABLE_INSERT(insert_some, KEY, CALL_NODE);
DEFINE_HASHTABLE_SEARCH(search_some, KEY, callnodeptr);
DEFINE_HASHTABLE_REMOVE(remove_some, KEY, callnodeptr);
DEFINE_HASHTABLE_ITERATOR_SEARCH(search_itr_some, KEY);

/*****************************************************************************/
/* Debugging */

void print_call_list(CTXTdeclc calllistptr affected_ptr,char * string) {

  printf("%s",string);
  if (calllist_next(affected_ptr) == NULL) printf(" (empty) \n");
  else{
    while ( calllist_next(affected_ptr) != NULL) {
      printf("...item %p sf %p ",calllist_item(affected_ptr),callnode_sf(calllist_item(affected_ptr)));
      printf("next %p ",calllist_next(affected_ptr));  
      if (callnode_sf(calllist_item(affected_ptr)) != NULL) {
	print_subgoal(CTXTc stddbg, callnode_sf(calllist_item(affected_ptr)));
      }
      printf("\n");
      affected_ptr = (calllistptr) calllist_next(affected_ptr);
    } ;
  }
}

void call_list_length(CTXTdeclc calllistptr affected_ptr,char * string) {
  int count = 0;

  if (calllist_next(affected_ptr) == NULL) printf("%s is empty) \n",string);
  else {
    while ( calllist_next(affected_ptr) != NULL) {
      affected_ptr = (calllistptr) calllist_next(affected_ptr);
    } ;
  }
  printf("length of %s is %d",string,count);
}

/*****************************************************************************/
static unsigned int hashid(void *ky)
{
  return (unsigned int)(UInteger)ky;
}

static unsigned int hashfromkey(void *ky)
{
    KEY *k = (KEY *)ky;
    return (int)(k->goal);
}

static int equalkeys(void *k1, void *k2)
{
    return (0 == memcmp(k1,k2,sizeof(KEY)));
}



/* Creates a call node */
callnodeptr makecallnode(VariantSF sf){
  
  callnodeptr cn;

  SM_AllocateStruct(smCallNode,cn);
  cn->deleted = 0;
  cn->changed = 0;
  cn->no_of_answers = 0;
  cn->falsecount = 0;
  cn->recomputable = COMPUTE_DEPENDENCIES_FIRST;
  cn->prev_call = NULL;
  cn->aln = NULL;
  cn->inedges = NULL;
  cn->goal = sf;
  cn->outedges=NULL;
  cn->id=total_call_node_count_gl++; 
  cn->outcount=0;

  //    print_callnode(stddbg,cn);printf("\n");

  current_call_node_count_gl++;
  //  printf("makecallnode %p sf %p\n",cn,sf);
  return cn;
}

//---------------------------------------------------------------------------
void print_inedges(callnodeptr cn) {
  calllistptr temp; 

  printf("Inedges of %d = ",cn->id);
  temp=cn->inedges;
  while(temp!=NULL){
    printf("   %d",temp->inedge_node->callnode->id);      
    temp=temp->next;
  }
  printf("\n");
}

void print_outedges(callnodeptr cn) {
  struct hashtable *h; 
  struct hashtable_itr *itr;                                                                                             
  callnodeptr cn_itr;

  printf("Outedges of %d = ",cn->id);
  h=cn->outedges->hasht;
  itr = hashtable1_iterator(h);

  if (hashtable1_count(h) > 0){
    do {                                                                                                                 
      cn_itr = hashtable1_iterator_value(itr);        
      printf(" %d ",cn_itr->id);
    } while (hashtable1_iterator_advance(itr)); 
  }
  else {
    printf("no outedges\n");
  }
  printf("\n");


}

void deleteinedges(CTXTdeclc callnodeptr callnode){
  calllistptr tmpin,in;
  
  KEY *ownkey;
  struct hashtable* hasht;
  SM_AllocateStruct(smKey, ownkey);
  ownkey->goal=callnode->id;	
	
  in = callnode->inedges;
  
#ifdef INCR_DEBUG	
  printf("--------------------------------- deleteinedges (id %d) before \n",callnode->id);
  print_inedges(callnode);
#endif

  while(IsNonNULL(in)){
    tmpin = in->next;
    if (in->inedge_node->callnode->id != callnode->id) {
      hasht = in->inedge_node->hasht;
#ifdef INCR_DEBUG1
      printf("  removing affects ptr from "); print_callnode(CTXTc stddbg,in->inedge_node->callnode);
      printf(" to ");print_callnode(CTXTc stddbg,callnode);printf("\n");
      printf("  removing affects ptr from %d to %d\n",in->inedge_node->callnode->id,callnode->id); printf("\n");
      print_outedges(in->inedge_node->callnode);
#endif
      //    printf("remove some callnode %x / ownkey %d\n",callnode,ownkey);
      if (remove_some(hasht,ownkey) == NULL) {
	xsb_abort("BUG: key not found for removal (deleteinedges)\n");
      }
      current_call_edge_count_gl--;
      in->inedge_node->callnode->outcount--;
      SM_DeallocateStruct(smCallList, in);      
    }
    in = tmpin;
  }
  SM_DeallocateSmallStruct(smKey, ownkey);      
#ifdef INCR_DEBUG	
  printf("--------------------------------- deleteinedges after \n");
  //  print_inedges(callnode);
#endif

  return;
}

//---------------------------------------------------------------------------

void deleteoutedges(CTXTdeclc callnodeptr callnode){
  struct hashtable *h; 
  struct hashtable_itr *itr;                                                                                             
  callnodeptr cn_itr;
  //  callnodeptr cn_itr_sav;
  calllistptr in;
  //  calllistptr * last;
  calllistptr last = NULL;
  int i;

  h=callnode->outedges->hasht;
  itr = hashtable1_iterator(h);
  #ifdef INCR_DEBUG1
  printf("----- deleteoutedges (id %d) before hashtable count %d outedges_count %d\n",callnode->id,hashtable1_count(h),callnode->outcount);
  #endif

  if (hashtable1_count(h) > 0){
    do {                                                                                                                 
      cn_itr = hashtable1_iterator_value(itr);        
#ifdef INCR_DEBUG1
      printf("iterating (id %d)",cn_itr->id);print_callnode(CTXTc stddbg,cn_itr); printf("\n");
      printf("before deleting link "); print_inedges(cn_itr);
#endif
      if (cn_itr != callnode) {  /* messes things up, otherwise.  This will be dealloc'd anyway */
	in = cn_itr->inedges;
	i = 0;
	while(IsNonNULL(in)){
	  if (in->inedge_node->callnode == callnode) {
#ifdef INCR_DEBUG1
	    print_callnode(CTXTc stddbg,in->inedge_node->callnode);printf("\n");
#endif
	    if (i == 0) {
	      cn_itr->inedges = in->next;
	    }
	    else last->next =  in->next;  // need to deallcoate
	    SM_DeallocateStruct(smCallList, in);      
	    //	  printf("again! ");print_inedges(cn_itr);
	    break;
	  }
	//printf("skipping\n");
	  last = in; in = in->next; i++;
	}
      }
      current_call_edge_count_gl--;
#ifdef INCR_DEBUG1
      printf("after deleting link "); print_inedges(hashtable1_iterator_value(itr));
#endif
    } while (hashtable1_iterator_advance(itr)); 
    callnode->outcount = 0;  // hashtable will be deallocated in delete callnode
  }
  #ifdef INCR_DEBUG1
  printf("--- deleteoutedges (id %d) after  hashtable count %d outcount %d\n",callnode->id,hashtable1_count(h),callnode->outcount);
  #endif

}
  

  //  calllistptr tmpin,in;
  
  //  KEY *ownkey;
  //  struct hashtable* hasht;
  //  SM_AllocateStruct(smKey, ownkey);
  //  ownkey->goal=callnode->id;	
	
  //  in = callnode->inedges;
  
  //  while(IsNonNULL(in)){
  //    tmpin = in->next;
  //    hasht = in->inedge_node->hasht;
  //    Printf("removing affects ptr from "); print_callnode(stddbg,in->inedge_node->callnode);
  //    printf(" to "),print_callnode(stddbg,callnode);printf("\n");
  //    printf("remove some callnode %x / ownkey %d\n",callnode,ownkey);
  //    if (remove_some(hasht,ownkey) == NULL) {
  //      xsb_abort("BUG: key not found for removal\n");
  //    }
  //    current_call_edge_count_gl--;
  //   SM_DeallocateStruct(smCallList, in);      
  //    in = tmpin;
  //  }
  //  SM_DeallocateSmallStruct(smKey, ownkey);      
  //  return;
  //}

//---------------------------------------------------------------------------

/* used for abolishes -- its known that outcount is 0 */
void deletecallnode(callnodeptr callnode){
  current_call_node_count_gl--;
 
  if(callnode->outcount==0){
    hashtable1_destroy(callnode->outedges->hasht,0);
    SM_DeallocateStruct(smOutEdge, callnode->outedges);      
    SM_DeallocateStruct(smCallNode, callnode);        
  }else {
    printf("aborting id %d outcount %d\n",callnode->id,callnode->outcount);
    xsb_abort("outcount is nonzero\n");
  }
  
  //  return;
}


void deallocate_previous_call(callnodeptr callnode){
  
  calllistptr tmpin,in;
  
  KEY *ownkey;
  /*  callnodeptr  inedge_node; */
  struct hashtable* hasht;
  SM_AllocateStruct(smKey, ownkey);
  ownkey->goal=callnode->id;	
	
  in = callnode->inedges;
  current_call_node_count_gl--;
  
  while(IsNonNULL(in)){
    tmpin = in->next;
    hasht = in->inedge_node->hasht;
    if (remove_some(hasht,ownkey) == NULL) {
      /*
      prevnode=in->prevnode->callnode;
      if(IsNonNULL(prevnode->goal)){
	sfPrintGoal(stdout,(VariantSF)prevnode->goal,NO); printf("(%d)",prevnode->id);
      }
      if(IsNonNULL(callnode->goal)){
	sfPrintGoal(stdout,(VariantSF)callnode->goal,NO); printf("(%d)",callnode->id);
      }
      */
      xsb_abort("BUG: key not found for removal (deallocate previous call)\n");
    }
    in->inedge_node->callnode->outcount--;
    current_call_edge_count_gl--;
    SM_DeallocateStruct(smCallList, in);      
    in = tmpin;
  }
  
  SM_DeallocateStruct(smCallNode, callnode);      
  SM_DeallocateSmallStruct(smKey, ownkey);      
}

void initoutedges(CTXTdeclc callnodeptr cn){
  outedgeptr out;

#ifdef INCR_DEBUG
  printf("Initoutedges for ");  print_callnode(CTXTc stddbg, cn);  printf(" (id %d) \n",cn->id);
#endif

  SM_AllocateStruct(smOutEdge,out);
  cn->outedges = out;
  out->callnode = cn; 	  
  out->hasht =create_hashtable1(HASH_TABLE_SIZE, hashfromkey, equalkeys);
  return;
}


/*
propagate_no_change(c)
	for c->c'
	  if(c'.deleted=false)
	     c'->falsecount>0
	       c'->falsecount--
	       if(c'->falsecount==0)
		 propagate_no_change(c')		

When invalidation is done a parameter 'falsecount' is maintained with
each call which signifies that these many predecessor calls have been
affected. So if a call A has two pred node B and C and both of them
are affected then A's falsecount is 2. Now when B is reevaluated and
turns out it has not been changed (its old and new answer table is the
same) completion of B calls propagate_no_change(B) which reduces the
falsecount of A by 1. If for example turns out that C was also not
changed the falsecount of A is going to be reduced to 0. Now when call
A is executed it's just going to do answer clause resolution.
*/
void propagate_no_change(callnodeptr c){
  callnodeptr cn;
  struct hashtable *h;	
  struct hashtable_itr *itr;
  if(IsNonNULL(c)){
    h=c->outedges->hasht;
    itr = hashtable1_iterator(h);
    if (hashtable1_count(h) > 0){
      do {
	cn= hashtable1_iterator_value(itr);
	if(cn->falsecount>0){ /* this check is required for the new dependencies that can arise bcoz of the re-evaluation */
	  cn->falsecount--;
	  if(cn->falsecount==0){
	    cn->deleted = 0;
	    propagate_no_change(cn);
	  }
	}
      } while (hashtable1_iterator_advance(itr));
    }
  }		
}

/* Enter a call to calllist */
static void inline add_callnode_sub(calllistptr *list, callnodeptr item){
  calllistptr  temp;
  SM_AllocateStruct(smCallList,temp);
  temp->item=item;
  temp->next=*list;
  *list=temp;  
  //  printf("added list %p @list %p\n",list,*list);
}

/* used in addcalledge */
static void inline addcalledge_1(calllistptr *list, outedgeptr item){
  calllistptr  temp;
  SM_AllocateStruct(smCallList,temp);
  temp->inedge_node=item;
  temp->next=*list;
  *list=temp;  
}

void addcalledge(callnodeptr fromcn, callnodeptr tocn){
  KEY *k1;
  SM_AllocateStruct(smKey, k1);
  k1->goal = tocn->id;
  
  // #ifdef INCR_DEBUG
  //  printf("%d-->%d",fromcn->id,tocn->id);	
  // #endif
  
  if (NULL == search_some(fromcn->outedges->hasht,k1)) {
    insert_some(fromcn->outedges->hasht,k1,tocn);

#ifdef INCR_DEBUG	
    printf("--------------- addcalledge (from %d to %d) before \n",fromcn->id,tocn->id);
    print_inedges(tocn);
    print_outedges(fromcn);
#endif
    
    addcalledge_1(&(tocn->inedges),fromcn->outedges);      
    current_call_edge_count_gl++;
    fromcn->outcount++;
    
#ifdef INCR_DEBUG		
    if(IsNonNULL(fromcn->goal)){
      sfPrintGoal(stdout,(VariantSF)fromcn->goal,NO);printf("(%d)",fromcn->id);
    }else
      printf("--------------- addcalledge after  (%d)",fromcn->id);
    
    if(IsNonNULL(tocn->goal)){
      printf("-->");	
      sfPrintGoal(stdout,(VariantSF)tocn->goal,NO);printf("(%d)",tocn->id);
    }
    printf("\n");	
#endif
    
    
  }

#ifdef INCR_DEBUG	
    printf("--------------------------------- addcalledge after \n");
    print_inedges(tocn);
#endif
}

#define EMPTY NULL

//calllistptr eneetq(){ 
calllistptr empty_calllist(){ 
  
  calllistptr  temp;
  SM_AllocateStruct(smCallList,temp);
  temp->item = (callnodeptr)EMPTY;
  temp->next = NULL;

  return temp;
}


void add_callnode(calllistptr *cl,callnodeptr c){
  add_callnode_sub(cl,c);
}

callnodeptr delete_calllist_elt(CTXTdeclc calllistptr *cl){   
  
  calllistptr tmp;
  callnodeptr c;

  #ifdef INCR_DEBUG1
    printf(" in deallocate call list elt %p *%p\n",cl,*cl);
  #endif
  c = (*cl)->item;
  tmp = *cl;
  *cl = (*cl)->next;
  #ifdef INCR_DEBUG1
  if (c) {printf("deleting from call list: "); print_callnode(CTXTc stddbg, c); printf("\n");}
  #endif
  //  printf("calllist %p item %p next %p\n",tmp,c,*cl);
  SM_DeallocateStruct(smCallList, tmp);      
  
  return c;  
}

/* Used to deallocate dfs-created call lists when encountering
   visitors or incomplete tables. */
void deallocate_call_list(CTXTdeclc calllistptr cl)  {
    callnodeptr tmp_call;

    //    printf(" in deallocate call list %p *%p\n",cl,*cl);
    while ((tmp_call = delete_calllist_elt(CTXTc &cl)) != EMPTY){
      ;
    }
    //    SM_DeallocateStruct(smCallList, cl);      
  }

void dfs_outedges_check_non_completed(CTXTdeclc callnodeptr call1) {
  //  char bufferb[MAXTERMBUFSIZE]; 

  if(IsNonNULL(call1->goal) && !subg_is_completed((VariantSF)call1->goal)){
    //    if (calllist_next(affected_gl) != NULL) {
    //      print_call_list(affected_gl);
    //  deallocate_call_list(CTXTc affected_gl);
    //    }
    //    printf("outedges affected_gl %p %p\n",affected_gl,*affected_gl);
    sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)call1->goal);     
    //    sprintf(bufferb,"Incremental tabling is trying to invalidate an incomplete table \n %s\n",
    //	    forest_log_buffer_1->fl_buffer);
    //    sprintf(bufferb,"%s",forest_log_buffer_1->fl_buffer);
    xsb_new_table_error(CTXTc "incremental_tabling",forest_log_buffer_1->fl_buffer,
			"An incremental update is trying to invalidate the goal %s",forest_log_buffer_1->fl_buffer);
  }
}

typedef struct incr_callgraph_dfs_frame {
  hashtable_itr_ptr itr;
  callnodeptr cn;
} IncrCallgraphDFSFrame;

#define push_dfs_frame(CN,ITR)	{					\
    /*    printf("pushing cn %p   ",CN);				\
    if (CN -> goal) { printf("*** ");print_subgoal(stddbg,CN->goal);}  printf("\n"); */\
    ctr ++;								\
    if (++incr_callgraph_dfs_top == incr_callgraph_dfs_size) {		\
      printf("reallocing to %d\n",incr_callgraph_dfs_size*2);		\
      mem_realloc(incr_callgraph_dfs, incr_callgraph_dfs_size*sizeof(IncrCallgraphDFSFrame), \
		  2*incr_callgraph_dfs_size*sizeof(IncrCallgraphDFSFrame), TABLE_SPACE); \
      incr_callgraph_dfs_size =     2*incr_callgraph_dfs_size;		\
    }									\
    incr_callgraph_dfs[incr_callgraph_dfs_top].itr = ITR;		\
    incr_callgraph_dfs[incr_callgraph_dfs_top].cn = CN;		\
  }

#define pop_dfs_frame {incr_callgraph_dfs_top--;}

static void dfs_outedges(CTXTdeclc callnodeptr call1,xsbBool inval_context){
  callnodeptr cn;
  struct hashtable *h;	
  struct hashtable_itr *itr;
  int ctr = 0;

  int incr_callgraph_dfs_top = -1;
  int incr_callgraph_dfs_size; 
  IncrCallgraphDFSFrame   *incr_callgraph_dfs;

  //  printf("1) calling dfs %p; subgoal ",call1);  print_callnode(stddbg,call1); printf("\n");

  incr_callgraph_dfs =
    (IncrCallgraphDFSFrame *)  mem_alloc(10000*sizeof(IncrCallgraphDFSFrame), 
					 TABLE_SPACE);
  incr_callgraph_dfs_size = 10000;
  dfs_outedges_check_non_completed(CTXTc call1);
  if (!is_fact_in_callgraph(call1)) call1->deleted = 1;
  h=call1->outedges->hasht;
  if (hashtable1_count(h) > 0){
    //    printf("1-call1: %p \n",call1);
    push_dfs_frame(call1,0);
  }
  while (incr_callgraph_dfs_top > -1) {
    itr = 0;
    do {
      if (incr_callgraph_dfs[incr_callgraph_dfs_top].itr) {
	itr = incr_callgraph_dfs[incr_callgraph_dfs_top].itr;
	//	printf("1-top %d itr %p\n",incr_callgraph_dfs_top,itr);
	if (!hashtable1_iterator_advance(itr)) {  // last element in the hash
	  itr = 0;
	  cn = incr_callgraph_dfs[incr_callgraph_dfs_top].cn;
	  //	  if (inval_context == NOT_ABOLISHING || cn != call1)  {
	  //	    add_callnode(&affected_gl,cn);	
	  //	    abolish_dbg(("+ (dfsoutedges)  adding callnode %p sf %p\n",cn,cn->goal));
	  //}
	  pop_dfs_frame;
	}
      }
      else {
	h=incr_callgraph_dfs[incr_callgraph_dfs_top].cn->outedges->hasht;
	if (hashtable1_count(h) > 0) {
	  itr = hashtable1_iterator(h);          // initialize
	  incr_callgraph_dfs[incr_callgraph_dfs_top].itr = itr;
	}
	else {
	  cn = incr_callgraph_dfs[incr_callgraph_dfs_top].cn;
	  //	  if (inval_context == NOT_ABOLISHING || cn != call1)  {
	  //        add_callnode(&affected_gl,cn);		
	  //	    abolish_dbg(("+ (dfsoutedges) adding callnode %p sf %p\n",cn,cn->goal));
	  //	  }
	  pop_dfs_frame;
	}
      }
    } while (itr == 0 && incr_callgraph_dfs_top > -1);
    if (incr_callgraph_dfs_top > -1) {
      cn = hashtable1_iterator_value(itr);

      //      printf("top %d cn: %p itr: %p\n",incr_callgraph_dfs_top,cn,itr);
      //      printf("2) recursive dfs %p; subgoal ",cn);  print_callnode(stddbg,cn); printf("\n");

      cn->falsecount++;
      if (cn->deleted==0) {
	dfs_outedges_check_non_completed(CTXTc cn);
	cn->deleted = 1;
	//	h=cn->outedges->hasht;
	//	if (hashtable1_count(h) > 0) {
	  push_dfs_frame(cn,0);
	  //	}
      }
    }
  }
  mem_dealloc(incr_callgraph_dfs, incr_callgraph_dfs_size*sizeof(IncrCallgraphDFSFrame), 
	      TABLE_SPACE);
}

/*  
 * void dfs_outedges(CTXTdeclc callnodeptr call1){
 *   callnodeptr cn;
 *   struct hashtable *h;	
 *   struct hashtable_itr *itr;
 * 
 *   //    if (call1->goal) {
 *   //      printf("dfs outedges "); print_subgoal(stddbg,call1->goal);printf("\n");
 *   //    }
 *   if(IsNonNULL(call1->goal) && !subg_is_completed((VariantSF)call1->goal)){
 *     dfs_outedges_new_table_error(CTXTc call1);
 *   }
 *   call1->deleted = 1;
 *   h=call1->outedges->hasht;
 *   
 *   itr = hashtable1_iterator(h);       
 *   if (hashtable1_count(h) > -1){
 *     do {
 *       cn = hashtable1_iterator_value(itr);
 *       cn->falsecount++;
 *       if(cn->deleted==0)
 * 	dfs_outedges(CTXTc cn);
 *     } while (hashtable1_iterator_advance(itr));
 *   } 
 *  add_callnode(&affected_gl,call1);		
 * }
 * */
// TLS: factored out this warning because dfs_inedges is recursive and
// this makes the stack frames too big. 
void dfs_inedges_warning(CTXTdeclc callnodeptr call1,calllistptr *lazy_affected) {
  deallocate_call_list(CTXTc *lazy_affected);
  sprint_subgoal(CTXTc forest_log_buffer_1,0,call1->goal);
    xsb_warn(CTXTc "%d Choice point(s) exist to the table for %s -- cannot incrementally update (dfs_inedges)\n",
	     subg_visitors(call1->goal),forest_log_buffer_1->fl_buffer);
  }

extern BTNptr TrieNodeFromCP(CPtr);
extern ALNptr traverse_variant_answer_trie( VariantSF, CPtr, CPtr) ;
extern Cell list_of_answers_from_answer_list(VariantSF,int,int,ALNptr);

//extern int instr_flag;
extern CPtr hreg_pos;

void find_the_visitors(CTXTdeclc VariantSF subgoal) {
  CPtr cp_top1,cp_bot1 ; CPtr cp_root; 
  #ifdef INCR_DEBUG1
  CPtr cp_first;
  #endif
  byte cp_inst; Cell listHead;
  int ans_subst_num, i, attv_num;
  BTNptr trieNode;
  ALNptr ALNlist;

#ifdef NON_OPT_COMPILE
  printf("find the visitors: subg %p trie root %p\n",subgoal,subg_ans_root_ptr(subgoal));
#endif

  cp_top1 = breg ;				 
  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;
  if (xwammode && hreg < hfreg) {
    printf("uh-oh! hreg was less than hfreg in in find the visitors\n");
    hreg = hfreg;
  }
  while ( cp_top1 < cp_bot1 ) {
    //    printf("1 cp_top1 %p cp_bot1 %p prev %p\n",cp_top1,cp_bot1,cp_prevtop(cp_top1));
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but need to distinguish from asserted and interned tries
    //    printf("cp_inst %x\n",cp_inst);
    if ( is_trie_instruction(cp_inst) ) {
      //      printf("found trie instr\n");
      // Below we want basic_answer_trie_tt, ts_answer_trie_tt
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode)) {
	//	printf("in answer trie\n");
	if (subgoal == get_subgoal_frame_for_answer_trie_cp(CTXTc trieNode,cp_top1))  {
	  //	  	  printf("found top of run %p ",cp_top1);
	  //	  	  print_subgoal(CTXTc stdout, subgoal); printf("\n");
	  cp_root = cp_top1; 
	  #ifdef INCR_DEBUG1
	  cp_first = cp_top1;
	  #endif
	  while (*cp_pcreg(cp_root) != trie_fail) {
	  #ifdef INCR_DEBUG1
	    cp_first = cp_root;
	  #endif
	    cp_root = cp_prevbreg(cp_root);
	    if (*cp_pcreg(cp_root) != trie_fail && 
		subgoal != get_subgoal_frame_for_answer_trie_cp(CTXTc TrieNodeFromCP(cp_root),cp_top1))
	      printf(" couldn't find incr trie root -- whoa, whu? (%p\n",cp_root);
	  }
	  ALNlist = traverse_variant_answer_trie(subgoal, cp_root,cp_top1);
	  ans_subst_num = (int)int_val(cell(cp_root + CP_SIZE + 1)) ;  // account for sf ptr of trie root cp
	  attv_num = (int)int_val(cell(breg+CP_SIZE+1+ans_subst_num)) + 1;;
	  #ifdef INCR_DEBUG1
	  printf("found root %p first %p top %p ans_subst_num %d & %p attv_num %d\n",cp_root,cp_first,cp_top1,ans_subst_num,breg+CP_SIZE, attv_num); 
	  #endif
	  listHead = list_of_answers_from_answer_list(subgoal,ans_subst_num,attv_num,ALNlist);
	  // Free ALNlist;
	  cp_pcreg(cp_top1) = (byte *) &completed_trie_member_inst;
       	  cp_ebreg(cp_top1) = cp_ebreg(cp_root);
	  cp_hreg(cp_top1) = hreg;	  
	  cp_ereg(cp_top1) = cp_ereg(cp_root);
	  cp_trreg(cp_top1) = cp_trreg(cp_root);
	  cp_prevbreg(cp_top1) = cp_prevbreg(cp_root);	  cp_prevtop(cp_top1) = cp_prevtop(cp_root);
	  // cpreg, ereg, pdreg, ptcpreg should not need to be reset (prob not ebreg?)
	  //	  printf("sf %p\n",* (cp_root + CP_SIZE + 2));
	  * (cp_top1 + CP_SIZE) = makeint(ans_subst_num);
	  for (i = 0;i < ans_subst_num ;i++) {                              // Use registers for root of trie, not leaf (top)
	    * (cp_top1 + CP_SIZE + 1 + i) =  * (cp_root + CP_SIZE + 2 +i);  // account for sf ptr or root
	  }
	  * (cp_top1 + CP_SIZE + 1+ ans_subst_num) = listHead;
	  * (cp_top1 + CP_SIZE + 2+ ans_subst_num) = (Cell)hfreg;
	  //	  printf("4 cp_root %p prev %p\n",cp_root,cp_prevtop(cp_root));
	  //	  printf("constructed listhead hreg %x\n",hreg);
	  //	  cp_top1 = cp_root;  // next iteration
	  //	  printf("7 cp_top1 %p cp_bot1 %p prev %p\n",cp_top1,cp_bot1,cp_prevtop(cp_top1));
	}
      }
    }
    cp_top1 = cp_prevtop(cp_top1);
  }
  if (xwammode) hfreg = hreg;
  //  printf("constructed listhead hreg %x hfreg %x\n",hreg,hfreg);
  subg_visitors(subgoal) = 0;
  //  instr_flag = 1;  printf("setting instr_flag\n");  hreg_pos = hreg;
}

void throw_dfs_inedges_error(CTXTdeclc callnodeptr call1) {
  char bufferb[MAXTERMBUFSIZE]; 
  sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)call1->goal);     
  sprintf(bufferb,"Incremental tabling is trying to invalidate an incomplete table \n %s\n",
	  forest_log_buffer_1->fl_buffer);
  xsb_new_table_error(CTXTc "incremental_tabling",bufferb,"in predicate %s/%d",
		      get_name(TIF_PSC(subg_tif_ptr(call1->goal))),get_arity(TIF_PSC(subg_tif_ptr(call1->goal))));
}

/* If ret != 0 (= CANNOT_UPDATE) then we'll use the old table, and we
   wont lazily update at all. */
int dfs_inedges(CTXTdeclc callnodeptr call1, calllistptr * lazy_affected, int flag ){
  calllistptr inedge_list;
  VariantSF subgoal;
  int ret = 0;

  if(IsNonNULL(call1->goal)) {
    if (!subg_is_completed((VariantSF)call1->goal)){

      deallocate_call_list(CTXTc *lazy_affected);
      throw_dfs_inedges_error(CTXTc call1);

    }
    if (subg_visitors(call1->goal)) {
      #ifdef ISO_INCR_TABLING
      find_the_visitors(CTXTc call1->goal);
      #else
      dfs_inedges_warning(CTXTc call1,lazy_affected);
      return CANNOT_UPDATE;
      #endif
    }
  }
  // TLS: handles dags&cycles -- no need to traverse more than once.
  if (call1 -> recomputable == COMPUTE_DEPENDENCIES_FIRST)
    call1 -> recomputable = COMPUTE_DIRECTLY;
  else {     //    printf("found directly computable call \n");
    return 0;
  }
  //  printf(" dfs_i affected "); print_subgoal(stddbg,call1->goal);printf("\n");
  inedge_list= call1-> inedges;
  while(IsNonNULL(inedge_list) && !ret){
    subgoal = (VariantSF) inedge_list->inedge_node->callnode->goal;
    if(IsNonNULL(subgoal)){ /* fact check */
      //      count++;
      if (inedge_list->inedge_node->callnode->falsecount > 0)  {
	ret = ret | dfs_inedges(CTXTc inedge_list->inedge_node->callnode, lazy_affected,flag);
      }
      else {
	; //	printf(" dfs_i non_affected "); print_subgoal(stddbg,subgoal);printf("\n");
      }
    }
    inedge_list = inedge_list->next;
  }
  if(IsNonNULL(call1->goal) & !ret){ /* fact check */
    //    printf(" dfs_i adding "); print_subgoal(stddbg,call1->goal);printf("\n");
    add_callnode(lazy_affected,call1);		
  }
  return ret;
}

 
void invalidate_call(CTXTdeclc callnodeptr cn,xsbBool abolishing){

  //#ifdef MULTI_THREAD
  //  xsb_abort("Incremental Maintenance of tables in not available for multithreaded engine\n");
  //#endif
  //  printf("invalidate call: ");print_callnode(stddbg,cn); printf(" deleted (%d)\n",cn->deleted);
  if(cn->deleted==0){
    cn->falsecount++;
    dfs_outedges(CTXTc cn,abolishing);
  }
}

#define WARN_ON_UNSAFE_UPDATE 1

//---------------------------------------------------------------------------
// destroys

int return_lazy_call_list(CTXTdeclc  callnodeptr call1){
  VariantSF subgoal;
  TIFptr tif;
  int j,count=0,arity; 
  Psc psc;
  CPtr oldhreg=NULL;

  //  printf("beginning return_lazy_call_list\n");
  //  call_list_length(lazy_affected,"lazy call_list");

  reg[6] = reg[5] = makelist(hreg);  // reg 5 first not-used, use regs in case of stack expanson
  new_heap_free(hreg);   // make heap consistent
  new_heap_free(hreg);
  while((call1 = delete_calllist_elt(CTXTc &lazy_affected)) != EMPTY){
    subgoal = (VariantSF) call1->goal;      
    //    fprintf(stddbg,"  considering ");print_subgoal(stdout,subgoal);printf("\n");
    if(IsNULL(subgoal)){ /* fact predicates */
      call1->deleted = 0; 
      continue;
    }
    if (subg_visitors(subgoal)) {
      #ifdef ISO_INCR_TABLING
      find_the_visitors(CTXTc subgoal);
      #else
      sprint_subgoal(CTXTc forest_log_buffer_1,0,subgoal);
      #ifdef WARN_ON_UNSAFE_UPDATE
            xsb_warn(CTXTc "%d Choice point(s) exist to the table for %s -- cannot incrementally update (create_lazy_call_list)\n",
      	       subg_visitors(subgoal),forest_log_buffer_1->fl_buffer);
	    continue;
      #else
            xsb_abort("%d Choice point(s) exist to the table for %s -- cannot incrementally update (create_lazy_call_list)\n",
      	       subg_visitors(subgoal),forest_log_buffer_1->fl_buffer);
      #endif
      #endif
    }
    //    fprintf(stddbg,"adding dependency for ");print_subgoal(stdout,subgoal);printf("\n");
    count++;
    tif = (TIFptr) subgoal->tif_ptr;
    //    if (!(psc = TIF_PSC(tif)))
    //	xsb_table_error(CTXTc "Cannot access dynamic incremental table\n");	
    psc = TIF_PSC(tif);
    arity = get_arity(psc);
    //    check_glstack_overflow(6,pcreg,2+arity*20000); // don't know how much for build_subgoal_args...
    check_glstack_overflow(6,pcreg,2+(sizeof(Cell)*trie_path_heap_size(CTXTc subg_leaf_ptr(subgoal)))); 

    oldhreg = clref_val(reg[6]);  // maybe updated by re-alloc
    if(arity>0){
      sreg = hreg;
      follow(oldhreg++) = makecs(sreg);
      hreg += arity + 1;  // had 10, why 10?  why not 3? 2 for list, 1 for functor (dsw)
      new_heap_functor(sreg, psc);
      for (j = 1; j <= arity; j++) {
	new_heap_free(sreg);
	cell_array1[arity-j] = cell(sreg-1);
      }
      //      build_subgoal_args(arity,cell_array1,subgoal);		
      /* Need to do separate heapcheck above to protect regs 1-6 */
      load_solution_trie_no_heapcheck(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(subgoal));
    } else {
      follow(oldhreg++) = makestring(get_name(psc));
    }
    reg[6] = follow(oldhreg) = makelist(hreg);
    new_heap_free(hreg);
    new_heap_free(hreg);
  }
  if(count > 0) {
    follow(oldhreg) = makenil;
    hreg -= 2;  /* take back the extra words allocated... */
  } else
    reg[5] = makenil;

  //  printf("end of return lazy call list\n");
  return unify(CTXTc reg_term(CTXTc 4),reg_term(CTXTc 5));

  /*int i;
    for(i=0;i<callqptr;i++){
      if(IsNonNULL(callq[i]) && (callq[i]->deleted==1)){
    sfPrintGoal(stdout,(VariantSF)callq[i]->goal,NO);
    printf(" %d %d\n",callq[i]->falsecount,callq[i]->deleted);
    }
    }
  printf("-----------------------------\n");   */
}

//---------------------------------------------------------------------------
// Utility for next two functions

int in_reg2_list(CTXTdeclc Psc psc) {
  Cell list,term;

  list = reg[2];
  XSB_Deref(list);
  if (isnil(list)) return TRUE; /* if filter is empty, return all */
  while (!isnil(list)) {
    term = get_list_head(list);
    XSB_Deref(term);
    if (isconstr(term)) {
      if (psc == get_str_psc(term)) return TRUE;
    } else if (isstring(term)) {
      if (get_name(psc) == string_val(term)) return TRUE;
    }
    list = get_list_tail(list);
  }
  return FALSE;
}

//---------------------------------------------------------------------------

int immediate_outedges_list(CTXTdeclc callnodeptr call1){
 
  VariantSF subgoal;
  TIFptr tif;
  int j, count = 0,arity;
  Psc psc;
  CPtr oldhreg = NULL;
  struct hashtable *h;	
  struct hashtable_itr *itr;
  callnodeptr cn;
    
  reg[4] = makelist(hreg);
  new_heap_free(hreg);
  new_heap_free(hreg);
  
  if(IsNonNULL(call1)){ /* This can be called from some non incremental predicate */
    h=call1->outedges->hasht;
    
    itr = hashtable1_iterator(h);       
    if (hashtable1_count(h) > 0){
      do {
	cn = hashtable1_iterator_value(itr);
	//	printf("found outedge ");print_callnode(stddbg,cn);printf("\n");
	if(IsNonNULL(cn->goal)){
	  count++;
	  subgoal = (VariantSF) cn->goal;      
	  tif = (TIFptr) subgoal->tif_ptr;
	  psc = TIF_PSC(tif);
	  arity = get_arity(psc);
	  //	  check_glstack_overflow(4,pcreg,2+arity*200); // don't know how much for build_subgoal_args...
	  check_glstack_overflow(4,pcreg,2+(sizeof(Cell)*trie_path_heap_size(CTXTc subg_leaf_ptr(subgoal)))); 
	  oldhreg=hreg-2;
	  if(arity>0){
	    sreg = hreg;
	    follow(oldhreg++) = makecs(sreg);
	    hreg += arity + 1;
	    new_heap_functor(sreg, psc);
	    for (j = 1; j <= arity; j++) {
	      new_heap_free(sreg);
	      cell_array1[arity-j] = cell(sreg-1);
	    }
	    load_solution_trie_no_heapcheck(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(subgoal));
	    //    build_subgoal_args(arity,cell_array1,subgoal);		
	  }else{
	    follow(oldhreg++) = makestring(get_name(psc));
	  }
	  follow(oldhreg) = makelist(hreg);
	  new_heap_free(hreg);
	  new_heap_free(hreg);
	}
      } while (hashtable1_iterator_advance(itr));
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[4] = makenil;
  }else{
    xsb_warn(CTXTc "Called with non-incremental predicate\n");
    reg[4] = makenil;
  }

  //  printterm(stdout,call_list,100);
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
}

int get_outedges_num(CTXTdeclc callnodeptr call1) {
  struct hashtable *h;	
  struct hashtable_itr *itr;

  h=call1->outedges->hasht;
  itr = hashtable1_iterator(h);       
  SQUASH_LINUX_COMPILER_WARN(itr) ;
  
  return hashtable1_count(h);
}

int immediate_affects_ptrlist(CTXTdeclc callnodeptr call1){
 
  VariantSF subgoal;
  int count = 0;
  CPtr oldhreg = NULL;
  struct hashtable *h;	
  struct hashtable_itr *itr;
  callnodeptr cn;
    
  reg[4] = makelist(hreg);
  new_heap_free(hreg);
  new_heap_free(hreg);
  
  if(IsNonNULL(call1)){ /* This can be called from some non incremental predicate */
    h=call1->outedges->hasht;
    
    itr = hashtable1_iterator(h);       
    if (hashtable1_count(h) > 0){
      do {
	cn = hashtable1_iterator_value(itr);
	if(IsNonNULL(cn->goal)){
	  count++;
	  subgoal = (VariantSF) cn->goal;      
	  check_glstack_overflow(4,pcreg,2); 
	  oldhreg=hreg-2;
          follow(oldhreg++) = makeint(subgoal);
	  follow(oldhreg) = makelist(hreg);
	  new_heap_free(hreg);
	  new_heap_free(hreg);
	}
      } while (hashtable1_iterator_advance(itr));
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[4] = makenil;
  }
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
}

int immediate_depends_ptrlist(CTXTdeclc callnodeptr call1){

  VariantSF subgoal;
  int  count = 0;
  CPtr oldhreg = NULL;
  calllistptr cl;

  reg[4] = makelist(hreg);
  new_heap_free(hreg);
  new_heap_free(hreg);
  if(IsNonNULL(call1)){ /* This can be called from some non incremental predicate */
    cl= call1->inedges;
    
    while(IsNonNULL(cl)){
      subgoal = (VariantSF) cl->inedge_node->callnode->goal;    
      if(IsNonNULL(subgoal)){/* fact check */
	count++;
	check_glstack_overflow(4,pcreg,2); 
	oldhreg = hreg-2;
	follow(oldhreg++) = makeint(subgoal);
	follow(oldhreg) = makelist(hreg);
	new_heap_free(hreg);
	new_heap_free(hreg);
      }
      cl=cl->next;
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[4] = makenil;
  }
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
}


/*
For a callnode call1 returns a Prolog list of callnode on which call1
immediately depends.
*/
int immediate_inedges_list(CTXTdeclc callnodeptr call1){

  VariantSF subgoal;
  TIFptr tif;
  int j, count = 0,arity;
  Psc psc;
  CPtr oldhreg = NULL;
  calllistptr cl;

  reg[4] = makelist(hreg);
  new_heap_free(hreg);
  new_heap_free(hreg);
  if(IsNonNULL(call1)){ /* This can be called from some non incremental predicate */
    cl= call1->inedges;
    
    while(IsNonNULL(cl)){
      subgoal = (VariantSF) cl->inedge_node->callnode->goal;    
      if(IsNonNULL(subgoal)){/* fact check */
	count++;
	tif = (TIFptr) subgoal->tif_ptr;
	psc = TIF_PSC(tif);
	arity = get_arity(psc);
	check_glstack_overflow(4,pcreg,2+(sizeof(Cell)*trie_path_heap_size(CTXTc subg_leaf_ptr(subgoal)))); 
	//	check_glstack_overflow(4,pcreg,2+arity*200); // don't know how much for build_subgoal_args...
	oldhreg = hreg-2;
	if(arity>0){
	  sreg = hreg;
	  follow(oldhreg++) = makecs(hreg);
	  hreg += arity + 1;
	  new_heap_functor(sreg, psc);
	  for (j = 1; j <= arity; j++) {
	    new_heap_free(sreg);
	    cell_array1[arity-j] = cell(sreg-1);
	  }		
	  load_solution_trie_no_heapcheck(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(subgoal));
	  //	  build_subgoal_args(arity,cell_array1,subgoal);		
	}else{
	  follow(oldhreg++) = makestring(get_name(psc));
	}
	follow(oldhreg) = makelist(hreg);
	new_heap_free(hreg);
	new_heap_free(hreg);
      }
      cl=cl->next;
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[4] = makenil;
  }else{
    xsb_warn(CTXTc "Called with non-incremental predicate\n");
    reg[4] = makenil;
  }
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
}


void print_call_node(callnodeptr call1){
  // not implemented
}


void print_call_graph(){
	// not implemented
}


/* Abolish Incremental Calls: 

To abolish a call for an incremental predicate it requires to check
whether any call is dependent on this call or not. If there are calls
that are dependent on this call (not cyclically) then this call should
not be deleted. Otherwise this call is deleted and the calls that are
supporting this call will also be checked for deletion. So deletion of
an incremental call can have a cascading effect.  

As cyclicity check has to be done we have a two phase algorithm to
deal with this problem. In the first phase we mark all the calls that
can be potentially deleted. In the next phase we unmarking the calls -
which should not be deleted. 
*/

void mark_for_incr_abol(CTXTdeclc callnodeptr);
void check_assumption_list(CTXTdecl);
call2listptr create_cdbllist(void);

/* Double Linked List functions */

call2listptr create_cdbllist(void){
  call2listptr l;
  SM_AllocateStruct(smCall2List,l);
  l->next=l->prev=l;
  l->item=NULL;
  return l;    
}

call2listptr insert_cdbllist(call2listptr cl,callnodeptr n){
  call2listptr l;
  SM_AllocateStruct(smCall2List,l);
  l->next=cl->next;
  l->prev=cl;
  l->item=n;    
  cl->next=l;
  l->next->prev=l;  
  return l;
}

void remove_callnode_from_list(call2listptr n){
  n->next->prev=n->prev;
  n->prev->next=n->next;

  return;
}


//--------------------------------------------------------------------------------------------

void unmark(CTXTdeclc callnodeptr c){
  callnodeptr c1;
  calllistptr in=c->inedges;

#ifdef INCR_DEBUG1
  printf("unmarking ");print_callnode(CTXTc stddbg,c);printf("\n");
#endif
  
  c->deleted=0;
  while(IsNonNULL(in)){
    c1=in->inedge_node->callnode;
    c1->outcount++;
    if(c1->deleted)
      unmark(CTXTc c1);
    in=in->next;
  }  
  
  return;
}




void check_assumption_list(CTXTdecl){
  calllistptr tempin,in=assumption_list_gl;
  call2listptr marked_ptr;
  callnodeptr c;

  while(in){
    tempin=in->next;
    marked_ptr=in->item2;
    c=marked_ptr->item;
#ifdef INCR_DEBUG1
      printf("in check assumption ");print_callnode(CTXTc stddbg,c);printf("\n");
#endif
    if(c->outcount>0){
      remove_callnode_from_list(marked_ptr);
      SM_DeallocateStruct(smCall2List,marked_ptr);

#ifdef INCR_DEBUG1
      printf("deleting ");print_callnode(CTXTc stddbg,c);printf("\n");
#endif
      
      if(c->deleted)   
	unmark(CTXTc c);      
    }
    SM_DeallocateStruct(smCallList, in);      
    in=tempin;
  }
  
  
  assumption_list_gl=NULL;
  return;
}

void free_incr_hashtables(TIFptr tif) {
  printf("free incr hash tables not implemented, memory leak\n");
}

extern void *hashtable1_iterator_key(struct hashtable_itr *);

void xsb_compute_scc(SCCNode * ,int * ,int,int *,struct hashtable*,int *,int * );
int return_scc_list(CTXTdeclc SCCNode *, Integer);

int  get_incr_sccs(CTXTdeclc Cell listterm) {
  Cell orig_listterm, intterm, node;
    Integer node_num=0;
    int i = 0, dfn, component = 1;     int * dfn_stack; int dfn_top = 0, ret;
    SCCNode * nodes;
    struct hashtable_itr *itr;     struct hashtable* hasht; 
    XSB_Deref(listterm);
    hasht = create_hashtable1(HASH_TABLE_SIZE, hashid, equalkeys);
    orig_listterm = listterm;
    intterm = get_list_head(listterm);
    XSB_Deref(intterm);
    //    printf("listptr %p @%p\n",listptr,(CPtr) int_val(*listptr));
    insert_some(hasht,(void *) oint_val(intterm),(void *) node_num);
    node_num++; 

    listterm = get_list_tail(listterm);
    XSB_Deref(listterm);
    while (!isnil(listterm)) {
      intterm = get_list_head(listterm);
      XSB_Deref(intterm);
      node = oint_val(intterm);
      if (NULL == search_some(hasht, (void *)node)) {
	insert_some(hasht,(void *)node,(void *)node_num);
	node_num++;
      }
      listterm = get_list_tail(listterm);
      XSB_Deref(listterm);
    }
    nodes = (SCCNode *) mem_calloc(node_num, sizeof(SCCNode),OTHER_SPACE); 
    dfn_stack = (int *) mem_alloc(node_num*sizeof(int),OTHER_SPACE); 
    listterm = orig_listterm;; 
    //printf("listptr %p @%p\n",listptr,(void *)int_val(*(listptr)));
    intterm = get_list_head(listterm);
    XSB_Deref(intterm);
    nodes[0].node = (CPtr) oint_val(intterm);
    listterm = get_list_tail(listterm);
    XSB_Deref(listterm);
    i = 1;
    while (!isnil(listterm)) {
      intterm = get_list_head(listterm);
      XSB_Deref(intterm);
      node = oint_val(intterm);
      nodes[i].node = (CPtr) node;
      listterm = get_list_tail(listterm);
      XSB_Deref(listterm);
      i++;
    }
    itr = hashtable1_iterator(hasht);       
    SQUASH_LINUX_COMPILER_WARN(itr);
    //    do {
    //      printf("k %p val %p\n",hashtable1_iterator_key(itr),hashtable1_iterator_value(itr));
    //    } while (hashtable1_iterator_advance(itr));

    listterm = orig_listterm;
    //    printf("2: k %p v %p\n",(void *) int_val(*listptr),
    //	   search_some(hasht,(void *) int_val(*listptr)));

    //    while (!isnil(*listptr)) {  now all wrong...
    //      listptr = listptr + 1;
    //      node = int_val(*clref_val(listptr));
    //      printf("2: k %p v %p\n",(CPtr) node,search_some(hasht,(void *) node));
    //      listptr = listptr + 1;
    //    }
    dfn = 1;
    for (i = 0; i < node_num; i++) {
      if (nodes[i].dfn == 0) 
	xsb_compute_scc(nodes,dfn_stack,i,&dfn_top,hasht,&dfn,&component);
      //      printf("++component for node %d is %d (high %d)\n",i,nodes[i].component,component);
    }
    ret = return_scc_list(CTXTc  nodes, node_num);
    hashtable1_destroy(hasht,0);
    mem_dealloc(nodes,node_num*sizeof(SCCNode),OTHER_SPACE); 
    mem_dealloc(dfn_stack,node_num*sizeof(int),OTHER_SPACE); 
    return ret;
}

void xsb_compute_scc(SCCNode * nodes,int * dfn_stack,int node_from, int * dfn_top,
		     struct hashtable* hasht,int * dfn,int * component ) {
  struct hashtable_itr *itr;
  struct hashtable* edges_hash;
  CPtr sf;
  Integer node_to;
  int j;

  //  printf("xsb_compute_scc for %d %p %s/%d dfn %d dfn_top %d\n",
  //	 node_from,nodes[node_from].node,
  //	 get_name(TIF_PSC(subg_tif_ptr(nodes[node_from].node))),
  //	 get_arity(TIF_PSC(subg_tif_ptr(nodes[node_from].node))),*dfn,*dfn_top);
  nodes[node_from].low = nodes[node_from].dfn = (*dfn)++;
  dfn_stack[*dfn_top] = node_from;
  nodes[node_from].stack = *dfn_top;
  (*dfn_top)++;
  edges_hash = subg_callnode_ptr(nodes[node_from].node)->outedges->hasht;
  itr = hashtable1_iterator(edges_hash);       
  if (hashtable1_count(edges_hash) > 0) {
    //    printf("found %d edges\n",hashtable1_count(edges_hash));
    do {
      sf = ((callnodeptr) hashtable1_iterator_value(itr))-> goal;
      node_to = (Integer) search_some(hasht, (void *)sf);
      //      printf("edge from %p to %p (%d)\n",(void *)nodes[node_from].node,sf,node_to);
      if (nodes[node_to].dfn == 0) {
	xsb_compute_scc(nodes,dfn_stack,(int)node_to, dfn_top,hasht,dfn,component );
	if (nodes[node_to].low < nodes[node_from].low) 
	  nodes[node_from].low = nodes[node_to].low;
	}	  
	else if (nodes[node_to].dfn < nodes[node_from].dfn  && nodes[node_to].component == 0) {
	  if (nodes[node_to].low < nodes[node_from].low) { nodes[node_from].low = nodes[node_to].low; }
	}
      } while (hashtable1_iterator_advance(itr));
    //    printf("nodes[%d] low %d dfn %d\n",node_from,nodes[node_from].low, nodes[node_from].dfn);
    if (nodes[node_from].low == nodes[node_from].dfn) {
      for (j = (*dfn_top)-1 ; j >= nodes[node_from].stack ; j--) {
	//	printf(" pop %d and assign %d\n",j,*component);
	nodes[dfn_stack[j]].component = *component;
      }
      (*component)++;       *dfn_top = j+1;
    }
  } 
  else nodes[node_from].component = (*component)++;
}


int return_scc_list(CTXTdeclc SCCNode * nodes, Integer num_nodes){
 
  VariantSF subgoal;
  TIFptr tif;

  int cur_node = 0,arity, j;
  Psc psc;
  CPtr oldhreg = NULL;

  reg[4] = makelist(hreg);
  new_heap_free(hreg);  new_heap_free(hreg);
  do {
    subgoal = (VariantSF) nodes[cur_node].node;
    tif = (TIFptr) subgoal->tif_ptr;
    psc = TIF_PSC(tif);
    arity = get_arity(psc);
    //    printf("subgoal %p, %s/%d\n",subgoal,get_name(psc),arity);
    check_glstack_overflow(4,pcreg,2+(sizeof(Cell)*trie_path_heap_size(CTXTc subg_leaf_ptr(subgoal)))); 
    //    check_glstack_overflow(4,pcreg,2+arity*200); // don't know how much for build_subgoal_args..
    oldhreg=hreg-2;                          // ptr to car
    if(arity>0){
      sreg = hreg;
      follow(oldhreg++) = makecs(sreg);      
      new_heap_functor(sreg,get_ret_psc(2)); //  car pts to ret/2  psc
      hreg += 3;                             //  hreg pts past ret/2
      sreg = hreg;
      follow(hreg-1) = makeint(nodes[cur_node].component);  // arg 2 of ret/2 pts to component
      follow(hreg-2) = makecs(sreg);         
      new_heap_functor(sreg, psc);           //  arg 1 of ret/2 pts to goal psc
      hreg += arity + 1;
      for (j = 1; j <= arity; j++) {
	new_heap_free(sreg);
	cell_array1[arity-j] = cell(sreg-1);
      }
      load_solution_trie_no_heapcheck(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(subgoal));
      //      build_subgoal_args(arity,cell_array1,subgoal);		
    } else{
      follow(oldhreg++) = makestring(get_name(psc));
    }
    follow(oldhreg) = makelist(hreg);        // cdr points to next car
    new_heap_free(hreg); new_heap_free(hreg);
    cur_node++;
  } while (cur_node  < num_nodes);
  follow(oldhreg) = makenil;                // cdr points to next car
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
}

// End of file
/*****************************************************************************/
/* Obsolete: use print_callnode() in debug_xsb, which also prints leaves 
 * void print_callnode_subgoal(callnodeptr c){
 *   //  printf("%d",c->id);
 *   if(IsNonNULL(c->goal))
 *     sfPrintGoal(stdout,c->goal,NO);
 *   else
 *     printf("fact");
 *   return;
 * }
*/
/*******
************* GENERATION OF CALLED_BY GRAPH ********************/

#ifdef UNDEFINED
%%% OBSOLETE
%%% void abolish_incr_call(CTXTdeclc callnodeptr p){
%%% 
%%%   marked_list_gl=create_cdbllist();
%%% 
%%% #ifdef INCR_DEBUG1
%%%   printf("marking phase starts\n");
%%% #endif
%%%   
%%% %%%   mark_for_incr_abol(CTXTc p);
%%%   check_assumption_list(CTXT);
%%% #ifdef INCR_DEBUG1
%%%   printf("assumption check ends \n");
%%% #endif
%%% 
%%%   delete_calls(CTXT);
%%% 
%%% 
%%% #ifdef INCR_DEBUG1
%%%   printf("delete call ends\n");
%%% #endif
%%% }
%%% 
%%% /* This is part of a "transitive abolish" to support eager recomputation */
%%% void delete_calls(CTXTdecl){
%%% 
%%%   call2listptr  n=marked_list_gl->next,temp;
%%%   callnodeptr c;
%%%   VariantSF goal;
%%% 
%%%   /* first iteration to delete inedges */
%%%   
%%%   while(n!=marked_list_gl){    
%%% %%%     c=n->item;
%%% %%%     if(c->deleted){
%%%       /* facts are not deleted */       
%%%       if(IsNonNULL(c->goal)){
%%% 	deleteinedges(CTXTc c);
%%%       }
%%%     }
%%%     n=n->next;
  }
%%% 
%%%   /* second iteration is to delete outedges and callnode */
%%% 
%%%   n=marked_list_gl->next;
%%%   while(n!=marked_list_gl){    
%%%     temp=n->next;
%%%     c=n->item;
%%%     if(c->deleted){
%%%       /* facts are not deleted */       
%%% %%%       if(IsNonNULL(c->goal)){

%%% 	goal=c->goal;
%%% 	deletecallnode(c);
%%% 	
%%% 	abolish_table_call(CTXTc goal,ABOLISH_TABLES_DEFAULT);  // will call abol_incr, but checks trans/single
%%%       }
%%%     }
%%%     SM_DeallocateStruct(smCall2List,n);
%%%     n=temp;
%%% %%%   }
%%%   
%%%   SM_DeallocateStruct(smCall2List,marked_list_gl);
%%%   marked_list_gl=NULL;
%%% %%%   return;
%%% }

%%%/*
%%%
%%%Input: takes a callnode Output: puts the callnode to the marked list
%%%and sets the deleted bit; puts it to the assumption list if it has any
%%%dependent calls not marked 
%%%
%%%*/
%%%
%%%void mark_for_incr_abol(CTXTdeclc callnodeptr c){
%%%  calllistptr in=c->inedges;
%%%  call2listptr markedlistptr;
%%%  callnodeptr c1;
%%%
%%%#ifdef INCR_DEBUG1 
%%%  printf("marking ");print_callnode(CTXTc stddbg, c);printf("\n");
%%%#endif
%%%
%%%  c->deleted=1;
%%%  markedlistptr=insert_cdbllist(marked_list_gl,c);
%%%  if(c->outcount)
%%%    ecall3(&assumption_list_gl,markedlistptr);
%%%    
%%%  while(IsNonNULL(in)){
%%%    c1=in->inedge_node->callnode;
%%%    c1->outcount--;
%%%    if(c1->deleted==0){
%%%      mark_for_incr_abol(CTXTc c1);
%%%    }
%%%    in=in->next;
%%%  }  
%%%  return;
%%%}

%%%static void inline ecall3(calllistptr *list, call2listptr item){
%%%  calllistptr  temp;
%%%  SM_AllocateStruct(smCallList,temp);
%%%  temp->item2=item;
%%%  temp->next=*list;
%%%  *list=temp;  
%%%}


//int cellarridx_gl;
//int maximum_dl_gl=0;
//callnodeptr callq[20000000];
//int callqptr=0;
//int no_add_call_edge_gl=0;
//int saved_call_gl=0,
//int factcount_gl=0;

//call2listptr marked_list_gl=NULL; /* required for abolishing incremental calls */ 

#endif

#ifdef UNDEFINED
%%%/* Creates list for eager update, and empties affected_gl list */
%%%int return_affected_list_for_update(CTXTdecl){
%%%  callnodeptr call1;
%%%  VariantSF subgoal;
%%%  TIFptr tif;
%%%  int j,count=0,arity;
%%%  Psc psc;
%%%  CPtr oldhreg=NULL;
%%%
%%%%%%  //  print_call_list(affected_gl);
%%%  if (!incr_table_update_safe_gl) {
%%%    xsb_abort("An incremental table has been abolished since this list was created.  Updates must be done lazily.\n");
%%%    return FALSE;
%%%  }
%%%  reg[4] = reg[3] = makelist(hreg);  // reg 3 first not-used, use regs in case of stack expanson
%%%  new_heap_free(hreg);   // make heap consistent
%%%  new_heap_free(hreg);
%%%  while((call1 = delete_calllist_elt(CTXTc &affected_gl)) != EMPTY){
%%%    subgoal = (VariantSF) call1->goal;      
%%%    if(IsNULL(subgoal)){ /* fact predicates */
%%%      call1->deleted = 0; 
%%%      continue;
%%%    }
%%%    //    fprintf(stddbg,"incrementally updating table for ");print_subgoal(stdout,subgoal);printf("\n");
%%%    if (subg_visitors(subgoal)) {
%%%      #ifdef ISO_INCR_TABLING
%%%      find_the_visitors(CTXTc subgoal);
%%%      #else
%%%      //      sprint_subgoal(CTXTc forest_log_buffer_1,0,subgoal);
%%%      #ifdef WARN_ON_UNSAFE_UPDATE
%%%      xsb_warn(CTXTc "%d Choice point(s) exist to the table for %s -- cannot incrementally update (create_call_list)\n",
%%%	       subg_visitors(subgoal),forest_log_buffer_1->fl_buffer);
%%%      #else
%%%      xsb_abort("%d Choice point(s) exist to the table for %s -- cannot incrementally update (create call list)\n",
%%%		subg_visitors(subgoal),forest_log_buffer_1->fl_buffer);
%%%      #endif
%%%      #endif
%%%      break;
%%%    }
%%%
%%%    count++;
%%%    tif = (TIFptr) subgoal->tif_ptr;
%%%    //    if (!(psc = TIF_PSC(tif)))
%%%    //	xsb_table_error(CTXTc "Cannot access dynamic incremental table\n");	
%%%    psc = TIF_PSC(tif);
%%%    arity = get_arity(psc);
%%%    check_glstack_overflow(4,pcreg,2+arity*200); // don't know how much for build_subgoal_args...
%%%    oldhreg = clref_val(reg[4]);  // maybe updated by re-alloc
%%%    if(arity>0){
%%%      sreg = hreg;
%%%      follow(oldhreg++) = makecs(sreg);
%%%      hreg += arity + 1;  // had 10, why 10?  why not 3? 2 for list, 1 for functor (dsw)
%%%      new_heap_functor(sreg, psc);
%%%      for (j = 1; j <= arity; j++) {
%%%	new_heap_free(sreg);
%%%	cell_array1[arity-j] = cell(sreg-1);
%%%      }
%%%      build_subgoal_args(arity,cell_array1,subgoal);		
%%%    }else{
%%%      follow(oldhreg++) = makestring(get_name(psc));
%%%    }
%%%    reg[4] = follow(oldhreg) = makelist(hreg);
%%%    new_heap_free(hreg);
%%%    new_heap_free(hreg);
%%%  }
%%%  if(count > 0) {
%%%    follow(oldhreg) = makenil;
%%%    hreg -= 2;  /* take back the extra words allocated... */
%%%  } else
%%%    reg[3] = makenil;
    
%%%  //  printterm(stdout,call_list,100);
%%%
%%%  return unify(CTXTc reg_term(CTXTc 2),reg_term(CTXTc 3));
%%%
%%%  /*
%%%    int i;
%%%    for(i=0;i<callqptr;i++){
%%%      if(IsNonNULL(callq[i]) && (callq[i]->deleted==1)){
%%%    sfPrintGoal(stdout,(VariantSF)callq[i]->goal,NO);
%%%    printf(" %d %d\n",callq[i]->falsecount,callq[i]->deleted);
%%%    }
%%%    }
%%%    printf("-----------------------------\n");
%%%  */
%%%}

/* reg 1: tag for this call
   reg 2: filter list of goals to keep (keep all if [])
   reg 3: returned list of changed goals
   reg 4: used as temp (in case of heap expansion)
 */
// int return_changed_call_list(CTXTdecl){
//   callnodeptr call1;
//   VariantSF subgoal;
//   TIFptr tif;
//   int j, count = 0,arity;
//   Psc psc;
//   CPtr oldhreg = NULL;
// 
//   if (!incr_table_update_safe_gl) {
//     xsb_abort("An incremental table has been abolished since this list was created.  Updates must be done lazily.\n");
//     return FALSE;
//   }
//   reg[4] = makelist(hreg);
//   new_heap_free(hreg);   // make heap consistent
//   new_heap_free(hreg);
//   while ((call1 = delete_calllist_elt(CTXTc &changed_gl)) != EMPTY){
//     subgoal = (VariantSF) call1->goal;      
//     tif = (TIFptr) subgoal->tif_ptr;
//     psc = TIF_PSC(tif);
//     if (in_reg2_list(CTXTc psc)) {
//       count++;
//       arity = get_arity(psc);
//       check_glstack_overflow(4,pcreg,2+arity*200); // guess for build_subgoal_args...
//       oldhreg = hreg-2;
//       if(arity>0){
// 	sreg = hreg;
// 	follow(oldhreg++) = makecs(hreg);
// 	hreg += arity + 1;
// 	new_heap_functor(sreg, psc);
// 	for (j = 1; j <= arity; j++) {
// 	  new_heap_free(sreg);
// 	  cell_array1[arity-j] = cell(sreg-1);
// 	}
// 	build_subgoal_args(arity,cell_array1,subgoal);		
//       }else{
// 	follow(oldhreg++) = makestring(get_name(psc));
//       }
//       follow(oldhreg) = makelist(hreg);
//       new_heap_free(hreg);   // make heap consistent
//       new_heap_free(hreg);
//     }
//   }
//   if (count>0)
//     follow(oldhreg) = makenil;
//   else
//     reg[4] = makenil;
//     
//   return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
// }
// 

//---------------------------------------------------------------------------
// does not destroy list

//int call_list_to_prolog(CTXTdeclc calllistptr list_head){
//  calllistptr listptr = list_head;
//  callnodeptr call1 = list_head->item;
//  VariantSF subgoal;
//  TIFptr tif;
//  int j, count = 0,arity;
//  Psc psc;
//  CPtr oldhreg = NULL;
//  //  print_call_list(affected_gl);
//  if (!incr_table_update_safe_gl) {
//    xsb_abort("An incremental table has been abolished since this list was created.  Updates must be done lazily.\n");
//    return FALSE;
//  }
//  //  printf("cltp %p %p\n",listptr,call1);
//  //  printterm(stddbg,reg[3],25); printf(" -3.1- \n");
//  reg[4] = reg[3] = makelist(hreg);  // reg 3 first not-used, use regs in case of stack expanson
//  new_heap_free(hreg);   // make heap consistent
//  new_heap_free(hreg);
//
//  while (call1  != EMPTY){
//    if (call1->goal) {
//      subgoal = (VariantSF) call1->goal;      
//    count++;
//    tif = (TIFptr) subgoal->tif_ptr;
//    //    if (!(psc = TIF_PSC(tif)))
//    //	xsb_table_error(CTXTc "Cannot access dynamic incremental table\n");	
//    psc = TIF_PSC(tif);
//    arity = get_arity(psc);
//    check_glstack_overflow(4,pcreg,2+arity*200); // don't know how much for build_subgoal_args...
//    oldhreg = clref_val(reg[4]);  // maybe updated by re-alloc
//    if(arity>0){
//      sreg = hreg;
//      follow(oldhreg++) = makecs(sreg);
//      hreg += arity + 1;  // had 10, why 10?  why not 3? 2 for list, 1 for functor (dsw)
//      new_heap_functor(sreg, psc);
//      for (j = 1; j <= arity; j++) {
//	new_heap_free(sreg);
//	cell_array1[arity-j] = cell(sreg-1);
//      }
//      build_subgoal_args(arity,cell_array1,subgoal);		
//    }else{
//      follow(oldhreg++) = makestring(get_name(psc));
//    }
//    reg[4] = follow(oldhreg) = makelist(hreg);
//    new_heap_free(hreg);
//    new_heap_free(hreg);
//    }
//    listptr = listptr->next;
//    call1 = listptr->item;
//  }
//
//  if(count > 0) {
//    follow(oldhreg) = makenil;
//    hreg -= 2;  /* take back the extra words allocated... */
//  } else
//    reg[3] = makenil;
//    
//  //  printterm(stdout,call_list,100);
//
//  return unify(CTXTc reg_term(CTXTc 2),reg_term(CTXTc 3));
//}

#endif
