/* File:      thread_xsb.h
** Author(s): Marques
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

#ifndef __THREAD_XSB_H__

#define __THREAD_XSB_H__

#include "context.h"
#include "thread_defs_xsb.h"
#include "basictypes.h"
#include "export.h"

xsbBool xsb_thread_request( CTXTdecl ) ;
xsbBool mt_random_request( CTXTdecl ) ;
int xsb_thread_self() ;

#define INC_MASK_RIGHT			0x3ff

extern int max_mqueues_glc ;
extern int max_threads_glc ;
extern void tripwire_interrupt(CTXTdeclc char *);

#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
#include <pthread.h>
#endif

#ifdef MULTI_THREAD

#ifdef WIN_NT
typedef pthread_t* pthread_t_p;
#define PTHREAD_CREATE(a,b,c,d) pthread_create(&a,b,c,d);
#define PTHREAD_DETACH(a) pthread_detach(*a);
#define PTHREAD_CANCEL(a) pthread_cancel(*a);
#else
typedef pthread_t pthread_t_p;
#define PTHREAD_CREATE(a,b,c,d) pthread_create(a,b,c,d);
#define PTHREAD_DETACH(a) pthread_detach(a);
#define PTHREAD_CANCEL(a) pthread_cancel(a);
#endif

/* Lower 20 bits of thread are entry, others are incarnation
   TLS: problems with INC_MASK_RIGHT and 64-bits? */
#define INC_SHIFT			20
#define INC_MASK			(INC_MASK_RIGHT<<INC_SHIFT)
#define ENTRY_MASK			0x000fffff

#define DEFAULT_MQ_SIZE 1024

#define THREAD_INCARN(TID)		((((Integer) TID)&INC_MASK)>>INC_SHIFT)
#define SET_THREAD_INCARN(TID,INC)	((TID) = ((TID & ~INC_MASK) |\
					 (((INC)<<INC_SHIFT) & INC_MASK)))

typedef struct Mutex_Frame {
  pthread_mutex_t th_mutex;
  int num_locks;
  Integer owner;
} MutexFrame;

typedef struct Dynamic_Mutex_Frame *DynMutPtr;
typedef struct Dynamic_Mutex_Frame {
  pthread_mutex_t th_mutex;
  int             num_locks;
  int             tot_locks;
  int             owner;
  DynMutPtr       next_dynmut;
  DynMutPtr       prev_dynmut;
} DynMutexFrame;

extern MutexFrame sys_mut[MAX_SYS_MUTEXES];

#define MUTARRAY_MUTEX(i) &(sys_mut[(i)].th_mutex)
#define MUTARRAY_NUMLOCKS(i) sys_mut[(i)].num_locks
#define MUTARRAY_OWNER(i) sys_mut[(i)].owner

extern void print_mutex_use(void);
extern void release_held_mutexes(CTXTdecl);

extern pthread_mutexattr_t attr_rec_gl ;
extern pthread_mutex_t completing_mut;
extern pthread_cond_t completing_cond;

extern pthread_attr_t detached_attr_gl;
extern pthread_attr_t normal_attr_gl;

extern counter max_threads_sofar ;

extern th_context *main_thread_gl ;

#ifdef __cplusplus
extern "C" {
#endif

extern int call_conv xsb_ccall_thread_create(th_context *th,th_context **thread_return);

#ifdef __cplusplus
}
#endif

#ifdef NON_OPT_COMPILE

#define SYS_MUTEX_LOCK( M )   {pthread_mutex_lock(MUTARRAY_MUTEX(M));	      \
                               MUTARRAY_OWNER(M) = xsb_thread_id;	      \
                               MUTARRAY_NUMLOCKS(M)++; }

#define SYS_MUTEX_LOCK_NOERROR( M )   {pthread_mutex_lock(MUTARRAY_MUTEX(M));  \
                                       MUTARRAY_NUMLOCKS(M)++; }

#define SYS_MUTEX_INCR( M )	{ MUTARRAY_NUMLOCKS(M)++; }

#else

#define SYS_MUTEX_LOCK( M )   {pthread_mutex_lock( MUTARRAY_MUTEX(M));	      \
                               MUTARRAY_OWNER(M) = xsb_thread_id; }

#define SYS_MUTEX_LOCK_NOERROR( M )   {pthread_mutex_lock(MUTARRAY_MUTEX(M)); }

#define SYS_MUTEX_INCR( M )

#endif /* NON_OPT_COMPILE */

#define SYS_MUTEX_UNLOCK( M )   {pthread_mutex_unlock( MUTARRAY_MUTEX(M) );        \
                                 MUTARRAY_OWNER(M) = -1; }

#define SYS_MUTEX_UNLOCK_NOERROR( M )   {pthread_mutex_unlock( MUTARRAY_MUTEX(M) );        \
                                         MUTARRAY_OWNER(M) = -1; }
#define THREAD_ENTRY(TID)		((Integer)(TID)&ENTRY_MASK)
#else  /* not MULTI_THREAD */
#define SYS_MUTEX_LOCK( M )
#define SYS_MUTEX_LOCK_NOERROR( M )
#define SYS_MUTEX_UNLOCK( M )
#define SYS_MUTEX_UNLOCK_NOERROR( M )
#define SYS_MUTEX_INCR( M )
#define SET_THREAD_INCARN(TID,INC) 
extern void nonmt_init_mq_table(void);
#define THREAD_ENTRY(TID)		0

#endif /* MULTI_THREAD */

typedef struct XSB_Message_Queue {
#if !defined(WIN_NT) || defined(MULTI_THREAD)  //TES mq ifdef
  pthread_mutex_t              mq_mutex;
  pthread_cond_t               mq_has_free_cells;
  pthread_cond_t               mq_has_messages;
#endif
  MQ_Cell_Ptr                  first_message;
  MQ_Cell_Ptr                  last_message;
  int                          size;		/* number of messages in the queue */
  int                          max_size;
  unsigned int	               n_threads: 16;   /* number of threads waiting */
  unsigned int                 initted: 1;      /* check to see if mutexes/condvars must be set */
  unsigned int		       deleted: 1;	/* true if is to be deleted */
  unsigned int 		       incarn : 12 ;    /* used to generate diff ids for public mqs */
  int id ;
  int mutex_owner;
  struct XSB_Message_Queue     *next_entry,	/* either next free slot or next thread */
			       *prev_entry ;	/* only valid for slots used for threads */
} XSB_MQ;
typedef XSB_MQ *XSB_MQ_Ptr;
extern XSB_MQ_Ptr  mq_table;

#ifdef MULTI_THREAD
void init_system_mutexes( void ) ;
void init_system_threads( th_context * ctxt ) ;

th_context *find_context( Integer tid );
int valid_tid( int tid );
#ifdef SHARED_COMPL_TABLES
int get_waiting_for_tid( int t );
#endif
#else /* not MULTI_THREAD */
#endif

#define ENSURE_ONE_THREAD()						\
  { if( flags[NUM_THREADS] > 1 )					\
      xsb_abort( "Operation is permitted only when a single thread is active" ) ; \
  }

/*
  TLS: the mt engine does not yet work for enable no cygwin, but this
   allows random and srandom to be used.  They're both defined in
   stdlib.h as they should be, but Windows calls them rand and srand.
*/

#if defined(WIN_NT)

#define RANDOM_CALL rand
#define SRANDOM_CALL srand

#else

#define RANDOM_CALL random
#define SRANDOM_CALL srandom

#endif


/* TLS: for Cygwin, these constants must be re-defined */

#if defined(DARWIN) || defined(FREEBSD) || defined(SOLARIS) || defined(CYGWIN)

#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_ERRORCHECK_NP PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_FAST_NP PTHREAD_MUTEX_NORMAL

#endif

extern int max_mqueues_glc ;
extern int max_threads_glc ;
extern void init_message_queue(XSB_MQ_Ptr, int);

#endif
