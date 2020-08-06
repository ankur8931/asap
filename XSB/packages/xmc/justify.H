/* File:      justify.H
** Author(s): Abhik Roychoudhury
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
** $Id: justify.H,v 1.6 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- import conset/2, conget/2, coninc/1 from gensym.
:- import member/2, append/3 from basics.
:- import get_residual/2, get_calls_for_table/2 from tables.
:- import variant/2 from subsumes.
:- import close_open_tables/0 from machine.
:- import trie_intern/4, trie_interned/4, trie_unintern/2, 
		new_trie/1, delete_trie/1 from intern.
