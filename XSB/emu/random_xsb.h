/* File:      random_xsb.h
** Author(s): Baoqiu Cui
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: random_xsb.h,v 1.7 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

extern int ret_random(CTXTdecl);
extern int getrand(CTXTdecl);
extern void setrand(CTXTdecl);

#define RET_RANDOM	1
#define GET_RAND	2
#define SET_RAND	3
