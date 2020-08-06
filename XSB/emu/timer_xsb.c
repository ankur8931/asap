/* File:      timer_xsb.c
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
** $Id: timer_xsb.c,v 1.28 2011-05-18 19:21:41 dwarren Exp $
** 
*/



#undef __STRICT_ANSI__

#include "xsb_debug.h"
#include "xsb_config.h"

#ifdef WIN_NT
#include <windows.h>
#include <winuser.h>
#else /* UNIX */
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <stdio.h>

#include "xsb_time.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "setjmp_xsb.h"
#include "timer_xsb.h"
#include "basicdefs.h"
#include "thread_xsb.h"

/* To set a timeout for a function call such as: 
  ... ...
  int foo(char *X, int Y, long Z);
  ... ...
 
   do the following steps:

	       1: Define a structure

	          struct xsb_timeout {
		     xsbTimeoutInfo timeout_info;
		     ???  return_value;
		     ???  arg1;
		     .....
		     ???  arg_n;
		  }

		  that will be used to hold the parameters of foo().
		  The member TIMEOUT_INFO is mandatory and must be the
		  first in the structure.
		  The other members of the structure are optional and they are
		  intended to represent the return value and the arguments of
		  foo().
		  Next, make a wrapper, say

		      void new_foo(xsbTimeout *pptr)

		  The type xsbTimeout is a typedef to struct xsb_timeout.
		  It is defined in timer_xsb.h.
		  In our case (given the declaration of foo() above), the
		  xsb_timeout structure will look as follows: 
	   
		   struct xsb_timeout {
                       xsbTimeoutInfo timeout_info;
		       int  return_value;
		       char *X;
		       int   Y;
		       long  Z;
		   }
   		
	       Then the wrapper will look as follows:

               void new_foo(xsbTimeout *pptr) {
		   pptr->return_value = foo(pptr->X, pptr->Y, pptr->Z);

		   NOTIFY_PARENT_THREAD(pptr);
	       }
    
       step 2: Instead of calling foo() directly, now call the generic timeout
               control function MAKE_TIMED_CALL:  
   
   	       int make_timed_call(CTXTdeclc xsbTimeout*, void (*)(xsbTimeout*));

	       For instance, 
   
	       xsbTimeout *pptr = NEW_TIMEOUT_OBJECT; // defined in timer_xsb.h
	       int timeout_flag;

	       // timeout is set in Prolog using set_timer/1 call
	       if (CHECK_TIMER_SET) {
		  timeout=make_timed_call(pptr, new_foo);
		  if (timeout_flag == TIMER_SETUP_ERR) {
		     // problem setting up timer (Windows only)
	          } else if (timeout_flag) {
	            // timeout happened
		    .....
		  }
	       } else {
	         // timer is not set
		 new_foo(pptr); 
	       }

       step 3: Free the xsbTimeout object when appropriate.
*/

/* TLS comment: the implementation differs depending on whether XSB is
 * multi-threaded, single threaded UNIX or single-threaded Windows.
 * In the mt case, a timed pthread is created and detached, and a
 * timed condition variable is set.  The timed condition variable is
 * responsible for waking up the calling thread, which then checks to
 * see whether the call has timed out, or has succeeded.  In the
 * single-thread case of UNIX, an alarm and SIGALRM signal is used;
 * In the single-thread case of Windows, thread messaging is used.
 */

#ifdef MULTI_THREAD

void init_machine(CTXTdeclc int, int, int, int);

#else /* not multithreaded */

#ifdef WIN_NT
static int exitFlag = STILL_WAITING;
static Integer timedThread;
HANDLE sockEvent = NULL;
#else /* UNIX */ 
sigjmp_buf xsb_timer_env;
#endif

#endif /* MULTI_THREAD */

#ifdef MULTI_THREAD
int op_timed_out(CTXTdeclc xsbTimeout *timeout)
{
  struct timespec wakeup_time;                            // time.h
  int rc;

  wakeup_time.tv_sec = time(NULL) + (int)pflags[SYS_TIMER];
  pthread_mutex_lock(&timeout->timeout_info.mutex);
  rc = pthread_cond_timedwait(&timeout->timeout_info.condition, 
			      &timeout->timeout_info.mutex, &wakeup_time);
  pthread_mutex_unlock(&timeout->timeout_info.mutex);
  if (rc != 0) {
    switch(rc) {
    case EINVAL:
      xsb_bug("pthread_cond_timedwait returned EINVAL");
      break;
    case ETIMEDOUT:
      break;
    case ENOMEM:
      xsb_error("Not enough memory to wait\n");
      break;
    default:
      xsb_bug("pthread_cond_timedwait returned an unexpected value (%d)\n", rc);      
    }
  }
  TURNOFFALARM;  // mt-noop(?)
  switch (timeout->timeout_info.exitFlag) {
  case STILL_WAITING: /* The call timed out */
    PTHREAD_CANCEL(timeout->timeout_info.timedThread);
    return TRUE;
  case TIMED_OUT:
    return TRUE;
  case NORMAL_TERMINATION:
    return FALSE;
  default:
    xsb_bug("timed call's exit flag is an unexpected value (%d)", timeout->timeout_info.exitFlag);
    return FALSE;
  }
}

#else /* NOT MULTITHREADED */

#ifdef WIN_NT
VOID CALLBACK xsb_timer_handler(HWND wind, UINT msg, UINT eventid, DWORD time)
{
  if (exitFlag == STILL_WAITING)
    exitFlag = TIMED_OUT; /* tell the timed thread to quit */
  TerminateThread((HANDLE)timedThread, 1);
}

/* message_pump is also known as OP_TIMED_OUT (when WIN_NT is defined) */
int message_pump()
{
  MSG msg;
  if ((xsb_timer_id = SetTimer(NULL,
			       0,
			       /* set timeout period */
			       (UINT)((int)pflags[SYS_TIMER] * 1000),
			       (TIMERPROC)xsb_timer_handler))
      == 0) {
    xsb_error("SOCKET_REQUEST: Can't create timer: %d\n", GetLastError());
    return TIMER_SETUP_ERR;
  }

  exitFlag=STILL_WAITING;
  while ((exitFlag==STILL_WAITING) && GetMessage(&msg,NULL,0,0)) {
    DispatchMessage(&msg);
    if (msg.wParam == NORMAL_TERMINATION)
      break;
  }

  if (xsb_timer_id != 0)
    TURNOFFALARM;

  if (exitFlag == TIMED_OUT) 
    return TRUE;  /* timed out */
  else
    return FALSE;  /* not timed out */
}

#else  /* UNIX */

/* SIGALRM handler for controlling time outs */
void xsb_timer_handler(int signo)
{ 
  siglongjmp(xsb_timer_env,1);
}

#endif

#endif /* MULTI_THREAD */

/* the following function is a general format for timeout control. it takes 
   function calls which need timeout control as argument and controls the
   timeout for different platform */ 
int make_timed_call(CTXTdeclc xsbTimeout *pptr,  void (*fptr)(xsbTimeout *))
{
#if defined(WIN_NT) || defined(MULTI_THREAD)   
  int return_msg; /* message_pump() return value */
#endif

#ifdef MULTI_THREAD /* USE PTHREADS */

#ifdef WIN_NT
  pptr->timeout_info.timedThread = mem_alloc(sizeof(pthread_t),LEAK_SPACE);
#define TIMED_THREAD_CREATE_ARG pptr->timeout_info.timedThread
#else
#define TIMED_THREAD_CREATE_ARG &pptr->timeout_info.timedThread
#endif
  pptr->timeout_info.th=th;
  // below, fptr is pointer to start routine, pptr is pointer to arg-array.
  // TIMED_THREAD_CREATE_ARG is a cell of timeout_info.
  if (pthread_create(TIMED_THREAD_CREATE_ARG, NULL, fptr, pptr)) {
    xsb_error("SOCKET_REQUEST: Can't create concurrent timer thread\n");
    return TIMER_SETUP_ERR;
  }
  PTHREAD_DETACH(pptr->timeout_info.timedThread);
  return_msg = OP_TIMED_OUT(pptr);
#ifdef WIN_NT
  mem_dealloc(pptr->timeout_info.timedThread,sizeof(pthread_t),LEAK_SPACE);
#endif
  if (return_msg == TIMER_SETUP_ERR) {
    return TIMER_SETUP_ERR;  
  } else if (!return_msg) { /* no timeout */
    TURNOFFALARM;
    return FALSE;
  } else { /* timeout */
    TURNOFFALARM;
    return TRUE;
  }

#else /* not multithreaded */

#ifdef WIN_NT
  /* create a concurrent timed thread; 
     pptr points to the procedure to be timed */  
  pptr->timeout_info.parent_thread = (Integer)GetCurrentThreadId();
  if((timedThread = _beginthread((void *)fptr,0,(void*)(pptr)))==-1) { 
    xsb_error("SOCKET_REQUEST: Can't create concurrent timer thread\n");
    return TIMER_SETUP_ERR;
  }
  return_msg = OP_TIMED_OUT(pptr);
  /* OP_TIMED_OUT returns TRUE/FALSE/TIMER_SETUP_ERR */
  if (return_msg == TIMER_SETUP_ERR) {
    return TIMER_SETUP_ERR;  
  } else if (!return_msg) { /* no timeout */
    TURNOFFALARM;
    return FALSE;
  } else { /* timeout */
    TURNOFFALARM;
    return TRUE;
  }
#else /* UNIX */
  SETALARM;     /* specify the timer handler in Unix;
		   Noop in Windows (done in SetTimer) */

  if ( !OP_TIMED_OUT ) { /* no timeout */
    SET_TIMER; /* specify the timeout period */       
    (*fptr)(CTXTc pptr); /* procedure call that needs timeout control */
    TURNOFFALARM;
    return FALSE;
  } else {  /* timeout */
    TURNOFFALARM;
    return TRUE;
  }

#endif

#endif

}
