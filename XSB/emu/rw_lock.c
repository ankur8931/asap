/* File:      rw_lock.c
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
** $Id: rw_lock.c,v 1.9 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

/* Implementation of rwlocks in pthreads

   Gives preference to writers to avoid writer starvation.
   Assumes there are few enough write locks so that read locks won't starvate.
 */

#include "xsb_config.h"

#ifdef MULTI_THREAD_RWL

#include "rw_lock.h"

rw_lock trie_rw_lock ;

void rw_lock_init(rw_lock *l)
{
	l->writers = l->havelock = 0 ;
	pthread_mutex_init(&l->lock, NULL );
	pthread_cond_init(&l->waiting, NULL);
}

void rw_lock_read(rw_lock *l)
{
	pthread_mutex_lock(&l->lock) ;
	while( l->writers > 0 )
		pthread_cond_wait(&l->waiting,&l->lock) ;
	l->havelock++;
	pthread_mutex_unlock(&l->lock) ;
}

void rw_unlock_read(rw_lock *l)
{
	pthread_mutex_lock(&l->lock) ;
	l->havelock -- ;
	if( l->havelock == 0 )
		pthread_cond_broadcast(&l->waiting) ;
	pthread_mutex_unlock(&l->lock) ;
}

void rw_lock_write(rw_lock *l)
{
	pthread_mutex_lock(&l->lock) ;
	l->writers ++ ;
	while( l->havelock > 0 )
		pthread_cond_wait(&l->waiting,&l->lock) ;
	l->havelock ++ ;
	pthread_mutex_unlock(&l->lock) ;
}

void rw_unlock_write(rw_lock *l)
{
	pthread_mutex_lock(&l->lock) ;
	l->havelock -- ;
	l->writers -- ;
	if( l->havelock == 0 )
		pthread_cond_broadcast(&l->waiting) ;
	pthread_mutex_unlock(&l->lock) ;
}

#endif /* MULTI_THREAD */
