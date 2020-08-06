/* File:      trie_defs.h
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
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License
** for more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** 
*/

#define PRGE_TRIE                  0
#define PRAS_TRIE                  1
#define SHAS_TRIE                  2
#define TRIE_TYPE(PROPS)           (PROPS & 0xf)
#define PRIVATE_TRIE(PROPS)        ((PROPS & 0xf) < 2)

#define INCREMENTAL_TRIE           0x10
#define NON_INCREMENTAL_TRIE           0
#define IS_INCREMENTAL_TRIE(TYPE)     ((TYPE & 0xf0) > 0)
