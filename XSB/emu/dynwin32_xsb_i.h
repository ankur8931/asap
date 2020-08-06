/* File:      dynwin32_xsb_i.h
** Author(s): Luis Castro
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999, 2000
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
** $Id: dynwin32_xsb_i.h,v 1.27 2012-10-12 16:42:57 tswift Exp $
** 
*/


#include <stdio.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
/* wind2unix.h must be included after sys/stat.h */
#include "wind2unix.h"
#include <errno.h>
#include <string.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "memory_xsb.h"
#include "error_xsb.h"
#include "inst_xsb.h"
#include "psc_xsb.h"
#include "string_xsb.h"
#include "extensions_xsb.h"
#include "xsb_config.h"
#include "basictypes.h"

#define BUFFEXTRA 1024

extern char *xsb_config_file_gl;

/*----------------------------------------------------------------------*/

xsbBool dummy()
{
    xsb_error("Trying to use an undefined foreign procedure");
    return FALSE;
}

/*----------------------------------------------------------------------*/

// construct a path to config\bin\cfile_name.dll by removing "lib\xsb_configuration.P"
// from xsb_config_file location and appending "bin\cfile_name.dll"
static char *create_bin_dll_path(char *xsb_config_file_location, char *dll_file_name, size_t *dirlen){
  char *xsb_bin_dir;
  size_t char_count;
  char_count = strlen(xsb_config_file_location)-strlen("lib")-1-strlen("xsb_configuration.P");
  *dirlen = sizeof(char)*(char_count+strlen(dll_file_name)+5); // 5 stands for bin\\ + null
  xsb_bin_dir = mem_alloc(*dirlen,FOR_CODE_SPACE);
  strncpy(xsb_bin_dir, xsb_config_file_location,char_count);
  xsb_bin_dir[char_count]='\0';
  strcat(xsb_bin_dir, "bin");
  char_count += 3;
  xsb_bin_dir[char_count]=SLASH;
  xsb_bin_dir[char_count+1]='\0';
  strcat(xsb_bin_dir, dll_file_name);
  return xsb_bin_dir;
}

static byte *load_obj_dyn(CTXTdeclc char *pofilename, Psc cur_mod, char *ld_option)
{
  char	*name;
#ifndef WIN64
#ifdef XSB_DLL
  char tempname[MAXFILENAME];
  size_t  tempsize;
#endif
#endif
  Pair	search_ptr;
  char	sofilename[MAXFILENAME];
  HMODULE handle;
  void	*funcep;
  char  *file_extension_ptr;
  xsbBool	dummy();
  char *basename_ptr;
  char *xsb_bin_dir;
  size_t dirlen;
  
  /* (1) create filename.so */
  
  strcpy(sofilename, pofilename);

  file_extension_ptr = xsb_strrstr(sofilename, XSB_OBJ_EXTENSION_STRING);
  /* replace the OBJ file suffix with the so suffix */
  strcpy(file_extension_ptr+1, "dll");
  
  /* (2) open the needed object */
  if (( handle = LoadLibrary(sofilename)) == 0 ) {
    // if DLL is not found in c file's path
    // look for it in bin path, if still not found
    // let OS find it
    basename_ptr = strrchr(sofilename, SLASH); // get \file.dll
    if(basename_ptr != NULL){
      basename_ptr = basename_ptr + 1;
      xsb_bin_dir = create_bin_dll_path(xsb_config_file_gl, basename_ptr,&dirlen);
      if(( handle = LoadLibrary(xsb_bin_dir)) == 0 ){
	if (( handle = LoadLibrary(basename_ptr)) == 0 ) {
	  mem_dealloc(xsb_bin_dir,dirlen,FOR_CODE_SPACE);
	  xsb_warn(CTXTc "Cannot load library %s or %s; error #%d",basename_ptr,sofilename,GetLastError());
	  return 0;
	}
      }
      mem_dealloc(xsb_bin_dir,dirlen,FOR_CODE_SPACE);
    }
  }
  
  /* (3) find address of function and data objects
  **
  ** dyn_link_all(loc, cur_mod);
  */
  
  search_ptr = (Pair)get_data(cur_mod);
  
  while (search_ptr) {
    name = get_name(search_ptr->psc_ptr);
#ifndef WIN64
#ifdef XSB_DLL
    tempname[0] = '_';
    /*    tempname[1] = '_'; */
    strcpy(tempname+1,name);
    tempsize=strlen(tempname);
    tempname[tempsize++] = '@';
#ifndef MULTI_THREAD
    tempname[tempsize++] = '0';
#else
    tempname[tempsize++] = '4';
#endif
    tempname[tempsize++] = '\0';
    name = tempname;
#endif
#endif
    if (get_type(search_ptr->psc_ptr) == T_FORN) {
      if ((funcep = (int (*)) GetProcAddress(handle, name)) == NULL) {
	xsb_warn(CTXTc "Cannot find foreign procedure %s (error #%d)", name,GetLastError());
	set_forn(search_ptr->psc_ptr, (byte *)(dummy));
      } else { 
	set_forn(search_ptr->psc_ptr, (byte *)(funcep));
      }
      
    }
    search_ptr = search_ptr->next;
  }
  return (byte *)4;
}




