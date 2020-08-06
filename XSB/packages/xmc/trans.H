/* File:      trans.H	-- varieties of trans relation
** Author(s): Yifei Dong
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
** $Id: trans.H,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- import member/2 from basics.
:- import trans/3 from usermod.

:- export trans_not_a/3.
:- export trans_not_member/3.
:- export trans_member/3.
