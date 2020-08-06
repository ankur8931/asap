/* File:      aggregs.H
** Author(s): David S. Warren, Swift
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, , 1993-1998
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
** $Id: aggregs1.H,v 1.3 2010-08-19 15:03:38 spyrosh Exp $
** 
*/


% To use some of these predicates, the indicated HiLog predicates must
% be defined in usermod

:- export 
	bagReduce/4,
	bagReduce/5,
   bagReduce1/5,
	bagPO/3,
	bagPO/4,
	bagPO1/3,
	bagOf/2,
% to use bagMin, must define in usermod: 
%	:- hilog minimum.  minimum(X,Y,Z) :- X @< Y -> Z=X ; Z=Y.
	bagMin/2,
% to use bagMax, must define in usermod: 
%	:- hilog maximum.  maximum(X,Y,Z) :- X @< Y -> Z=Y ; Z=X.
	bagMax/2,
% to use bagSum, must define in usermod: 
%	:- hilog sum.  sum(X,Y,Z) :- Z is X+Y.
	bagSum/2,
% to use bagCount, must define in usermod: 
%	:- hilog successor.  successor(X,_Y,Z) :- Z is X+1.
	bagCount/2,
% to use bagAvg, must define in usermod: 
%	:- hilog sumcount.  sumcount([S|C],X,[S1|C1]) :- S1 is S+X, C1 is C+1.
	bagAvg/2,
	filterPO/2,
	filterPO/3,
	filterReduce/4,
	filterReduce_u/4,
        filterReduce1/4,  % this predicate only for demonstration purposes.
	table_once/1.
:- export filterMax/2, filterMin/2, filterPOA/4.

:- import apply/2,apply/3,apply/4 from usermod.

:- import arg/3, call/1, call_c/1, atom/1, '=..'/2 from standard.
:- import call/2, call/3, call/4 from standard.
:- import member/2, copy_term/2 from basics.
:- import sort/2, findall/3, bagof/3, excess_vars/4 from setof.
:- import ','/2 from usermod.
:- import subsumes/2, variant/2 from subsumes.

:- import delete_return/2,
	  get_producer_call/3,
	  get_ptcp/1,
	  get_returns/3,
	  is_incomplete/2,
	  table_state/4
     from tables.
