/* File:      tries.h
** Author(s): Ernie Johnson, Prasad Rao, Kostis Sagonas
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
** $Id: tries.h,v 1.85 2013-04-20 19:33:51 tswift Exp $
** 
*/


#ifndef PUBLIC_TRIE_DEFS


#define PUBLIC_TRIE_DEFS

#include "struct_manager.h"

/*===========================================================================*/

/*
 *	A B S T R A C T E D   T R I E   R E P R E S E N T A T I O N
 *	===========================================================
 *
 *  There are several types of operations which are common to any trie,
 *  either to the structure as a whole, or to its components.  Likewise,
 *  there are places in the engine where it won't be apparent what type
 *  of trie (respectively, component) one is inspecting.  Although this
 *  can be ascertained, adherence to the following guidelines will make
 *  this unnecessary, as well as the need for any type-dependent special
 *  processing.  A set of macros are supplied for use in these "don't
 *  know" or "don't care" situations.
 *
 *  All types of trie nodes must be laid out in the same fashion, with
 *  common fields appearing in the same order.  Fields which point to
 *  like-component records may have different types, but the others
 *  (Info and Symbol) are fixed.  Currently, the layout is as indicated
 *  below.  Except for the Info field which MUST be first, the ordering
 *  of the other fields is arbitrary.  Field names have also been
 *  standardized to help ensure conformity.
 *
 *  We use the following notation:
 *    TN_*         Trie Node operation
 *    TrieHT_*     Trie Hash Table operation  (see trie_internals.h)
 *    TSC_*        Trie SubComponent operation (applicable to a TN or THT)
 *    pTN          pointer to a Trie Node
 *    pTHT         pointer to a Trie Hash Table
 *    pTSC pointer to a Trie SubComponent
 */

#define TN_Instr(pTN)		TSC_Instr(pTN)
#define TN_Status(pTN)		TSC_Status(pTN)
#define TN_TrieType(pTN)	TSC_TrieType(pTN)
#define TN_NodeType(pTN)	TSC_NodeType(pTN)
#define TN_IncrMarking(pTN)	TSC_IncrMarking(pTN)

#define TN_Parent(pTN)		( (pTN)->parent )
#define TN_Child(pTN)		( (pTN)->child )
#define TN_Sibling(pTN)		( (pTN)->sibling )
#define TN_Symbol(pTN)		( (pTN)->symbol )

#define TSC_Instr(pTSC)		IPT_Instr((pTSC)->info)
#define TSC_Status(pTSC)	IPT_Status((pTSC)->info)
#define TSC_TrieType(pTSC)	IPT_TrieType((pTSC)->info)
#define TSC_NodeType(pTSC)	IPT_NodeType((pTSC)->info)
#define TSC_IncrMarking(pTSC)	IPT_IncrMarking((pTSC)->info)


#define TN_SetHashHdr(pTN,pTHT)		TN_Child(pTN) = (void *)(pTHT)
#define TN_GetHashHdr(pTN)		TN_Child(pTN)

#define FREE_TRIE_NODE_MARK -1
#define FREE_TRIE_BLOCK_MARK -2

/*===========================================================================*/

/*
 *                      Trie Component Info Field
 *                      =========================
 *
 *  Defines a field one word in size which contains miscellaneous info
 *  needed for maximizing trie use: An instruction field allows tries to
 *  be traversed for term unification with speeds comparable to code
 *  execution for facts.  A status field signifies whether a particular
 *  node represents a valid subbranch of the trie.  Nodes (paths) may
 *  temporarily be marked as deleted until such time they can be safely
 *  removed from the trie.  Two type fields contain annotations which
 *  describe the type of trie in which a structure resides and the role
 *  the structure plays within this trie, respectively.
 *
 *  Each of the two main components of a trie contain this field: trie
 *  nodes and hash table headers.
 *
 *  For valid trie-embedded instruction values, see file "inst_xsb.h".  */

typedef struct InstructionPlusTypeFrame {
  byte instr;		/* contains compiled trie code */
  byte status;		/* whether the node has been deleted */
  byte trie_type;	/* global info: what type of trie is this? */
  byte node_type:6;	/* local info: what is the role of this struct? */
  byte incr_marking:2;	/* local info: what is the role of this struct? */
#ifdef BITS64
  byte padding[4];
#endif
} InstrPlusType;

#define IPT_Instr(IPT)		((IPT).instr)
#define IPT_Status(IPT)		((IPT).status)
#define IPT_TrieType(IPT)	((IPT).trie_type)
#define IPT_NodeType(IPT)	((IPT).node_type)
#define IPT_IncrMarking(IPT)	((IPT).incr_marking)

extern char *trie_node_type_table[9];
extern char *trie_trie_type_table[6];

#define PREVIOUSLY_UNCONDITIONAL 1

/* Information for initializing dynamic trie structure managers */

#define  BTN_NAME       0
#define  BTHT_NAME      1
#define  PRODSF_NAME    2
#define  CONSSF_NAME    3
#define  TSTNSF_NAME    4
#define  TSINSF_NAME    5
#define  TSTHT_NAME     6

extern char *TrieSMNameTable[7];

/*===========================================================================*/

/*
 *                         B A S I C   T R I E S
 *                         =====================
 *
 *  Each symbol in a term is maintained in a separate node in a trie.
 *  Paths from the root to the leaves trace out the terms stored in the
 *  trie.  Sharing between terms takes place only high-up within a trie,
 *  corresponding to left-to-right term factoring.
 *
 *  Children of a particular node are maintained in a linked-list pointed
 *  to by the Child field.  This list is maintained through the Sibling
 *  fields of the child nodes.  To facilitate rapid access when the
 *  number of children becomes large, a hash table is employed.  In this
 *  case, the parent node's Child field points to a header structure for
 *  the hash table, rather than directly to one of the children.
 *
 *  When used to build Call Tries, the Child field of a leaf trie node
 *  points to a SubgoalFrame which contains additional info for that call.
 */

typedef struct Basic_Trie_Node *BTNptr;
typedef struct Basic_Trie_Node {
  InstrPlusType info;
  BTNptr sibling;
  BTNptr child;
  BTNptr parent;
  Cell symbol;
} BasicTrieNode;

/* - - Preferred macros - - - - */
#define BTN_Instr(pBTN)		TN_Instr(pBTN)
#define BTN_Status(pBTN)	TN_Status(pBTN)
#define BTN_TrieType(pBTN)	TN_TrieType(pBTN)
#define BTN_NodeType(pBTN)	TN_NodeType(pBTN)
#define BTN_IncrMarking(pBTN)	TN_IncrMarking(pBTN)
#define BTN_Parent(pBTN)	TN_Parent(pBTN)
#define BTN_Child(pBTN)		TN_Child(pBTN)
#define BTN_Sibling(pBTN)	TN_Sibling(pBTN)
#define BTN_Symbol(pBTN)	TN_Symbol(pBTN)

/* - - For backwards compatibility - - - - */
typedef struct Basic_Trie_Node *NODEptr;
#define Instr(X)	BTN_Instr(X)
#define TrieType(X)     BTN_TrieType(X)
#define NodeType(X)     BTN_NodeType(X)
#define Parent(X)	BTN_Parent(X)
#define Child(X)	BTN_Child(X)
#define Sibl(X)		BTN_Sibling(X)
#define Atom(X)		BTN_Symbol(X)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *                        Basic Trie Hash Tables
 *                        ----------------------
 */

typedef struct Basic_Trie_HashTable *BTHTptr;
typedef struct Basic_Trie_HashTable {
  InstrPlusType info;
  UInteger  numContents;
  UInteger  numBuckets;
  BTNptr *pBucketArray;
  BTHTptr prev, next;		   /* DLL needed for branch deletion */
} BasicTrieHT;

/* Field Access Macros
   ------------------- */
#define BTHT_Instr(pTHT)		TrieHT_Instr(pTHT)
#define BTHT_Status(pTHT)		TrieHT_Status(pTHT)
#define BTHT_TrieType(pTHT)		TrieHT_TrieType(pTHT)
#define BTHT_NodeType(pTHT)		TrieHT_NodeType(pTHT)
#define BTHT_NumBuckets(pTHT)		TrieHT_NumBuckets(pTHT)
#define BTHT_NumContents(pTHT)		TrieHT_NumContents(pTHT)
#define BTHT_BucketArray(pTHT)		TrieHT_BucketArray(pTHT)
#define BTHT_PrevBTHT(pTHT)		TrieHT_PrevHT(pTHT)
#define BTHT_NextBTHT(pTHT)		TrieHT_NextHT(pTHT)

#define BTHT_GetHashSeed(pTHT)		TrieHT_GetHashSeed(pTHT)


/*===========================================================================*/

/*
 *                  T I M E - S T A M P E D   T R I E S
 *                  ===================================
 *
 *  Similar in construction and maintenance to normal tries, these extend
 *  the basic design in order to support answer subsumption.  Conditions
 *  under which hash tables are created and the way symbols are stored in
 *  the nodes are identical.  (See the file trie_internals.h for details.)
 *
 *  A timestamp is maintained in each node of the Time-Stamped Trie (TST).
 *  When used for Answer Sets, the timestamp kept in each node is the
 *  maximum of the timestamps of its children.  Since timestamps
 *  monotonically increase as terms are entered, this property can be
 *  easily maintained by propagating the timestamp of a newly interned
 *  term from the leaf to the root.  Hence, the root ALWAYS contains the
 *  timestamp of the largest-timestamped answer contained in the Answer
 *  Set.
 *
 *  For facilitating certain subsumptive operations, it is important to
 *  quickly identify nodes having a timestamp greater than a given one.
 *  When a sibling chain becomes long, it is no longer acceptable to
 *  perform a linear scan in order to identify these timestamp-valid
 *  nodes.  Therefore, when we create a hash table for a group of nodes,
 *  we also create an auxiliary structure which maintains the nodes in
 *  decreasing timestamp order.  Each node's TimeStamp field is then used
 *  for pointing to an associated frame in this structure, where the
 *  timestamp is now kept.  The hash header is extended -- over the basic
 *  trie hash header -- to contain fields for maintaining these frames in
 *  a doubly linked list.  Once the Answer Set is completed, these
 *  structures can be disposed.  To facilitate this, hash tables, within
 *  a particular TST, are chained together from the root, accessible from
 *  its Sibling field.  Lazy evaluation...
 */

typedef UInteger  TimeStamp;

typedef struct Time_Stamped_Trie_Node *TSTNptr;
typedef struct Time_Stamped_Trie_Node {
  InstrPlusType info;
  TSTNptr sibling;
  TSTNptr child;
  TSTNptr parent;
  Cell symbol;
  TimeStamp ts;
} TS_TrieNode;

/* Field Access Macros
   ------------------- */
#define TSTN_Instr(pTSTN)	TN_Instr(pTSTN)
#define TSTN_Status(pTSTN)	TN_Status(pTSTN)
#define TSTN_TrieType(pTSTN)	TN_TrieType(pTSTN)
#define TSTN_NodeType(pTSTN)	TN_NodeType(pTSTN)
#define TSTN_Parent(pTSTN)	TN_Parent(pTSTN)
#define TSTN_Child(pTSTN)	TN_Child(pTSTN)
#define TSTN_Sibling(pTSTN)	TN_Sibling(pTSTN)
#define TSTN_Symbol(pTSTN)	TN_Symbol(pTSTN)
#define TSTN_TimeStamp(pTSTN)	( (pTSTN)->ts )

/*===========================================================================*/

/*
 *                             Answer List Node
 *                             ================
 *
 *  A global resource for ALNs is currently implemented.  Blocks of memory
 *  for ALN storage are allocated whenever this resource is depleted.  All
 *  ALNs are allocated from this resource.  To allow rapid deallocation of
 *  these block-malloc'ed structures, the first word in the structure must
 *  contain the field used to link them into a chain when in use.
 */

typedef struct Answer_List_Node *ALNptr;
typedef struct Answer_List_Node {
  ALNptr link;
  BTNptr answer_leaf;
  //  ALNptr backlink;
} AnsListNode;

#define ALN_Next(pALN)		((pALN)->link)
#define ALN_Answer(pALN)	((pALN)->answer_leaf)

/*===========================================================================*/

/*
 *                      Tabled-Call Lookup Structures
 *                      =============================
 *
 *  Data structures for parameter passing to and from the call
 *  check/insert routines.
 */

typedef struct Tabled_Call_Info_Record {
  struct Table_Info_Frame *table_info_record;
  int call_arity;
  CPtr arg_vector;         /* input arguments of call (ptr to wam registers) */
  CPtr var_vector_loc;     /* location to store the call var vector */
} TabledCallInfo;

#define CallInfo_TableInfo(CallInfo)	( (CallInfo).table_info_record )
#define CallInfo_CallArity(CallInfo)	( (CallInfo).call_arity )
#define CallInfo_Arguments(CallInfo)	( (CallInfo).arg_vector )
#define CallInfo_AnsTempl(CallInfo)	( (CallInfo).var_vector_loc )

/* TLS: there isn't any need for AnsTempl (var_vector) here that I can see */
typedef struct Call_Check_Insert_Results {
  BTNptr call_trie_term;
  struct subgoal_frame *subsumers_sgf;
  int variant_found;
  CPtr var_vector;         /* pointer to the answer template */
} CallLookupResults;

#define CallLUR_Leaf(CLUR)		( (CLUR).call_trie_term )
#define CallLUR_Subsumer(CLUR)		( (CLUR).subsumers_sgf )
#define CallLUR_VariantFound(CLUR)	( (CLUR).variant_found )
#define CallLUR_AnsTempl(CLUR)		( (CLUR).var_vector )

/*===========================================================================*/


/*-- exported trie functions ------------------------------------------*/

#ifndef MULTI_THREAD
extern int unify_abstractions_from_AT(CPtr , int);
extern void unify_abstractions_from_absStk(void);
extern void copy_abstractions_to_AT(CPtr ,int);

extern int AnsVarCtr;
extern CPtr *VarEnumerator_trail_top;

extern BTNptr   newBasicTrie(Cell,int);
extern byte *	trie_get_calls(void);
extern Cell	get_lastnode_cs_retskel(Cell);
extern byte *	trie_get_return(struct subgoal_frame *, Cell, int);
extern void     init_trie_aux_areas(void);
extern void     free_trie_aux_areas(void);
extern void     load_solution_trie(int, int, CPtr, BTNptr);
extern void     load_solution_trie_no_heapcheck(int, int, CPtr, BTNptr);
extern void     load_solution_trie_notrail(int, int, CPtr, BTNptr);
extern int      variant_call_search(TabledCallInfo *, CallLookupResults *);
extern BTNptr   trie_assert_chk_ins(CPtr, BTNptr, int *);
extern BTNptr   trie_intern_chk_ins(Cell, BTNptr *, int *, int, int);
extern BTNptr	get_next_trie_solution(ALNptr *);
extern BTNptr	variant_answer_search(int, int, CPtr, struct subgoal_frame *,
				      xsbBool *,xsbBool *);
extern BTNptr   delay_chk_insert(int, CPtr, CPtr *);
//extern void     undo_answer_bindings(void);
extern void	load_delay_trie(int, CPtr, BTNptr);
extern xsbBool  bottom_up_unify(void);
extern BTHTptr  New_BTHT(Structure_Manager *, int);
extern int trie_path_heap_size(BTNptr pLeaf);
#else
struct th_context ;

extern int unify_abstractions_from_AT(struct th_context *, CPtr , int);
extern void unify_abstractions_from_absStk(struct th_context *);
extern void copy_abstractions_to_AT(struct th_context *, CPtr, int);

extern BTHTptr  New_BTHT(struct th_context *, Structure_Manager *, int);
extern BTNptr   newBasicTrie(struct th_context *, Cell,int);
extern byte *	trie_get_calls(struct th_context *);
extern Cell	get_lastnode_cs_retskel(struct th_context *, Cell);
extern byte *	trie_get_return(struct th_context *, struct subgoal_frame *, Cell, int);
extern void     init_trie_aux_areas(struct th_context *);
extern void     free_trie_aux_areas(struct th_context *);
extern void     load_solution_trie(struct th_context *, int, int, CPtr, BTNptr);
extern void     load_solution_trie_no_heapcheck(struct th_context *, int, int, CPtr, BTNptr);
extern void     load_solution_trie_notrail(struct th_context *, int, int, CPtr, BTNptr);
extern int      variant_call_search(struct th_context *, TabledCallInfo *, CallLookupResults *);
extern BTNptr   trie_assert_chk_ins(struct th_context *, CPtr, BTNptr, int *);
extern BTNptr   trie_intern_chk_ins(struct th_context *, Cell, BTNptr *, int *, int, int);
extern BTNptr	get_next_trie_solution(ALNptr *);
extern BTNptr	variant_answer_search(struct th_context *, int, int, CPtr, 
				      struct subgoal_frame *, xsbBool *, xsbBool *);
extern BTNptr   delay_chk_insert(struct th_context *, int, CPtr, CPtr *);
//extern void     undo_answer_bindings(struct th_context *);
extern void	load_delay_trie(struct th_context *, int, CPtr, BTNptr);
extern xsbBool  bottom_up_unify(struct th_context *);
extern int trie_path_heap_size(struct th_context *, BTNptr pLeaf);
#endif

#ifndef MULTI_THREAD
extern void    consume_subsumptive_answer(BTNptr, int, CPtr);
extern ALNptr  tst_collect_relevant_answers(TSTNptr, TimeStamp, size_t, CPtr);
extern void    delete_subsumptive_predicate_table(struct Table_Info_Frame *);
#else
extern void    consume_subsumptive_answer(struct th_context *, BTNptr, int, CPtr);
extern ALNptr  tst_collect_relevant_answers(struct th_context *, TSTNptr, TimeStamp, size_t, CPtr);
extern void    delete_subsumptive_predicate_table(struct th_context *, struct Table_Info_Frame *);
#endif

#ifndef MULTI_THREAD
extern void    tstShrinkDynStacks(void);

extern TSTNptr subsumptive_tst_search(TSTNptr, int, CPtr, xsbBool, xsbBool *);
//extern BTNptr  subsumptive_bt_search(BTNptr, int, CPtr, xsbBool *);
// extern TSTNptr variant_tst_search(TSTNptr, int, CPtr, xsbBool, xsbBool *);
// extern BTNptr  variant_bt_search(BTNptr, int, CPtr, xsbBool *);
#else
extern void    tstShrinkDynStacks(struct th_context *);

extern TSTNptr subsumptive_tst_search(struct th_context *,
				TSTNptr, int, CPtr, xsbBool, xsbBool *);
//extern BTNptr  subsumptive_bt_search(struct th_context *,
//				BTNptr, int, CPtr, xsbBool *);
//extern TSTNptr variant_tst_search(struct th_context *,
//				TSTNptr, int, CPtr, xsbBool, xsbBool *);
// extern BTNptr  variant_bt_search(struct th_context *,
//				 BTNptr, int, CPtr, xsbBool *);
#endif


typedef enum Trie_Path_Type {
  NO_PATH, VARIANT_PATH, SUBSUMPTIVE_PATH
} TriePathType;

#ifndef MULTI_THREAD
extern void *subsumptive_trie_lookup(void *root, int, CPtr,
				     TriePathType *, Cell[]);
extern void *variant_trie_lookup(void *root, int, CPtr, Cell[]);
#else
extern void *subsumptive_trie_lookup(struct th_context *th, void *root, int, CPtr,
				     TriePathType *, Cell[]);
extern void *variant_trie_lookup(struct th_context *th, void *root, int, CPtr, Cell[]);
#endif

/*---------------------------------------------------------------------*/

/* slg variables */
#ifndef MULTI_THREAD
/* Pointer to answers subst fact (in heap) */
extern CPtr ans_var_pos_reg;

/* Used by trie instructions */
extern int  trieinstr_vars_num;
extern int  global_trieinstr_vars_num;
extern CPtr trieinstr_unif_stkptr;
extern CPtr *trieinstr_vars;
#endif

/* used for statistics */
extern counter subg_chk_ins, subg_inserts, ans_chk_ins, ans_inserts;

#define undo_answer_bindings					\
  while (VarEnumerator_trail_top >= VarEnumerator_trail) {		\
    /*printf("reset %p=VarEnum[%d]\n",*VarEnumerator_trail_top,VarEnumerator_trail_top-VarEnumerator_trail);*/ \
    untrail(*VarEnumerator_trail_top);					\
    VarEnumerator_trail_top--;						\
  }	

typedef struct SL_node_t *SL_node_ptr;
typedef struct SL_node_t {
  SL_node_ptr SL_next;
  void *SL_down;
  TimeStamp SL_ts;
} SL_node;

#ifndef MULTI_THREAD
/* trie routine variables */
extern BTNptr Last_Nod_Sav;

extern int vcs_tnot_call;
#endif


/*----------------------------------------------------------------------*/

/* expand (or allocate) the array by doubling its size or the size needed, whichever is larger*/
#define trie_expand_array(ArrType,ArrayNam, ArraySz, NeededSz, Nam) {\
    Integer Siz = ArraySz;\
    if (Siz == 0) ArraySz = DEFAULT_ARRAYSIZ;\
    else ArraySz = 2 * ArraySz;\
    if (ArraySz < NeededSz) ArraySz = NeededSz;\
    ArrayNam = mem_realloc(ArrayNam,Siz*sizeof(ArrType),ArraySz*sizeof(ArrType),TABLE_SPACE);\
    if (ArrayNam == NULL) \
      xsb_exit("No More memory for reallocating Array");\
}

#define will_overflow_trieinstr_unif_stk(x) {\
   if (x >= trieinstr_unif_stk+trieinstr_unif_stk_size) {\
     Integer idx = trieinstr_unif_stkptr - trieinstr_unif_stk;	\
     trie_expand_array(Cell,trieinstr_unif_stk,trieinstr_unif_stk_size,x-trieinstr_unif_stk,"trieinstr_unif_stk");\
     trieinstr_unif_stkptr = trieinstr_unif_stk + idx;\
   }\
}

#define push_trieinstr_unif_stk(X) {\
   will_overflow_trieinstr_unif_stk(trieinstr_unif_stkptr+1);\
   (*(++trieinstr_unif_stkptr)) = X;\
}

#define trieinstr_unif_stack_print  {					\
    CPtr temp_reg = trieinstr_unif_stkptr; ;				\
    while (!(temp_reg == trieinstr_unif_stk)) {				\
      print_cp_cell("trieinstr", temp_reg, *temp_reg);			\
      temp_reg--;							\
    }						  \
  }						 


/*----------------------------------------------------------------------*/

/* used in bottom-up return of answers (e.g. load_solution_trie) */
extern int  num_heap_term_vars;
extern CPtr *var_addr;
extern int  var_addr_arraysz;

/* similar .. used in get_residual */
extern CPtr *var_addr_accum;
extern int  var_addr_accum_num;

/*----------------------------------------------------------------------*/

extern Cell * trieinstr_unif_stk;
extern Integer trieinstr_unif_stk_size;
extern int delay_it;

//#define DEFAULT_ARRAYSIZ 512 
#define DEFAULT_ARRAYSIZ 16

#define DEFAULT_NUM_TRIEVARS 512
extern size_t current_num_trievars;

/*=========================================================================*/

typedef struct Abstraction_Frame {
  CPtr originalTerm;
  CPtr abstractedTerm;
} AbstractionFrame;

extern int callAbsStk_index;
extern AbstractionFrame *callAbsStk;
extern int callAbsStk_size;

#define pop_AbsStk callAbsStk[--callAbsStk_index]

#define push_AbsStk(Orig,Abs) {                 \
    if (callAbsStk_index+1 >= callAbsStk_size) {\
      trie_expand_array(CPtr, callAbsStk ,callAbsStk_size,0,"callAbsStk");\
    }\
    callAbsStk[callAbsStk_index].originalTerm = ((CPtr) Orig);\
    callAbsStk[callAbsStk_index].abstractedTerm = ((CPtr) Abs);\
    callAbsStk_index++; \
  }

#define  CallAbsStk_ResetTOS callAbsStk_index = 0;

/*=========================================================================*/
/* Handling calls in call subsumption. */

struct VariantContinuation {
  BTNptr last_node_matched;
  struct subterms_desc {
    counter num;		/* number of subterms in the stack */
    struct termstack_desc{
      size_t size;		/* number of elements in the stack */
      Cell *ptr;		/* dynamic memory allocated for the stack */
    } stack;
  } subterms;
  struct bindings_desc {
    counter num;		/* number of bindings in the trail */
    struct trail_desc{
      size_t size;		/* number of elements in the trail */
      struct frame {
	CPtr var;
	Cell value;
      } *ptr;			/* dynamic memory allocated for the trail */
    } stack;
  } bindings;
};

typedef struct {
  BTNptr alt_node;	/* node from which to continue the search */
  BTNptr var_chain;	/* beginning of variable chain */
  int termstk_top_index;  /* current top-of-tstTermStack at CP creation */
  int log_top_index;	/* current top-of-tstTermStackLog at CP creation */
  int trail_top_index;	/* current top-of-tstTrail at CP creation */
} tstCallChoicePointFrame;

typedef tstCallChoicePointFrame *pCPFrame;
#define CALL_CPSTACK_SIZE   1024

struct tstCCPStack_t {
  pCPFrame top;          /* next available location to place an entry */
  pCPFrame ceiling;      /* overflow pointer: ptr to CPF off array end */
  tstCallChoicePointFrame base[CALL_CPSTACK_SIZE];
};

typedef struct {
  TSTNptr alt_node;	/* sibling of the TSTN whose child ptr we took */
  size_t ts_top_index;	/* current top-of-tstTermStack at CP creation */
  size_t log_top_index;	/* current top-of-tstTermStackLog at CP creation */
  CPtr *trail_top;	/* current top-of-trail at CP creation */
  CPtr heap_bktrk;	/* current hbreg at time of CP creation */
} tstChoicePointFrame;

#define TST_CPSTACK_SIZE   1024

struct tstCPStack_t {
  tstChoicePointFrame *top;     /* next available location to place an entry */
  tstChoicePointFrame *ceiling; /* overflow pointer: points beyond array end */
  tstChoicePointFrame base[TST_CPSTACK_SIZE];
};


/*=========================================================================*/

/* Called-by Graph: 
   additional structures to handle incremental evaluation  
*/


typedef struct callnodetag* callnodeptr;
typedef struct _calllist* calllistptr;
typedef struct _call2list* call2listptr;
typedef struct outedge* outedgeptr;


typedef struct outedge{
	struct hashtable* hasht; 
	callnodeptr callnode;
}OUTEDGE;

/* tls: this is a callnodeptr;
   goal --          For tabled calls, this is a pointer to the SF; null for dynamics
   no_of_answers -- Used as estimate of whether the answers in the subgoal have changed. 
   deleted --       This callnodeptr refers to an affected call that is being/has been updated.
                    used for cycles in dfs_outedges.
   changed --       This call has updated its answers.
   prev_call --     When reevaluating an affected call, used to compare this to prev.
*/

typedef struct callnodetag{
  outedgeptr  outedges;
  calllistptr inedges; 
  void* goal;  
  unsigned int no_of_answers;
  unsigned int deleted:1, changed:1,recomputable:1,is_incremental_trie:1,falsecount:14,outcount:14;
  union{
    callnodeptr prev_call;  
    void* tif_ptr;	  /* Table of which this call is a part */
  };
  union{
    ALNptr aln; 
    BTNptr leaf_ptr;
  };
  int id; 
  //  unsigned int no_of_undefs;
}CALL_NODE;

#define callnode_sf(Ptr) ( ((callnodeptr)(Ptr)) -> goal)
#define callnode_no_undefs(Ptr)  ( ((callnodeptr)(Ptr)) -> no_of_undefs)
#define callnode_tif_ptr(Ptr) ( ((callnodeptr)(Ptr)) -> tif_ptr)
#define callnode_leaf_ptr(Ptr) ( ((callnodeptr)(Ptr)) -> leaf_ptr)

#define is_fact_in_callgraph(Ptr)  (callnode_sf(Ptr) == 0)

typedef struct key{
	prolog_int goal;
} KEY;


typedef struct _calllist{
  union{
    callnodeptr item;  /* when used in list as in nq */
    outedgeptr inedge_node; /* when used in inedges */
    call2listptr item2; /* when used in abolishing*/
  };
  calllistptr next;
}CALLLIST;

#define calllist_item(Ptr) ( ((calllistptr)(Ptr)) -> item)
#define calllist_next(Ptr) ( ((calllistptr)(Ptr)) -> next)

typedef struct _call2list{
  callnodeptr item;
  call2listptr next;
  call2listptr prev;
}CALL2LIST;

/*=========================================================================*/
/* For backtrackable updates*/

typedef struct InternGarbageRootFrame *IGRptr;
typedef struct InternGarbageLeafFrame *IGLptr;

typedef struct InternGarbageLeafFrame{
  BTNptr leaf;
  IGLptr next;  
} InternGarbageLeaf;

typedef struct InternGarbageRootFrame{
  Integer root;
  IGLptr leaves;
  IGRptr next;
} InternGarbageRoot;

#endif
