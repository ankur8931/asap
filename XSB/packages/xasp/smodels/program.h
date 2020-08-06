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
#ifndef PROGRAM_H
#define PROGRAM_H

#include "queue.h"
#include "list.h"

class OptimizeRule;

class Program
{
public:
  Program ();
  ~Program ();
  void init ();
  void print ();
  void print_internal (long models = 1);
  void set_optimum ();

  Queue equeue;
  Queue queue;

  AtomList atoms;
  RuleList rules;
  OptimizeRule *optimize;
  long number_of_atoms;
  long number_of_rules;

  bool conflict;
};

#endif



