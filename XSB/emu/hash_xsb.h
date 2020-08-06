/* File:      hash_xsb.h
** Author(s): Ernie Johnson
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
** $Id: hash_xsb.h,v 1.12 2011-05-22 16:02:22 tswift Exp $
** 
*/


/*
 *  Structure of a dynamic hash table for dynamic objects.
 */

typedef struct hash_table {
  UInteger size;
  UInteger contains;
  void **table;
} Hash_Table;


/*
 *  Non-numerical objects are stored in these dynamic hash tables.
 */

extern Hash_Table symbol_table;
extern Hash_Table string_table;


/*
 *  Macros for hash table overflow checks.
 */

#ifndef DEBUG_ASSERTIONS
#define string_table_increment_and_check_for_overflow               \
  {                                                                 \
    string_table.contains++;                                        \
    if (string_table.contains > (string_table.size << 2))           \
      expand_string_table();                                        \
  }

#define symbol_table_increment_and_check_for_overflow               \
  {                                                                 \
    symbol_table.contains++;                                        \
    if (symbol_table.contains > (symbol_table.size << 2))           \
      expand_symbol_table();                                        \
  }

#else

#define string_table_increment_and_check_for_overflow               \
  {                                                                 \
    string_table.contains++;                                        \
    if (string_table.contains > (string_table.size << 2)) {         \
      printf("\nBEFORE:\n");                                        \
      string_table_stats();                                         \
      expand_string_table();                                        \
      printf("\nAFTER:\n");                                         \
      string_table_stats();                                         \
    }                                                               \
  }

#define symbol_table_increment_and_check_for_overflow               \
  {                                                                 \
    symbol_table.contains++;                                        \
    if (symbol_table.contains > (symbol_table.size << 2)) {         \
      printf("\nBEFORE:\n");                                        \
      symbol_table_stats();                                         \
      expand_symbol_table();                                        \
      printf("\nAFTER:\n");                                         \
      symbol_table_stats();                                         \
    }                                                               \
  }
#endif


/*
 *  Function prototypes for dynamic hash table use.
 */

UInteger next_prime(UInteger some_integer);
//size_t  next_prime(UInteger some_integer);
UInteger hash(const char *atom_name, byte arity, UInteger hash_table_size);
void  expand_symbol_table();
void  expand_string_table();
void  symbol_table_stats();
void  string_table_stats();
