/* File:      sig_xsb.h
** Author(s): Jiyang Xu
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
** $Id: sig_xsb.h,v 1.12 2012-03-11 00:55:47 tswift Exp $
** 
*/


/* signals 1 and 8-15 are asynchronous. They are flaged by the variable  */
/* asynint_val, which can be 8-15 and maybe OR'ed with KEYINT_MARK.	 */
/* At some interval (currently at the entry of "call", etc), a check is  */
/* made to see if there is an asynchronous interrupt occurred. If yes,	 */
/* the procedure "interrupt_proc" is invoked. The rest of signals (0 and */
/* 2-4) are synchronous. The procedure "interrupt_proc" is invoked       */
/* directly.								 */

#define MYSIG_UNDEF    0       		/* undefined predicate */
#define MYSIG_KEYB     1	       	/* keyboard interrupt (^C) */
#define MYSIG_SPY      3		/* spy point */
#define MYSIG_TRACE    4	       	/* trace point */
#define MYSIG_ATTV     8		/* attributed var interrupt */
#define TIMER_INTERRUPT    0xd
#define MYSIG_PSC          0xe                /* new PSC creation interrupt */
#define THREADSIG_CANCEL   0xf
#define MYSIG_CLAUSE  16	       	/* clause interrupt */

#define LAZY_REEVAL_INTERRUPT 0xc

#define PROFINT_MARK 0x10		/* XSB Profiling interrupt */
#define MSGINT_MARK  0x20		/* software message interrupt */
#define ATTVINT_MARK 0x40		/* attributed var interrupt */
#define KEYINT_MARK  0x80		/* keyboard interrupt ^C */
#define TIMER_MARK   0x100
#define THREADINT_MARK 0x1000

