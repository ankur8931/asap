/* File:      system_defs_xsb.h
** Author(s): kifer
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
** $Id: system_defs_xsb.h,v 1.11 2009/08/26 23:58:41 tswift Exp $
** 
*/

#define PLAIN_SYSTEM_CALL    	 0
#define SPAWN_PROCESS	         1
#define SHELL		         2
#define GET_PROCESS_TABLE    	 3
#define PROCESS_STATUS    	 4
#define PROCESS_CONTROL    	 5
#define SLEEP_FOR_SECS	      	 6

#define IS_PLAIN_FILE	     	 7
#define IS_DIRECTORY	     	 8

#define STAT_FILE_TIME           9
#define STAT_FILE_SIZE           10
#define EXEC                     11
#define GET_TMP_FILENAME         12
#define LIST_DIRECTORY           13
#define STATISTICS_2             14

// For statistics/2
#define RUNTIME 0
#define WALLTIME 1
#define TOTALMEMORY 2
#define GLMEMORY 3
#define TCMEMORY 4
#define TABLESPACE 5
#define TRIEASSERTMEM 6
#define HEAPMEM 7
#define TRAILMEM 8
#define CPMEM 9 
#define LOCALMEM 10
#define OPENTABLECOUNT 11
#define SHARED_TABLESPACE 12
#define ATOMMEM           13
#define IDG_COUNTS        14
#define TABLE_OPS         15

// For statistics/1
#define STAT_RESET 0
#define STAT_DEFAULT 1
#define STAT_TABLE 2
#define STAT_MUTEX 4
#define STAT_ATOM 8
