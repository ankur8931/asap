/* File:      orient_xsb.c - find out where xsb stuff is
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1998
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
** $Id: orient_xsb.c,v 1.29 2013-01-04 14:56:22 dwarren Exp $
** 
*/



#include "xsb_config.h"

#ifdef WIN_NT
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
/* wind2unix.h must be included after sys/stat.h */
#include "wind2unix.h"
#include "export.h"
#include "basicdefs.h"
#include "basictypes.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "extensions_xsb.h"
#include "memory_xsb.h"
#include "auxlry.h"
#include "cinterf.h"

char executable_path_gl[MAXPATHLEN] = {'\0'};	/* This is set to a real name below */

char *install_dir_gl; 			/* installation directory */
char *xsb_config_file_gl;     		/* XSB configuration file */
char *xsb_config_file_gl_xwam; 		/* XSB config file - xwam version */
char *user_home_gl;    	     	     	/* the user $HOME dir or install dir,
					   if $HOME is null */ 


extern xsbBool is_absolute_filename(char *);
DllExport extern char * call_conv strip_names_from_path(char*, int);

static void check_create_dir(CTXTdeclc char *);

extern void transform_cygwin_pathname(char *);
extern void fix_cygwin_pathname(char*);

char current_dir_gl[MAXPATHLEN];
char xsbinfo_dir_gl[MAXPATHLEN];


void set_xsbinfo_dir (CTXTdecl) {
  struct stat *fileinfo = mem_alloc(1*sizeof(struct stat),LEAK_SPACE);
  char old_xinitrc[MAXPATHLEN], new_xinitrc[MAXPATHLEN],
    user_config_dir[MAXPATHLEN], user_arch_dir[MAXPATHLEN];
  int retcode;

  if (!fileinfo) {
    xsb_abort("No core memory to allocate stat structure.\n");
  }
  snprintf(xsbinfo_dir_gl, MAXPATHLEN, "%s%c.xsb", user_home_gl, SLASH);
  snprintf(old_xinitrc, MAXPATHLEN, "%s%c.xsbrc", user_home_gl, SLASH);
  snprintf(new_xinitrc, MAXPATHLEN, "%s%cxsbrc", xsbinfo_dir_gl, SLASH);
  snprintf(user_config_dir, MAXPATHLEN, "%s%cconfig", xsbinfo_dir_gl, SLASH);
  snprintf(user_arch_dir, MAXPATHLEN, "%s%c%s", user_config_dir, SLASH, FULL_CONFIG_NAME);

  /* Create USER_HOME/.xsb directory, if it doesn't exist. */
  check_create_dir(CTXTc xsbinfo_dir_gl);
  check_create_dir(CTXTc user_config_dir);
  check_create_dir(CTXTc user_arch_dir);
  retcode = stat(old_xinitrc, fileinfo);

  if ((retcode == 0) && (stat(new_xinitrc, fileinfo) != 0)) {
    xsb_warn(CTXTc "It appears that you have an old-style `.xsbrc' file!\n           The XSB initialization file is now %s.\n           If your `.xinitrc' defines the `library_directory' predicate,\n           please consult the XSB manual for the new conventions.", new_xinitrc);
  }
  mem_dealloc(fileinfo,1*sizeof(struct stat),LEAK_SPACE);
}


/* Check if PATH exists. Create if it doesn't. Bark if it can't create or if
   PATH exists, but isn't a directory. */
static void check_create_dir(CTXTdeclc char *path) {
  struct stat *fileinfo = mem_alloc(1*sizeof(struct stat),LEAK_SPACE);
  int retcode = stat(path, fileinfo);

  if (!fileinfo) {
    xsb_abort("No core memory to allocate stat structure.\n");
  }

  if (retcode == 0 && ! S_ISDIR(fileinfo->st_mode)) {
    xsb_warn(CTXTc "File `%s' is not a directory!\n           XSB uses this directory to store data.", path);
    /* exit(1); */
  }

  if (retcode != 0) 
#ifdef WIN_NT
    retcode = mkdir(path);
#else
    retcode = mkdir(path, 0755);
#endif

  if (retcode != 0) {
    xsb_warn(CTXTc "Cannot create directory `%s'!\n           XSB uses this directory to store data.", path);
    /* exit(1); */
  }
  mem_dealloc(fileinfo,1*sizeof(struct stat),LEAK_SPACE);
}

/* uses the global executable var */
DllExport char *xsb_executable_full_path(char *myname)
{
  struct stat fileinfo;
  char *path = getenv("PATH");
  int len, found = 0;
  char *pathcounter, save;
  static char myname_augmented[MAXPATHLEN];
  char *dummy; /* to squash warnings */
#ifndef WIN_NT
  int link_len;
#endif

#ifndef WIN_NT
#ifndef SIMPLESCALAR
  /* Unix */
  /* if we can read symlink, then it is a symlink */
  if ( (link_len = readlink(myname, myname_augmented, MAXPATHLEN)) > 0 ) {
    /* we can't assume that the value of the link is null-terminated */
    if ( *(myname_augmented+link_len) != '\0' )
      *(myname_augmented+link_len+1) = '\0';
  } else
    strcpy(myname_augmented, myname);
#endif
#else
  /* Windows doesn't seem to have readlink() */
  strcpy(myname_augmented, myname);
  /* if executable doesn't end with .exe, then add it */
  if ( *(myname_augmented + strlen(myname) - 4) != '.'
       || tolower(*(myname_augmented + strlen(myname) - 3)) != 'e'
       || tolower(*(myname_augmented + strlen(myname) - 2)) != 'x'
       || tolower(*(myname_augmented + strlen(myname) - 1)) != 'e' )
    snprintf(myname_augmented, MAXPATHLEN, "%s.exe", myname);
#endif

#if defined(WIN_NT)
  /* CYGWIN uses absolute paths like this:
     /<drive letter>/dir1/dir2/...
     If we find such a path, we transform it to a windows-like pathname.
     This assumes that XSB has been compiled using the native Windows
     API, and is being run from CYGWIN bash (like from the test
     scripts). */
  transform_cygwin_pathname(myname_augmented);
#endif
#if defined(CYGWIN)
  // converts Letter:/dir/dir into /cygdrive/Letter/dir/dir
  fix_cygwin_pathname(myname_augmented);
#endif

  if (is_absolute_filename(myname_augmented))
    strcpy(executable_path_gl, myname_augmented);
  else {
    dummy = getcwd(current_dir_gl, MAXPATHLEN-1);
    snprintf(executable_path_gl, MAXPATHLEN, "%s%c%s", current_dir_gl, SLASH, myname_augmented);
  }

  SQUASH_LINUX_COMPILER_WARN(dummy) ; 

  /* found executable by prepending cwd. Make sure we haven't found a directory named xsb */
  if ((!stat(executable_path_gl, &fileinfo)) && (S_ISREG(fileinfo.st_mode))) return executable_path_gl;
                                          //  or (!S_ISDIR(fileinfo.st_mode))

  /* Otherwise, search PATH environment var.
     This code is a modified "which" shell builtin */
  pathcounter = path;
  while (*pathcounter != '\0' && found == 0) {
    len = 0;
    while (*pathcounter != PATH_SEPARATOR && *pathcounter != '\0') {
      len++;
      pathcounter++;
    }

    /* save the separator ':' (or ';' on NT and replace it with \0) */
    save = *pathcounter;
    *pathcounter = '\0';

    /* Now `len' holds the length of the PATH component 
       we are currently looking at.
       `pathcounter' points to the end of this component. */
    snprintf(executable_path_gl, MAXPATHLEN, "%s%c%s", pathcounter - len, SLASH, myname_augmented);

    /* restore the separator and addvance the pathcounter */
    *pathcounter = save;
    if (*pathcounter) pathcounter++;

#ifdef WIN_NT
    found = (0 == access(executable_path_gl, 02));	/* readable */
#else
    found = (0 == access(executable_path_gl, 01));	/* executable */
#endif
    if (found) return executable_path_gl;
  }

  /* XSB executable isn't found after searching PATH */
  if (xsb_mode != C_CALLING_XSB) {
    fprintf(stderr,
	  "*************************************************************\n");
    fprintf(stderr, 
	  "PANIC!!! Cannot determine the full name of the XSB executable!\n");
    fprintf(stderr, 
	  "Please report this problem using the XSB bug tracking system accessible from\n");
    fprintf(stderr, "\t http://sourceforge.net/projects/xsb\n");
    fprintf(stderr,
	    "*************************************************************\n");
    exit(1);
  }
  else { /* Dont want to exit when calling from C */
    xsb_initialization_exit("Cannot determine the full name of the XSB executable!\n"
		       "Please report this problem using the XSB bug tracking system accessible from\n"
		       "\t http://sourceforge.net/projects/xsb')\n");
  }
  /* This return is needed just to pacify the compiler */
  return FALSE;
}

/**************************************/

void set_install_dir(void) {

  /* strip 4 levels, since executable is always of this form:
     install_dir/config/<arch>/bin/xsb */
  install_dir_gl = strip_names_from_path(executable_path_gl, 4);
  if (install_dir_gl == NULL) {
    if (xsb_mode != C_CALLING_XSB) {
      fprintf(stderr,
	      "*************************************************************\n");
      fprintf(stderr, "PANIC!! Can't find the XSB installation directory.\n");
      fprintf(stderr, "Perhaps, you moved the XSB executable out of \n");
      fprintf(stderr, "its normal place in the XSB directory structure?\n");
      fprintf(stderr,
	      "*************************************************************\n");
      exit(1);
    }
    else { /* Dont want to exit when calling from C */
      xsb_initialization_exit("Cant find the XSB installation directory.\n"
			 "Perhaps, you moved the XSB executable out of \n"
			 "its normal place in the XSB directory structure?')\n");
    }
  }
}

void set_config_file(void) {
  int retcode, retcode_xwam;
  struct stat fileinfo, fileinfo_xwam;

  /* The config file is in the lib directory at the same 
     level as the xsb executable. */
  xsb_config_file_gl = strip_names_from_path(executable_path_gl, 2);
  snprintf(xsb_config_file_gl+strlen(xsb_config_file_gl),
	   (MAXPATHLEN-strlen(xsb_config_file_gl)),
	  "%clib%cxsb_configuration%s", SLASH, SLASH,XSB_SRC_EXTENSION_STRING);
  xsb_config_file_gl_xwam = strip_names_from_path(executable_path_gl, 2);
  snprintf(xsb_config_file_gl_xwam+strlen(xsb_config_file_gl_xwam),
	   (MAXPATHLEN-strlen(xsb_config_file_gl_xwam)),
	  "%clib%cxsb_configuration%s", SLASH, SLASH,XSB_OBJ_EXTENSION_STRING);

  /* Perform sanity checks: xsb_config_file must be in install_dir/config
     This is probably redundant */
  if ( strncmp(install_dir_gl, xsb_config_file_gl, strlen(install_dir_gl)) != 0 
       || (strstr(xsb_config_file_gl, "config") == NULL) ) {

    if (xsb_mode != C_CALLING_XSB) {
      fprintf(stderr,
	    "*************************************************************\n");
      fprintf(stderr,
	    "PANIC!! The file configuration%s\n", XSB_SRC_EXTENSION_STRING);
      fprintf(stderr,
	      "is not where it is expected: %s%cconfig%c%s%clib\n",
	      install_dir_gl, SLASH, SLASH, FULL_CONFIG_NAME, SLASH);
      fprintf(stderr, "Perhaps you moved the XSB executable %s\n", executable_path_gl);
      fprintf(stderr, "away from its usual place?\n");
      fprintf(stderr,
	      "*************************************************************\n");
      exit(1);
    }
    else {
      xsb_initialization_exit("The file configuration%s\n"
			       "is not where it is expected: %s%cconfig%c%s%clib\n"
			       "Perhaps you moved the XSB executable %s\n", 
			       "away from its usual place?\n"	
	      XSB_SRC_EXTENSION_STRING,install_dir_gl, SLASH, SLASH, FULL_CONFIG_NAME, SLASH,
	      executable_path_gl);
    }
  }

  /* Check if configuration.P exists and is readable */
  retcode = stat(xsb_config_file_gl, &fileinfo);
  retcode_xwam = stat(xsb_config_file_gl_xwam, &fileinfo_xwam);
#ifdef WIN_NT
  if (( (retcode != 0) || !(S_IREAD & fileinfo.st_mode) ) &&
      ( (retcode_xwam != 0) || !(S_IREAD & fileinfo_xwam.st_mode) )) {
#else
    if (( (retcode != 0) || !(S_IRUSR & fileinfo.st_mode) ) &&
	( (retcode_xwam != 0) || !(S_IRUSR & fileinfo_xwam.st_mode) )) {
#endif  
    if (xsb_mode != C_CALLING_XSB) {
      fprintf(stderr,
	    "*************************************************************\n");
      fprintf(stderr, "PANIC! XSB configuration files %s[%s]\n", xsb_config_file_gl, XSB_OBJ_EXTENSION_STRING);
      fprintf(stderr, "don't exist or are not readable by you.\n");
      fprintf(stderr,
	      "*************************************************************\n");
      exit(1);
    }
    else {
      xsb_initialization_exit("XSB configuration file %s does not exist or is not readable by you.\n",
			      xsb_config_file_gl);
    }
  }
  }

#ifdef WIN_NT
void transform_cygwin_pathname(char*);
#endif

void set_user_home() {
  user_home_gl = (char *) getenv("HOME");
  if ( user_home_gl == NULL ) {
    user_home_gl = (char *) getenv("USERPROFILE"); /* often used in Windows */
    if ( user_home_gl == NULL )
      user_home_gl = install_dir_gl;
  }
#ifdef WIN_NT
  transform_cygwin_pathname(user_home_gl);
#endif
}
