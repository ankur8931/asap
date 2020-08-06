/* File:      pathname_xsb.c -- utilities to manipulate path/file names
** Author(s): kifer, kostis
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
** $Id: pathname_xsb.c,v 1.44 2012-10-12 16:42:57 tswift Exp $
** 
*/

#include "xsb_config.h"
#include "xsb_debug.h"

#ifdef WIN_NT
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
/* wind2unix.h must be included after sys/stat.h */
#include "wind2unix.h"
#include "export.h"

#if (!defined(WIN_NT))
#include <pwd.h>
#endif

#include "setjmp_xsb.h"
#include "auxlry.h"
#include "context.h"
#include "psc_xsb.h"
#include "cell_xsb.h"
#include "cinterf.h"
#include "error_xsb.h"
#include "flags_xsb.h"
#include "extensions_xsb.h"
#include "memory_xsb.h"

/*=========================================================================*/

#define DDOT	".."
#define DOT	"."

extern char *user_home_gl;  	  /* from main_xsb.c: the user $HOME dir or
				     install dir, if $HOME is null */
static char *rectify_pathname(char *, char *);
void transform_cygwin_pathname(char *);
void fix_cygwin_pathname(char *);

/*=========================================================================*/

/* check if the path name is absolute */
xsbBool is_absolute_filename(char *filename) {

#if defined(WIN_NT) 
  /*  If the file name begins with a "\" or with an "X:", where X is some
   *  character, then the file name is absolute.
   *  We also assume if it starts with a // or /cygdrive/, it's absolute */
  if ( (filename[0] == SLASH) || 
       (isalpha(filename[0]) && filename[1] == ':') ||
       (filename[0] == '/')
       )
    return TRUE;
#else /* Unix */
  if (filename[0] == '/')
    return TRUE;
#endif

  return FALSE;
}
  

/* if file is a directory and is missing a trailing slash, add it 
   passes file name and the slash to be added. can be backslash on NT
   Also, strips off excessive trailing slashes. 
*/
char *dirname_canonic(char *filename) {
  char canonicized[MAXPATHLEN];
  int retcode, len = (int)strlen(filename);
  struct stat fileinfo;
  rectify_pathname(filename, canonicized);
  retcode = stat(canonicized, &fileinfo);

  /* if directory, add trailing slash */
  if ((retcode==0) && S_ISDIR(fileinfo.st_mode)
      && (canonicized[len-1] != SLASH)) {
    canonicized[len] = SLASH;
    canonicized[len+1] = '\0';
  }
  return string_find(canonicized,1);
}


static void normalize_file_slashes(char *path) {
  int ANTISLASH = (SLASH == '/' ? '\\' : '/');

  path = strchr(path,ANTISLASH);
  while (path != NULL) {
    *path = SLASH;
    path = strchr(path,ANTISLASH);
  }
}

/* Like tilde_expand_filename, but doesn't rectify */
char *tilde_expand_filename_norectify(char *filename, char *expanded) {
  
#if defined(WIN_NT)
  strcpy(expanded, filename);
  transform_cygwin_pathname(expanded);
  normalize_file_slashes(expanded);
  return expanded;

#else /* Unix */
  char *path_prefix;        /* ptr to a (sub)string containing what will
			       become the prefix for the absolute filename. */
  char *path_suffix;        /* ptr to a (sub)string containing what will
			       become the suffix for the absolute filename. */
  char username[MAXFILENAME]; /* the username if filename has ~<name> */
  int username_len;
  struct passwd *pw_struct;     /* receives passwd structure from getpwnum() */

  if (filename[0] != '~') {
    strcpy(expanded, filename);
    normalize_file_slashes(expanded);
    return expanded;
  } 
  if (filename[1] == '/' || filename[1] == '\0') {
    /*  The file name begins with "~/" or is simply "~" -- so replace
	'~' with the user's home directory. */
    path_prefix = user_home_gl;
    path_suffix = filename + 1;
  } else {
    /*  The file name begins with ~<username>.  Use a system call to
	determine this directory's path. */
    path_prefix = path_suffix = filename + 1;
    while ( (*path_suffix != '\0') && (*path_suffix != '/') )
      path_suffix++;
    username_len = path_suffix - path_prefix;
    memmove(username, path_prefix, username_len);
    username[username_len] = '\0';
    
    pw_struct = (struct passwd *) getpwnam(username);
    if (!pw_struct) {
      /*  The system has no info on this user, so we can't
	  construct the absolute path -- abort. */
      char message[100];
      snprintf(message, 100, "[PATHNAME] `%s': unknown user\n", username);
      xsb_abort(message);
    } else
      path_prefix = pw_struct -> pw_dir;
  }
   
  // Dont know about this -- should probably be a snprintf.
  sprintf(expanded, "%s%c%s", path_prefix, SLASH, path_suffix);
  normalize_file_slashes(expanded);
  return expanded;
#endif /* Unix */
}

/*
** Like expand_filename, but ONLY expands Unix tilde by replacing '~', '~user'
** with the home directory of the appropriate user.
** Does nothing on Windows.
*/
char *tilde_expand_filename(char *filename) {
  char aux_filename[MAXPATHLEN];
  char absolute_filename[MAXPATHLEN]; /* abs filename composed here */

  tilde_expand_filename_norectify(filename, aux_filename);
  return string_find(rectify_pathname(aux_filename, absolute_filename),1);
}


/*
 *  Return full path name for the file passed in as argument.
 *  This is called from cinterf and may be called before XSB is initialized,
 *  so to make thread-safe, malloc space to return value (leaky)
 */

char *expand_filename(char *filename) {
  char aux_filename[MAXPATHLEN],
   aux_filename2[MAXPATHLEN];
  char *absolute_filename = mem_alloc(MAXPATHLEN,OTHER_SPACE); // since xsb may not be initialized
  char *dummy; /* to squash warnings */

  if (is_absolute_filename(filename)) {
    return rectify_pathname(filename, absolute_filename);
#ifndef WIN_NT
  } else if (filename[0] == '~') {
    tilde_expand_filename_norectify(filename, aux_filename);
    return rectify_pathname(aux_filename, absolute_filename);
#endif
  } else {
    dummy = getcwd(aux_filename2, MAXPATHLEN-1);
    snprintf(aux_filename,MAXPATHLEN, "%s%c%s", aux_filename2,
	    SLASH, filename);
    return rectify_pathname(aux_filename, absolute_filename);
  }
  SQUASH_LINUX_COMPILER_WARN(dummy) ; 
}

/* strip names from the back of path 
   PATH is the path name from which to strip.
   HOW_MANY is the number of names to strip.
   E.g., strip_names_from_path("a/b/c/d", 2) returns "a/b" 

   This function is smart about . and ..: 
       	   strip_names_from_path("a/b/c/./d", 2)
   is still "a/b" and
           strip_names_from_path("a/b/c/../d", 2)
   is "a".
   If we ask to strip too many names from path, it'll abort.

   This function copies the result into a large buffer, so we can add more
   stuff to it. These buffers stay forever, but we call this func only a couple
   of times, so it's ok. */
DllExport char * call_conv strip_names_from_path(char* path, int how_many)
{
	int i, abort_flag=FALSE;
	char *cutoff_ptr;
	char *buffer = malloc(MAXPATHLEN);  // can't change to mem_alloc for mt, lock not initted?
	
	if (!buffer)
		printf("no space to allocate buffer in strip_names_from_path.\n");
	else
#ifdef SIMPLESCALAR
	{
		/*   rectify_pathname(path,buffer); */
		strcpy(buffer,path);
		
		cutoff_ptr = buffer + strlen(buffer);
		
		while (cutoff_ptr != buffer && how_many > 0) {
			if (*cutoff_ptr == SLASH) {
				how_many--;
				*cutoff_ptr = '\0';
			}
			cutoff_ptr--;
		}
		if (how_many > 0)
		xsb_abort("[PATHNAME] There is no directory %d levels below %s", how_many, path);
	}
#endif
	{
		rectify_pathname(path,buffer);
		
		for (i=0; i < how_many; i++) {
			if (abort_flag) {
				xsb_abort("[PATHNAME] There is no directory %d levels below %s",how_many, path);
			}
			cutoff_ptr = strrchr(buffer, SLASH);
			if (cutoff_ptr == NULL)
				return "";
			if ((cutoff_ptr - buffer) > 0)
			/* we have more than just a slash between the beginning of buffer and
			 cutoff_ptr: replace slash with end of string */
				*cutoff_ptr = '\0';
			else {       /* we are at the top of the file hierarchy */
				*(cutoff_ptr+1) = '\0';
				abort_flag=TRUE;
			}
		}
	}
	return buffer;
}


/* Get the base name of PATH, e.g., basename of a/b/c is c if path is a/b/c/
   then basename is "". This op preserves extension, i.e., basename of a/b.c 
   is b.c */
static char *get_file_basename(char *path) {
  char *ptr;
  char canonicized[MAXPATHLEN];
  //ptr = strrchr(path, SLASH);
  ptr = strrchr(rectify_pathname(path,canonicized), SLASH);
  if (ptr == NULL)
    return path;
  else
    return ptr+1;
}

/* get directory part of PATH. It first rectifies PATH, then returns result.
   E.g., a/b/.././c --> a/ (even if c is a directory itself)
   a/b/ --> a/b/ 
   Doesn't expand the directory name.
   Always leaves trailing slash at the end. 

   Expects a string storage as 2nd arg, returns second arg.
*/
static char *get_file_dirname(char *path, char *dir) {
  char *ptr;
  ptr = strrchr(rectify_pathname(path,dir), SLASH);
  if (ptr == NULL)
    /* No slash in filename, return empty string */
    return "";
  /* the whole thing might be just "/". In this case, it is the dirname of the
     file */
  else if (*ptr==SLASH && *(ptr+1)=='\0')
    return dir;
  else {
    *(ptr+1) = '\0';
    return dir;
  }
}

/* Get file extension, i.e., "c" in abc.c, etc. If the file name is of the form
   ".abc", where none of a,b,c is a '.', then the extension is empty string. */

static char *get_file_extension(char *path) {
  char *ptr, *base=get_file_basename(path);
  ptr = strrchr(base, '.');
  if ((ptr==base) || (ptr==NULL))
    return NULL;
  else return (ptr+1);
}

#define MAXPATHNAMES 256 /* max number of file names in a path name */

/* 
** Go over path name and get rid of `..', `.', and multiple slashes 
** Won't delete the leading `..'.
** Expects two strings (with allocated storage) as params: the input path and
** the output path. Returns the second argument.
*/
static char *rectify_pathname(char *inpath, char *outpath) {
  char names[MAXPATHNAMES][MAXFILENAME];  /* array of filenames in inpath.
					 1st index: enumerates names in inpath;
					 2nd index: scan file names */
  char expanded_inpath[MAXPATHLEN];
  char *inptr1, *inptr2, *inpath_end;
  int length; /* length=inptr2-inptr1 */
  int i, outidx=0, nameidx=0; /* nameidx: 1st index to names */
  xsbBool leading_slash, leading_slash2, trailing_slash;

  tilde_expand_filename_norectify(inpath, expanded_inpath);
  
  /* initialization */
  inptr1 = inptr2 = expanded_inpath;
  inpath_end = expanded_inpath + strlen(expanded_inpath);

  /* check if expanded inpath has trailing/leading slash */
  leading_slash = (*expanded_inpath == SLASH ? TRUE : FALSE);
#if defined(WIN_NT) || defined(CYGWIN)
  /* In windows, the leading \\foo means remote drive;
     In CYGWIN, absolute path starts with //driveletter */
  leading_slash2 = (*(expanded_inpath+1) == SLASH ? TRUE : FALSE);
#else
  leading_slash2 = FALSE;
#endif
  trailing_slash = (*(inpath_end - 1) == SLASH ? TRUE : FALSE);

  while ( inptr2 < inpath_end ) {
    inptr2 = strchr(inptr1, SLASH);
    if (inptr2==NULL)
      inptr2 = inpath_end;

    /* skip slashes */
    if ((length = (int)(inptr2 - inptr1)) == 0) {
      if (inptr2 == inpath_end)
	break; /* out of the while loop */
      else {
	inptr2++;
	inptr1++;
	continue;
      }
    }

    switch (length) {
    case 1:
      if (*inptr1 == '.' && inptr1 != expanded_inpath) {
	inptr1 = inptr2;
	continue; /* the loop */
      }
      break; /* we found a file name, will process it */
    case 2: 
      if ((*inptr1 == '.') && (*(inptr1+1) == '.')) {
	nameidx--; /* drop the previous file name from the names array */
	if (nameidx < 0) {
	  /* These are leading ..'s -- leave them */
	  nameidx++;
	  break;
	} else if (strcmp(names[nameidx], "..") == 0) {
	  /* the previous name was also ".." -- leave the ..'s intact: we must
	     be looking at the leading sequence of ../../../something */
	  nameidx++;
	} else {
	  /* Discard .. and the previous file name */
	  inptr1 = inptr2;
	  continue;
	}
      }
      break;
    } /* done processing '.' and '..' */
    
    /* copy the filename just found to the right slot in the names array */
    strncpy(names[nameidx], inptr1, length);
    names[nameidx][length] = '\0'; /* make a string out of the file name */
    nameidx++;
    inptr1=inptr2;
    if (nameidx >= MAXPATHNAMES)
      xsb_abort("[PATHNAME] Directory depth in pathname exceeds maximum, %s",
		inpath);
  }

  /* at this point, we've copied all file names into names array and eliminated
     . and .. (in case of .. we also got rid of the preceding file name).
     So, we are ready to construct  the outpath. */

  if (leading_slash) {
    outpath[outidx] = SLASH;
    outidx++;
  }
  if (leading_slash && leading_slash2) {
#if defined(CYGWIN)
    strncpy(outpath+outidx,"cygdrive",8);
    outidx += 8;
#endif
    outpath[outidx] = SLASH;
    outidx++;
  }

  if (nameidx==0) outpath[outidx] = '\0';
  for (i=0; i<nameidx; i++) {
    strcpy(outpath+outidx, names[i]);
    outidx = outidx + (int)strlen(names[i]);
    /* put slash in place of '\0', if we are not at the end yet */
    if (i < nameidx-1) {
      outpath[outidx] = SLASH;
      outidx++;
    }
  }

  /* don't add trailing slash if the file name is "/" */
  if (trailing_slash && (nameidx > 0)) {
    outpath[outidx] = SLASH;
    outpath[outidx+1] = '\0';
  }
  return(outpath);
}

/* Takes filename, gets back dir, base (sans the extension), and extension. 
   The dirname isn't expanded, even for tildas, but it is rectified.
   +++NOTE: this procedure modifies the input file name by inserting '\0' in
   place of the dot that separates the extension from the base name.
   This is ok, since this function is used only as a built in, so the input
   file name is discarded anyway. 
 */
void parse_filename(char *filename, char **dir, char **base, char **extension)
{
  char absolute_dirname[MAXPATHLEN]; /* abs dirname composed here */
  char basename[MAXFILENAME];    	    /* the rest of the filename  */

  *base = strcpy(basename, get_file_basename(filename));
  *dir = get_file_dirname(filename, absolute_dirname);
  *extension = get_file_extension(basename);
  /* cut off the extension from the base */
  if (NULL == *extension)
    *extension = "";
  else
    *(*extension-1) = '\0';

  *base = string_find(*base,1);
  *dir = string_find(*dir,1);
  *extension = string_find(*extension,1);
  return;
}

/* transform_cygwin_pathname takes cygwin-like pathnames
/cygdrive/D/..  and transforms them into windows-like pathnames D:\
(in-place).  It assumes that the given pathname is a valid cygwin
absolute pathname */

void transform_cygwin_pathname(char *filename) 
{
  char *pointer;
  char tmp[MAXPATHLEN];
  int diff = 0;

  if (filename[0] == '/') {
    /* MK: unclear what this was supposed to do in case of the files starting
       with // or files of the form /Letter. Changed this to no-op. */
    if (filename[1] == '/') return; /* diff = 1; */
    else if (filename[2] == '\0') return; /* diff = 1; */
    else if (filename[1] == 'c' &&
	     filename[2] == 'y' &&
	     filename[3] == 'g' &&
	     filename[4] == 'd' &&
	     filename[5] == 'r' &&
	     filename[6] == 'i' &&
	     filename[7] == 'v' &&
	     filename[8] == 'e' &&
	     filename[9] == '/')
      diff = 9;
    else {
      strcpy(tmp,filename);
      strcpy(filename,(char *)flags[USER_HOME]);
      strcpy(filename+strlen((char *)flags[USER_HOME]),tmp);
      return;
    }

    pointer=filename+diff+1;
    filename[0]=*pointer;
    filename[1]=':';
    filename[2]='\\';
    for(pointer+=1;*pointer;pointer++) 
      if (*pointer == '/')
	*(pointer-diff) = '\\';
      else
	*(pointer-diff) = *pointer;
  
    *(pointer-diff) = '\0';
    return;
  }
}

/*
  CYGWIN's bash understands Letter:/dir/dir/ but CYGWIN does not seem to take
  this internally. Transform this to /cygdrive/Letter/dir1/dir2/...
*/
void fix_cygwin_pathname(char *filename) 
{
  char tmp[MAXPATHLEN];

  if (strlen(filename) < MAXPATHLEN - 9    // /cygdrive = 9 chars
      && filename[1] == ':'
      && filename[2] == '/'
      && isalpha(((int)filename[0])))
    {
      strcpy(tmp,"/cygdrive/");
      tmp[10] = filename[0];
      strcpy(tmp+11,filename+2);
      strcpy(filename,tmp);
    }
  else return;
}

/*=========================================================================*/
#ifdef WIN_NT
#define not_a_dir(fileinfo) !(fileinfo.st_mode & _S_IFDIR)
#else
#define not_a_dir(fileinfo) !(fileinfo.st_mode & S_IFDIR)
#endif

char *existing_file_extension(char *basename)
{
  char filename[MAXPATHLEN];
  struct stat fileinfo;

  strcpy(filename, basename); strcat(filename, XSB_SRC_EXTENSION_STRING);
  /*  +1 skips the "."   */
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return XSB_SRC_EXTENSION_STRING+1;

  strcpy(filename, basename); strcat(filename, ".c");
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return "c";

  strcpy(filename, basename); strcat(filename, ".cpp");
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return "cpp";

  strcpy(filename, basename); strcat(filename, ".pl");
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return "pl";

  strcpy(filename, basename); strcat(filename, ".prolog");
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return "prolog";

  strcpy(filename, basename);
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return ""; /* no extension */

  snprintf(filename, MAXPATHLEN, "%s%s", basename, XSB_OBJ_EXTENSION_STRING);
  /*  +1 skips the "."   */
  if (! stat(filename, &fileinfo) && not_a_dir(fileinfo)) return XSB_OBJ_EXTENSION_STRING+1;

  return NULL; /* signifies that the search was unsuccessful */
}

/*=========================================================================*/

xsbBool almost_search_module(CTXTdeclc char *filename)
{
  char *fullname, *dir, *basename, *extension;
  struct stat fileinfo;

  fullname = tilde_expand_filename(filename);
  parse_filename(fullname, &dir, &basename, &extension);
  if (! strcmp(filename, basename)) { /* only a module name given */
    /* this is the case that we have to resort to a non-deterministic
     * search in Prolog using the predicate libpath/1; that's why the
     * function is called "almost_".  Note that arguments 4 and 5 are
     * left unbound here.
     */
    ctop_string(CTXTc 2, dir);
    ctop_string(CTXTc 3, filename); /* Mod = FileName */
  } else { /* input argument is a full file name */
    if (! strcmp(extension, "")) {
      extension = existing_file_extension(fullname);
      if (! extension) return FALSE; /* file was not found */
      extension = string_find(extension,1);
    } else {
      if (stat(fullname, &fileinfo) && !strcmp(dir,"")) {
	/* file not found, so let search through dirs try to find it */ 
	ctop_string(CTXTc 2, dir);
	ctop_string(CTXTc 3, basename);
	ctop_string(CTXTc 4, extension);
	return TRUE;
      }
    }
    if (! strcmp(dir, "")) {
      char dot_dir[MAXPATHLEN];
      dot_dir[0] = '.';
      dot_dir[1] = SLASH;
      dot_dir[2] = '\0';
      ctop_string(CTXTc 2, dot_dir);
      strcat(dot_dir, basename); /* dot_dir is updated here */
      ctop_string(CTXTc 5, dot_dir);
    } else {
      char dirtmp[MAXPATHLEN];
      ctop_string(CTXTc 2, dir);
      strcpy(dirtmp,dir);
      strcat(dirtmp, basename);
      ctop_string(CTXTc 5, dirtmp);
    }
    ctop_string(CTXTc 3, basename);
    ctop_string(CTXTc 4, extension);
  }
  return TRUE;
}

/*=========================================================================*/
