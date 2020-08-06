/* File:      dynload.c
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
** $Id: dynload.c,v 1.14 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


#include "xsb_config.h"  /* This should appear BEFORE the ifdef! */

extern char executable_path_gl[];         /* from orient_xsb.c */

#ifdef FOREIGN

#ifdef FOREIGN_ELF
#include "dynelf_xsb_i.h"
#else
#ifdef FOREIGN_AOUT
#include "dynaout_xsb_i.h"
#else
#ifdef FOREIGN_WIN32
#include "dynwin32_xsb_i.h"
#endif /* FOREIGN_WIN32 */
#endif /* FOREIGN_AOUT */
#endif /* FOREIGN_ELF */

/*----------------------------------------------------------------------*/

byte *load_obj(CTXTdeclc char *pofilename, Psc cur_mod, char *ld_option)
{
/*
    if (static_foreign_libraries(get_name(cur_mod)))
        return load_obj_sta(cur_mod);
    else
 */
        return load_obj_dyn(CTXTc pofilename, cur_mod, ld_option);
}

/*----------------------------------------------------------------------*/

#endif /* FOREIGN */
