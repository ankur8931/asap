/* File:      suppltab.H
** Author(s): Kostis F. Sagonas
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
** $Id: suppltab.H,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export supplementary_tabling/2.

:- dynamic edb_dep(_,_,_), suppl_folding(_,_,_).

:- import name/2 from standard.
:- import gensym_pred/2 from preproc.
:- import conget/2 from gensym.
:- import call_themselves/3, table_it/1 from tabdef.
:- import merge/3, reverse/2, closetail/1 from listutil.
:- import (dynamic)/1, assert/1, retractall/1 from assert.
:- import sym_prop/2, sym_name/3, sym_insert/5 from symtab.
:- import option/1, error/1, message/1 from auxlry.
:- import memberchk/2, member/2 from basics.

/* --------------------- end of file suppltab.H ----------------------- */
