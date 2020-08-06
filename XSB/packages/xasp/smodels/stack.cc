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
#include "stack.h"

Stack::Stack (long size)
{
  if (size == 0)
    {
      stack = 0;
      return;
    }
  stack = new Atom *[size];
  top = 0;
  last = size;
}

Stack::~Stack ()
{
  delete[] stack;
}

void
Stack::Init (long size)
{
  delete[] stack;
  stack = new Atom *[size];
  top = 0;
  last = size;
}
