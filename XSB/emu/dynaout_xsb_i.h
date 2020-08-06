/* File:      dynaout_xsb_i.h
** Author(s): Jiyang Xu, Kostis Sagonas, Steve Dawson
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
** $Id: dynaout_xsb_i.h,v 1.20 2012-10-12 16:42:57 tswift Exp $
** 
*/


#include <a.out.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
/* wind2unix.h must be included after sys/stat.h */
#include "wind2unix.h"
#include <errno.h>
#include <stdio.h>
#include <stdio.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "memory_xsb.h"
#include "inst_xsb.h"
#include "psc_xsb.h"
#include "flags_xsb.h"
#include "error_xsb.h"
#include "io_builtins_xsb.h"
#include "string_xsb.h"
#include "extensions_xsb.h"
#include "basictypes.h"

#define BUFFEXTRA 1024

char tfile[128];	/* uniquely-named tmp file for "ld" */

/*----------------------------------------------------------------------*/

xsbBool dummy()
{
    xsb_error("Trying to use an undefined foreign procedure");
    return FALSE;
}

/*----------------------------------------------------------------------*/

static void dyn_link_all(char *symtab, Psc cur_mod)
{
  int count, i;
  char *ptr, *strtab;
  struct nlist *sym_entry;
  char *name;
  Pair search_ptr;

  count = *(int *)(symtab+4);
  symtab += 8;
  strtab = symtab + count*sizeof(struct nlist);
  search_ptr = (Pair)get_data(cur_mod);
  while (search_ptr) {
    name = get_name(search_ptr->psc_ptr);
/* Jiyang changed it to the form ``module_pred'':
|    sprintf(name, "%s_%s", get_name(cur_mod), get_name(search_ptr->psc_ptr));
 */
    if (get_type(search_ptr->psc_ptr) == T_FORN) {
      for (i=0; i<count; i++) {
	sym_entry = (struct nlist *)(symtab + i * sizeof(struct nlist));
	ptr = strtab + sym_entry->n_un.n_strx;
	if (*ptr++ == '_' && strcmp(name, ptr)==0) { 
	  set_forn(search_ptr->psc_ptr, (byte *)(sym_entry->n_value));
	  break;
	}
      }
      if (i>= count) {          /* does not find the name */
	  xsb_warn(CTXTc "Cannot find foreign procedure %s", name);
	  set_forn(search_ptr->psc_ptr, (byte *)(dummy));
      }
    }
    search_ptr = search_ptr->next;
  }
}

/*----------------------------------------------------------------------*/

// TLS: changing 128 -> MAXFILENAME (= 256).  I think this will work
// for Cygwin 1.7 and later.
static byte *load_obj_dyn(CTXTdeclc char *pofilename, Psc cur_mod, char *ld_option)
{
  int buffsize, fd, loadsize;
  byte *start;	/* Changed from int -- Kostis.	*/
  int *loc;	/* Changed from int -- Kostis.	*/
  struct exec header;
  char buff[3*MAXPATHLEN], subfile[MAXPATHLEN];
  struct stat statbuff;
  char  *file_extension_ptr;
  
  if (MAXFILENAME < snprintf(tfile, MAXFILENAME, "/tmp/xsb-dyn.%d", (int)getpid()))
    xsb_abort("Cannot load foreign file: process id too long\n");
  
  /* first step: get the header entries of the *.o file, in order	*/
  /* to obtain the size of the object code and then allocate space	*/
  /* for it.							*/
  if (strlen(pofilename) >= MAXFILENAME-1) return 0;

  /* create filename.o */
  strcpy(subfile, pofilename);
  file_extension_ptr = xsb_strrstr(subfile, XSB_OBJ_EXTENSION_STRING);
  /* replace the OBJ file suffix with the "o" suffix */
  strcpy(file_extension_ptr+1, "o");

  fd = open(subfile, O_RDONLY, 0);
  if (fd < 0) {
    xsb_error("Cannot find the C object file: %s", subfile);
    return 0;
  }
  read(fd, &header, sizeof(struct exec));
  close(fd);
  
  /* second step: run incremental ld and generate a temporary 	*/
  /* object file (including orginal *.o and libraries) ready to be	*/
  /* read in.							*/
  buffsize = header.a_text + header.a_data + header.a_bss;
  start = mem_alloc(buffsize,FOR_CODE_SPACE);
  /* The "-T hex" option of ld starts the text segment at location	*/
  /* hex. Specifying -T is the same as using the -Ttext option.	*/
  sprintf(buff, "/usr/bin/ld -N -A %s -T %x -o %s %s %s -lc",
	  executable_path_gl, (int)start, tfile, subfile, ld_option);
  system(buff);
  
  /* third step: check if the size of the buffer just allocated is	*/
  /* big enough to load the object (when the object code uses other	*/
  /* libraries, the buffer may not be big enough). If this is the	*/
  /* case, redo the second step with a bigger buffer.		*/
  fd = open(tfile, O_RDONLY, 0);
  if (fd < 0) {
    xsb_error("The file is not generated by the loader");
    return 0;
  }
  read(fd, &header, sizeof(struct exec));
  loadsize = header.a_text + header.a_data + header.a_bss;
  if (loadsize > buffsize) {
    close(fd);			/* need to reallocate buffer */
    mem_dealloc(start, buffsize,FOR_CODE_SPACE);
    start = mem_alloc(loadsize+BUFFEXTRA,FOR_CODE_SPACE);
    sprintf(buff, "/usr/bin/ld -N -A %s -T %x -o %s %s %s -lc",
	    executable_path_gl, (int)start, tfile, subfile, ld_option);
    system(buff);
    fd = open(tfile, O_RDONLY, 0);
    read(fd, &header, sizeof(struct exec));
  }
  
  /* fourth step: read in the intermediate object files.		*/
  /* load text and data segment */
  loadsize = header.a_text + header.a_data;
  lseek(fd, N_TXTOFF(header), 0);
  read(fd, start, loadsize);
  /* load symbol table and string table */
  fstat(fd, &statbuff);
  loadsize = statbuff.st_size-N_SYMOFF(header);
  loc = (int *)mem_alloc(loadsize+8,FOR_CODE_SPACE);
  *loc = loadsize+8;
  *(loc+1) = header.a_syms/sizeof(struct nlist);
  lseek(fd, N_SYMOFF(header), 0);
  read(fd, loc+2, loadsize);
  close(fd);
  
  /* fifth step: link C procedure names with Prolog names.		*/
  dyn_link_all((char *)loc, cur_mod);
  mem_dealloc((byte *)loc, loadsize+8,FOR_CODE_SPACE);
  return (byte *)4;
}
