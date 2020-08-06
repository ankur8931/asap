/* File:      count.H
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
** $Id: count.H,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- export
	zero/1, incr/1,
	state/1, reachable_transitions/3,
	count/2, count/3, write_count/0,
	checkit/1,
	timeit/1, retimeit/1.

:- import transition/3 from mucalculus.
:- import conget/2, conset/2 from gensym.
:- import start/1 from usermod.
