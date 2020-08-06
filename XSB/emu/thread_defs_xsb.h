/* File:      thread_defs_xsb.h
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


#ifndef __THREAD_DEFS_XSB_H__

#define __THREAD_DEFS_XSB_H__

 
/* THREAD PRIMITIVES */
#define XSB_THREAD_CREATE_FLAGS	 1
#define XSB_THREAD_EXIT		 2
#define XSB_THREAD_JOIN		 3
#define XSB_THREAD_DETACH	 4
#define XSB_THREAD_SELF		 5

#define XSB_MUTEX_INIT		 6
#define XSB_MUTEX_LOCK		 7
#define XSB_MUTEX_TRYLOCK	 8
#define XSB_MUTEX_UNLOCK 	 9
#define XSB_MUTEX_DESTROY       10

#define XSB_SYS_MUTEX_LOCK	11
#define XSB_SYS_MUTEX_UNLOCK	12

#define XSB_ENSURE_ONE_THREAD	13
#define XSB_THREAD_YIELD	14

#define XSB_SHOW_MUTEXES        15

#define XSB_SET_INIT_GLSTACK_SIZE	16
#define XSB_SET_INIT_TCPSTACK_SIZE	17
#define XSB_SET_INIT_PDL_SIZE		18 
#define XSB_SET_INIT_COMPLSTACK_SIZE	19

#define XSB_THREAD_PROPERTY             20
#define XSB_THREAD_INTERRUPT            21

#define ABOLISH_PRIVATE_TABLES          22
#define ABOLISH_SHARED_TABLES           23

#define GET_FIRST_MUTEX_PROPERTY        24
#define GET_NEXT_MUTEX_PROPERTY         25
#define MUTEX_UNLOCK_ALL                26

#define ABOLISH_ALL_PRIVATE_TABLES      27
#define ABOLISH_ALL_SHARED_TABLES       28

#define PTHREAD_SETCONCURRENCY          29
#define PTHREAD_GETCONCURRENCY          30

#define SET_XSB_READY                   31

#define MESSAGE_QUEUE_CREATE            32
#define THREAD_SEND_MESSAGE             33
#define THREAD_TRY_MESSAGE              34
#define THREAD_RETRY_MESSAGE            35
#define THREAD_ACCEPT_MESSAGE           36
#define XSB_MESSAGE_QUEUE_DESTROY       37

#define PRINT_MESSAGE_QUEUE             40

#define XSB_THREAD_CREATE_PARAMS        41
#define XSB_USLEEP                      42
#define XSB_THREAD_SETUP                43
#define XSB_THREAD_CREATE_ALIAS         44
#define XSB_CHECK_ALIASES_ON_EXIT       45
#define XSB_RECLAIM_THREAD_SETUP        46

#define THREAD_ENABLE_CANCEL		47
#define THREAD_DISABLE_CANCEL		48
#define THREAD_PEEK_MESSAGE             49
#define THREAD_REPEEK_MESSAGE           50
#define THREAD_UNLOCK_QUEUE             51
#define XSB_FIRST_THREAD_PROPERTY       52
#define XSB_NEXT_THREAD_PROPERTY        53
#define XSB_SET_EXIT_STATUS             54
#define INTERRUPT_WITH_GOAL             55
#define HANDLE_GOAL_INTERRUPT           56
#define INTERRUPT_DEALLOC               57

/* MUTEX KINDS (under LINUX) */

#define XSB_FAST_MUTEX		1
#define XSB_RECURSIVE_MUTEX	2
#define XSB_ERRORCHECK_MUTEX	3

/* Mutexes to protect execution of critical system stuff */

#define MAX_SYS_MUTEXES		40

/* Be sure to update this if you add a recusive mutex */

/* Handles various data structures for dynamic code, including
   dispatch blocks. */
#define MUTEX_DYNAMIC		0

/* Use this one only for the stream_table itself: see I/O code for 
   locking the streams themselves. */
#define MUTEX_IO		1	/* Must be recursive */

/* MUTEX_TABLE handles global table data structures: lists of tifs,
   dispatch blocks, table dispatch blocks, and
   deleted_table_info_frames. */
/* If you add a mutex, also update mutex_names[] in thread_xsb.c */
/* first mutexes are recursive */

#define MUTEX_TABLE		2
#define MUTEX_TRIE		3
#define MUTEX_SYMBOL		4
#define MUTEX_FLAGS		5
#define MUTEX_LOAD_UNDEF	6	/* Must be recursive */
#define MUTEX_DELAY		7
#define MUTEX_SYS_SYSTEM	8      /* recursive prob. not necess */
#define MUTEX_LOADER            9

#define LAST_REC_MUTEX		10

/* pseudo mutexes are used to count accesses to mutexes or sets of mutexes
   that couldn't adequately be treated has system mutexes */
   
/* Non-recursive */
#define MUTEX_CONS_LIST		13
#define MUTEX_COMPL		14	/* pseudo mutex */
#define MUTEX_STRING		15
#define MUTEX_CALL_TRIE		16	/* pseudo mutex */
#define MUTEX_SM		17	/* pseudo mutex */
#define MUTEX_THREADS		18	/* pseudo mutex */
#define MUTEX_SOCKETS		19
#define MUTEX_MEM		20
#define MUTEX_ODBC		21
#define MUTEX_GENTAG		22
/* probably obsolete */
#define MUTEX_DISPBLKHDR        23

#define MUTEX_DYNMUT            24


/* Some mutexes available to users (obsolete) */
#define MUTEX_CONSOLE		30
#define MUTEX_USER1		31
#define MUTEX_USER2		32
#define MUTEX_USER3		33
#define MUTEX_USER4		34
#define MUTEX_USER5		35
#define MUTEX_USER6		36
#define MUTEX_USER7		37
#define MUTEX_USER8		38
#define MUTEX_USER9		39

/* Used for random number generation in testing modules */

#define INIT_MT_RANDOM          0
#define MT_RANDOM               1
#define MT_RANDOM_INTERVAL      2

#define THREAD_RUNNING          0
#define THREAD_SUCCEEDED        1
#define THREAD_FAILED           2
#define THREAD_EXITED           3
#define THREAD_EXCEPTION        4
#define THREAD_CANCELLED        5

#define MQ_UNBOUNDED            0
#define MQ_CHECK_FLAGS          -1

#ifdef MULTI_THREAD
#define MAX_THREADS		1024
#define MAX_MQUEUES		1024
#endif

#ifndef MULTI_THREAD
#define MAX_THREADS		1
#define MAX_MQUEUES		1
#endif

#endif /* __THREAD_DEFS_XSB_H__ */
