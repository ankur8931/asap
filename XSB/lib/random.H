/* File:      random.H
** Author(s): Baoqiu Cui
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1998
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
** $Id: random.H,v 1.10 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

:- export datime_setrand/0,
          random/1,
	  random/3,
	  randseq/3,
	  randset/3,
	  randset/3,
	  getrand/1,
   setrand/1,
   gauss/2,
   exponential/2,
   weibull/3.

:- import abort/1, integer/1, repeat/0 from standard.
:- import keysort/2 from setof.
:- import datime/1 from standard.
