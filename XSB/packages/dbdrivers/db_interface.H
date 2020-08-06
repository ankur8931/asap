/*
**  File: db_interface.H
** Author: Saikat Mukherjee
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2002-2006
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
*/

/*
**  export/imports for the db_interface module
*/


:- import
	openConnection/6,
	queryConnection/4,
	closeConnection/1,
	prepareStatement/3,
	executePreparedStatement/3,
	closeStatement/1,
	exception/2,
	moreResults/1
   from driver_manager.
			

:- export
	db_connect/6,
	db_connect/5,
	db_disconnect/1,
	db_query/4,
	db_prepare/3,
	db_prepare_execute/3,
	db_statement_close/1.


