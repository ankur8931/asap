/* File:      register.h
** Author(s): Warren, Swift, Xu, Sagonas
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
** $Id: register.h,v 1.20 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "psc_xsb.h"
#include "varstring_xsb.h"

#ifndef MULTI_THREAD

/* Argument Registers
   ------------------ */
#define MAX_REGS    257
extern Cell reg[MAX_REGS];

/*---- special registers -----------------------------------------------*/

extern CPtr ereg;	/* last activation record       */
extern CPtr breg;	/* last choice point            */
extern CPtr hreg;	/* top of heap                  */
extern CPtr *trreg;	/* top of trail stack           */
extern CPtr hbreg;	/* heap back track point        */
extern CPtr sreg;	/* current build or unify field */
extern byte *cpreg;	/* return point register        */
extern byte *pcreg;	/* program counter              */

// #define CP_DEBUG 1

#ifdef CP_DEBUG
extern Psc pscreg;
#endif
/*---- registers added for the SLG-WAM ---------------------------------*/

extern CPtr efreg;
extern CPtr bfreg;
extern CPtr hfreg;
extern CPtr *trfreg;
extern CPtr pdlreg;
extern CPtr openreg;

extern CPtr ptcpreg;	/* parent tabled CP (subgoal)	*/
extern CPtr delayreg;
extern CPtr interrupt_reg;

/*---- registers added for demand support ------------------------------*/
#ifdef DEMAND
/* demand-freeze registers */
extern CPtr edfreg;
extern CPtr bdfreg;
extern CPtr hdfreg;
extern CPtr *trdfreg;
#endif

/*---- global thread-specific char buffers for local builtins ----------*/
extern VarString *tsgLBuff1;
extern VarString *tsgLBuff2;
extern VarString *tsgSBuff1;
extern VarString *tsgSBuff2;

/*---- other stuff added for the SLG-WAM -------------------------------*/

extern int  xwammode;
extern int  level_num;
extern CPtr root_address;

/*---- for splitting stack mode ----------------------------------------*/

extern CPtr ebreg;	/* breg into environment stack */

#endif


#endif /* __REGISTER_H__ */
