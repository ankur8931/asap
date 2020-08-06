/* File:      timer_xsb.h
** Author(s): Songmei Yu, kifer
** Contact:   xsb-contact@cs.sunysb.edu
**
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: timer_xsb.h,v 1.22 2011-05-18 19:21:41 dwarren Exp $
** 
*/

#ifndef _TIMER_XSB_H_
#define _TIMER_XSB_H_

#ifndef CONFIG_INCLUDED
#error "File xsb_config.h must be included before this file"
#endif

#ifndef CELL_DEFS_INCLUDED
#error "File cell_xsb.h must be included before this file"
#endif

#ifndef SYSTEM_FLAGS
#include "flags_xsb.h"
#endif

#include "timer_defs_xsb.h"
#include "context.h"
#include "setjmp_xsb.h" 
#include "thread_xsb.h"

#ifndef MULTI_THREAD
#ifdef WIN_NT
#include <windows.h>
#endif
#endif

#define NORMAL_TERMINATION  -1
#define TIMED_OUT            1
#define STILL_WAITING        0
#if !defined(WIN_NT) && !defined(MULTI_THREAD) 
extern sigjmp_buf xsb_timer_env;
#endif

/* TLS: changed xsb_timeout_info so to be non-empty when MULTI_THREAD
   is not defined.  Some compilers complain about empty structures. */
struct xsb_timeout_info {
  int             exitFlag;
#ifdef MULTI_THREAD
  pthread_t_p     timedThread;
  pthread_cond_t  condition;
  pthread_mutex_t mutex;
  th_context *th;
#else /* not multithreaded */
#ifdef WIN_NT
  Integer parent_thread;
#else /* UNIX */
  /* Nothing currently needed here */
#endif /* WIN_NT else */
#endif /* MULTI_THREAD else */
};

typedef struct xsb_timeout_info xsbTimeoutInfo;

struct xsb_timeout {
  xsbTimeoutInfo timeout_info;
};


typedef struct xsb_timeout xsbTimeout;

/* generic function to control the timeout */

int make_timed_call(CTXTdeclc xsbTimeout*, void (*) (xsbTimeout*));

// #define NEW_TIMEOUT_OBJECT  (xsbTimeout *)mem_alloc(sizeof(xsbTimeout))



#ifdef MULTI_THREAD /* PTHREADS */

#define SETALARM            ;  /* no-op */
#define OP_TIMED_OUT(timeout)        (op_timed_out(CTXTc timeout))

#define TURNOFFALARM        /* no-op */
#define CHECK_TIMER_SET     ((int)pflags[SYS_TIMER] > 0)
#define NOTIFY_PARENT_THREAD(timeout)                          \
   if (CHECK_TIMER_SET) {                                      \
      /* Send msg to the timer thread immediately,             \
	 so it won't wait to time out */                       \
      pthread_mutex_lock(&timeout->timeout_info.mutex);         \
      timeout->timeout_info.exitFlag = NORMAL_TERMINATION;     \
      pthread_cond_signal(&timeout->timeout_info.condition); \
      pthread_mutex_unlock(&timeout->timeout_info.mutex);       \
   }

#else /* NOT PTHREADS */

#ifdef WIN_NT

VOID CALLBACK xsb_timer_handler(HWND wind, UINT msg, UINT eventid, DWORD time);

#if defined(WIN_NT) && defined(BITS64)
_CRTIMP uintptr_t __cdecl _beginthread (_In_ void (__cdecl * _StartAddress) (void *),
        _In_ unsigned _StackSize, _In_opt_ void * _ArgList);
#elif defined(CYGWIN)
unsigned long _beginthread (void (*)(void *), unsigned, void*);
#else
unsigned int _beginthread(void(__cdecl *start_address) (void *),
			   unsigned stack_size,
			   void *arglist);
#endif

int message_pump();
//UINT xsb_timer_id; 
UINT_PTR xsb_timer_id; 

#define TURNOFFALARM        KillTimer(NULL, xsb_timer_id);
#define CHECK_TIMER_SET     ((int)pflags[SYS_TIMER] > 0)
#define NOTIFY_PARENT_THREAD(timeout)       \
   if (CHECK_TIMER_SET) {\
      /* Send msg to the timer thread immediately, \
	 so it won't wait to time out */ \
      PostThreadMessage((DWORD)(timeout->timeout_info.parent_thread), \
                                WM_COMMAND,NORMAL_TERMINATION,0);\
   }
#define OP_TIMED_OUT(timeout)        (message_pump())

#else  /* Unix */

void xsb_timer_handler(int signo);

#define SETALARM     	              (signal(SIGALRM, xsb_timer_handler))
#define TURNOFFALARM                  alarm(0);
#define CHECK_TIMER_SET               ((int)pflags[SYS_TIMER] > 0)
#define NOTIFY_PARENT_THREAD(timeout)  ;  /* no-op */
#define SET_TIMER                     alarm((int)pflags[SYS_TIMER])
#define OP_TIMED_OUT                  (sigsetjmp(xsb_timer_env,1) != 0)

#endif /* WIN_NT (else part) */

#endif /* MULTI_THREAD (else part) */

#endif /* _TIMER_XSB_H_ */
