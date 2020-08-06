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
#ifndef QUEUE_H
#define QUEUE_H

#include <assert.h>

class Atom;

class Queue
{
public:
  Queue (long size = 0);
  ~Queue ();
  void Init (long size);

  void reset ();
  void push (Atom *n);
  Atom *pop ();
  int empty ();

  Atom **queue;
  long last;
  long top;
  long bottom;
};

inline int
Queue::empty ()
{
  return top == bottom;
}

inline void
Queue::reset ()
{
  top = 0;
  bottom = 0;
}

inline void
Queue::push (Atom *n)
{
  queue[bottom] = n;
  bottom++;
  if (bottom > last)
    bottom = 0;
  assert (top != bottom);
}

inline Atom *
Queue::pop ()
{
  assert (top != bottom);
  Atom *n = queue[top];
  top++;
  if (top > last)
    top = 0;
  return n;
}

#endif
