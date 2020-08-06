/* File:      intern.P
** Author(s): Prasad Rao, Hasan Davulcu
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1998-2000
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
** $Id: storage.H,v 1.16 2010-08-19 15:03:38 spyrosh Exp $
** 
*/


:- export
	storage_commit/1,
	storage_reclaim_space/1,
	incr_storage_update/1,
	storage_find_fact/2,
	storage_find_fact/3,
	storage_find_keypair/3,
	incr_storage_find_fact/2,
	incr_storage_find_fact/3,
	incr_storage_find_keypair/3,
	storage_handle/4,
	%% this is for debugging/profiling
	storage_show_table_state/0.

:- export
	storage_insert_fact_bt/3, storage_delete_fact_bt/3, 
	storage_insert_fact_bt/5, storage_delete_fact_bt/5, 
	storage_insert_fact/3, storage_delete_fact/3,
	storage_insert_keypair_bt/4, storage_delete_keypair_bt/3,
	storage_insert_keypair_bt/6, storage_delete_keypair_bt/5,
	storage_insert_keypair/4, storage_delete_keypair/3,
	storage_delete_all/1.

:- export
	incr_storage_insert_fact_bt/3, incr_storage_delete_fact_bt/3, 
	incr_storage_insert_fact_bt/5, incr_storage_delete_fact_bt/5, 
	incr_storage_insert_fact/3, incr_storage_delete_fact/3,
	incr_storage_insert_keypair_bt/4, incr_storage_delete_keypair_bt/3,
	incr_storage_insert_keypair_bt/6, incr_storage_delete_keypair_bt/5,
	incr_storage_insert_keypair/4, incr_storage_delete_keypair/3,
	incr_storage_delete_all/1.

:- import
	unmark_uninterned_nr/2,
	delete_trie/1,
	trie_reclaim_uninterned_nr/1
   from intern.


:- import
	call/1,
	throw/1,
	atom/1
   from standard.
