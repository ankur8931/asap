// Copyright 1998 by Patrik Simons
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.
//
// Patrik.Simons@hut.fi

//#define PRINTALL
//#define PRINTDCL
//#define PRINTCONFLICT
//#define PRINTBACKTRACK
//#define PRINTBF
//#define PRINTPICK
//#define PRINTSTACK
//#define PRINTPROGRAM
// ------------------

#ifdef PRINTALL
#undef PRINTDCL
#undef PRINTCONFLICT
#undef PRINTBACKTRACK
#undef PRINTBF
#undef PRINTPICK
#define PRINTDCL
#define PRINTCONFLICT
#define PRINTBACKTRACK
#define PRINTBF
#define PRINTPICK
#endif

#ifdef PRINTDCL
#define PRINT_DCL(x) x
#else
#define PRINT_DCL(x)
#endif

#ifdef PRINTCONFLICT
#define PRINT_CONFLICT(x) x
#else
#define PRINT_CONFLICT(x)
#endif

#ifdef PRINTBACKTRACK
#define PRINT_BACKTRACK(x) x
#else
#define PRINT_BACKTRACK(x)
#endif

#ifdef PRINTBF
#define PRINT_BF(x) x
#else
#define PRINT_BF(x)
#endif

#ifdef PRINTPICK
#define PRINT_PICK(x) x
#else
#define PRINT_PICK(x)
#endif

#ifdef PRINTSTACK
#define PRINT_STACK(x) x
#else
#define PRINT_STACK(x)
#endif

#ifdef PRINTPROGRAM
#define PRINT_PROGRAM(x) x
#else
#define PRINT_PROGRAM(x)
#endif
