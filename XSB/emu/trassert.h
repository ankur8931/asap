/* File:      trassert.h
** Author(s): Prasad Rao
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
** $Id: trassert.h,v 1.16 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

#include "context.h"

extern int trie_assert(CTXTdecl);
extern int trie_retract(CTXTdecl);
extern int trie_retract_safe(CTXTdecl);


#ifdef MULTI_THREAD
#define switch_to_shared_trie_assert(MUTEX_PTR) {	\
    threads_current_sm = SHARED_SM;			\
    smBTN = &smAssertBTN;				\
    smBTHT = &smAssertBTHT;				\
    pthread_mutex_lock( MUTEX_PTR ) ;			\
}
#define switch_from_shared_trie_assert(MUTEX_PTR) {	\
    pthread_mutex_unlock( MUTEX_PTR ) ;			\
    threads_current_sm = SHARED_SM;			\
    smBTN = &smAssertBTN;				\
    smBTHT = &smAssertBTHT;				\
}
#define switch_to_trie_assert {			\
    threads_current_sm = PRIVATE_SM;		\
    smBTN = private_smAssertBTN;			\
    smBTHT = private_smAssertBTHT;			\
}

#else
#define switch_to_trie_assert {			\
    smBTN = &smAssertBTN;			\
    smBTHT = &smAssertBTHT;			\
}
#endif

#define switch_from_trie_assert {	    \
    smBTN = &smTableBTN;		    \
    smBTHT = &smTableBTHT;		    \
}

