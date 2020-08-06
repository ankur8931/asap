/* File:      scc_xsb.c
** Author(s): Kostis Sagonas
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
** $Id: scc_xsb.c,v 1.14 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "register.h"
#include "psc_xsb.h"
#include "tries.h"
#include "tab_structs.h"
#include "scc_xsb.h"

#if (!defined(LOCAL_EVAL))

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/* Documentation added 02/02 TLS.

What does this do?  Well, it wants to break up the ASCC into SCCs.  It
doesnt quite do this exactly, rather it constructs a series of n DFS
visits of the DGT of the ASCC starting with the leader.  Thus, the n
SCCs that constitute the ASCC are traversed, and for each SCC_n, max_u
is set to the oldest subgoal in SCC_n.  The idea is that after all
SCCs are checked, max_u is set to the oldest subgoal in an independant
SCC.

While there could be several independent SCCs, only the last
independent SCC to be checked will be accessible 

Also, suppose there are 2 independent SCCs, SCC_1 and SCC_2.  Also
suppose SCC_1 has no loop through negation, but that SCC_2 does.  In
this case Delaying may be prescribed for the ASCC because SCC_2 is
acessible, but SCC_1 is not.  

The algorithm could be modified by marking compl_visited with a ptr to
the oldest subgoal of the SCC.  In this case, if the first returned
SCC had a loop through negation, the second could be checked, and so
on until an SCC that had no loop through negation was obtained.  

In addition, such a change would obviate the need for the
find_independent_scc called in wfs_xsb.i

*/

/* Note that this function does not need to check whether a given
   subgoal is completed -- that has already been checked by
   add_ascc_edges() in construct_dep_graph() */

static void DFS_DGT_visit(ComplStackFrame u)
{
    EPtr eptr;

    compl_visited(u) = TRUE;
    for (eptr=compl_DGT_edges(u); eptr != NULL; eptr=next_edge(eptr)) {
      if (!compl_visited(edge_to_node(eptr)))
	DFS_DGT_visit(edge_to_node(eptr));
    }
}

ComplStackFrame DFS_DGT(CTXTdeclc ComplStackFrame leader)
{
    ComplStackFrame u, max_u = NULL;

    for (u = leader; u >= (ComplStackFrame)openreg; u--)
      if (!compl_visited(u)) {
	DFS_DGT_visit(u); max_u = u;
      }
    return max_u;
}

/*----------------------------------------------------------------------*/
/*  find_independent_scc(ComplStackFrame)				*/
/*	Finds the subgoals in the same SCC as the subgoal that is	*/
/*	given as input.  The subgoals are indicated by marking the	*/
/*	"visited" field of their completion stack frame.		*/
/*----------------------------------------------------------------------*/

void find_independent_scc(ComplStackFrame u)
{
    EPtr eptr;

    compl_visited(u) = TRUE;
    for (eptr=compl_DG_edges(u); eptr != NULL; eptr=next_edge(eptr)) {
      if (!compl_visited(edge_to_node(eptr)))
	find_independent_scc(edge_to_node(eptr));
    }
}

/*----------------------------------------------------------------------*/

#endif
