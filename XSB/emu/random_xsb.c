/* File:      random_xsb.c
** Author(s): Baoqiu Cui
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
** $Id: random_xsb.c,v 1.20 2011-06-06 20:20:29 dwarren Exp $
** 
*/

/*
 * This is the implementation of Wichmann-Hill Random Number Generator:
 *
 *     B. A. Wichmann and I. D. Hill.  Algorithm AS 183: An efficient and
 *     portable pseudo-random number generator.  Applied Statistics 31
 *     (1982), 188--190.
 *
 * See also:
 *
 *     Correction to Algorithm AS 183.  Applied Statistics 33 (1984) 123,
 *
 *     A. I. McLeod.  A remark on Algorithm AS 183.  Applied Statistics 34
 *     (1985), 198--200.
 */

#include <stdio.h>
#include <stdlib.h>

#include "xsb_config.h"
#include "context.h"
#include "cell_xsb.h"
#include "cinterf.h"
#include "deref.h"
#include "register.h"
#include "ptoc_tag_xsb_i.h"
#include "error_xsb.h"
#include "memory_xsb.h"

#ifndef MULTI_THREAD
struct random_seeds_t *random_seeds = 0;
#endif

struct random_seeds_t *init_random_seeds() {

  struct random_seeds_t *seeds;
  seeds = (struct random_seeds_t *)mem_alloc(sizeof(struct random_seeds_t),OTHER_SPACE);
  if (!seeds) xsb_abort("No space for random seeds!!");
  seeds->IX = 6293;
  seeds->IY = 21877;
  seeds->IZ = 7943;
  seeds->TX = 1.0/30269.0;
  seeds->TY = 1.0/30307.0;
  seeds->TZ = 1.0/30323.0;
  return seeds;
}

/*
 * Returns a float number within the range [0.0, 1.0) in reg 2.
 * ret_random() returns FALSE if there is an error, TRUE if everything is OK.
 */
int ret_random(CTXTdecl) {
  short X, Y, Z;
  double T;
  Cell term;

  if (!random_seeds) random_seeds = init_random_seeds();

  X = (random_seeds->IX*171) % 30269;
  Y = (random_seeds->IY*172) % 30307;
  Z = (random_seeds->IZ*170) % 30323;
  T = X*random_seeds->TX + Y*random_seeds->TY + Z*random_seeds->TZ;
  random_seeds->IX = X;
  random_seeds->IY = Y;
  random_seeds->IZ = Z;

  term = ptoc_tag(CTXTc 2);
  if (isref(term)) {
    ctop_float(CTXTc 2, (Float)(T - (int)T));
    return TRUE;
  }
  else return FALSE;
}    

/*
 * Unifies reg 2,3,4 with the random seeds IX, IY, IZ.  
 * getrand() returns FALSE if there is an error, TRUE if everything is OK.
 */
int getrand(CTXTdecl) {
  Cell term;

  if (!random_seeds) random_seeds = init_random_seeds();
  term = ptoc_tag(CTXTc 2);
  if (isref(term))
    ctop_int(CTXTc 2, random_seeds->IX);
  else if (!isointeger(term) || (oint_val(term) != random_seeds->IX))
    return FALSE;

  term = ptoc_tag(CTXTc 3);
  if (isref(term))
    ctop_int(CTXTc 3, random_seeds->IY);
  else if (!isointeger(term) || (oint_val(term) != random_seeds->IY))
    return FALSE;

  term = ptoc_tag(CTXTc 4);
  if (isref(term))
    ctop_int(CTXTc 4, random_seeds->IZ);
  else if (!isointeger(term) || (oint_val(term) != random_seeds->IZ))
    return FALSE;

  return TRUE;
}

/*
 * Sets the random seeds IX, IY and IZ using reg 2,3,4.  The type and
 * range checking are done in random.P.
 */
void setrand(CTXTdecl) {
  if (!random_seeds) random_seeds = init_random_seeds();
  random_seeds->IX = (short)ptoc_int(CTXTc 2);
  random_seeds->IY = (short)ptoc_int(CTXTc 3);
  random_seeds->IZ = (short)ptoc_int(CTXTc 4);
}
