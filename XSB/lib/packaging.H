/* File:      packages.H -- Package bootstrapping aid
** Author(s): kifer
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
** $Id: packaging.H,v 1.9 2011-04-15 16:05:00 dwarren Exp $
** 
*/


:- export bootstrap_package/2, bootstrap_syspackage/2,
	  ensure_package_bootstrapped/2,
	  bootstrap_sitepackage/2, bootstrap_userpackage/3,
	  unload_package/1.
:- export xsb_running_under/1, force_rename_file/2.

:- import (dynamic)/1, assert/1, retract/1
   from assert.

%% :- import abort/1 from standard.

:- import slash/1, expand_filename/2
   from machine.
:- import xsb_configuration/2 from xsb_configuration.

:- import fmt_write_string/3 from file_io.

:- import str_sub/2 from string.
:- import
        path_sysop/2,
        path_sysop/3
   from file_io.


