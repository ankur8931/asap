/* File:      rw_lock.h
** Author(s): Rui Marques
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
** $Id: rw_lock.h,v 1.9 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

#ifdef MULTI_THREAD_RWL

#ifdef WIN_NT
#include "pthread.h"
#else /* UNIX */
#include <pthread.h>
#endif

typedef struct
{
	int havelock ;
	int writers ;
	pthread_mutex_t lock ;
	pthread_cond_t waiting ;
}
rw_lock ;

extern rw_lock trie_rw_lock ;

void rw_lock_init(rw_lock *l) ;
void rw_lock_read(rw_lock *l) ;
void rw_unlock_read(rw_lock *l) ;
void rw_lock_write(rw_lock *l) ;
void rw_unlock_write(rw_lock *l) ;

#define TRIE_R_LOCK()				\
{	if( !th->trie_locked )			\
	{	th->trie_locked = 1 ;		\
		rw_lock_read( &trie_rw_lock ) ;	\
}	}

#define	TRIE_R_UNLOCK()				\
{	if( th->trie_locked )			\
	{	th->trie_locked = 0 ;		\
		rw_unlock_read( &trie_rw_lock );\
}	}

#define TRIE_W_LOCK()				\
  rw_lock_write(&trie_rw_lock);
#define	TRIE_W_UNLOCK()				\
  rw_unlock_write(&trie_rw_lock);
#else

#define TRIE_R_LOCK()
#define TRIE_R_UNLOCK()
#define TRIE_W_LOCK()
#define	TRIE_W_UNLOCK()

#endif /* MULTI_THREAD_RWL */

