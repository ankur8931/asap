/* File:      flag_defs_xsb.h
** Author(s): Jiyang Xu, Kostis Sagonas, Ernie Johnson
** Contact:   xsb-contact@cs.sunysb.edu
**
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1999
** Copyright (C) ECRC, Germany, 1990
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
**
** $Id: flag_defs_xsb.h,v 1.52 2013-02-14 22:03:54 tswift Exp $
**
*/


/* -------- system flags --------------------------------------------------*/

/*	used by interpreter ->  Y/N ; read only -> R/W/N (no access)	   */

/* Thread private flags */

#define STACK_REALLOC    1        /* 1 = allow realloc of stacks on Oflow YW */
#define CURRENT_INPUT	 2	  /* current input file descriptor	NW */
#define CURRENT_OUTPUT	 3	  /* current output file descriptor	NW */
#define SYS_TIMER        4        /* XSB Timer	    	    	    	MK */
#define VERBOSENESS_LEVEL 5       /* how verbose debug messages should be    */
#define GARBAGE_COLLECT  6        /* type of garbage collection employed:    */
                                  /* 0 = none; 1 = sliding; 2 = copying.  NW */
#define CLAUSE_INT	 7	  /* for clause interrupt			YW */
#define BACKTRACE	 8	  /* for backtracing on error      Y/N       */
/* the following three flags are only in effect when
   the system is compiled with --enable-debug         --lfcastro           */
#define VERBOSE_GC       9        /* be verbose on garbage collection        */
#define COUNT_CHAINS    10        /* count size of chains on GC              */
#define EXAMINE_DATA    11        /* examine data on GC                      */
#define PROFFLAG        12
#define TABLING_METHOD	13      /* Default method used to evaluate tabled */
#define CLAUSE_GARBAGE_COLLECT 14 /* Turn clause gc on or off */
#define WRITE_ATTRIBUTES 15       /* Action to take when writing an attv     */

/* Flags 15-24 are open to use for thread-private flags*/

/*
 *  Flags 25-41 are reserved for Interrupt Handler PSCs.
 */
#define INT_HANDLERS_FLAGS_START   25  /* the first interrupt flag */

/* ----------------------------------------------------------------------------
   The following exist/are defined in sig_xsb.h:

 MYSIG_UNDEF      0    // _$load_undef
 MYSIG_KEYB       1    // _$keyboard_int
 MYSIG_SPY        3    // _$deb_spy
 MYSIG_TRACE      4    // _$deb_trace
 MYSIG_ATTV       8    // _$attv_int
 MYSIG_CLAUSE    16    // _$clause_int

These values are added to INT_HANDLERS_FLAGS_START to obtain the actual
interrupt flag
---------------------------------------------------------------------------- */
#define MAX_PRIVATE_FLAGS 42

/* Thread shared flags */

#define MAX_USAGE	42	/* 1 = keep max tablespace usage        YW */
#define CURRENT_MODULE	43	/* current module. USERMOD_PSC=usermod  YW */
#define MOD_LIST	44	/* the list of module (Psc) entries	YR */
#define BANNER_CTL      45      /* Controls whether banner, "loaded" msgs
				   are displayed; passed to Prolog side.
				   Check BANNER_CTL values at the end      */
#define CMD_LINE_GOAL  	46	/* The Prolog goal passed on cmd
				   line with -e	       	       	           */
#define USER_HOME  	47	/* $HOME, if not null. Else INSTALL_DIR    */
#define INSTALL_DIR	48	/* set dynamically in orient_xsb.c         */
#define CONFIG_FILE	49	/* Where xsb_configuration.P lives	   */
/* loader uses CONFIG_NAME flag before xsb_configuration is loaded */
#define CONFIG_NAME	50	/* this looks like this: cpu-vendor-os	   */
#define LETTER_VARS	51      /* For printing vars in the interpreter */
#define BOOT_MODULE     52      /* First file loaded; usually loader.P  */
#define CMD_LOOP_DRIVER 53      /* File that contains top-level command
				     loop driver */
#define NUM_THREADS     54      /* always 1 in the sequential system     NW*/
#define THREAD_RUN	55      /* PSC for the thread handler predicate  NN*/
#define STDERR_BUFFERED	56      /* Allow Cinterface to read Stderr back from buffer*/

/* The following flags may be made private in the future */

#define PIL_TRACE 	57	/* 0 = pil trace off, 1 = on		YW */
#define HITRACE		58	/* 0 = hitrace off, 1 = on		YW */
#define DEBUG_ON	59	/* 1 = debug on; 0 = off 		YW */
#define HIDE_STATE	60	/* 0 = no hide, >0 = hide level 	YW */
#define TRACE		61	/* 1 = trace on, 0 = trace off	    	YW */
#define INVOKE_NUM	62	/* debugger, the ordinal invoke number 	NW */
#define SKIPPING	63	/* debugger, 1 = skip, 0 = not	   	NW */
#define QUASI_SKIPPING	64	/* debugger, 1 = quasi skip, 0 = not	NW */

#define DCG_MODE        65      /* DGC mode: standard or xsb	           */

/* This flag is used by the loader to tell itself whether it should look into
   user-supplied library search paths or not. If 0, the loader will look only
   in lib/syslib/cmplib. If 1, the loader will look in library_directory/1
   before checking the standard places. */
#define LIBS_LOADED	  66

/* The following Oracle flags may be obsolete */
#define ORA_INPUTARRAY_LENGTH     67   /* max # simultaneous input tuples */
#define ORA_INPUTARRAY_WIDTH      68   /* max size of each input value    */
#define ORA_OUTPUTARRAY_LENGTH    69   /* max # simultaneous output tuples */

#define STRING_GARBAGE_COLLECT    70   /* Turn string gc on or off */
#define TABLE_GC_ACTION           71   /* Action for recl. of tables with cond answers */
#define THREAD_GLSIZE             72   /* Initial GLSize for created thread */
#define THREAD_TCPSIZE            73   /* Initial TCSize for created thread */
#define THREAD_COMPLSIZE          74   /* Initial COMPLSize for created thread */
#define THREAD_PDLSIZE            75   /* Initial PDLSize for created thread */
#define THREAD_DETACHED           76   /* Initial Detached flag for created thread */
#define MAX_THREAD_FLAG           77   /* Maximum number of threads (not changeable after startup) */
#define MAX_QUEUE_TERMS           78   /* Default Maximum number of terms in a message queue*/
#define RECOMPILE_OVERRIDE        79   /* Allows compilation when more than 1 thread */
#define PRIVSHAR_DEFAULT          80   /* Default for shared or private predicates */
#define WARNING_ACTION            81   /* Action to take on warnings: print,silent,exception */
#define HEAP_GC_MARGIN            82   /* Size of heap overflow margin */
#define ANSWER_COMPLETION         83   /* amp: Incremental Answer Completion switch */   
#define MAX_TABLE_SUBGOAL_SIZE    84   /* maximum size for terms in a tabled subgoal */
#define MAX_TABLE_SUBGOAL_ACTION  85   /* abort/fail/abstract (abstract not yet impld.)*/
#define MAX_ANSWERS_FOR_SUBGOAL   86
#define MAX_ANSWERS_FOR_SUBGOAL_ACTION   87
//#define MAX_TABLE_ANSWER_LIST_DEPTH    86   /* maximum depth for lists in a tabled answer */
//#define MAX_TABLE_ANSWER_LIST_ACTION   87   /* abort/warn/abstract (abstract not yet impld.)*/
#define MAXTOINDEX_FLAG           88   /* Experimental only */             
#define CTRACE_CALLS              89
#define EC_REMOVE_SCC             90
#define MAX_TABLE_ANSWER_METRIC    91   /* maximum depth for non-list terms in a tabled answer */
#define MAX_TABLE_ANSWER_ACTION   92   /* abort/warn/abstract (abstract not yet impld.)*/
#define UNIFY_WITH_OCCURS_CHECK_FLAG 93   /* make all unifications use occur_check, under development by DSW */
#define MAX_MEMORY                94
#define MEMORY_ERROR_FLAG         95
#define WRITE_DEPTH               96
#define EXCEPTION_PRE_ACTION      97
#define LOG_UNINDEXED		  98  /* 0 no logging; 1 log first time only; 2 log every time; 
					>2 is psc to print backtrace on unindexed access to that psc, every time. */ 
#define LOG_ALL_FILES_USED	  99  /* log all files at open */
#define EXCEPTION_ACTION         100
#define CHARACTER_SET            101
#define ERRORS_WITH_POSITION     102
#define SIMPLIFICATION_DONE	 103  /* set when simplification done during completion.*/
#define MAX_INCOMPLETE_SUBGOALS  104
#define MAX_INCOMPLETE_SUBGOALS_ACTION  105
#define MAX_SCC_SUBGOALS                106
#define MAX_SCC_SUBGOALS_ACTION         107
#define CYCLIC_CHECK_SIZE               108
#define MAX_TABLE_SUBGOAL_VAR_NUM       109
#define MAX_TABLE_ANSWER_VAR_NUM        110
#define MAX_MEMORY_ACTION               111

#define MAX_FLAGS		 120

#define MAXTOINDEX 20              /* maximum depth in term to go when using deep indexing; 
				      the actual depth is now specified by MAXTOINDEX_FLAG*/

#define SYSTEM_MEMORY_LIMIT        1
#define USER_MEMORY_LIMIT          2
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define PRINT_INCOMPLETE_ON_ABORT  1
#define UNDEFINED_TRUTH_VALUE  1

/* Banner control values recognized on the Prolog side.
   MUST BE PRIME NUMBERS */
#define NOBANNER          2   /* don't display XSB banner */
#define QUIETLOAD    	  3   /* don't display "module loaded" msgs */
#define NOPROMPT    	  5   /* display no prompt--useful in spawned
				 subprocesses */
#define NOFEEDBACK    	  7   /* display no feedback--useful in spawned
				 subprocesses */

#define DEFAULT_PRIVATE   0
#define DEFAULT_SHARING   1

#define PRINT_WARNING     0
#define SILENT_WARNING    1
#define ERROR_WARNING     2

/* for WRITE_ATTRIBUTES */
#define WA_PORTRAY           0
#define WA_DOTS              1
#define WA_IGNORE            2
#define WA_WRITE             3

/* for  MAX_TABLE_SUBGOAL_SIZE and call abstraction / restraint */
#define XSB_ABSTRACT         4
#define XSB_BRAT             5
#define XSB_SUSPEND          6
