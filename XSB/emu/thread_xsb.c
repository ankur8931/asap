/* File:      thread_xsb.c
** Author(s): Marques, Swift
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
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include "xsb_debug.h"
#include "xsb_config.h"

#if !defined(WIN_NT)
#include <unistd.h>
#else
#include <Windows.h>
//#include <stdint.h>
#include "windows_stdint.h"
#endif

#include "basictypes.h"
#include "basicdefs.h"
#include "auxlry.h"

#include "context.h"
#include "cell_xsb.h"
#include "register.h"
#include "cinterf.h"
#include "error_xsb.h"

#ifndef SYSTEM_FLAGS
#include "flags_xsb.h"
#endif

#include "io_defs_xsb.h"
#include "io_builtins_xsb.h"
#include "token_xsb.h"
#include "flag_defs_xsb.h"
#include "deref.h"
#include "ptoc_tag_xsb_i.h"
#include "thread_xsb.h"
#include "rw_lock.h"
#include "memory_xsb.h"
#include "sig_xsb.h"
#include "loader_xsb.h"
#include "memory_xsb.h"
#include "heap_xsb.h"

#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
pthread_mutexattr_t attr_errorcheck_gl;
#endif

void interrupt_with_goal(CTXTdecl);

#ifndef MULTI_THREAD
extern struct asrtBuff_t * asrtBuff;
static  MQ_Cell_Ptr current_mq_cell_seq;
#endif

XSB_MQ_Ptr  mq_table;
static XSB_MQ_Ptr  mq_first_free, mq_last_free, mq_first_queue;

int max_mqueues_glc;
int max_threads_glc;
#define PUBLIC_MQ(mq)		( (mq) >= (mq_table+2*max_threads_glc) )

#ifdef MULTI_THREAD

/* different things are included based on context.h -- easiest to
   include this conditionally. */
#include "tab_structs.h"

#include <errno.h>

int emuloop(CTXTdeclc byte *startaddr);
void cleanup_thread_structures(CTXTdecl);
void init_machine(CTXTdeclc int, int, int, int);
Cell copy_term_from_thread( th_context *th, th_context *from, Cell arg1 );
int copy_term_to_message_queue(th_context *th, Cell arg1);

/* Used to create detached thread -- process global. */
pthread_attr_t detached_attr_gl;
/* Used to create thread with reasonable stack size -- process global. */
pthread_attr_t normal_attr_gl;

pthread_mutexattr_t attr_rec_gl;

static int threads_initialized = FALSE;

th_context * main_thread_gl = NULL;

typedef struct xsb_thread_s
{	
	pthread_t		tid;
	struct xsb_thread_s	*next_entry,	/* either next free slot or next thread */
				*prev_entry ;	/* only valid for slots used for threads */
	unsigned int		incarn : 12;
	unsigned int		valid : 1;
	unsigned int		detached : 1;
	unsigned int		exited : 1;
	unsigned int		status : 3;
        unsigned int            aliased : 1;
	th_context *		ctxt;
} xsb_thread_t;

typedef enum 
  {
    MESG_SEND, MESG_RECV, MESG_PEEK, MESG_DESTROY
  } op_type;

#define VALID_THREAD(tid)	( tid >= 0 &&\
				th_vec[THREAD_ENTRY(tid)].incarn == THREAD_INCARN(tid)\
				&& th_vec[THREAD_ENTRY(tid)].valid )

static xsb_thread_t *th_vec;
static xsb_thread_t *th_first_free, *th_last_free, *th_first_thread;

extern void findall_clean_all(CTXTdecl);
extern void release_private_dynamic_resources(CTXTdecl);
extern void release_private_tabling_resources(CTXTdecl);

//extern void thread_free_dyn_blks(CTXTdecl);
//extern void thread_free_tab_blks(CTXTdecl);
extern void delete_predicate_table(CTXTdeclc TIFptr);

MutexFrame sys_mut[MAX_SYS_MUTEXES];

/* locks initialized at compile time */
pthread_mutex_t th_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pub_mq_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t completing_mut;
/*pthread_cond_t completing_cond;*/

counter max_threads_sofar;

// pthread_t is a pointer in Unix, structure in Windows libraries
#ifdef WIN_NT
#define P_PTHREAD_T_P &tid
#define P_PTHREAD_T *tid
#else
#define P_PTHREAD_T_P tid
#define P_PTHREAD_T tid
#endif

char *mutex_names[] = {
"mutex_dynamic","mutex_io","mutex_table","mutex_trie","mutex_symbol",
"mutex_flags"," mutex_load_undef","mutex_delay","mutex_sys_system",
"mutex_loader", "unused","unused","unused",
"mutex_cons_list", "mutex_compl", 
"mutex_string","mutex_call_trie","mutex_sm","mutex_threads", "mutex_sockets",
"mutex_mem","mutex_odbc","mutex_gentag","mutex_dispbkhdr","unused",
"unused","unused","unused","unused","unused",
"mutex_console","mutex_user1","mutex_user2","mutex_user3","mutex_user4",
"mutex_user5","mutex_user6","mutex_user7","mutex_user9","mutex_user9"};


/*-------------------------------------------------------------------------*/
/* General Routines */

int thread_exited(int tid) {
  return th_vec[tid].exited;
}

th_context *find_context( Integer id )
{
	if( !threads_initialized )
		return main_thread_gl;
	else if ( th_vec[THREAD_ENTRY(id)].incarn == THREAD_INCARN(id) )
		return th_vec[THREAD_ENTRY(id)].ctxt;
	else
		return NULL;
}

int valid_tid( int t )
{
	return VALID_THREAD(t);
}

#ifdef SHARED_COMPL_TABLES
int get_waiting_for_tid( int t )
{
	int wtid;
	th_context *ctxt;

	pthread_mutex_lock( &th_mutex );
        SYS_MUTEX_INCR( MUTEX_THREADS );
	if( !VALID_THREAD(t) )
		ctxt = NULL;
	else
		ctxt = th_vec[THREAD_ENTRY(t)].ctxt;
	if( ctxt )
		wtid = ctxt->waiting_for_tid;
	else
		wtid = -1;
	pthread_mutex_unlock( &th_mutex );

	return wtid;
}
#endif


static void init_thread_table(void)
{
	int i;

	th_vec = mem_calloc(max_threads_glc, sizeof(xsb_thread_t), THREAD_SPACE);

	for( i = 0; i < max_threads_glc; i++ )
	{
		th_vec[i].incarn = INC_MASK_RIGHT;	/* EffeCtively -1 */
		th_vec[i].valid = FALSE;
		th_vec[i].next_entry = &th_vec[i+1];
		th_vec[i].ctxt = NULL;
	}
	th_first_free = &th_vec[0];
	th_last_free = &th_vec[max_threads_glc-1];
	th_last_free->next_entry = NULL;
	th_first_thread = NULL;

	threads_initialized = TRUE;
}

/* 
   Array elements 0..max_threads-1 -- private message queues
                  max_threads..2*max_threads-1 -- private signal queues
		  2*max_threads..3*max_threads-1 -- public message queues

   The use of calloc means that this routine needs only to do basic
   initialization of the free list for public message queues: the
   mutexes, conditional variables, etc are taken care of in
   init_message_queue() called by message_queue_create/[1,2].  Note
   that neither the private message queues not the private signal
   queues need a free list initialization.
*/

#define PUBLIC_MESSAGE_QUEUE(mqid)  (THREAD_ENTRY(mqid) >= 2*max_threads_glc)

static void init_mq_table(void)
{
  int i, status;

  pthread_mutexattr_init( &attr_errorcheck_gl );
  status = pthread_mutexattr_settype( &attr_errorcheck_gl,PTHREAD_MUTEX_ERRORCHECK_NP );
  if ( status )
    xsb_initialization_exit("Error (%s) initializing errorcheck mutex attr -- exiting\n",
			    strerror(status));

  mq_table = mem_calloc(2*max_threads_glc+max_mqueues_glc, 
				sizeof(XSB_MQ), BUFF_SPACE);

  for( i=2*max_threads_glc; i<2*max_threads_glc+max_mqueues_glc; i++ )
    {
      mq_table[i].next_entry = &mq_table[i+1];
      mq_table[i].incarn = INC_MASK_RIGHT ; /* -1 */
    }
  mq_first_free = &mq_table[2*max_threads_glc];
  mq_last_free = &mq_table[2*max_threads_glc+max_mqueues_glc-1];
  mq_last_free->next_entry = NULL;
  mq_first_queue = NULL;

}

#define lock_message_queue(Ptr,String)					\
{ int status;								\
  printf("lock mq_mutex %p %p\n",&Ptr,&Ptr->mq_mutex);			\
  status = pthread_mutex_lock(&Ptr->mq_mutex);				\
  if (status) printf("Error (%s) (thread %"Intfmt") locking queue %"Intfmt" in routine %s\n",strerror(status), \
		      xsb_thread_entry,Ptr - mq_table,String);		\
  if (Ptr->mutex_owner != -1)						\
    printf("Error (thread %"Intfmt") locking queue %"Intfmt" in routine %s (owner is %d)\n", \
	   xsb_thread_entry,Ptr - mq_table,String,Ptr->mutex_owner);			\
  Ptr->mutex_owner = xsb_thread_entry;					\
}

#define unlock_message_queue(Ptr,String)				\
{ int status;								\
  if (Ptr->mutex_owner != xsb_thread_entry)						\
    printf("Error (thread %"Intfmt") unlocking queue %"Intfmt" in routine %s (owner is %d)\n", \
	   xsb_thread_entry,Ptr - mq_table,String,Ptr->mutex_owner);			\
  Ptr->mutex_owner = -1;						\
  status = pthread_mutex_unlock(&Ptr->mq_mutex);				\
  if (status) printf("Error %s (thread %"Intfmt") unlocking queue %"Intfmt" in routine %s\n",strerror(status), \
		      xsb_thread_entry,Ptr - mq_table,String);		\
}

#define checklock_message_queue(Ptr,String)					\
  { if (Ptr->mutex_owner != xsb_thread_entry)	{			\
      printf("Error (thread %"Intfmt") accessing queue %"Intfmt" in routine %s (queue owned by %d)\n",xsb_thread_entry, \
	     Ptr - mq_table,String,Ptr->mutex_owner);					\
      lock_message_queue(Ptr,String);					\
    }									\
}

#define check_message_queue(Ptr,String)					\
  { if (Ptr->mutex_owner != xsb_thread_entry)	{			\
      printf("Error (thread %"Intfmt") accessing queue %"Intfmt" in routine %s (queue owned by %d)\n",xsb_thread_entry, \
	     Ptr - mq_table,String,Ptr->mutex_owner);					\
    }									\
}


void check_deleted( th_context* , int id, XSB_MQ_Ptr , op_type );

/* releasing the space for messages from a message queue (as opposed
   to the queue structure itself)  */
void queue_dealloc( XSB_MQ_Ptr q )
{
  //  printf("warning! queue_dealloc queue %d\n",q-mq_table);

	MQ_Cell_Ptr p, p1;

	p = q->first_message ; 
	while( p != NULL )
	{
		p1 = p->next;
		mem_dealloc( p, p->size, BUFF_SPACE );
		p = p1;
	}
/* can't deallocate the memory of the term queue, as there may be
   references to it from other threads 
	mem_dealloc( q, sizeof(XSB_MQ), THREAD_SPACE );
 */
	if( PUBLIC_MQ(q) )
        {
		/* update the public mq lists */

		pthread_mutex_lock( &pub_mq_mutex );
	        /* delete from queue list */
	        if( q->prev_entry != NULL )
	        	q->prev_entry->next_entry = q->next_entry;
	  	if( q->next_entry != NULL )
	    		q->next_entry->prev_entry = q->prev_entry;
	  	if( mq_first_queue == q )
                	mq_first_queue = q->next_entry;

	  	/* add to free queue list */
	  	if (mq_first_free == NULL ) /* mq_last_free is NULL */
	    		mq_first_free = mq_last_free = q;
	  	else 
		{   /* add new hole at tail to minimise re-use of slots */
	    		mq_last_free->next_entry = q;
	    		mq_last_free = q;
	    		mq_last_free->next_entry = NULL;
	  	}
		pthread_mutex_unlock( &pub_mq_mutex );
        }
}

void destroy_message_queue(CTXTdeclc int mqid) {

  int mq_index = THREAD_ENTRY(mqid);
  //  printf("warning! destroy_message_queue %d\n",mq_index);
  XSB_MQ_Ptr xsb_mq = &(mq_table[mq_index]);

  pthread_mutex_lock( &xsb_mq->mq_mutex );
  if( PUBLIC_MQ( xsb_mq ) )
  	check_deleted(CTXTc mqid, xsb_mq, MESG_DESTROY);
  xsb_mq->deleted = TRUE;
  if( xsb_mq->n_threads == 0 ) {	
    pthread_mutex_unlock( &xsb_mq->mq_mutex );
    queue_dealloc( xsb_mq );
  }
  else  {
    /* if there are threads waiting in the queue, mark it
       as deleted, wake up the thread, and let the last
       one really delete it */
    pthread_mutex_unlock( &xsb_mq->mq_mutex );
    pthread_cond_broadcast( &xsb_mq->mq_has_messages );
    pthread_cond_broadcast( &xsb_mq->mq_has_free_cells );
  }
}

/*-------------------------------------------------------------------------*/
/* Thread Creation, Destruction, etc. */

/* finds thread in the thread-vector, returning its index if found, -1
   otherwise */
static int th_find( pthread_t_p tid )
{
	xsb_thread_t *pos;

	pos = th_first_thread;

	while( pos )
		if( pthread_equal( P_PTHREAD_T, pos->tid ) )
			return pos - th_vec;
		else
			pos = pos->next_entry;

	return -1;
}

/* On normal termination, returns xsb_thread_id for a (usu. newly
   created) thread.  This is called whether creating a thread from
   Prolog or from C.

   Need to ensure this is called with thread mutex locked (except for
   initialization)*/
static int th_new( th_context *ctxt, int is_detached, int is_aliased )
{
	xsb_thread_t *pos;
	int index;

	/* get entry from free list */
	if( !th_first_free )
		return -1;

	pos = th_first_free;
	th_first_free = th_first_free->next_entry;

	/* add new entry to thread list */
	/* keep it ordered in the same way as the table */
	if ( th_first_thread == NULL || th_first_thread > pos )
	{	/* insert at head */
		if( th_first_thread != NULL )
			th_first_thread->prev_entry = pos;
		pos->next_entry = th_first_thread;
		th_first_thread = pos;
		pos->prev_entry = NULL;
	}
	else
	{	xsb_thread_t *p;

		p = th_first_thread;
		/* if p->next_entry == NULL the cycle stops */
		while( pos < p->next_entry )
			p = p->next_entry;

		/* p->next_entry is where we want to insert the entry */
		pos->prev_entry = p;
		pos->next_entry = p->next_entry;
		p->next_entry = pos;
		if( pos->next_entry != NULL )
			pos->next_entry->prev_entry = pos;
	}
	pos->ctxt = ctxt;
	pos->incarn = (pos->incarn+1) & INC_MASK_RIGHT;
	pos->detached = is_detached;
	pos->exited = FALSE;
	pos->valid = FALSE;
	pos->aliased = is_aliased;
	pos->status = THREAD_RUNNING;
	index = pos - th_vec;

	return index;
}

static pthread_t_p th_get( Integer i )
{
	int pos;
	unsigned int incarn;

	if( i < 0 )
		return (pthread_t_p)0;

	pos    = THREAD_ENTRY(i);
	incarn = THREAD_INCARN(i);

	if( th_vec[pos].incarn == incarn && th_vec[pos].valid )
#ifdef WIN_NT
	  return &th_vec[pos].tid;
#else
	  return th_vec[pos].tid;
#endif
	else
	  return (pthread_t_p)0;
}

static void th_delete( int i )
{
	/* delete from thread list */
	if( th_vec[i].prev_entry != NULL )
		th_vec[i].prev_entry->next_entry = th_vec[i].next_entry;
	if( th_vec[i].next_entry != NULL )
		th_vec[i].next_entry->prev_entry = th_vec[i].prev_entry;
	if( th_first_thread == &th_vec[i] )
		th_first_thread = th_vec[i].next_entry;

	/* add to free list */
	if (th_first_free == NULL ) /* th_last_free is NULL */
		th_first_free = th_last_free = &th_vec[i];
	else
	{	/* add new hole at tail to minimise re-use of slots */
		th_last_free->next_entry = &th_vec[i];
		th_last_free = &th_vec[i];
		th_last_free->next_entry = NULL;
	}
	th_vec[i].valid = FALSE;
}

/* calls _$thread_run/1 in thread.P */
static void *xsb_thread_run( void *arg )
{
	pthread_t tid;
	th_context *ctxt = (th_context *)arg;
	Integer pos = THREAD_ENTRY(ctxt->tid);
	
	//	printf("pos %d ctxt %p reg1 %x\n",pos,ctxt,ctxt->_reg[1]);
	
	pthread_mutex_lock( &th_mutex );
	SYS_MUTEX_INCR( MUTEX_THREADS );
	tid = pthread_self();
	/* if the xsb thread id was just created we need to re-initialize 
	thread pthread id on the thread table */
	th_vec[pos].tid = tid;
	th_vec[pos].valid = TRUE;
	pthread_mutex_unlock( &th_mutex );
	
	emuloop( ctxt, get_ep((Psc)flags[THREAD_RUN]) );
	
	/* execution shouldn't arrive here */
	xsb_bug( "emuloop returned from thread" );
	
	return NULL;
}

/*----------------------------------------------------------------------------------*/

/* calls _$thread_run/1 in thread.P */
static void *ccall_xsb_thread_run( void *arg )
{
        pthread_t tid;
	CPtr term_ptr;
	th_context *ctxt = (th_context *)arg;
        int pos = THREAD_ENTRY(ctxt->tid);

	pthread_mutex_lock( &th_mutex );
        SYS_MUTEX_INCR( MUTEX_THREADS );
	tid = pthread_self();
/* if the xsb thread id was just created we need to re-initialize 
   thread pthread id on the thread table */
        th_vec[pos].tid = tid;
        th_vec[pos].valid = TRUE;
        pthread_mutex_unlock( &th_mutex );

	pthread_mutex_lock( &ctxt->_xsb_synch_mut );

	term_ptr = ctxt->_hreg;
	bld_functor((ctxt->_hreg)++,get_ret_psc(0));
	bld_cs(((ctxt->_reg)+1), ((Cell)term_ptr));

	emuloop( ctxt, get_ep(c_callloop_psc));
	//	fprintf(stderr,"exiting emuloop\n");

	//	printf("exiting thread\n");

	return NULL;
}

static void copy_pflags( th_context *to, th_context *from )
{
	int i;

	for( i = 0; i < MAX_PRIVATE_FLAGS; i++ )
		to->_pflags[i] = from->_pflags[i];
}

/*-------------------*/

#define increment_thread_nums \
  flags[NUM_THREADS]++ ; \
  max_threads_sofar = xsb_max( max_threads_sofar, flags[NUM_THREADS] ); 

#define decrement_thread_nums   flags[NUM_THREADS]-- ; 

static Integer xsb_thread_setup(th_context *th, int is_detached, int is_aliased) {
  th_context *new_th_ctxt;
  Integer pos;
  Integer id;

  new_th_ctxt = mem_alloc(sizeof(th_context),THREAD_SPACE);

  pthread_mutex_lock( &th_mutex );
  SYS_MUTEX_INCR( MUTEX_THREADS );
  id = pos = th_new( new_th_ctxt, is_detached, is_aliased );
  if (pos < 0) 
  {     pthread_mutex_unlock( &th_mutex );
        mem_dealloc(new_th_ctxt,sizeof(th_context),THREAD_SPACE);
        xsb_resource_error(CTXTc "threads","thread_create",3);
  }
  /* initialize the thread's private message queue */
  init_message_queue(&mq_table[pos], MQ_CHECK_FLAGS);
  //  printf("thread init_mq_mutex %p %d\n",&mq_table[pos],pos);
  init_message_queue(&mq_table[pos+max_threads_glc], MQ_CHECK_FLAGS);
  //  printf("thread init_mq_mutex %p %d\n",&mq_table[pos+max_threads_glc],pos+max_threads_glc);
  increment_thread_nums;
  pthread_mutex_unlock( &th_mutex );

  SET_THREAD_INCARN(id, th_vec[pos].incarn );
  new_th_ctxt->tid = (Thread_T)id;
  //  printf("id is %d ctxt is %p\n",id,new_th_ctxt);
  ctop_int( th, 3, id );

  return pos;
}

/* cancelling is disabled at first, but re-enabled via thread_run */
static int xsb_thread_create_1(th_context *th, Cell goal, int glsize, int tcsize,
			       int complsize, int pdlsize, int is_detached, int pos){
  int rc;
  pthread_t *thr;
  th_context *new_th_ctxt = th_vec[pos].ctxt;
/* the Thread Id needs to be saved because it somehow gets 
   changed in the following function calls */
  Integer Id = (Integer)new_th_ctxt->tid ; 

  thr = &th_vec[pos].tid;
  copy_pflags(new_th_ctxt, th);
  init_machine(new_th_ctxt,glsize,tcsize,complsize,pdlsize);
  new_th_ctxt->_reg[1] = copy_term_from_thread(new_th_ctxt, th, goal);
  new_th_ctxt->tid = (Thread_T)Id;
  new_th_ctxt->enable_cancel = FALSE;
  new_th_ctxt->to_be_cancelled = FALSE;
  new_th_ctxt->cond_var_ptr = NULL;
  if (is_detached) { /* set detached */
    rc = pthread_create(thr, &detached_attr_gl, &xsb_thread_run, 
			 (void *)new_th_ctxt );
  }
  else {
    rc = pthread_create(thr, &normal_attr_gl, &xsb_thread_run, (void *)new_th_ctxt );
  }
  th_vec[pos].valid = TRUE;

   // printf("creating %p %p\n",thr,th_vec[pos].tid);
  if (rc == EAGAIN) {
    decrement_thread_nums;
    cleanup_thread_structures(new_th_ctxt);
    th_delete(pos);
    mem_dealloc(new_th_ctxt,sizeof(th_context),THREAD_SPACE);
    xsb_resource_error(th,"system threads","xsb_thread_create",2);
  } else {
    if (rc != 0) {
      decrement_thread_nums;
      cleanup_thread_structures(new_th_ctxt);
      th_delete(pos);
      mem_dealloc(new_th_ctxt,sizeof(th_context),THREAD_SPACE);
      xsb_abort("[THREAD] Failure to create thread: error %d\n",rc);
    }
  }

  return rc;
}  /* xsb_thread_create */

static int xsb_thread_create(th_context *th, int glsize, int tcsize, int complsize,int pdlsize,
			     int is_detached, int is_aliased) {
  Cell goal;
  Integer pos;
       
  goal = iso_ptoc_callable(th, 2,"thread_create/[2,3]");

  pos = xsb_thread_setup(th,  is_detached, is_aliased);
  return xsb_thread_create_1(th,goal, glsize, tcsize, complsize,pdlsize,is_detached,pos);
}


/*-------------------*/

int call_conv xsb_ccall_thread_create(th_context *th,th_context **thread_return)
{
  int rc;
  th_context *new_th_ctxt;
  pthread_t *thr;
  Integer id, pos;

  new_th_ctxt = mem_alloc(sizeof(th_context),THREAD_SPACE);

  pthread_mutex_lock( &th_mutex );
  SYS_MUTEX_INCR( MUTEX_THREADS );
  id = pos = th_new( new_th_ctxt, 0, 0 );
  if (pos < 0) 
  {     pthread_mutex_unlock( &th_mutex );
        xsb_resource_error(CTXTc "threads","thread_create",3);
  }
  flags[NUM_THREADS]++;
  max_threads_sofar = xsb_max( max_threads_sofar, flags[NUM_THREADS] );

  pthread_mutex_unlock( &th_mutex );

  new_th_ctxt->_xsb_ready = XSB_IN_Prolog;  
  pthread_mutex_init( &new_th_ctxt->_xsb_synch_mut, &attr_rec_gl );
  pthread_mutex_lock(&(new_th_ctxt->_xsb_synch_mut));

  copy_pflags(new_th_ctxt, th);

  init_machine(new_th_ctxt,0,0,0,0);
  new_th_ctxt->enable_cancel = FALSE;
  new_th_ctxt->to_be_cancelled = FALSE;
  new_th_ctxt->cond_var_ptr = NULL;

  SET_THREAD_INCARN(id, th_vec[pos].incarn );
  new_th_ctxt->tid = (Thread_T)id;

  pthread_cond_init( &new_th_ctxt->_xsb_started_cond, NULL );
  pthread_cond_init( &new_th_ctxt->_xsb_done_cond, NULL );
  pthread_mutex_init( &new_th_ctxt->_xsb_ready_mut, NULL );
  pthread_mutex_init( &new_th_ctxt->_xsb_query_mut, NULL );
  new_th_ctxt->_xsb_inquery = 0;

  *thread_return = new_th_ctxt;

  thr = &th_vec[pos].tid;
  rc = pthread_create(thr, &normal_attr_gl, &ccall_xsb_thread_run, (void *)new_th_ctxt );
  th_vec[pos].valid = TRUE;

  if (rc == EAGAIN) {
    xsb_resource_error(th,"system threads","xsb_thread_create",2);
  } else {
    if (rc != 0) 
      xsb_abort("[THREAD] Failure to create thread: error %d\n",rc);
  }

  while (XSB_IN_Prolog == new_th_ctxt->_xsb_ready)
	pthread_cond_wait( &new_th_ctxt->_xsb_done_cond, 
			   &new_th_ctxt->_xsb_synch_mut  );
  pthread_mutex_unlock( &new_th_ctxt->_xsb_synch_mut );
  return rc;
}  /* xsb_thread_create */

/*-------------------------------------------------------------------------*/
/* System Initialization Stuff */

void init_system_threads( th_context *ctxt )
{
  pthread_t tid = pthread_self();
  Integer id, pos;

  init_mq_table();
  /* this should build an invalid thread id */
  init_thread_table();
  id = pos = th_new(ctxt, 0, 0);
  th_vec[pos].tid = tid;
  th_vec[pos].valid = TRUE;
  if( pos != 0 )
    SET_THREAD_INCARN(id, th_vec[pos].incarn );

  ctxt->tid = (Thread_T)id;

  if( id != 0 )
	xsb_abort( "[THREAD] Error initializing thread table" );


  max_threads_sofar = 1;
}

/* * * * * * */
void init_system_mutexes( void )
{
	int i;
	pthread_mutexattr_t attr_std;

/* make system mutex recursive, for there are recursive prolog calls	*/
/* to stuff that must be executed in mutual exclusion			*/

	pthread_mutexattr_init( &attr_rec_gl );
	if( pthread_mutexattr_settype( &attr_rec_gl,
				       PTHREAD_MUTEX_RECURSIVE_NP )<0 )
		xsb_abort( "[THREAD] Error initializing mutexes" );

	pthread_mutexattr_init( &attr_std );

	for( i = 0; i <=  LAST_REC_MUTEX ; i++ ) {
	  pthread_mutex_init( MUTARRAY_MUTEX(i), &attr_rec_gl );
	  MUTARRAY_OWNER(i) = -1;
	}
	for( i = LAST_REC_MUTEX + 1 ; i < MAX_SYS_MUTEXES ; i++ ) {
	  pthread_mutex_init( MUTARRAY_MUTEX(i), &attr_std );
	  MUTARRAY_OWNER(i) = -1;
	}

#ifdef MULTI_THREAD_RWL
	rw_lock_init(&trie_rw_lock);
#endif

	pthread_mutex_init( &completing_mut, &attr_std );
/*	pthread_cond_init( &completing_cond, NULL );*/
}

/*-------------------------------------------------------------------------*/
/* Routines for dynamic user mutexes. It may at first seem strange to
   have MUTEX_DYNMUT guard the creation and deletion of mutexes, but
   since the mutexes are held in a doubly linked list that is
   traversed by various routines, we have to be careful.  However
   mutex handling that affects a single user mutex (locking,
   unlocking) does not need to synchronize with MUTEX_DYNMUT.  */

/* Used only for sys mutexes -- I may fold this into mutex statistics
   at some point.  */
void print_mutex_use() {
  int i;

  printf("Mutexes used since last statistics:\n");
  for (i = 0; i < MAX_SYS_MUTEXES; i++) {
    if (sys_mut[i].num_locks > 0) 
      printf("Mutex %s (%d): %d\n",mutex_names[i],i,sys_mut[i].num_locks);
  }
  for (i = 0; i < MAX_SYS_MUTEXES; i++) {
    sys_mut[i].num_locks = 0;
  }
}

DynMutPtr dynmut_chain_begin = NULL;

/* Add new dynmutframe to beginning of chain */

DynMutPtr create_new_dynMutFrame() {
  DynMutPtr new_dynmut = mem_alloc(sizeof(DynMutexFrame),THREAD_SPACE);
  pthread_mutex_init( &(new_dynmut->th_mutex), &attr_rec_gl );
  new_dynmut->num_locks = 0;
  new_dynmut->tot_locks = 0;
  new_dynmut->owner = -1;
  new_dynmut->next_dynmut = dynmut_chain_begin;
  if (dynmut_chain_begin != NULL) 
    (*dynmut_chain_begin).prev_dynmut = new_dynmut;
  new_dynmut->prev_dynmut = NULL;
  dynmut_chain_begin = new_dynmut;
  return new_dynmut;
}

void delete_dynMutFrame(DynMutPtr old_dynmut) {
  if (old_dynmut->prev_dynmut != NULL)
    (old_dynmut->prev_dynmut)->next_dynmut = old_dynmut->next_dynmut;
  if (old_dynmut->next_dynmut != NULL)
    (old_dynmut->next_dynmut)->prev_dynmut = old_dynmut->prev_dynmut;
  if (dynmut_chain_begin == old_dynmut)
    dynmut_chain_begin = old_dynmut->next_dynmut;
  mem_dealloc(old_dynmut,sizeof(DynMutexFrame),THREAD_SPACE);
}
  
/* Need to make sure that owner is not over-written falsely.  This one
   is for user mutexes.*/
void unlock_mutex(CTXTdeclc DynMutPtr id) {

  Integer rc;

  if ( id->owner == xsb_thread_id) 
    id->owner = -1;
  
  rc = pthread_mutex_unlock( &(id->th_mutex) );
  if (rc == EINVAL) {
    xsb_permission_error(CTXTc "unlock mutex","invalid mutex",
			 xsb_thread_id,"xsb_mutex_unlock",2); 
  } else if (rc == EPERM) { 
    xsb_permission_error(CTXTc "unlock mutex",
			 "mutex not held by thread",
			 xsb_thread_id,"xsb_mutex_unlock",2); 
  } 
}

/* This just unlocks the user mutexes held by a thread -- the name
   comes from the ISO document. */
void mutex_unlock_all(CTXTdecl) {
  DynMutPtr dmp = dynmut_chain_begin;
  while (dmp != NULL) {
    if (dmp-> owner == xsb_thread_id) {
      //      printf("unlocking all %p\n",dmp);
      while (dmp->num_locks > 0) {
	unlock_mutex(CTXTc dmp);
	dmp->num_locks--;
      }
    }
    dmp = dmp->next_dynmut;
  }
}

/* Unlocks all system and user mutexes held by a thread */
void release_held_mutexes(CTXTdecl) {
  int i;

  //  printf("releasing held mutexes\n");
  for( i = 0; i <=  LAST_REC_MUTEX ; i++ ) {
    if ( MUTARRAY_OWNER(i) == xsb_thread_id) {
      pthread_mutex_unlock( MUTARRAY_MUTEX(i));
    }
  }
  for( i = LAST_REC_MUTEX + 1 ; i < MAX_SYS_MUTEXES ; i++ ) {
    if ( MUTARRAY_OWNER(i) == xsb_thread_id) {
      pthread_mutex_unlock( MUTARRAY_MUTEX(i));
    }
    pthread_mutex_unlock( MUTARRAY_MUTEX(i));
  }
  for( i = 0; i < MAX_OPEN_FILES; i++ )
	if( OPENFILES_MUTEX_OWNER(i) == xsb_thread_id )
		pthread_mutex_unlock(OPENFILES_MUTEX(i));

  mutex_unlock_all(CTXT);
}

void close_str(CTXTdecl)
{
  int i;
  for( i = 0; i < MAXIOSTRS; i++ )
	if( iostrs[i] && iostrs[i]->owner == xsb_thread_id )
		strclose( iostrdecode(i) );
}

//void tripwire_interrupt(char * tripwire_call) {
//}

#else /* Not MULTI_THREAD */

void print_mutex_use() {
  xsb_abort("[THREAD] This engine is not configured for mutex profiling.");
}

#endif /* MULTI_THREAD */

int xsb_thread_self()
{
#ifdef MULTI_THREAD
	int pos, id;
        pthread_t tid = pthread_self();

	if( !threads_initialized )
		return 0;

        pthread_mutex_lock( &th_mutex );
        SYS_MUTEX_INCR( MUTEX_THREADS );
        id = pos = th_find( P_PTHREAD_T_P );
        pthread_mutex_unlock( &th_mutex );

	if( pos >= 0 )
		SET_THREAD_INCARN( id, th_vec[pos].incarn );
#ifdef DEBUG
	else 
		raise( SIGSEGV );
#endif

	return id;
#else
	return 0;
#endif
}

#ifdef MULTI_THREAD

void check_deleted( th_context *th, int id, XSB_MQ_Ptr q, op_type op )
{
	  char *pred;
	  int arity, arg;
	  Integer iq;
	  
	  if( q->id != id || q->deleted )
	  {
	    //	    printf("warning! check_deleted thread %d queue %d\n",xsb_thread_entry,q-mq_table);
		pthread_mutex_unlock( &q->mq_mutex );
	  	if( op == MESG_SEND )
		{	arg = arity = 2;
			pred = "thread_send_message";
		}
		else if( op == MESG_RECV )
		{	arg = arity = 2;
			pred = "thread_get_message";
		}
		else if( op == MESG_PEEK )
		{	arg = arity = 2;
			pred = "thread_peek_message";
		}
		else
		{	arg = arity = 1;
			pred = "message_queue_destroy";
		}
		iq = makeint(id);
		xsb_existence_error(th, "message queue", iq, pred, arity, arg);
	  }
}

/* waiting on message queues */
/* returns TRUE if thread was signaled */
int wait_on_queue( th_context *th, XSB_MQ_Ptr q, op_type send )
{
  int status;
  pthread_cond_t * cond_var;
  char *pred;
  Integer iq;

	if( send == MESG_SEND )
	{
		cond_var = &q->mq_has_free_cells;
		pred = "thread_send_message";
		
	}
	else
	{
		cond_var = &q->mq_has_messages;
		pred = "thread_get_message";
	}

	/* so that the signal handler can wake up the thread */
	th->cond_var_ptr = cond_var;
	q->n_threads++;
	if( !q->deleted ) {
	  q->mutex_owner = -1;
	  //	  printf("thread %d waiting on condition %s for queue %d\n",xsb_thread_entry,pred,q-mq_table);
	  status = pthread_cond_wait(cond_var,&q->mq_mutex);
	  if (status) printf("pthread_cond_wait error in wait_on_queue: %d\n",status);
	  q->mutex_owner = xsb_thread_entry;
	}
	q->n_threads--;
        th->cond_var_ptr = NULL;

	/* check for thread interrupt */
       	if( asynint_val & THREADINT_MARK )
        {
	  //	  	  printf("warning! Thread signal\n");
	  pthread_mutex_unlock( &q->mq_mutex );
	  return TRUE;
	}
	if( q->deleted )
	{
	  //	  printf("queue %d deleted\n",q-mq_table);
		if( q->n_threads == 0 )
		{	pthread_mutex_unlock( &q->mq_mutex );
			queue_dealloc( q );
		}
		else
			pthread_mutex_unlock( &q->mq_mutex );
  		iq = makeint((Integer)(q)); 
		xsb_existence_error( th, "message queue", iq, pred, 2, 2 );
	}
	return FALSE;
}

#endif

#if defined(WIN_NT)

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const UInteger EPOCH = ((UInteger) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    UInteger    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((UInteger)file_time.dwLowDateTime )      ;
    // this only works for 64-bit
    time += ((UInteger)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

/*-------------------------------------------------------------------------*/
/* Thread Requests  */

extern void release_private_tabling_resources(CTXTdecl);
extern void abolish_private_tables(CTXTdecl);
extern void abolish_shared_tables(CTXTdecl);
extern void abolish_all_private_tables(CTXTdecl);
extern void abolish_all_shared_tables(CTXTdecl);

xsbBool xsb_thread_request( CTXTdecl ) 
{
	Integer request_num = ptoc_int(CTXTc 1);
#ifdef MULTI_THREAD
	Integer id, rval;
	pthread_t_p tid;
	int i;
	Integer rc;
	xsbBool success = TRUE;
	switch( request_num )
	{
	    /* Flags use default values, params have explicit
	       parameters sent in */
	case XSB_THREAD_CREATE_FLAGS:
	  iso_check_var(th, 3,"thread_create/[2,3]"); // should check here, rather than at end
	  rc = xsb_thread_create(th,flags[THREAD_GLSIZE],flags[THREAD_TCPSIZE],flags[THREAD_COMPLSIZE],
				   flags[THREAD_PDLSIZE],flags[THREAD_DETACHED],0);
	  break;

	case XSB_THREAD_EXIT: {
	  int retract_aliases = 0;

	  //	  printf("thread exiting %d\n",xsb_thread_entry);
	  rval = iso_ptoc_int(CTXTc 2, "thread_exit/1" );
	  release_held_mutexes(CTXT);
	  release_private_tabling_resources(CTXT);
	  abolish_private_wfs_space(CTXT);
	  release_private_dynamic_resources(CTXT);
	  findall_clean_all(CTXT);
	  close_str(CTXT);
	  cleanup_thread_structures(CTXT);
	  pthread_mutex_lock( &th_mutex );
     SYS_MUTEX_INCR( MUTEX_THREADS );
     tid = th->tid;
     i = THREAD_ENTRY(tid);
	  th_vec[i].ctxt = NULL;
	  destroy_message_queue(CTXTc THREAD_ENTRY(i));			/* destory private mq */
	  destroy_message_queue(CTXTc THREAD_ENTRY(i)+max_threads_glc); /* destroy signal mq */
	  if( i >= 0 ) {
	    if ( th_vec[i].detached ) {
	      if (th_vec[i].aliased) 
		retract_aliases = 1;
	      th_delete(i);
	    }
	    else {
	      th_vec[i].exited = TRUE;
	      th_vec[i].status = rval;
	    }
	  }
	if (C_CALLING_XSB != xsb_mode)	// We are going to need this, the C side can dispose of it
		  mem_dealloc(th,sizeof(th_context),THREAD_SPACE);
	  pthread_mutex_unlock( &th_mutex );
	  if( i == -1 )
		xsb_abort("[THREAD] Couldn't find thread in thread table!");
	  flags[NUM_THREADS]--;
	  
	if (C_CALLING_XSB == xsb_mode)
		{					// Let the C side finish up
      xsb_ready = XSB_IN_C;
      xsb_unlock_mutex(&xsb_synch_mut, "XSB_THREAD_EXIT", __FILE__, __LINE__);
      (void)xsb_cond_signal(&xsb_done_cond, "XSB_THREAD_EXIT", __FILE__, __LINE__);
		}
	  //	  printf("thread %ld is exiting\n",pthread_self());
	  pthread_exit((void *) rval );
		ctop_int(CTXTc 3,retract_aliases);
	  rc = 0 ; /* keep compiler happy */
	  break;
	}

	  /* TLS: replaced thread_free_tab_blks() by
	     thread_free_private_tabling_resources, which sets
	     appropriate tifs to 0, but doesn't use
	     delete_predicate_table -- rather it deallocates the
	     structure managers directly.  */

	case XSB_THREAD_JOIN: {
	  id = iso_ptoc_int( CTXTc 2 ,"thread_join/[1,2]");
          iso_check_var(th, 3,"thread_join/[1,1]"); 
	  pthread_mutex_lock( &th_mutex );
          SYS_MUTEX_INCR( MUTEX_THREADS );
	  tid = th_get( id );
	  pthread_mutex_unlock( &th_mutex );
	  if( tid == (pthread_t_p)0 )
	    xsb_existence_error(CTXTc "thread",reg[2],"xsb_thread_join",1,1); 
	  rc = pthread_join(P_PTHREAD_T, (void **)&rval );
	  if (rc != 0) {
	    if (rc == EINVAL) { /* pthread found, but not joinable */
	      xsb_permission_error(CTXTc "thread_join","non-joinable thread",
				   reg[2],"thread_join",1); 
	    } else {
	      if (rc == ESRCH)  { /* no such pthread found */
		xsb_existence_error(CTXTc "thread",reg[2],
				    "thread_join",1,1); 
	      }
	    }
	  }

	  ctop_int( CTXTc 4, th_vec[THREAD_ENTRY(id)].aliased);
	  pthread_mutex_lock( &th_mutex );
          SYS_MUTEX_INCR( MUTEX_THREADS );
	  th_delete(THREAD_ENTRY(id));
	  pthread_mutex_unlock( &th_mutex );
	  ctop_int( CTXTc 3, rval );
	  break;
	}

	case XSB_THREAD_DETACH: {
	  int retract_aliases = 0;
	  int retract_exitball = 0;
	  
	  id = iso_ptoc_int( CTXTc 2 ,"thread_detach/1");

	  pthread_mutex_lock( &th_mutex );
          SYS_MUTEX_INCR( MUTEX_THREADS );
	  tid = th_get( id );
	  pthread_mutex_unlock( &th_mutex );

	  if( tid == (pthread_t_p)0 )
	    xsb_abort("[THREAD] Thread detach - invalid thread id" );
	  rc = PTHREAD_DETACH( tid );
	  if (rc == EINVAL) { /* pthread found, but not joinable */
	    xsb_permission_error(CTXTc "thread_detach","thread",reg[2],"thread_detach",1); 
	  } else {
	    if (rc == ESRCH)  { /* no such pthread found */
	      xsb_existence_error(CTXTc "thread",reg[2], "thread_detach",1,1); 
	    }
	  }

	  id = THREAD_ENTRY(id);
	  pthread_mutex_lock( &th_mutex );
          SYS_MUTEX_INCR( MUTEX_THREADS );
	  if ( th_vec[id].exited ) {
	    if (th_vec[THREAD_ENTRY(id)].detached == FALSE 
		&& th_vec[THREAD_ENTRY(id)].aliased == TRUE ) retract_aliases = 1;
	    if (th_vec[THREAD_ENTRY(id)].status > THREAD_FAILED) retract_exitball = 1;
	    th_delete(id);
	  }
	  else 
	    th_vec[THREAD_ENTRY(id)].detached = TRUE;
	  pthread_mutex_unlock( &th_mutex );

	  ctop_int(CTXTc 3,retract_aliases);
	  ctop_int(CTXTc 4,retract_exitball);

	  break;
	}

       case XSB_THREAD_SELF:
	 rc = id = (Integer)th->tid;
	 ctop_int( CTXTc 2, id );
	 break;

 	case XSB_MUTEX_INIT:		
	  ctop_int(CTXTc 2, (prolog_int) create_new_dynMutFrame());
	  break;

	case XSB_MUTEX_LOCK: {
	  DynMutPtr id = (DynMutPtr) ptoc_int(CTXTc 2);
	  rc = pthread_mutex_lock( &(id->th_mutex) );
	  id->num_locks++;
	  id->tot_locks++;
	  id->owner = xsb_thread_id;
	  if (rc == EINVAL) {
	    xsb_existence_error(CTXTc "invalid mutex",
				reg[2],"xsb_mutex_lock",2,2); 
	  } else if (rc == EDEADLK) { 
	    xsb_permission_error(CTXTc "lock mutex","deadlocking mutex",
				 reg[2],"xsb_mutex_lock",2); 
	  } 
	  break;
	}

	case XSB_MUTEX_TRYLOCK: {
	  DynMutPtr id = (DynMutPtr) ptoc_int(CTXTc 2);
	  rc = pthread_mutex_trylock( &(id->th_mutex) );
	  if (rc == EINVAL) {
	    xsb_permission_error(CTXTc "lock mutex","invalid mutex",
				 reg[2],"xsb_mutex_lock",2); 
	  } else success = ( rc != EBUSY );
	  break;
	}

	case XSB_MUTEX_UNLOCK: {
	  DynMutPtr id = (DynMutPtr) ptoc_int(CTXTc 2);
	  id->num_locks--;
	  unlock_mutex(CTXTc id);
	  break;
	}

	case XSB_MUTEX_DESTROY: {
	  DynMutPtr id = (DynMutPtr) ptoc_int(CTXTc 2);
	  rc = pthread_mutex_destroy( &(id->th_mutex) );
	  if (rc == EINVAL) {
	    xsb_permission_error(CTXTc "destroy mutex","invalid mutex",
				 reg[2],"xsb_mutex_destroy",1); 
	  } else {
	    if (rc == EBUSY) { 
	      xsb_permission_error(CTXTc "destroy mutex","busy mutex",
				   reg[2],"xsb_mutex_destroy",1); 
	    } else 
	      delete_dynMutFrame(id);
	  }
	  break;
	}
	case XSB_SYS_MUTEX_LOCK:
	  id = ptoc_int(CTXTc 2);
#ifdef DEBUG_MUTEXES
	  fprintf( stddbg, "S LOCK(%" Intfmt ")\n", (Integer)id );
#endif
	  rc = pthread_mutex_lock( MUTARRAY_MUTEX(id) );
#ifdef DEBUG_MUTEXES
	  fprintf( stddbg, "RC=%" Intfmt "\n", (Integer)rc );
#endif
	  break;
	case XSB_SYS_MUTEX_UNLOCK:
	  id = ptoc_int(CTXTc 2);
	  rc = pthread_mutex_unlock( MUTARRAY_MUTEX(id) );
	  break;

	case XSB_ENSURE_ONE_THREAD:
	  ENSURE_ONE_THREAD();
	  rc = 0;
	  break;

	  /*TLS: I should make the configuration check for existence
	    of sched_yield somehow. */
	case XSB_THREAD_YIELD:
#if !defined(SOLARIS)
	  rc = sched_yield();
	  if (rc == ENOSYS) /* Need support for POSIX 1b for this */
#endif
	    xsb_abort("[THREAD] Real-time extensions not supported on this platform");
	  break;

	case XSB_SHOW_MUTEXES: 
	  printf("mutex owners\n");
	  for (i = 0; i < MAX_SYS_MUTEXES; i++) {
	    if (sys_mut[i].owner > 0) 
	      printf("Mutex %s (%d): %d\n",mutex_names[i],i,(int)sys_mut[i].owner);
	  }
	  rc = 0;
	  break;

	  /* TLS: XSB_FIRST_THREAD_PROPERTY, XSB_NEXT_THREAD_PROPERTY
	     are used when backtracking through threads. */
	case XSB_THREAD_PROPERTY: 
	  i = iso_ptoc_int(CTXTc 2,"thread_property/2");
	  if( !VALID_THREAD(i) )
	    xsb_existence_error(th, "thread", reg[2], "thread_property", 1,1);
	  ctop_int(CTXTc 3, th_vec[ THREAD_ENTRY(i) ].detached);
	  ctop_int(CTXTc 4, th_vec[ THREAD_ENTRY(i) ].status);
	  break;

	  /* If all goes well, the cancelled thread will execute
	     '_$thread_int/2, by checking THREADINT_MARK */

	case XSB_THREAD_INTERRUPT: {
	  th_context *	ctxt_ptr;
	  i = ptoc_int(CTXTc 2);
	  if( VALID_THREAD(i) ) {
	    pthread_mutex_lock( &th_mutex );
            SYS_MUTEX_INCR( MUTEX_THREADS );
	    ctxt_ptr = th_vec[THREAD_ENTRY(i)].ctxt;
	    if( ctxt_ptr )
	    {   if( ctxt_ptr->enable_cancel )
	    	{	ctxt_ptr->_asynint_val |= THREADINT_MARK;
		  //	    		pthread_kill( th_vec[THREAD_ENTRY(i)].tid, SIGABRT );
		  //#ifdef WIN_NT
			{	pthread_cond_t *pcond = ctxt_ptr->cond_var_ptr;
			  if( pcond != NULL ) {
			    //			    printf("broadcasting on %p\n",pcond);
			    pthread_cond_broadcast( pcond );
			  }
			}
			//#endif
	    	}
		else
			ctxt_ptr->to_be_cancelled = TRUE;
            }
	    pthread_mutex_unlock( &th_mutex );
	  } else {
	    bld_int(reg+2,i);
	    xsb_permission_error(CTXTc "thread_interrupt","invalid_thread",
				   reg[2],"xsb_thread_interrupt",1); 
	  }
	  break;
	}

	case ABOLISH_PRIVATE_TABLES: {
	  abolish_private_tables(CTXT);
	  break;
	}

	case ABOLISH_SHARED_TABLES: {
	  abolish_shared_tables(CTXT);
	  break;
	}

	case GET_FIRST_MUTEX_PROPERTY: {
	  if (dynmut_chain_begin != NULL) {
	    ctop_int(CTXTc 2 , (prolog_int) dynmut_chain_begin);
	    ctop_int(CTXTc 3 , (*dynmut_chain_begin).num_locks);
	    ctop_int(CTXTc 4 , (*dynmut_chain_begin).owner);
	    ctop_int(CTXTc 5 , (prolog_int) (*dynmut_chain_begin).next_dynmut);
	  }
	  else {
	    ctop_int(CTXTc 2 , 0);
	    ctop_int(CTXTc 3 , 0);
	    ctop_int(CTXTc 4 , 0);
	    ctop_int(CTXTc 5 , 0);
	  }
	  break;
	}

	case GET_NEXT_MUTEX_PROPERTY: {
	  DynMutPtr dmp = (DynMutPtr) ptoc_int(CTXTc 2);
	  ctop_int(CTXTc 3 , (*dmp).num_locks);
	  ctop_int(CTXTc 4 , (*dmp).owner);
	  ctop_int(CTXTc 5 , (prolog_int) (*dmp).next_dynmut);
	  break;
	}

	case MUTEX_UNLOCK_ALL: 
	  mutex_unlock_all(CTXT);
	  break;

	case ABOLISH_ALL_PRIVATE_TABLES: {
	  abolish_all_private_tables(CTXT);
	  break;
	}

	case ABOLISH_ALL_SHARED_TABLES: {
	  abolish_all_shared_tables(CTXT);
	  break;
	}

	case SET_XSB_READY: {
	  unlock_xsb_ready("set_xsb_ready");
	  break;
	}

	case MESSAGE_QUEUE_CREATE: {
	XSB_MQ_Ptr xsb_mq;

	pthread_mutex_lock( &pub_mq_mutex );

	/* get entry from free list */
	if( !mq_first_free ) 	
        {
	  pthread_mutex_unlock( &pub_mq_mutex );
	  xsb_resource_error(CTXTc "message queues","message_queue_create",2);
	}
	xsb_mq = mq_first_free;
	mq_first_free = mq_first_free->next_entry;

	/* I'm not keeping the public queues ordered, just inserting at head */
	if (mq_first_queue != NULL)
	  mq_first_queue->prev_entry = xsb_mq;
	xsb_mq->next_entry = mq_first_queue;
	mq_first_queue = xsb_mq;
	xsb_mq->prev_entry = NULL;

	pthread_mutex_unlock( &pub_mq_mutex );

	init_message_queue(xsb_mq,ptoc_int(CTXTc 3));

	ctop_int(CTXTc 2,(int) (xsb_mq->id));

	break;
	}

	  /* Adding convention that max size of 0 is an unbounded queue */
	case THREAD_SEND_MESSAGE: {
	  int id = ptoc_int(CTXTc 2);
	  XSB_MQ_Ptr message_queue;
          int index = THREAD_ENTRY(id);
	  MQ_Cell_Ptr this_cell;
	  
	  message_queue = &mq_table[index];

	  lock_message_queue(message_queue,"thread_send_message");
	  
	  check_deleted(th, id, message_queue, MESG_SEND);
	  while (message_queue->max_size != MQ_UNBOUNDED 
		  && message_queue->size >= message_queue->max_size) {
	    if( wait_on_queue( th, message_queue, MESG_SEND ) )
	      return success ;	    // return if thread was signalled
	   }

	  check_message_queue(message_queue,"tsm1");

	  this_cell = mem_calloc(asrtBuff->Size+sizeof(MQ_Cell),1,THREAD_SPACE);
	  this_cell->prev = message_queue->last_message;
	  //	  printf("start thread %d sending queue %d owner %d (last %p prev %p)\n",
	  // xsb_thread_entry,index,message_queue->mutex_owner,
	  // message_queue->last_message,this_cell->prev);
	  this_cell->next = 0;
	  this_cell->size = asrtBuff->Size+sizeof(MQ_Cell);
	  /* Moves assert buffer to word just after MQ_Cell */
	  memmove(this_cell+1,asrtBuff->Buff,asrtBuff->Size); 

	  if (message_queue->last_message) 
	    (message_queue->last_message)->next = this_cell;
	  message_queue->last_message = this_cell;
	  message_queue->size++;
	  
	  //	  printf("thread %d sending queue %d owner %d (new cell %p prev %p last %p)\n",
	  // xsb_thread_entry,index,message_queue->mutex_owner,
	  // this_cell,this_cell->prev,message_queue->last_message);

	  if (!message_queue->first_message) {
	    message_queue->first_message = this_cell;
	  }

	  unlock_message_queue(message_queue,"thread_send_message");

	  /* Need to broadcast whenever you add a new message `as
	     threads may be waiting at the end of a non-full queue*/
	  pthread_cond_broadcast(&message_queue->mq_has_messages);
	  break;
	}

case THREAD_TRY_MESSAGE: {	 
  int id = ptoc_int(CTXTc 2);
  XSB_MQ_Ptr message_queue;
  int index = THREAD_ENTRY(id);
	  
	  message_queue = &mq_table[index];

	  //	  printf("thread %d trying queue %d first %p\n",xsb_thread_entry,index,message_queue->first_message);

	  lock_message_queue(message_queue,"thread_try_message")

	    //	  printf("thread %d locked queue %d first %p\n",xsb_thread_entry,index,message_queue->first_message);
          /* if all goes well this lock will only be released
             in THREAD_ACCEPT_MESSAGE */

	  check_deleted(th, id, message_queue, MESG_RECV);
	  while (!message_queue->first_message) {
	    if(  wait_on_queue( th, message_queue, MESG_RECV ) )
		return success ;	    
	  }
	  current_mq_cell = message_queue->first_message;
	  check_glstack_overflow(3,pcreg, (512+2*current_mq_cell->size*sizeof(Cell)));
	  pcreg =  (byte *)(current_mq_cell+1);
	  break;
	}

  /* THREAD_RETRY_MESSAGE will have the lock as set up by THREAD_TRY_MESSAGE (or by
     succeeding out of pthread_cond_wait() The lock will be unlocked
     either by THREAD_ACCEPT_MESSAGE or by suspending in
     pthread_cond_wait() */
case THREAD_RETRY_MESSAGE: {	 
	  int id = ptoc_int(CTXTc 2);
	  XSB_MQ_Ptr message_queue;
          int index = THREAD_ENTRY(id);
	  
	  message_queue = &mq_table[index];

  // This shouldn't be needed, and I want to take it out once mqs become more stable.
	  checklock_message_queue(message_queue,"thread_retry_message");

	  //  	  printf("  thread %d REtrying queue %d last %p (cur_mq_cell %p next %p)\n",xsb_thread_entry,index,
	  //		 message_queue->last_message,current_mq_cell,current_mq_cell->next);

	  /* If current_mq_cell is last message, the thread has made a
	     traversal through the queue without finding a term that
	     unifies.  It gives up the lock and goes to sleep.  Before
	     it wakes again, another thread may have changed the queue
	     substantially -- so its old state (current_mq_cell) is
	     invalid.  This cell may even have been reclaimed.  Thus,
	     there is no way of relating our old position to the new
	     queue.  All you can do is start again from the beginning
	     (checking, of course, that there is a beginning) */

	  check_deleted(th, id, message_queue, MESG_RECV);
	  if (current_mq_cell == message_queue->last_message) {
	    if(  wait_on_queue( th, message_queue, MESG_RECV ) )
		return success ;	    
	    while (!message_queue->first_message) {
	      //	      printf("no first message\n");
	      if(  wait_on_queue( th, message_queue, MESG_RECV ) )
		  return success ;	    
	    }
	    current_mq_cell = message_queue->first_message;
	  }
	  else current_mq_cell = current_mq_cell->next;

	  check_glstack_overflow(3,pcreg, (512+2*current_mq_cell->size*sizeof(Cell)));
	  pcreg = (byte *) (current_mq_cell+1); // offset for compiled code.
	  break;
	}

  /* Broadcasts whenever it goes from "full" to "not_full" so that
     writers will be awakened. */	
case THREAD_ACCEPT_MESSAGE: {	 
  int id = ptoc_int(CTXTc 2);
  XSB_MQ_Ptr message_queue;
  int index = THREAD_ENTRY(id);
  
  message_queue = &mq_table[index];

  // This shouldn't be needed, and I want to take it out once mqs become more stable.
  checklock_message_queue(message_queue,"thread_retry_message");

  //  printf("    thread %d %p accepting queue %d (curr cell %p prev %p) last %p\n",
  //	 xsb_thread_entry,th,index,current_mq_cell,current_mq_cell->prev,message_queue->last_message);

  /* Take MQ_Cell out of chain, and clear its contents */
  if (message_queue->first_message == current_mq_cell) 
      message_queue->first_message = current_mq_cell->next;

  if (message_queue->last_message == current_mq_cell) {
      message_queue->last_message = current_mq_cell->prev;
      //      printf("      q %d last set to %p\n",index,current_mq_cell->prev);
  }

  if (current_mq_cell->prev) 
    (current_mq_cell->prev)->next = (current_mq_cell->next);

  if (current_mq_cell->next) 
    (current_mq_cell->next)->prev = (current_mq_cell->prev);

  mem_dealloc(current_mq_cell,current_mq_cell->size,BUFF_SPACE);

  message_queue->size--;
  if (message_queue->size+1 == message_queue->max_size) {
    unlock_message_queue(message_queue,"thread_accept_message")
    pthread_cond_broadcast(&message_queue->mq_has_free_cells);
    }
  else   {
    unlock_message_queue(message_queue,"thread_accept_message")
  }
  break;
 }

 case XSB_MESSAGE_QUEUE_DESTROY:
   {
     int mq_id;
     mq_id = ptoc_int(CTXTc 2);

     if (!PUBLIC_MESSAGE_QUEUE(mq_id)) 
       xsb_permission_error(CTXTc "destroy","private_signal_or_message_queue",
			    reg[2],"message_queue_destroy",1); 

     //     printf("destroying message queue %d\n",mq_id);
     destroy_message_queue( CTXTc mq_id );
     
     break;}
     

	case XSB_THREAD_CREATE_PARAMS:
	  iso_check_var(th, 3,"thread_create/[2,3]"); // should check here, rather than at end
	  id = xsb_thread_create(th,ptoc_int(CTXTc 4),ptoc_int(CTXTc 5),
				 ptoc_int(CTXTc 6),ptoc_int(CTXTc 7), ptoc_int(CTXTc 8), 0);
	  break;

	case XSB_USLEEP: {

	  struct timespec   ts;
	  struct timeval    tp;
	  int naptime_s, naptime_n, rc;
	  pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
	  pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;
	  pthread_mutex_lock(&mutex);

	  //	  checkResults("gettimeofday()\n", rc);
	  naptime_n = iso_ptoc_int_arg(CTXTc 2,"thread_sleep/1",1);

	  naptime_s = naptime_n/1000;
	  naptime_n = naptime_n%1000;
	  /* Convert from timeval to timespec */
	  rc =  gettimeofday(&tp, NULL);

	  ts.tv_sec = tp.tv_sec;
	  ts.tv_sec += naptime_s;
	  ts.tv_nsec = tp.tv_usec * 1000;
	  ts.tv_nsec += naptime_n*1000000;

	  // to broadcase for signal/cancel
	  th->cond_var_ptr = &cond;
	  //	  cond_var = &cond;
	  //	  rc = pthread_cond_timedwait(&cond,&mutex);
	  //	  if (rc != ETIMEDOUT)

	  rc = pthread_cond_timedwait(&cond,&mutex,&ts);
	  if (rc && rc != ETIMEDOUT)
	    printf("Error in pthread_cond_timedwait in thread_sleep/1 %s\n",strerror(rc));
	  /*
#ifdef WIN_NT
	  Sleep(floor(iso_ptoc_int_arg(CTXTc 2,"usleep/1",1) / 1000));
#else
	  usleep(iso_ptoc_int_arg(CTXTc 2,"usleep/1",1));
#endif
	  */
	  pthread_mutex_destroy(&mutex);
	  pthread_cond_destroy(&cond);
	  th->cond_var_ptr = NULL;

	  break;
	}

	case XSB_THREAD_SETUP:
	  iso_check_var(th, 3,"thread_create/[2,3]"); // should check here, rather than at end
	  id = xsb_thread_setup(th, ptoc_int(CTXTc 8), 1);
	  ctop_int( CTXTc 9, id);
	  break;

	case XSB_THREAD_CREATE_ALIAS:
	  // 1: Request_num, 2: Goal, 3: _, 4: Glsize, 5: Tcpsize, 6: Complsize, 7: pdlsize, 8: detached, 9: Pos
	    rc = xsb_thread_create_1(th, iso_ptoc_callable(CTXTc 2,"thread_create/3"),ptoc_int(CTXTc 4),
				   ptoc_int(CTXTc 5),ptoc_int(CTXTc 6),ptoc_int(CTXTc 7), ptoc_int(CTXTc 8), 
				   ptoc_int(CTXTc 9));

	  break;

	case PRINT_MESSAGE_QUEUE: {
	  XSB_MQ_Ptr xsb_mq = (XSB_MQ_Ptr) ptoc_int(CTXTc 2);
  	  MQ_Cell_Ptr cur_cell;

	  printf("first message %p last_message %p size %d\n",
		 xsb_mq->first_message,xsb_mq->last_message,xsb_mq->size);
	  cur_cell = xsb_mq->first_message;
	  while (cur_cell != 0) {
	    printf("cell %p next %p, prev %p, size %d\n",
		   cur_cell,cur_cell->next,cur_cell->prev,cur_cell->size);
	    cur_cell = cur_cell->next;
	  }
	  break;
	}

        case XSB_CHECK_ALIASES_ON_EXIT: {
	  int i = THREAD_ENTRY( th->tid );
	  if (th_vec[i].detached && th_vec[i].aliased)
	    ctop_int(CTXTc 2,1);
	  else 
	    ctop_int(CTXTc 2,0);
	  if (th_vec[i].detached)
	    ctop_int(CTXTc 3,1);
	  else 
	    ctop_int(CTXTc 3,0);
	  break;
	}

	case XSB_RECLAIM_THREAD_SETUP:
	    decrement_thread_nums;
	    th_delete(ptoc_int(CTXTc 2));
	    rc = 0;
	    break;

	case THREAD_ENABLE_CANCEL:
	    th->enable_cancel = TRUE;
	    if( th->to_be_cancelled )
	    {
	    	th->_asynint_val |= THREADINT_MARK;
		th->to_be_cancelled = 0;
	    }
	    rc = 0;
	    break;

	case THREAD_DISABLE_CANCEL:
	    th->enable_cancel = FALSE;
	    rc = 0;
	    break;

	case THREAD_PEEK_MESSAGE: {	 
	  int id = ptoc_int(CTXTc 2);
	  XSB_MQ_Ptr message_queue;
          int index = THREAD_ENTRY(id);
	  int islast = 0;

	  message_queue = &mq_table[index];

	  //	  printf("thread %d peek message %d\n",xsb_thread_entry,index);

	  pthread_mutex_lock(&message_queue->mq_mutex);
	  check_deleted(th, id, message_queue, MESG_PEEK);
	  if (!message_queue->first_message) {
	    islast = 1;
	    ctop_int(CTXTc 4,islast);
	    return success;
	  }
	  else {
	    current_mq_cell = message_queue->first_message;
	    ctop_int(CTXTc 4,islast);
	    pcreg =  (byte *)(current_mq_cell+1);
	  }
	  break;
	}

	case THREAD_REPEEK_MESSAGE: {	 
	  int id = ptoc_int(CTXTc 2);
	  XSB_MQ_Ptr message_queue;
          int index = THREAD_ENTRY(id);
	  int islast = 0;
	  
	  message_queue = &mq_table[index];

	  check_deleted(th, id, message_queue, MESG_PEEK);
	  if (message_queue->last_message == current_mq_cell) {
	    islast = 1;
	    ctop_int(CTXTc 4,islast);
	    return success;
	  }
	  else {
	    current_mq_cell = current_mq_cell->next;
	    ctop_int(CTXTc 4,islast);
	    pcreg = (byte *) (current_mq_cell+1); // offset for compiled code.
	  }
	  break;
	} 

	case THREAD_UNLOCK_QUEUE: {
	  //	  printf("warning! thread unlock queue\n");
	  int id = ptoc_int(CTXTc 2);
	  XSB_MQ_Ptr message_queue;
          int index = THREAD_ENTRY(id);
	  
	  message_queue = &mq_table[index];
	  pthread_mutex_unlock(&message_queue->mq_mutex);
	  break;
	}

	case XSB_FIRST_THREAD_PROPERTY: {
	  int tid;
	  int id;
	  if (th_first_thread != NULL) {
	    tid = (int) (th_first_thread - &th_vec[0]);
	    SET_THREAD_INCARN(tid,th_first_thread->incarn);
	    ctop_int(CTXTc 2, tid);
	    ctop_int(CTXTc 3, th_first_thread -> detached);
	    ctop_int(CTXTc 4, th_first_thread -> status);
	    if (th_first_thread -> next_entry) {
	      id = (th_first_thread-> next_entry) - (&th_vec[0]);
	      SET_THREAD_INCARN(id,th_vec[id].incarn);
	      ctop_int(CTXTc 5, id);
	    }
	    else ctop_int(CTXTc 5, 0);
	  }
	  else return FALSE;
	  break;
	}

	  /* On input, 2 is a tid w. incarn.  Need to return incarned tid,
	     property, as well as a pointer to the next.*/
	case XSB_NEXT_THREAD_PROPERTY: {
	  int tid = ptoc_int(CTXTc 2);
	  int index = THREAD_ENTRY(tid);
	  int next_index;
	  ctop_int(CTXTc 3, th_vec[index].detached);
	  ctop_int(CTXTc 4, th_vec[index].status);
	  if (th_vec[index].next_entry) {
	    next_index = (th_vec[index].next_entry - (&th_vec[0]));
	    SET_THREAD_INCARN(next_index,th_vec[next_index].incarn);
	    ctop_int(CTXTc 5, next_index);
	  }
	  else ctop_int(CTXTc 5, 0);
	  break;
	}

	  /* Used so that exit-status will be available to on-exit
	     handler.  Status is set regardless of whether thread is
	     detached -- if the thread is detached, thread entry will
	     be reclaimed on exit. */
	case XSB_SET_EXIT_STATUS: {

	  rval = iso_ptoc_int(CTXTc 2, "thread_exit/1" );
          i = THREAD_ENTRY( th->tid );
	  th_vec[i].status = rval;
	  rc = 0 ; /* keep compiler happy */
	  break;
	}

	default:
	  rc = 0 ; /* Keep compiler happy */
	  xsb_abort( "[THREAD] Invalid thread operation requested %d",request_num);
	  break;
	}
	//	ctop_int( CTXTc 5, rc );
	return success;
#else   /* Not MULTI_THREAD */
        switch( request_num )
        {
                
	case INTERRUPT_WITH_GOAL: {
	  interrupt_with_goal(CTXT);
	  break;
	}

case HANDLE_GOAL_INTERRUPT: {	 
  XSB_MQ_Ptr message_queue;
  int index = THREAD_ENTRY(id);
	  
  message_queue = &mq_table[index];
  
  current_mq_cell_seq = message_queue->first_message;
  check_glstack_overflow(3,pcreg, (512+2*current_mq_cell_seq->size*sizeof(Cell)));
  pcreg =  (byte *)(current_mq_cell_seq+1);
  //  printf("There are now %d messages in queue\n",message_queue->size);
  break;
 }

case INTERRUPT_DEALLOC: {	 
  //  int id = ptoc_int(CTXTc 2);
  XSB_MQ_Ptr message_queue;
  int index = THREAD_ENTRY(id);
  
  message_queue = &mq_table[index];

  /* Take MQ_Cell out of chain, and clear its contents.  (Will only be
     one, but in case we want to generalize at a later point) */
  if (message_queue->first_message == current_mq_cell_seq) 
      message_queue->first_message = current_mq_cell_seq->next;

  if (message_queue->last_message == current_mq_cell_seq) 
      message_queue->last_message = current_mq_cell_seq->prev;

  if (current_mq_cell_seq->prev) 
    (current_mq_cell_seq->prev)->next = (current_mq_cell_seq->next);

  if (current_mq_cell_seq->next) 
    (current_mq_cell_seq->next)->prev = (current_mq_cell_seq->prev);

  mem_dealloc(current_mq_cell_seq,current_mq_cell_seq->size,BUFF_SPACE);

  message_queue->size--;

  break;
 }
  /* Broadcasts whenever it goes from "full" to "not_full" so that
     writers will be awakened. */	
	case XSB_THREAD_SELF:
	  ctop_int( CTXTc 2, 0 );
	  break;
	case XSB_SYS_MUTEX_LOCK:
	case XSB_SYS_MUTEX_UNLOCK:
	case XSB_ENSURE_ONE_THREAD:
	case XSB_THREAD_YIELD:
	case SET_XSB_READY:
	  break;

	default:
	  xsb_abort( "[THREAD] Thread primitives not compiled in single-threaded engine (%d)",request_num );
	  break;
	}
	
	ctop_int( CTXTc 5, 0 );
	return TRUE;
#endif /* ELSE NOT MULTI_THREAD */
}


void nonmt_init_mq_table(void) {
  int i;

#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
  int status;
  pthread_mutexattr_init( &attr_errorcheck_gl );
  status = pthread_mutexattr_settype( &attr_errorcheck_gl,PTHREAD_MUTEX_ERRORCHECK_NP );
  if ( status )
    xsb_initialization_exit("Error (%s) initializing errorcheck mutex attr -- exiting\n",
  			    strerror(status));
#endif

  mq_table = mem_calloc(2*max_threads_glc+max_mqueues_glc, 
				sizeof(XSB_MQ), BUFF_SPACE);

  for( i=2*max_threads_glc; i<2*max_threads_glc+max_mqueues_glc; i++ )
    {
      mq_table[i].next_entry = &mq_table[i+1];
      mq_table[i].incarn = INC_MASK_RIGHT ; /* -1 */
    }
  mq_first_free = &mq_table[2*max_threads_glc];
  mq_last_free = &mq_table[2*max_threads_glc+max_mqueues_glc-1];
  mq_last_free->next_entry = NULL;
  mq_first_queue = NULL;

}

/* This does not yet properly init the queue lock for Windows
   sequential mode (where this routine is used for interrupt w. goal
*/
void init_message_queue(XSB_MQ_Ptr xsb_mq, int declared_size) {
  
  int reuse = xsb_mq->initted;
#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
  int status;
#endif

#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
  if( reuse ) {
    //      printf("reusing queue %d\n",xsb_mq - mq_table);
      pthread_mutex_lock( &xsb_mq->mq_mutex );
  }
#endif

  xsb_mq->first_message = 0;
  xsb_mq->last_message = 0;
  xsb_mq->size = 0;
  xsb_mq->n_threads = 0;
  xsb_mq->deleted = FALSE;
  xsb_mq->mutex_owner = -1;
  if (declared_size == MQ_CHECK_FLAGS)
    xsb_mq->max_size = (int)flags[MAX_QUEUE_TERMS];
  else xsb_mq->max_size = declared_size;

  if ( !reuse ) {

#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
#ifdef NON_OPT_COMPILE
    status = pthread_mutex_init(&xsb_mq->mq_mutex, &attr_errorcheck_gl );
    if (status) printf("Error queue initialization: queue %"Intfmt"\n",xsb_mq-mq_table);
#else 
      status = pthread_mutex_init(&xsb_mq->mq_mutex, NULL );
      if (status) printf("Error queue initialization: queue %"Intfmt"\n",xsb_mq-mq_table);
#endif

    pthread_cond_init( &xsb_mq->mq_has_free_cells, NULL );
    pthread_cond_init( &xsb_mq->mq_has_messages, NULL );
#endif

    xsb_mq->initted = TRUE;
  }

  if( PUBLIC_MQ(xsb_mq) )
  {
	xsb_mq->incarn = (xsb_mq->incarn+1) & INC_MASK_RIGHT;

	xsb_mq->id = (int)(xsb_mq - mq_table);
  	SET_THREAD_INCARN(xsb_mq->id, xsb_mq->incarn );
  }
  else
  {	int pos;

        /* for a private mq the mq id is equal to the thread id
           for a signal mq it is displaced max_threads_glc 
         */
	pos = (xsb_mq - mq_table ) % max_threads_glc;

	xsb_mq->id = (int)(xsb_mq - mq_table);
  	SET_THREAD_INCARN(xsb_mq->id, th_vec[pos].incarn );
  }

#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
  if( reuse )
	pthread_mutex_unlock( &xsb_mq->mq_mutex );
#endif
}

extern void c_assert_code_to_buff(CTXTdeclc prolog_term);

void tripwire_interrupt(CTXTdeclc char * tripwire_call) {
  int isnew;
  Psc tripwire_psc,call_psc;
  prolog_term term_to_assert;
  Cell* start_hreg;

  tripwire_psc = pair_psc((Pair)insert(tripwire_call, (byte)0, 
					pair_psc(insert_module(0,"tables")), 
					&isnew));
  call_psc = pair_psc((Pair)insert("call", (byte)3, 
					pair_psc(insert_module(0,"standard")), 
					&isnew));
  start_hreg = hreg;
  term_to_assert = makecs(hreg);
  bld_functor(hreg, call_psc); 
  hreg = hreg + 1;bld_free(hreg);
  hreg = hreg + 1;bld_free(hreg);
  hreg = hreg + 1;bld_cs(hreg,hreg+1),
  hreg = hreg + 1;bld_functor(hreg,tripwire_psc);
  hreg = hreg + 1;
  //  printf("***\n");printterm(stdout,term_to_assert,7);  printf("***\n");
  c_assert_code_to_buff(CTXTc term_to_assert);
  hreg = start_hreg;
  interrupt_with_goal(CTXT);
}


void interrupt_with_goal(CTXTdecl) {
  //          int index = THREAD_ENTRY(id);
          int index = THREAD_ENTRY(0);
	  XSB_MQ_Ptr message_queue;
	  MQ_Cell_Ptr this_cell;
	  message_queue = &mq_table[index];   // actually, only on thread/queue for seq engine 

	  while (message_queue->max_size != MQ_UNBOUNDED && message_queue->size >= message_queue->max_size) {
	    xsb_misc_error(CTXTc "exceeded size of message queue","thread_send_message",1);
	   }

	  this_cell = mem_calloc(asrtBuff->Size+sizeof(MQ_Cell),1,BUFF_SPACE);
	  this_cell->prev = message_queue->last_message;
	  //	  printf("about to copy (last %p prev %p)\n",message_queue->last_message,this_cell->prev);
	  this_cell->next = 0;
	  this_cell->size = asrtBuff->Size+sizeof(MQ_Cell);
	  /* Moves assert buffer to word just after MQ_Cell */
	  memmove(this_cell+1,asrtBuff->Buff,asrtBuff->Size); 
	  //	  printf("just moved %d\n",asrtBuff->Size);
	  
	  if (message_queue->last_message) (message_queue->last_message)->next = this_cell;
	  message_queue->last_message = this_cell;
	  message_queue->size++;
	  
	  //	    printf("There are now %d messages in queue\n",message_queue->size);
	  if (!message_queue->first_message) {
	    message_queue->first_message = this_cell;
	    //	    printf("set this message as first\n");
	  }
	  asynint_val |= THREADINT_MARK;
	  //	  printf("Interrupt with goal finished\n");
}

xsbBool mt_random_request( CTXTdecl )
{

  Integer request_num = ptoc_int(CTXTc 1);

  switch( request_num )
    {
    case INIT_MT_RANDOM:
      SRANDOM_CALL((unsigned int)time(0)); 
      break;

    case MT_RANDOM:
            ctop_int(CTXTc 2,RANDOM_CALL());
      break;

    case MT_RANDOM_INTERVAL:
      {
	UInteger rval;
	UInteger scale = ptoc_int(CTXTc 2);
	UInteger interval = ((UInteger) pow(2,32) - 1) / scale;
	//	printf("max %lx\n",((unsigned long) pow(2,32)-1));
	//	printf("int %x scale %x s1 %d ex %x\n", interval,scale,scale,16);
	rval = RANDOM_CALL(); 
	//	printf("rval %x \n",rval);
	ctop_int(CTXTc 3,(Integer)floor((double)rval / interval));
	break;
      }

    default: 
      xsb_abort( "[THREAD] Improper case for mt_rand" );
    }
  return TRUE;
}

/*
static void show_policy(void) {
  int my_policy;
  struct sched_param my_param;
  int status, min_priority, max_priority;

  if ((status = pthread_getschedparam(pthread_self(),&my_policy,&my_param))) {
    xsb_abort("[THREAD] bad scheduling status");
  }
  printf("thread routine running at %s/%d\n",
	 (my_policy == SCHED_FIFO ? "FIFO" 
	  : (my_policy == SCHED_RR ? "RR"
	     : (my_policy == SCHED_OTHER ? "OTHER"
		: "unknown") )),my_param.sched_priority);

  printf("min %d max %d\n",sched_get_priority_min(my_policy),
	 sched_get_priority_max(my_policy));
  
}
*/

