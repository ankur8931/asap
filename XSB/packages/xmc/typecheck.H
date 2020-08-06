/* File:      typecheck.H -- XL typechecker
** Author(s): VN Venkatakrishnan
** Contact:   lmc@cs.sunysb.edu
** 
** Copyright (C) SUNY at Stony Brook, 1998-2000
** 
** XMC is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XMC is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XMC; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: typecheck.H,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- op(1100, fx, datatype).

:- export typecheck/1.
:- export type/2.

%:- import apply/2, apply/4 from usermod.
:- import append/3 from basics.

:- import pdef/3 from usermod.
%:- import qpdef/3 from usermod.
%:- import tc_error/4 from usermod.

%:- import get_returns/3, delete_return/2 from tables.
%:- import variant/2 from subsumes.

:- import atype/2 from usermod.

:- import reverse/2, ground/1 from basics.
