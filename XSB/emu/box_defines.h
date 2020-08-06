/* File:      box_defines.h
** Author(s): Rojo
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
*/

#ifndef BOX_DEFINES_H
#define BOX_DEFINES_H

#include "xsb_config.h"


/*16 to give space for the most significant 16 bits of 64bit numbers when using boxed representation.*/
#define BOX_ID_OFFSET 16 

/**********************************
*   BOX ID's, to be placed in the 24th through 17th least significant
*   bits of the first integer in a boxed value.
***********************************/
#define ID_BOXED_INT 1
#define ID_BOXED_FLOAT 2

//TODO: define different shift constants in the BIT extracting macros below and alternate
//depending on architecture implementation of ints, doubles, floats, etc... 
#define LOW_24_BITS_MASK 0xffffff
#define LOW_16_BITS_MASK 0xffff

#define INT_LOW_24_BITS(value) (((UInteger)(value)) >> 24)


#ifndef FAST_FLOATS

//The macros below expect "float" to be a double float variable local to the thread it is envoked in. 
// Any caller must ensure that it isn't something such as a macro that produces a function call, 
// a constant, an expression, or a static/global variable (for thread-safety)

#ifdef BITS64

#define FLOAT_HIGH_16_BITS(float) ((((UInteger)(*((UInteger *)((void *)(& float)))))>>48) & LOW_16_BITS_MASK)

#define FLOAT_MIDDLE_24_BITS(float)					\
  (	(((UInteger)(*(((UInteger *)((void *)(& float))))))>>24) & LOW_24_BITS_MASK)

#define FLOAT_LOW_24_BITS(float) (((UInteger)(*(((UInteger *)((void *)(& float)))))) & LOW_24_BITS_MASK)

#else

#define FLOAT_HIGH_16_BITS(float) ((((UInteger)(*((UInteger *)((void *)(& float)))))>>16) & LOW_16_BITS_MASK)

#define FLOAT_MIDDLE_24_BITS(float)					\
		(	(((UInteger)(*(((UInteger *)((void *)(& float)))+1)))>>24) \
		    |	(((UInteger)(*((UInteger *)((void *)&(float)))) & LOW_16_BITS_MASK)<<8)	\
		)										

#define FLOAT_LOW_24_BITS(float) (((UInteger)(*(((UInteger *)((void *)(& float)))+1))) & LOW_24_BITS_MASK)

#endif  /* BITS64 */

#endif  /* FAST_FLOATS */
 
#endif  /* BOX_DEFINES */
