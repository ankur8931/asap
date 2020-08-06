/* File:      init_xsb.c
** Author(s): Warren, Swift, Xu, Sagonas, Johnson, Rao
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
** $Id: init_xsb.c,v 1.190 2013-04-17 22:02:35 tswift Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN_NT
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <share.h>
#include <fcntl.h>
#include <process.h>
#else
#include <unistd.h>	
#include <stddef.h>
#include <sys/wait.h>
#endif

#include "wind2unix.h"

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "inst_xsb.h"
#include "psc_xsb.h"
#include "hash_xsb.h"
#include "heap_xsb.h"
#include "memory_xsb.h"
#include "register.h"
#include "tries.h"
#include "choice.h"
#include "flags_xsb.h"
#include "loader_xsb.h"
#include "extensions_xsb.h"
#include "tab_structs.h"
#include "tr_utils.h"
#include "export.h"
#include "io_builtins_xsb.h"
#include "timer_defs_xsb.h"
#include "sig_xsb.h"
#include "thread_xsb.h"
#include "varstring_xsb.h"
#include "struct_manager.h"
#include "trie_internals.h"
#include "call_graph_xsb.h" /* for incremental evaluation*/
#include "incr_xsb.h" /* for incremental evaluation */
#include "deadlock.h"
#include "cinterf.h"
#include "storage_xsb.h"
#include "orient_xsb.h"
#include "token_defs_xsb.h"
#include "trace_xsb.h"
/*-----------------------------------------------------------------------*/   

/* Sizes of the Data Regions in K-byte blocks
   ------------------------------------------ */
#ifdef BITS64
#ifdef SHARED_COMPL_TABLES
#define PDL_DEFAULT_SIZE         (8*2)
#define GLSTACK_DEFAULT_SIZE    (96*2)
#define TCPSTACK_DEFAULT_SIZE   (96*2)
#define COMPLSTACK_DEFAULT_SIZE  (8*2)
#else /* SEQUENTIAL OR CONC_COMPL */
#define PDL_DEFAULT_SIZE         (64*2)
//#define GLSTACK_DEFAULT_SIZE    (K*2)
#define GLSTACK_DEFAULT_SIZE    (K*8)
#define TCPSTACK_DEFAULT_SIZE   (K*2)
#define COMPLSTACK_DEFAULT_SIZE  (64*2)
#endif /* SHARED_COMPL_TABLES */
#else /* 32 BIT */
#ifdef SHARED_COMPL_TABLES
#define PDL_DEFAULT_SIZE         8
#define GLSTACK_DEFAULT_SIZE    96
#define TCPSTACK_DEFAULT_SIZE   96
#define COMPLSTACK_DEFAULT_SIZE  8
#else /* SEQUENTIAL OR CONC_COMPL*/
#define PDL_DEFAULT_SIZE         64
#define GLSTACK_DEFAULT_SIZE    K
#define TCPSTACK_DEFAULT_SIZE   K
#define COMPLSTACK_DEFAULT_SIZE  64
#endif
#endif

#ifndef fileno				/* fileno may be a  macro */
extern int    fileno(FILE *f);	        /* this is defined in POSIX */
#endif
/* In WIN_NT, this gets redefined into _fdopen by wind2unix.h */
extern FILE *fdopen(int fildes, const char *type);

#if defined(GENERAL_TAGGING)
extern void extend_enc_dec_as_nec(void *,void *);
#endif

extern int max_interned_tries_glc;

UInteger pspacesize[NUM_CATS_SPACE] = {0};	/* actual space dynamically allocated by loader.c */

/* The SLG-WAM data regions
   ------------------------ */
#ifndef MULTI_THREAD
System_Stack
pdl = {NULL, NULL, 0,
       PDL_DEFAULT_SIZE},             /* PDL                   */
  glstack = {NULL, NULL, 0,
	     GLSTACK_DEFAULT_SIZE},     /* Global + Local Stacks */
    tcpstack = {NULL, NULL, 0,
		TCPSTACK_DEFAULT_SIZE},   /* Trail + CP Stack      */
      complstack = {NULL, NULL, 0,
		    COMPLSTACK_DEFAULT_SIZE};   /* Completion Stack  */
#endif

Exec_Mode xsb_mode;     /* How XSB is run: interp, disassem, user spec, etc. */

int xsb_profiling_enabled = 0;

/* from pathname_xsb.c */
DllExport extern char * call_conv strip_names_from_path(char*, int);

#ifdef BITS64
#define pad64bits(Loc)	{ *(Loc) += 4; }
#else
#define pad64bits(Loc)	{}
#endif

#define write_word(Buff,Loc,w) { *(CPtr)((pb)Buff + *(Loc)) = (Cell)(w); *(Loc) += 4; \
				pad64bits(Loc); }
#define write_byte(Buff,Loc,w) { *(pb)((pb)Buff + *(Loc)) = (byte)(w); *(Loc) += 1; }

Cell answer_return_inst;
Cell resume_compl_suspension_inst;
Cell resume_compl_suspension_inst2;
CPtr check_complete_inst;
CPtr call_answer_completion_inst_addr;
Cell hash_handle_inst;
Cell fail_inst;
Cell trie_fail_inst;
Cell dynfail_inst;
//Cell trie_fail_unlock_inst;
Cell halt_inst;
Cell proceed_inst;
Cell completed_trie_member_inst;
byte *check_interrupts_restore_insts_addr;

/* these three are from orient_xsb.c */
extern char *xsb_config_file_gl; /* configuration.P */
extern char *user_home_gl; /* the user HOME dir or install dir, if HOME is null */

/*==========================================================================*/

static void display_file(char *infile_name)
{
  FILE *infile;
  char buffer[MAXBUFSIZE];

  if ((infile = fopen(infile_name, "r")) == NULL) {
    xsb_initialization_exit("Can't open `%s'; XSB installation might be corrupted\n",
			     infile_name);
  }
  if (flags[LOG_ALL_FILES_USED]) {
    char current_dir[MAX_CMD_LEN];
    char *dummy; /* to squash warnings */
    dummy = getcwd(current_dir, MAX_CMD_LEN-1);
    SQUASH_LINUX_COMPILER_WARN(dummy) ; 
    xsb_log("%s: %s\n",current_dir,infile_name);
  }
  while (fgets(buffer, MAXBUFSIZE-1, infile) != NULL)
    fprintf(stdmsg, "%s", buffer);

  fclose(infile);
}

static void version_message(void)
{
  char licensemsg[MAXPATHLEN], configmsg[MAXPATHLEN];
  char *stripped_config_file;

  snprintf(licensemsg, MAXPATHLEN, "%s%cetc%ccopying.msg", install_dir_gl, SLASH, SLASH);
  stripped_config_file = strip_names_from_path(xsb_config_file_gl, 2);
  snprintf(configmsg, MAXPATHLEN, "%s%cbanner.msg", 
	  stripped_config_file, SLASH);

  display_file(configmsg);
  fprintf(stdmsg, "\n");
  display_file(licensemsg);

  if (xsb_mode != C_CALLING_XSB) exit(0);
}

static void help_message(void)
{
  char helpmsg[MAXPATHLEN];

  snprintf(helpmsg, MAXPATHLEN, "%s%cetc%chelp.msg", install_dir_gl, SLASH, SLASH);
  puts("");
  display_file(helpmsg);
  if (xsb_mode != C_CALLING_XSB) exit(0);
}

static void parameter_error(char * param) {

  if (xsb_mode == C_CALLING_XSB) {
    xsb_initialization_exit("cannot use option ''%s'' when calling XSB from C.\n",param);
  }
  else help_message();  // dont need to check for return here.
}

/*==========================================================================*/

/* Initialize System Flags
   ----------------------- */
static void init_flags(CTXTdecl)
{
  int i;
  for (i=0; i<MAX_FLAGS; i++) flags[i] = 0;
  for (i=0; i<MAX_PRIVATE_FLAGS; i++) pflags[i] = 0;
  pflags[SYS_TIMER]  = TIMEOUT_ERR; /* start with expired timer */
  flags[BANNER_CTL] = 1;           /* a product of prime numbers; each prime
				      determines which banner isn't shown */
  flags[NUM_THREADS] = 1;          /* 1 thread will be run at start */
  pflags[BACKTRACE] = 1;           /* Backtrace on error by default */
  pflags[CLAUSE_GARBAGE_COLLECT] = 1;           /* Clause GC on by default */
  flags[STRING_GARBAGE_COLLECT] = 1;           /* String GC on by default */

  flags[THREAD_PDLSIZE] = PDL_DEFAULT_SIZE;
  flags[THREAD_GLSIZE] = GLSTACK_DEFAULT_SIZE;
  flags[THREAD_TCPSIZE] = TCPSTACK_DEFAULT_SIZE;
  flags[THREAD_COMPLSIZE] = COMPLSTACK_DEFAULT_SIZE;

  flags[VERBOSENESS_LEVEL] = 0;

  /* Tripwires */
  flags[MAX_TABLE_SUBGOAL_SIZE] = MY_MAXINT;
  flags[MAX_TABLE_SUBGOAL_ACTION] = XSB_ERROR;
  flags[MAX_INCOMPLETE_SUBGOALS] = MY_MAXINT;
  flags[MAX_INCOMPLETE_SUBGOALS_ACTION] = XSB_ERROR;
  flags[MAX_SCC_SUBGOALS] = MY_MAXINT;
  flags[MAX_SCC_SUBGOALS_ACTION] = XSB_ERROR;
  flags[MAX_TABLE_ANSWER_METRIC] = MY_MAXINT;
  flags[MAX_TABLE_ANSWER_ACTION] = XSB_ERROR;
  flags[MAX_ANSWERS_FOR_SUBGOAL] = MY_MAXINT;
  flags[MAX_ANSWERS_FOR_SUBGOAL_ACTION] = XSB_ERROR;
  flags[MAX_TABLE_SUBGOAL_VAR_NUM] = 2000;
  flags[MAX_TABLE_ANSWER_VAR_NUM] = 20000;
  flags[MAX_MEMORY_ACTION] = XSB_ERROR;

  flags[CYCLIC_CHECK_SIZE] = 1000;
  flags[MAXTOINDEX_FLAG] = 5;
#ifdef MULTI_THREAD
  flags[MAX_QUEUE_TERMS] = DEFAULT_MQ_SIZE; 
  flags[HEAP_GC_MARGIN] = 8192 * ZOOM_FACTOR;
#else
  flags[HEAP_GC_MARGIN] = 32*K * ZOOM_FACTOR;
#endif
  flags[WRITE_DEPTH] = 64;
  //  flags[UNIFY_WITH_OCCURS_CHECK_FLAG] = 0;
#ifndef MULTI_THREAD
  // not (yet) tested with multi-threaded, so leave off if MT
  flags[ANSWER_COMPLETION] = 1;
#endif
}

/*==========================================================================*/

/* In MT engine, now providing a separate mutex (default type) for
   each io stream.  */

char    standard_input_glc[]      = "stdin";
char    standard_output_glc[]      = "stdout";
char    standard_error_glc[]      = "stderr";
char    standard_warning_glc[]      = "stdwarn";
char    standard_message_glc[]      = "stdmsg";
char    standard_debug_glc[]      = "stddbg";
char    standard_feedback_glc[]      = "stdfdbk";

static int init_open_files(void)
{
  int i, msg_fd, dbg_fd, warn_fd, fdbk_fd;

#ifdef MULTI_THREAD
  pthread_mutexattr_t attr_std ;
  pthread_mutexattr_init( &attr_std ) ;
#endif

  open_files[0].file_ptr = stdin;
  open_files[0].io_mode = 'r';
  open_files[0].stream_type = CONSOLE_STREAM;
  open_files[0].file_name = standard_input_glc;
  open_files[0].charset = (int)flags[CHARACTER_SET];

  open_files[1].file_ptr = stdout;
  open_files[1].io_mode = 'w';
  open_files[1].stream_type = CONSOLE_STREAM;
  open_files[1].file_name = standard_output_glc;
  open_files[1].charset = (int)flags[CHARACTER_SET];

  open_files[2].file_ptr = stderr;
  open_files[2].io_mode = 'w';
  open_files[2].stream_type = CONSOLE_STREAM;
  open_files[2].file_name = standard_error_glc;
  open_files[2].charset = (int)flags[CHARACTER_SET];

  /* stream for xsb warning msgs */
  if ((warn_fd = dup(fileno(stderr))) < 0)
    xsb_initialization_exit("Can't open the standard stream for warnings\n");
  stdwarn = fdopen(warn_fd, "w");
  open_files[3].file_ptr = stdwarn;
  open_files[3].io_mode = 'w';
  open_files[3].stream_type = CONSOLE_STREAM;
  open_files[3].file_name = standard_warning_glc;
  open_files[3].charset = (int)flags[CHARACTER_SET];

  /* stream for xsb normal msgs */
  if ((msg_fd = dup(fileno(stderr))) < 0)
     xsb_initialization_exit("Can't open the standard stream for messages\n");
  stdmsg = fdopen(msg_fd, "w");
  open_files[4].file_ptr = stdmsg;
  open_files[4].io_mode = 'w';
  open_files[4].stream_type = CONSOLE_STREAM;
  open_files[4].file_name = standard_message_glc;
  open_files[4].charset = (int)flags[CHARACTER_SET];

  /* stream for xsb debugging msgs */
  if ((dbg_fd = dup(fileno(stderr))) < 0)
     xsb_initialization_exit("Can't open the standard stream for debugging messages\n");
  stddbg = fdopen(dbg_fd, "w");
  open_files[5].file_ptr = stddbg;
  open_files[5].io_mode = 'w';
  open_files[5].stream_type = CONSOLE_STREAM;
  open_files[5].file_name = standard_debug_glc;
  open_files[5].charset = (int)flags[CHARACTER_SET];

  /* stream for xsb feedback msgs */
  if ((fdbk_fd = dup(fileno(stdout))) < 0)
     xsb_initialization_exit("Can't open the standard stream for XSB feedback messages\n");
  stdfdbk = fdopen(fdbk_fd, "w");
  open_files[6].file_ptr = stdfdbk;
  open_files[6].io_mode = 'w';
  open_files[6].stream_type = CONSOLE_STREAM;
  open_files[6].file_name = standard_feedback_glc;
  open_files[6].charset = (int)flags[CHARACTER_SET];

  /* NT doesn't seem to think that dup should preserve the buffering mode of
     the original file. So we make all new descriptors unbuffered -- dunno if
     this is good or bad. Line-buffering _IOLBF is the coarsest that can be
     allowed. Without the buffering NT users won't see anything on the
     screen. -mk */
  /* We should use setvbuf, but -no-cygwin doesn't seem to do the
     right thing with it, but it does with setbuf.... go figure. -dsw */

  setbuf(stdmsg, NULL);
  setbuf(stdwarn, NULL);
  setbuf(stddbg, NULL);
  setbuf(stdfdbk, NULL);
  setbuf(stderr, NULL);

  for (i=MIN_USR_OPEN_FILE; i < MAX_OPEN_FILES; i++) open_files[i].file_ptr = NULL;

#ifdef MULTI_THREAD
  if( pthread_mutexattr_settype( &attr_std, PTHREAD_MUTEX_RECURSIVE_NP )<0 )
    xsb_initialization_exit( "[THREAD] Error initializing mutexes" ) ;

  for( i = 0; i < MAX_OPEN_FILES ; i++ ) {
    pthread_mutex_init(OPENFILES_MUTEX(i) , &attr_std ) ;
    OPENFILES_MUTEX_OWNER(i) = -1;
  }
#endif
  return(0);
}

/*==========================================================================*/

/* if command line option is long --optionname, then the arg here is
   'optionname'. Process it and return.  (Dont worry -- init_flags has
   already been done)
*/
static int process_long_option(CTXTdeclc char *option,int *ctr,char *argv[],int argc)
{
  if (0==strcmp(option, "nobanner")) {
    flags[BANNER_CTL] *= NOBANNER;
  } else if (0==strcmp(option, "quietload")) {
    flags[BANNER_CTL] *= QUIETLOAD;
  } else if (0==strcmp(option, "noprompt")) {
    flags[BANNER_CTL] *= NOPROMPT;
  } else if (0==strcmp(option, "nofeedback")) {
    flags[BANNER_CTL] *= NOFEEDBACK;
  } else if (0==strcmp(option, "shared_predicates")) {
    flags[PRIVSHAR_DEFAULT] = DEFAULT_SHARING;
  } else if (0==strcmp(option, "help")) {
    help_message();
  } else if (0==strcmp(option, "version")) {
    version_message();
  } else if (0==strcmp(option, "max_threads")) {
    if ((int) (*ctr) < argc) {
      (*ctr)++;
#ifdef MULTI_THREAD
      sscanf(argv[*ctr], "%d", &max_threads_glc);
#endif
    }
    else xsb_warn(CTXTc "Missing size value for --max_threads");
  }  else if (0==strcmp(option, "max_tries")) {
    if ((int) (*ctr) < argc) {
      (*ctr)++;
      sscanf(argv[*ctr], "%d", &max_interned_tries_glc);
    }
    else xsb_warn(CTXTc "Missing size value for --max_tries");
  } else if (0==strcmp(option, "max_mqueues")) {
    if ((int) (*ctr) < argc) {
      (*ctr)++;
#ifdef MULTI_THREAD
      sscanf(argv[*ctr], "%d", &max_mqueues_glc);
#endif
    }
    else xsb_warn(CTXTc "Missing size value for --max_mqueues");
  } else if (!strcmp(option,"max_subgoal_size")) {
    if ((int) (*ctr) < argc) {
      (*ctr)++;
      if (sscanf(argv[*ctr], "%d", (int*) &flags[MAX_TABLE_SUBGOAL_SIZE]) < 1)
	xsb_warn(CTXTc "Invalid size value for --max_subgoal_size");
    }
    else xsb_warn(CTXTc "Missing size value for --max_subgoal_size");
  }
  else if (!strcmp(option,"max_subgoal_action")) {
    char action;
    if ((int) (*ctr) < argc) {
      (*ctr)++;
      sscanf(argv[*ctr], "%c", &action);
      switch (action) {
      case 'a':  {
	flags[MAX_TABLE_SUBGOAL_ACTION] = XSB_ABSTRACT;
	break;
      }
      case 'e':  {
	flags[MAX_TABLE_SUBGOAL_ACTION] = XSB_ERROR;
	break;
      }
      case 'f':  {
	flags[MAX_TABLE_SUBGOAL_ACTION] = XSB_FAILURE;
	break;
      }
      default: xsb_warn(CTXTc "Invalid action for (%c) for --max_subgoal_action "
			"values can be (a)bstract (e)rror or (f)ail",action);
      }
    }
    else xsb_warn(CTXTc "Missing depth value for --max_subgoal_depth");
  } 
  else xsb_warn(CTXTc "Unknown option --%s",option);

  return(0);
}


/*==========================================================================*/
/* Currently done on process startup before init_para(). Do not use elsewhere, 
   to avoid problems with multi-threading. */

FILE *stream_err, *stream_out; 

void perform_IO_Redirect(CTXTdeclc int argc, char *argv[])
{
    int i;

    init_flags(CTXT);	// We set one of them
    /*
	    This needs to be done early so that embedded applications can catch meaningful 
	    initialization failures in the log files
    */

    for (i=1; i<argc; i++)
	{ /* check to see if should redirect output */
	    if (!strcmp(argv[i],"-q"))
		{
#ifdef WIN_NT
            fclose(stderr);
            fclose(stdout);
            stream_err = _fsopen("XSB_errlog",  "w+", _SH_DENYNO);
            stream_out = _fsopen("XSB_outlog",  "w", _SH_DENYNO);
            *stderr = *stream_err;
            *stdout = *stream_out;
#else
		    stream_err = freopen("XSB_errlog", "w+", stderr);
		    stream_out = freopen("XSB_outlog", "w", stdout);
#endif
            flags[STDERR_BUFFERED] = 1;
            break;
		}
	}
}


FILE * input_read_stream = NULL;
FILE * input_write_stream = NULL;

int pipe_input_stream() {
  /* create a pipe for the input. Pass XSB the read-end of this pipe, and
     place the write-end into stream_input_write  */
  int fileDescriptors[2] = {0,0};
#ifdef WIN_NT
  if (_pipe(fileDescriptors, 256, _O_TEXT) == 0) { 
#else
  if (pipe(fileDescriptors) == 0) {
#endif
    fclose(stdin);
    input_read_stream = fdopen(fileDescriptors[0], "r");
    *stdin = *input_read_stream;
    
    input_write_stream = fdopen(fileDescriptors[1], "w");
    return 0;
  }
  return 1;
}

static size_t get_memarea_size( char *s )
{
	size_t size ;
	char *endptr ;

	size = strtol( s, &endptr, 0 );

	if( size <= 0 )
		xsb_abort( "invalid size for memory area" );

        /* note : the sizes of the memory areas of XSB are kept in KiloBytes */

	switch( *endptr ) {
		case 0:
		case 'k':
		case 'K':
			return size ;
		case 'm':
		case 'M':
			return size * K ;
		case 'g':
		case 'G':
			return size * K * K ;
		default:
			xsb_abort("invalid unit in memory size parameter");
	}
	return 0; /* keep compiler happy */
}

/*==========================================================================*/
/* Initialize System Parameters: This is done only on process start
** up, not on thread startup. 
*/
 
char *init_para(CTXTdeclc int flag, int argc, char *argv[]) {
  int i;
  char warning[80];
  /* Boot module is usually the loader that loads the Prolog code of XSB.
  ** Or it can be a code to disassemble.
  ** Cmd loop driver is usually the XSB interpreter (x_interp.P).
  ** However, it can be any program that communicates with XSB and drives its
  ** comand loop.
  */
  char *boot_module, *cmd_loop_driver;
  char cmd_line_goal[MAXBUFSIZE+1] = "";
  size_t  strlen_instdir, strlen_initfile, strlen_2ndfile;

#ifdef SHARED_COMPL_TABLES
  num_deadlocks = 0;
#endif

  /* init_open_files needs this flag set. */

#ifdef WIN_NT
  flags[CHARACTER_SET] = CP1252;  //LATIN_1;
#else
  flags[CHARACTER_SET] = UTF_8;
#endif
  
  init_open_files();

  init_statistics();

  max_interned_tries_glc = MAX_INTERNED_TRIES; 
#ifdef MULTI_THREAD
  max_threads_glc = MAX_THREADS; 
  max_mqueues_glc = MAX_MQUEUES; 
#else 
  max_threads_glc = 1;   // max number of first-class XSB threads
  max_mqueues_glc = 1; 
#endif

  pflags[STACK_REALLOC] = TRUE;
#ifdef GC
  pflags[GARBAGE_COLLECT] = INDIRECTION_SLIDE_GC;
#else
  pflags[GARBAGE_COLLECT] = NO_GC;
#endif
  flags[DCG_MODE] = XSB_STYLE_DCG;

  /* Set default Prolog files. 
     File extension XSB_OBJ_EXTENSION_STRING added later. */
#ifdef WIN_NT
  boot_module = "\\syslib\\loader";
#else
  boot_module = "/syslib/loader";
#endif

  /* File extensions are automatically added for Loader-loaded files. */
  if( xsb_mode == C_CALLING_XSB) {
#ifdef WIN_NT
      cmd_loop_driver = "\\syslib\\xcallxsb";
#else
      cmd_loop_driver = "/syslib/xcallxsb";
#endif
  }
  else { /* xsb_mode is now DEFAULT, but may later be changed */
#ifdef WIN_NT
    cmd_loop_driver = "\\syslib\\x_interp";
#else
    cmd_loop_driver = "/syslib/x_interp";
#endif
  }

  pflags[TABLING_METHOD] = VARIANT_EVAL_METHOD;

  flags[ERRORS_WITH_POSITION] = 0;

  /* Modify Parameters Using Command Line Options
     -------------------------------------------- */
  for (i=1; i<argc; i++) {
    if (*argv[i] != '-') {        /* command-line module specified */
      if (xsb_mode != DEFAULT) {
	parameter_error(argv[i]);
	break;
      }
      xsb_mode = CUSTOM_CMD_LOOP_DRIVER;
      cmd_loop_driver = argv[i];
      continue;
    }

    /* Otherwise, get command-line switch (and arg).
       Will dump core if the accompanying argument is omitted. */
    switch((argv[i][1])) {
    case 'r':
      pflags[STACK_REALLOC] = FALSE;
      break;
    case 'g':
      i++;
#ifdef GC
      if (i < argc) {
	if (strcmp(argv[i],"sliding")==0)
	  pflags[GARBAGE_COLLECT] = SLIDING_GC;
	else
	if (strcmp(argv[i],"copying")==0)
	  pflags[GARBAGE_COLLECT] = COPYING_GC;
	else
        if (strcmp(argv[i],"indirection")==0)
          pflags[GARBAGE_COLLECT] = INDIRECTION_SLIDE_GC;
        else
	if (strcmp(argv[i],"none")==0)
	  pflags[GARBAGE_COLLECT] = NO_GC;
	else
	xsb_warn(CTXTc "Unrecognized garbage collection type");
      } else
        xsb_warn(CTXTc "Missing garbage collection type");
#else
      xsb_warn(CTXTc "-g option does not make sense in this XSB configuration");
#endif
      break;
    case 'u':
      if (argv[i][2] != '\0')
#ifndef MULTI_THREAD
	pdl.init_size = get_memarea_size( argv[i]+2 ) ;
#else
	flags[THREAD_PDLSIZE] = get_memarea_size( argv[i]+2 ) ;
#endif
      else {
	i++;
	if (i < argc)
#ifndef MULTI_THREAD
	  pdl.init_size = get_memarea_size( argv[i] ) ;
#else
	  flags[THREAD_PDLSIZE] = get_memarea_size( argv[i] ) ;
#endif
	else
	  xsb_warn(CTXTc "Missing size value for -u");
      }
      break;
    case 'm':
      if (argv[i][2] != '\0')
#ifndef MULTI_THREAD
	glstack.init_size = get_memarea_size( argv[i]+2 ) ;
#else
	flags[THREAD_GLSIZE] = get_memarea_size( argv[i]+2 ) ;
#endif
      else {
	i++;
	if (i < argc)
#ifndef MULTI_THREAD
	  glstack.init_size = get_memarea_size( argv[i] ) ;
#else
	  flags[THREAD_GLSIZE] = get_memarea_size( argv[i] ) ;
#endif
	else
	  xsb_warn(CTXTc "Missing size value for -m");
      }
      break;
    case 'c':
      if (argv[i][2] != '\0')
#ifndef MULTI_THREAD
	tcpstack.init_size = get_memarea_size( argv[i]+2 ) ;
#else
	flags[THREAD_TCPSIZE] = get_memarea_size( argv[i]+2 ) ;
#endif
      else {
	i++;
	if (i < argc)
#ifndef MULTI_THREAD
	  tcpstack.init_size = get_memarea_size( argv[i] ) ;
#else
	  flags[THREAD_TCPSIZE] = get_memarea_size( argv[i] ) ;
#endif
	else
	  xsb_warn(CTXTc "Missing size value for -c");
      }
      break;
    case 'o':
      if (argv[i][2] != '\0')
#ifndef MULTI_THREAD
	complstack.init_size = get_memarea_size( argv[i]+2 ) ;
#else
	flags[THREAD_COMPLSIZE] = get_memarea_size( argv[i]+2 ) ;
#endif
      else {
	i++;
	if (i < argc)
#ifndef MULTI_THREAD
	  complstack.init_size = get_memarea_size( argv[i] ) ;
#else
	  flags[THREAD_COMPLSIZE] = get_memarea_size( argv[i] ) ;
#endif
	else
	  xsb_warn(CTXTc "Missing size value for -o");
      }
      break;
    case 'S':
      pflags[TABLING_METHOD] = SUBSUMPTIVE_EVAL_METHOD;
      break;
    case 'd':
      if ( (xsb_mode != DEFAULT) && (xsb_mode != CUSTOM_BOOT_MODULE) ) 
	parameter_error("d");
      xsb_mode = DISASSEMBLE;
      break;
    case 'T': 
      flags[HITRACE] = 1;
      asynint_val |= MSGINT_MARK; 
      break;
    case 't': 
#ifdef DEBUG_VM
      flags[PIL_TRACE] = 1;
      flags[HITRACE] = 1;
      asynint_val |= MSGINT_MARK;
#else
      xsb_initialization_exit("-t option unavailable for this executable (non-debug mode)");
#endif
      break;
    case 'i':
      if (xsb_mode != DEFAULT)
	parameter_error("i");
      xsb_mode = INTERPRETER;
      break;
    case 'l':
      flags[LETTER_VARS] = 1;
      break;
    case 'n':  /* TLS: obsolete, but leaving in for backward compat */
      if (xsb_mode != C_CALLING_XSB)
	help_message();
      xsb_mode = C_CALLING_XSB;
#ifdef WIN_NT
      cmd_loop_driver = "\\syslib\\xcallxsb";
#else
      cmd_loop_driver = "/syslib/xcallxsb";
#endif
      break;
    case 'B':
      if (xsb_mode == DEFAULT)
	xsb_mode = CUSTOM_BOOT_MODULE;
      else if (xsb_mode != DISASSEMBLE)   /* retain disassemble command for */
	parameter_error("B"); /* -d -f <file> AWA -f <file> -d */
      if (argv[i][2] != '\0')
	boot_module = argv[i]+2;
      else {
	i++;
	if (i < argc)
	   boot_module = argv[i];
	 else
	   xsb_warn(CTXTc "Missing boot module's file name");
      }
      break;
    case 'D':
      if (xsb_mode == DEFAULT)
	xsb_mode = CUSTOM_CMD_LOOP_DRIVER;
      else if (xsb_mode != CUSTOM_BOOT_MODULE)
	parameter_error("D"); 
      if (argv[i][2] != '\0')
	cmd_loop_driver = argv[i]+2;
      else {
	i++;
	if (i < argc)
	   cmd_loop_driver = argv[i];
	 else
	   xsb_warn(CTXTc "Missing top-level command loop driver's file name");
      }
      break;
    case 'e': {
      char *tmp_goal=NULL;
      if (argv[i][2] != '\0')
	tmp_goal = argv[i]+2;
      else {
	i++;
	if (i < argc)
	   tmp_goal = argv[i];
	 else
	   xsb_warn(CTXTc "Missing command line goal");
      }

      if (strchr(tmp_goal, '.') == NULL) {
	xsb_initialization_exit("\n\nTerminating `.' missing in command line goal:\n\t`%s'",
		 tmp_goal);
      }

      if ((strlen(cmd_line_goal) + strlen(tmp_goal)) >= MAXBUFSIZE)
	xsb_initialization_exit("\n\nCommand line goal is too long (> %d)\n\n", MAXBUFSIZE);
      strcat(cmd_line_goal, " ");
      strcat(cmd_line_goal, tmp_goal);
      break;
    }
    case 'h':
      help_message();
      break;
    case 'v':
      version_message();
      break;
    case '-':
      if (0==strcmp(argv[i]+2, "ignore")) {
	/* long options of the form --ignore */
	i = argc;
      } else
	/* long options of the form --optionname */
      process_long_option(CTXTc argv[i]+2,&i,argv,argc);
      break;
    case 'p':
      xsb_profiling_enabled = 1;
      break;
    case 'q':
      break;
    default:
      sprintf(warning, "Unknown command line option %s", argv[i]);
      xsb_warn(CTXTc warning);
    } /* switch */
  } /* for */
  /* Done with command line arguments */

  /* This is where we will be looking for the .xsb directory */
  flags[USER_HOME] = (Cell) mem_alloc(strlen(user_home_gl) + 1,OTHER_SPACE);
  strcpy( (char *)flags[USER_HOME], user_home_gl );

  /* install_dir is computed dynamically at system startup (in orient_xsb.c).
     Therefore, the entire directory tree can be moved --- only the relative
     positions count.

     Initializing these flags could probably be done in init_flags --
     which would be cleaner.  However, this would mean rearranging
     main_xsb.c
  */ 
  flags[INSTALL_DIR] = (Cell) mem_alloc(strlen(install_dir_gl) + 1,OTHER_SPACE);   
  strcpy( (char *)flags[INSTALL_DIR], install_dir_gl );

  /* loader uses CONFIG_NAME flag before xsb_configuration is loaded */
  flags[CONFIG_NAME] = (Cell) mem_alloc(strlen(CONFIGURATION) + 1,OTHER_SPACE);
  strcpy( (char *)flags[CONFIG_NAME], CONFIGURATION );

  flags[CONFIG_FILE] = (Cell) mem_alloc(strlen(xsb_config_file_gl) + 1,OTHER_SPACE);
  strcpy( (char *)flags[CONFIG_FILE], xsb_config_file_gl );

  /* the default for cmd_line_goal goal is "" */
  flags[CMD_LINE_GOAL] = (Cell) mem_alloc(strlen(cmd_line_goal) + 1,OTHER_SPACE);
  strcpy( (char *)flags[CMD_LINE_GOAL], cmd_line_goal );
 
#ifdef MULTI_THREAD 
  flags[MAX_THREAD_FLAG] = max_threads_glc;
#else
  flags[MAX_THREAD_FLAG] = 1;
#endif

  /* Set the Prolog startup files.
     ----------------------------- */
  /* Default execution mode is to load and run the interpreter. */
  if (xsb_mode == DEFAULT)
    xsb_mode = INTERPRETER;

  strlen_instdir = strlen(install_dir_gl);
  strlen_initfile = strlen(boot_module)+XSB_OBJ_EXTENSION_LENGTH;
  strlen_2ndfile = strlen(cmd_loop_driver);

  switch(xsb_mode) {
  case INTERPRETER:
  case C_CALLING_XSB:
    /*
     *  A "short-cut" option in which the loader is the loader file and
     *  an XSB-supplied "server" program is the interpreter file.  Since
     *  it is known where these files exist, the full paths are built.
     */
    flags[BOOT_MODULE] = (Cell) mem_alloc(strlen_instdir + strlen_initfile + 2,OTHER_SPACE);
    flags[CMD_LOOP_DRIVER] = (Cell)mem_alloc(strlen_instdir + strlen_2ndfile + 2,OTHER_SPACE);
    sprintf( (char *)flags[BOOT_MODULE],
	     "%s%s%s",
	     install_dir_gl, boot_module, XSB_OBJ_EXTENSION_STRING );
    sprintf( (char *)flags[CMD_LOOP_DRIVER],
	     "%s%s",
	     install_dir_gl, cmd_loop_driver );
    break;
  case CUSTOM_BOOT_MODULE:
    /*
     *  The user has specified a private loader to be used instead of the
     *  standard one and possibly a top-level command loop driver as well.  In
     *  either case, we can 
     *  make no assumptions as to where these files exist, and so the 
     *  user must supply an adequate full path name in each case (including
     *  extension).
     */
    flags[BOOT_MODULE] = (Cell) mem_alloc(strlen_initfile + 1,OTHER_SPACE);
    flags[CMD_LOOP_DRIVER ] = (Cell) mem_alloc(strlen_2ndfile + 1,OTHER_SPACE);
    strcpy( (char *)flags[BOOT_MODULE], boot_module );
    strcpy( (char *)flags[CMD_LOOP_DRIVER], cmd_loop_driver );
    break;
  case CUSTOM_CMD_LOOP_DRIVER:
    /*
     *  The user has specified a private top-level command loop.
     *  The filename can be absolute; however if not, it will
     *  be looked for in XSB's library path.
     */
    flags[BOOT_MODULE] = (Cell) mem_alloc(strlen_instdir + strlen_initfile + 1,OTHER_SPACE);
    flags[CMD_LOOP_DRIVER ] = (Cell) mem_alloc(strlen_2ndfile + 1,OTHER_SPACE);
    sprintf( (char *)flags[BOOT_MODULE],
	     "%s%s%s",
	     install_dir_gl, boot_module, XSB_OBJ_EXTENSION_STRING );
    strcpy( (char *)flags[CMD_LOOP_DRIVER ], cmd_loop_driver );
    break;
  case DISASSEMBLE:
    /*
     *  A loader file should have been specified for disassembling.
     *  Should include extension and all.
     */
    flags[BOOT_MODULE] = (Cell) mem_alloc(strlen_initfile + 1,OTHER_SPACE);
    strcpy( (char *)flags[BOOT_MODULE], boot_module );
    break;
  default:
    xsb_initialization_exit("Setting startup files: Bad XSB mode!");
    break;
  }

  /* Other basic initializations
     --------------------------- */
  /* Multi Threaded Data Structure Initializations */
#ifdef MULTI_THREAD
  init_system_mutexes() ;
  init_system_threads(th) ;
#endif

  //#ifndef MULTI_THREAD
  forest_log_buffer_1 = &fl_buffer_1;
  forest_log_buffer_2 = &fl_buffer_2;
  forest_log_buffer_3 = &fl_buffer_3;

  forest_log_buffer_1->fl_buffer = (char *) mem_alloc(MAXTERMBUFSIZE, BUFF_SPACE);
  //  forest_log_buffer_1->fl_size = MAXTERMBUFSIZE;
  forest_log_buffer_1->fl_size = MAXTERMBUFSIZE;
  forest_log_buffer_2->fl_buffer = (char *) mem_alloc(MAXTERMBUFSIZE, BUFF_SPACE);
  forest_log_buffer_2->fl_size = MAXTERMBUFSIZE;
  forest_log_buffer_3->fl_buffer = (char *) mem_alloc(MAXTERMBUFSIZE, BUFF_SPACE);
  forest_log_buffer_3->fl_size = MAXTERMBUFSIZE;
  //#endif
  return ( (char *) flags[BOOT_MODULE] );

} /* init_para() */

/*==========================================================================*/
#ifdef MULTI_THREAD

/* To be called each time a thread is created: initializes
 * thread-private memory areas that are cleaned up in
 * cleanup_thread_structures() */

void init_thread_structures(CTXTdecl)
{

  asynint_code = 0;
  asynint_val = 0;

  pdl.low = NULL; pdl.high = NULL; pdl.size = 0, pdl.init_size = (size_t) flags[THREAD_PDLSIZE];
  tcpstack.low = NULL; tcpstack.high = NULL; tcpstack.size = 0, tcpstack.init_size = (size_t) flags[THREAD_TCPSIZE];
  glstack.low = NULL; glstack.high = NULL; glstack.size = 0, glstack.init_size = (size_t) flags[THREAD_GLSIZE];
  complstack.low = NULL; complstack.high = NULL; complstack.size = 0, complstack.init_size = (size_t) flags[THREAD_COMPLSIZE];

  findall_solutions = NULL;

#define MAXSBUFFS 30
  LSBuff = (VarString **)mem_calloc(sizeof(VarString *),MAXSBUFFS,OTHER_SPACE);

  /* vars for io_builtins_XXX */
  opstk_size = 0;
  funstk_size = 0;
  funstk = NULL;
  opstk = NULL;
  rc_vars = (struct vartype *)mem_alloc(MAXVAR*sizeof(struct vartype),OTHER_SPACE);

  /* vars for token_xsb_XXX */
  token = (struct xsb_token_t *)mem_alloc(sizeof(struct xsb_token_t),OTHER_SPACE);
  strbuff = NULL;
  lastc = ' ';
  strbuff_len = InitStrLen;

  random_seeds = 0;

  /* used in trie_lookup */
  a_tstCCPStack = (struct tstCCPStack_t *)mem_alloc(sizeof(struct tstCCPStack_t),OTHER_SPACE);
  a_variant_cont = (struct VariantContinuation *)mem_alloc(sizeof(struct VariantContinuation),OTHER_SPACE);
  a_tstCPStack = (struct tstCPStack_t *)mem_alloc(sizeof(struct tstCPStack_t),OTHER_SPACE);

  asrtBuff = (struct asrtBuff_t *)mem_alloc(sizeof(struct asrtBuff_t),OTHER_SPACE);
  asrtBuff->Buff = NULL;
  asrtBuff->Buff_size = 512;
  asrtBuff->Loc = NULL;
  asrtBuff->BLim = 0;
  asrtBuff->Size = 0;
  i_have_dyn_mutex = 0;

  last_answer = (VarString *)mem_alloc(sizeof(VarString),OTHER_SPACE);
  XSB_StrInit(last_answer);
  OldestCl = retracted_buffer;
  NewestCl = retracted_buffer;

  /* call_intercept = init_call_intercept ; */

  private_tif_list.first = NULL;
  private_tif_list.last = NULL;
  private_deltf_chain_begin = NULL;
  private_delcf_chain_begin = NULL;

  /* stuff for avoiding recursion in simplification */
  simplify_neg_fails_stack_top = 0;
  in_simplify_neg_fails = 0;

  /* Stuff for abolishing tables */
  trans_abol_answer_stack_top = 0;
  trans_abol_answer_stack = NULL;
  trans_abol_answer_stack_size = 0;

  ta_done_subgoal_stack_top = 0;
  ta_done_subgoal_stack = NULL;
  ta_done_subgoal_stack_size = 0;

  done_tif_stack_top = 0;
  done_tif_stack = NULL;
  done_tif_stack_size = 0;

  cycle_trail = 0;
  cycle_trail_size = 0;
  cycle_trail_top = -1;

  /******** Initialize Private structure managers ********/

  private_smTableBTN  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smTableBTN,BasicTrieNode, BTNs_PER_BLOCK,
		  "Basic Trie Node (Private)");

  private_smTableBTHT  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smTableBTHT,BasicTrieHT, BTHTs_PER_BLOCK,
		  "Basic Trie Hash Table (Private)");

  private_smTableBTHTArray  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  BuffM_InitDeclDyna(private_smTableBTHTArray,(TrieHT_INIT_SIZE*(sizeof(Cell))), BTHTs_PER_BLOCK,
		  "Basic Trie Hash Table Array(Private)");

  private_smTSTN = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smTSTN,TS_TrieNode, TSTNs_PER_BLOCK,
		  "Time-Stamped Trie Node (Private)");

  private_smTSTHT  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smTSTHT,TST_HashTable, TSTHTs_PER_BLOCK,
		    "Time-Stamped Trie Hash Table (Private)");

  private_smTSIN  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smTSIN,TS_IndexNode, TSINs_PER_BLOCK,
			    "Time-Stamp Indexing Node (Private)");

  private_smVarSF  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smVarSF,variant_subgoal_frame,
		  SUBGOAL_FRAMES_PER_BLOCK,"Variant Subgoal Frame (Private)");

  private_smProdSF  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smProdSF,subsumptive_producer_sf,
		  SUBGOAL_FRAMES_PER_BLOCK,
		  "Subsumptive Producer Subgoal Frame (Private)");

  private_smConsSF  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smConsSF,subsumptive_consumer_sf,
		  SUBGOAL_FRAMES_PER_BLOCK,
		  "Subsumptive Consumer Subgoal Frame (Private)");

  private_smALN  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smALN,AnsListNode, ALNs_PER_BLOCK,
		  "Answer List Node (Private)");

  private_smASI  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smASI,ASI_Node, ASIs_PER_BLOCK,
		  "Answer Substitution Info (Private)");

  private_smDelCF  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smDelCF, DeletedClauseFrame, DELCFs_PER_BLOCK,
		  "Deleted Clause Frames (Private)");

  private_smAssertBTN  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smAssertBTN,BasicTrieNode, BTNs_PER_BLOCK,
		  "Basic Trie Node (Asserted, Private)");

  private_smAssertBTHT  = 
    (struct Structure_Manager*) mem_alloc(sizeof(struct Structure_Manager),
					  MT_PRIVATE_SPACE);
  SM_InitDeclDyna(private_smAssertBTHT,BasicTrieHT, BTHTs_PER_BLOCK,
		     "Basic Trie Hash Table (Asserted Private)");

  private_current_de_block = NULL;
  private_current_dl_block = NULL;
  private_current_pnde_block = NULL;

  private_released_des  = NULL;
  private_released_dls   = NULL; 
  private_released_pndes  = NULL;

  private_next_free_de = NULL;
  private_next_free_dl = NULL;
  private_next_free_pnde = NULL; 

  private_current_de_block_top = NULL; 
  private_current_dl_block_top = NULL;
  private_current_pnde_block_top = NULL;

  bt_storage_hash_table.length = STORAGE_TBL_SIZE;
  bt_storage_hash_table.bucket_size = sizeof(STORAGE_HANDLE);
  bt_storage_hash_table.initted = FALSE;
  bt_storage_hash_table.table = NULL;

  num_gc = 0;
  total_time_gc = 0;
  total_collected = 0;

  token_too_long_warning = 1;
  IGRhead = NULL;

  {int i;
    for (i=0; i<MAX_BIND_VALS; i++) {
      term_string[i] = NULL;
    }
  }
  
  callAbsStk_index = 0;
  callAbsStk_size    = 0;

  /***************/

/* This is here just for the first thread - others initialize its xsb tid
   on xsb_thread_run - the first thread has always tid = 0 */
  th->tid = 0 ;

#ifdef SHARED_COMPL_TABLES
  th->waiting_for_tid = -1 ;
  th->is_deadlock_leader = FALSE ;
#endif
#ifdef CONC_COMPL
  th->completing = FALSE;
  th->last_ans = 1;
  pthread_cond_init( &th->cond_var, NULL );
#endif
  th->cond_var_ptr = NULL;
}

void cleanup_thread_structures(CTXTdecl)
{
  free(glstack.low) ;
  free(tcpstack.low) ;
  free(complstack.low) ;
  free(pdl.low) ;

  /* these are allocated in init_thread_structures() */
  mem_dealloc(LSBuff,sizeof(VarString *)*MAXSBUFFS,OTHER_SPACE);
  mem_dealloc(rc_vars,MAXVAR*sizeof(struct vartype),OTHER_SPACE);
  mem_dealloc(token,sizeof(struct xsb_token_t),OTHER_SPACE); 
  mem_dealloc(a_tstCCPStack,sizeof(struct tstCCPStack_t),OTHER_SPACE);
  mem_dealloc(a_variant_cont,sizeof(struct VariantContinuation),OTHER_SPACE);
  mem_dealloc(a_tstCPStack,sizeof(struct tstCPStack_t),OTHER_SPACE);
  mem_dealloc(asrtBuff,sizeof(struct asrtBuff_t),OTHER_SPACE);
  mem_dealloc(last_answer,sizeof(VarString),OTHER_SPACE);

  XSB_StrDestroy(tsgLBuff1);
  XSB_StrDestroy(tsgLBuff2);
  XSB_StrDestroy(tsgSBuff1);
  XSB_StrDestroy(tsgSBuff2);

  mem_dealloc(tsgLBuff1,sizeof(VarString),OTHER_SPACE);
  mem_dealloc(tsgLBuff2,sizeof(VarString),OTHER_SPACE);
  mem_dealloc(tsgSBuff1,sizeof(VarString),OTHER_SPACE);
  mem_dealloc(tsgSBuff2,sizeof(VarString),OTHER_SPACE);

  free_trie_aux_areas(CTXT) ;

  pthread_mutex_destroy(&private_smAssertBTN->sm_lock);
  mem_dealloc(private_smAssertBTN,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smAssertBTHT->sm_lock);
  mem_dealloc(private_smAssertBTHT,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smTableBTN->sm_lock);
  mem_dealloc(private_smTableBTN,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smTableBTHT->sm_lock);
  mem_dealloc(private_smTableBTHT,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smTSTN->sm_lock);
  mem_dealloc(private_smTSTN,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smTSTHT->sm_lock);
  mem_dealloc(private_smTSTHT,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smTSIN->sm_lock);
  mem_dealloc(private_smTSIN,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE); 
  pthread_mutex_destroy(&private_smVarSF->sm_lock);
  mem_dealloc(private_smVarSF,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE);
  pthread_mutex_destroy(&private_smProdSF->sm_lock);
  mem_dealloc(private_smProdSF,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE); 
  pthread_mutex_destroy(&private_smConsSF->sm_lock);
  mem_dealloc(private_smConsSF,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE); 
  pthread_mutex_destroy(&private_smALN->sm_lock);
  mem_dealloc(private_smALN,sizeof(struct Structure_Manager),
	      MT_PRIVATE_SPACE); 
  forest_log_buffer_1 = &fl_buffer_1;
  forest_log_buffer_2 = &fl_buffer_2;
  forest_log_buffer_3 = &fl_buffer_3;

  forest_log_buffer_1->fl_buffer = (char *) mem_alloc(MAXTERMBUFSIZE, BUFF_SPACE);
  //  forest_log_buffer_1->fl_size = MAXTERMBUFSIZE;
  forest_log_buffer_1->fl_size = MAXTERMBUFSIZE;
  forest_log_buffer_2->fl_buffer = (char *) mem_alloc(MAXTERMBUFSIZE, BUFF_SPACE);
  forest_log_buffer_2->fl_size = MAXTERMBUFSIZE;
  forest_log_buffer_3->fl_buffer = (char *) mem_alloc(MAXTERMBUFSIZE, BUFF_SPACE);
  forest_log_buffer_3->fl_size = MAXTERMBUFSIZE;
}
#endif /* MULTI_THREAD */

/*==========================================================================*/
/* Initialize Memory Regions and Related Variables at startup and for
   each thread.

   If non-null, use input parameters for initial sizes (for greater
   freedom in thread allocation) ; otherwise use process-level
   defaults.  
   ----------------------------------------------- */

void init_machine(CTXTdeclc int glsize, int tcpsize, 
		  int complstacksize, int pdlsize)
{
  void tstInitDataStructs(CTXTdecl);

  // single-threaded engine uses this for tries.
  init_private_trie_table(CTXT);

#ifdef MULTI_THREAD
  init_thread_structures(CTXT);
#endif

  tsgLBuff1 = (VarString *)mem_alloc(sizeof(VarString),OTHER_SPACE);
  XSB_StrInit(tsgLBuff1);
  tsgLBuff2 = (VarString *)mem_alloc(sizeof(VarString),OTHER_SPACE);
  XSB_StrInit(tsgLBuff2);
  tsgSBuff1 = (VarString *)mem_alloc(sizeof(VarString),OTHER_SPACE);
  XSB_StrInit(tsgSBuff1);
  tsgSBuff2 = (VarString *)mem_alloc(sizeof(VarString),OTHER_SPACE);
  XSB_StrInit(tsgSBuff2);

  /* Allocate Stack Spaces and set Boundary Parameters
     ------------------------------------------------- */

  if (pdlsize != 0)
	pdl.init_size = pdlsize ;
  if (glsize != 0)
	glstack.init_size = glsize ;
  if (tcpsize != 0)
	tcpstack.init_size = tcpsize ;
  if (complstacksize != 0)
	complstack.init_size = complstacksize ;

  pdl.low = (byte *)malloc(pdl.init_size * K);
  if (!pdl.low)
    xsb_initialization_exit("Not enough core for the PDL Stack!");
  pdl.high = pdl.low + pdl.init_size * K;
  pdl.size = pdl.init_size;
  pspace_tot_gl = pspace_tot_gl + pdl.init_size*K;

  glstack.low = (byte *)malloc(glstack.init_size * K);
  if (!glstack.low)
    xsb_initialization_exit("Not enough core for the Global and Local Stacks!");
  glstack.high = glstack.low + glstack.init_size * K;
  glstack.size = glstack.init_size;
  pspace_tot_gl = pspace_tot_gl + glstack.init_size*K;

#if defined(GENERAL_TAGGING)
  extend_enc_dec_as_nec(glstack.low,glstack.high);
#endif

  initialize_glstack((CPtr) glstack.low, ((CPtr)glstack.high) - 1);

  tcpstack.low = (byte *)malloc(tcpstack.init_size * K);
  if (!tcpstack.low)
    xsb_initialization_exit("Not enough core for the Trail and Choice Point Stack!");
  tcpstack.high = tcpstack.low + tcpstack.init_size * K;
  tcpstack.size = tcpstack.init_size;
  pspace_tot_gl = pspace_tot_gl + tcpstack.init_size*K;
#if defined(GENERAL_TAGGING)
  extend_enc_dec_as_nec(tcpstack.low,tcpstack.high);
#endif

  complstack.low = (byte *)malloc(complstack.init_size * K);
  if (!complstack.low)
    xsb_initialization_exit("Not enough core for the Completion Stack!");
  complstack.high = complstack.low + complstack.init_size * K;
  complstack.size = complstack.init_size;
  pspace_tot_gl = pspace_tot_gl + complstack.init_size;

  /* -------------------------------------------------------------------
     So, the layout of the memory looks as follows:

     pdl.low
             /\
     pdlreg   |
     pdl.high
     ===================
     glstack.low
     hreg   |
           \/
           /\
     ereg   |
     glstack.high
     ===================
     tcpstack.low
     trreg  |
           \/
           /\
     breg   |
     tcpstack.high
     ===================
     complstack.low
             /\
     openreg  |
     complstack.high
     --------------------------------------------------------------------- */

  /* Initialize Registers
     -------------------- */
  cpreg = (pb) &halt_inst;		/* halt on final success */

  pdlreg = (CPtr)(pdl.high) - 1;

  hbreg = hreg = (CPtr)(glstack.low);
  
  bld_free(hreg); hreg++;  // head of attv interrupt chain
  bld_free(hreg); hreg++;  // last cons of attv interrupt chain
  bld_free(hreg); hreg++;  // global variable, from Prolog via 'globalvar/1'

  ebreg = ereg = (CPtr)(glstack.high) - 1;

  *(ereg-1) = (Cell) cpreg;

  trreg	= (CPtr *)(tcpstack.low);
  *(trreg) = (CPtr) trreg;

  reset_freeze_registers;
  openreg = ((CPtr) complstack.high);
  delayreg = NULL;

  /* for incremenatal evaluation */
  //  affected_gl = empty_calllist(); 
  //  changed_gl = empty_calllist(); 

  /* Place a base choice point frame on the CP Stack: this choice point
     is needed for cut -- make sure you initialize all its fields.
     ------------------------------------------------------------------ */
  bfreg = breg = (CPtr)(tcpstack.high) - CP_SIZE;
  cp_pcreg(breg) = (pb) &halt_inst; 	  /* halt on last failure */
  cp_ebreg(breg) = ebreg;
  cp_hreg(breg) = hreg;
  cp_trreg(breg) = trreg;
  cp_cpreg(breg) = cpreg;
  cp_ereg(breg) = ereg;
  cp_prevbreg(breg) = breg;               /* note ! */
  cp_pdreg(breg) = delayreg;
#ifdef CP_DEBUG
  cp_psc(breg) = 0;
#endif
  cp_ptcp(breg) = ptcpreg;
  cp_prevtop(breg) = (CPtr)(tcpstack.high) - 1;

  /* init trie stuff */

#ifdef MULTI_THREAD
  th->trie_locked = 0 ;
#endif
  //  trieinstr_unif_stk_size = DEFAULT_ARRAYSIZ; not needed -- reinited below.
  trieinstr_vars_num = -1;
  init_trie_aux_areas(CTXT);
  tstInitDataStructs(CTXT);

  /* GC and Heap realloc stuff */ 

#ifdef MULTI_THREAD
  total_time_gc = 0;
  total_collected = 0;
  num_gc = 0;
  vcs_tnot_call = 0;

  heap_marks = NULL;
  ls_marks = NULL;
  tr_marks = NULL;
  cp_marks = NULL;

  slide_buf = NULL;
  slide_top = 0;
  slide_buffering = 0;
  slide_buf_size = 0;

  SL_header = NULL;  // init skiplist header...
#else
  nonmt_init_mq_table();
#endif

} /* init_machine() */

Psc make_code_psc_rec(CTXTdeclc char *name, int arity, Psc mod_psc) {
  Pair temp;
  int new;
  Psc new_psc;
  temp = (Pair)insert(name, (byte) arity, mod_psc, &new);
  new_psc = pair_psc(temp);
  set_data(new_psc, mod_psc);
  set_env(new_psc, T_UNLOADED);
  set_type(new_psc, T_ORDI);
  if (mod_psc != global_mod) link_sym(CTXTc new_psc, global_mod); /* Add to global module as well */
  return new_psc;
}

/*==========================================================================*/

/* Initialize Standard PSC Records and Thread Attributes
   ------------------------------- */
void init_symbols(CTXTdecl)
{
  Psc  tables_psc, standard_psc, loader_psc, setofmod_psc;
  Pair temp, tp;
  int  i, new_indicator;
#ifdef MULTI_THREAD
  int status;
#ifdef DEBUG
  size_t stack_size;
#endif
#endif
  int Loc = 0, new;

  /* set special SLG_WAM instruction addresses */

  cell_opcode(&halt_inst) = halt;
  cell_opcode(&proceed_inst) = proceed;         /* returned by load_obj */
  cell_opcode(&fail_inst) = fail;
  cell_opcode(&dynfail_inst) = dynfail;
  //  cell_opcode(&trie_fail_unlock_inst) = trie_fail_unlock;
  cell_opcode(&answer_return_inst) = answer_return;
  cell_opcode(&resume_compl_suspension_inst) = resume_compl_suspension;
  cell_opcode(&resume_compl_suspension_inst2) = resume_compl_suspension;
  cell_opcode(&check_complete_inst) = check_complete;
  cell_opcode(&hash_handle_inst) = hash_handle;
  cell_opcode(&trie_fail_inst) = trie_fail;
  cell_opcode(&completed_trie_member_inst) = completed_trie_member;    

  check_interrupts_restore_insts_addr = calloc((3+1),sizeof(Integer));
  write_byte(check_interrupts_restore_insts_addr,&Loc,check_interrupt);
  Loc += 2; 
  write_byte(check_interrupts_restore_insts_addr,&Loc,4); /* AR size */
  pad64bits(&Loc);
  write_word(check_interrupts_restore_insts_addr,&Loc,0); /* unused psc addr */
  write_byte(check_interrupts_restore_insts_addr,&Loc,restore_dealloc_proceed);
  /* length check_interrupt + length restore_dealloc_proceed */

  inst_begin_gl = 0;
  current_inst = 0;
  symbol_table.table = (void **)mem_calloc(symbol_table.size, sizeof(Pair),ATOM_SPACE);
  string_table.table = (void **)mem_calloc(string_table.size, sizeof(char *),STRING_SPACE);

  /* insert mod name global */
  /*tp = insert_module(T_MODU, "global");	/ loaded */
  tp = insert_module(T_MODU, "usermod");	/* loaded */
  set_data(pair_psc(tp), (Psc)USERMOD_PSC);	/* initialize global mod PSC */
  global_mod = pair_psc(tp);

  /* insert "."/2 into global list */
  temp = (Pair)insert(".", 2, global_mod, &new_indicator);
  list_pscPair = temp;
  list_psc = pair_psc(temp);
  list_dot_string = get_name(list_psc);

  if_psc = pair_psc(insert(":-", 2, global_mod, &new_indicator));

  /* insert symbol "$BOX$"/3 */
  box_psc = pair_psc(insert("$BOX$", 3, global_mod, &new_indicator));

  delay_psc = pair_psc(insert("DL", 3, global_mod, &new_indicator));

  standard_psc = pair_psc(insert_module(0, "standard"));	/* unloaded */
  setofmod_psc = pair_psc(insert_module(0, "setof"));	/* unloaded */
  loader_psc = pair_psc(insert_module(0, "loader"));	/* unloaded */

  true_psc = make_code_psc_rec(CTXTc "true", 0, standard_psc);
  true_string = get_name(true_psc);
  cut_string = string_find("!",1);
  cyclic_string = (char *) string_find("<cyclic>",1);

  visited_psc = make_code_psc_rec(CTXTc "_$visited", 0, standard_psc);

  load_undef_psc = make_code_psc_rec(CTXTc "_$load_undef", 1, loader_psc);
  comma_psc = make_code_psc_rec(CTXTc ",", 2, standard_psc);
  colon_psc = make_code_psc_rec(CTXTc ":", 2, standard_psc);
  caret_psc = make_code_psc_rec(CTXTc "^", 2, setofmod_psc);
  setof_psc = make_code_psc_rec(CTXTc "setof", 3, setofmod_psc);
  bagof_psc = make_code_psc_rec(CTXTc "bagof", 3, setofmod_psc);
  cut_psc = make_code_psc_rec(CTXTc "!", 0, standard_psc);
  cond_psc = make_code_psc_rec(CTXTc "->", 2, standard_psc);
  dollar_var_psc = make_code_psc_rec(CTXTc "$VAR", 1, global_mod);

  ccall_mod_psc = pair_psc(insert_module(0,"ccallxsb"));
  c_callloop_psc = pair_psc(insert("c_callloop_query_loop",1,ccall_mod_psc,&new));
  if (new) {
    set_data(c_callloop_psc,ccall_mod_psc);
    env_type_set(CTXTc c_callloop_psc, T_IMPORTED, T_ORDI, (xsbBool)new);
    link_sym(CTXTc c_callloop_psc, global_mod);
  }


  /* insert symbol tnot/1 into module tables */
  tables_psc = pair_psc(insert_module(0, "tables"));		/* unloaded */

  tnot_psc = make_code_psc_rec(CTXTc "tnot", 1, tables_psc);
  answer_completion_psc = make_code_psc_rec(CTXTc "answer_completion", 2, tables_psc);

  /* insert "[]"/0 into String Table */
  nil_string = string_find("[]", 1);

  /*
   * Initialize ret PSCs.  Notice that ret_psc[0] is set to a pointer
   * to STRING "ret".
   */
  ret_psc[0] = (Psc) string_find("ret", 1);
  for (i = 1; i < MAX_ARITY; i++) ret_psc[i] = NULL;

  /* Finally, eagerly insert pscs used for resource errors.  This way,
     we don't have to worry abt the symbol table growing when we're
     thowing a memory error. */
  temp = (Pair)insert("$$exception_ball", (byte)2, 
					pair_psc(insert_module(0,"standard")), 
		      &new_indicator);
  temp = (Pair) insert("error",3,global_mod,&new_indicator);
  temp = (Pair) insert("resource_error",1,global_mod,&new_indicator);

#ifdef MULTI_THREAD
  status = pthread_attr_init(&detached_attr_gl);
  if (status != 0) 
    xsb_initialization_exit("Cannot init pthread attr detached state during system initialization");
  
  status = pthread_attr_setdetachstate(&detached_attr_gl,PTHREAD_CREATE_DETACHED);
  if (status != 0) 
    xsb_initialization_exit("Cannot set pthread attr detached state during system initialization");

  pthread_attr_setscope(&detached_attr_gl,PTHREAD_SCOPE_SYSTEM);

  /* set minimal stack size to a reasonable value */
  pthread_attr_setstacksize(&detached_attr_gl,512*K*ZOOM_FACTOR);
  status = pthread_attr_init(&normal_attr_gl);
  if (status != 0) 
    xsb_initialization_exit("Cannot init pthread attr during system initialization");
  /* set minimal stack size to a reasonable value */
  status = pthread_attr_setstacksize(&normal_attr_gl,512*K*ZOOM_FACTOR);
#ifdef DEBUG
  if (status != 0) 
  {
  	status = pthread_attr_getstacksize(&normal_attr_gl,&stack_size);
  	if (status != 0) 
    		xsb_initialization_exit("Cannot determine thread stack size during system initialization");
	else
	  printf( "Minimum thread stack size set to %d\n", (int) stack_size ) ;
  }
#endif
  pthread_attr_setscope(&normal_attr_gl,PTHREAD_SCOPE_SYSTEM);
#ifdef MULTI_THREAD
  init_shared_trie_table();
#endif
#endif

}

/*==========================================================================*/
