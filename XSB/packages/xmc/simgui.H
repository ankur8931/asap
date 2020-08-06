/* File:      simgui.P -- Prolog interface procedures for simulator
** Author(s): Ping Yang
** Contact:   lmc@cs.sunysb.edu
** 
** Copyright (C) SUNY at Stony Brook, 1998-2001
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
** $Id: simgui.H,v 1.3 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- import trie_intern/3, trie_interned/3 from intern. 
:- import member/2, append/3, reverse/2 from basics.
:- import conget/2, conset/2 from gensym.
:- import startstate/2 from usermod.

