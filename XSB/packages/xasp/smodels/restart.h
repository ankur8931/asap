// Copyright 2006 by Patrik Simons
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
// patrik.simons@neotide.fi
#ifndef RESTART_H
#define RESTART_H

#include "atomrule.h"

class Restart
{
public:
  Restart();

  unsigned long restart_after();

  int level;
  int current;
};

class RestartNode
{
public:
  RestartNode();
  ~RestartNode();

  void add_node(Atom **stack, Atom **end, long top);

  bool backtracked : 1;
  bool pebble : 1;
  bool stop : 1;
  long length;
  Atom **computed;
  bool *computed_positive;
  long stack_top;
  RestartNode *parent;
  RestartNode *child;
  RestartNode *sibling;
};

extern RestartNode *find_restart(RestartNode *n);

#endif
