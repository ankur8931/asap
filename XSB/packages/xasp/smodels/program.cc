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
#include <iostream>
#include "program.h"
#include "atomrule.h"

using namespace std;

Program::Program ()
{
  optimize = 0;
  number_of_atoms = 0;
  number_of_rules = 0;
  conflict = false;
}

Program::~Program ()
{
}

void
Program::init ()
{
  equeue.Init (number_of_atoms);
  queue.Init (number_of_atoms);
}

void
Program::set_optimum ()
{
  for (OptimizeRule *r = optimize; r; r = r->next)
    r->setOptimum ();
}

void
Program::print ()
{
  Node *n;
  for (n = rules.head (); n; n = n->next)
    {
      if (n->rule->isInactive ())
	continue;
      n->rule->print ();
    }
  for (n = atoms.head (); n; n = n->next)
    if (n->atom->computeTrue || n->atom->computeFalse)
      break;
  if (n == 0)
    return;
  cout << "compute { ";
  bool comma = false;
  for (n = atoms.head (); n; n = n->next)
    {
      if (n->atom->computeTrue)
	{
	  if (comma)
	    cout << ", ";
	  cout << n->atom->atom_name ();
	  comma = true;
	}
      if (n->atom->computeFalse)
	{
	  if (comma)
	    cout << ", ";
	  cout << "not " << n->atom->atom_name ();
	  comma = true;
	}
    }
  cout << " }" << endl;
}

void
Program::print_internal (long models)
{
  Node *n;
  long i = 1;
  for (n = atoms.head (); n; n = n->next)
    {
      n->atom->posScore = i;
      n->atom->negScore = 0;
      i++;
    }
  for (n = rules.head (); n; n = n->next)
    {
      if (n->rule->isInactive ())
	continue;
      n->rule->print_internal ();
    }
  cout << 0 << endl;
  for (n = atoms.head (); n; n = n->next)
    if (n->atom->negScore && n->atom->name)
      cout << n->atom->posScore << ' ' << n->atom->name << endl;
  cout << 0 << endl;
  cout << "B+" << endl;
  for (n = atoms.head (); n; n = n->next)
    if (n->atom->negScore && (n->atom->computeTrue || n->atom->Bpos))
      cout << n->atom->posScore << endl;
  cout << 0 << endl;
  cout << "B-" << endl;
  for (n = atoms.head (); n; n = n->next)
    if (n->atom->negScore && (n->atom->computeFalse || n->atom->Bneg))
      cout << n->atom->posScore << endl;
  cout << 0 << endl;
  cout << models << endl;
}
