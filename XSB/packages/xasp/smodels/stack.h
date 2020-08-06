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
#ifndef STACK_H
#define STACK_H

#include <assert.h>

class Atom;

class Stack
{
public:
  Stack (long size = 0);
  ~Stack ();
  void Init (long size);

  void push (Atom *item);
  Atom *pop ();
  bool empty ();
  bool full ();
  void reset ();

  Atom **stack;
  long top;
  long last;
};

inline void
Stack::push (Atom *item)
{
  assert (top < last);
  stack[top] = item;
  top++;
}

inline Atom *
Stack::pop ()
{
  assert (top > 0);
  top--;
  return stack[top];
}

inline bool
Stack::empty ()
{
  return top == 0;
}

inline bool
Stack::full ()
{
  return top == last;
}

inline void
Stack::reset ()
{
  top = 0;
}

#endif
