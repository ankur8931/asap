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
#include "atomrule.h"
#include "stack.h"
#include "smodels.h"

using namespace std;

Dcl::Dcl (Smodels *s)
  : smodels (s), program (&s->program)
{
  tmpfalse = 0;
  tmpfalsetop = 0;
}

Dcl::~Dcl ()
{
  delete[] tmpfalse;
}

void
Dcl::init ()
{
  tmpfalse = new Atom *[program->number_of_atoms];
  tmpfalsetop = 0;
}

void
Dcl::setup ()
{
  for (Node *n = program->rules.head (); n; n = n->next)
    n->rule->fireInit ();
  // Find atoms that are true.
  while (!program->queue.empty ())
    {
      Atom *a = program->queue.pop ();
      if (a->closure == false)
	a->setTrue ();
      a->in_queue = false;
    }
  // Set not true atoms to explicitly false.
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      Atom *a = n->atom;
      if (a->closure == false)
	{
	  if (a->Bpos)
	    {
	      smodels->set_conflict ();
	      return;
	    }
	  else
	    {
	      smodels->setToBFalse (a);
	      // Rule::inactivate () changes upper
	      // and it shouldn't be changed here.
	      for (Follows *f = a->pos; f != a->endPos; f++)
		f->r->backtrackUpper (f);
	    }
	}
    }
}

void
Dcl::revert ()
{
  for (Node *n = program->rules.head (); n; n = n->next)
    n->rule->unInit ();
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      Atom *a = n->atom;
      a->source = 0;
      a->headof = a->endHead - a->head;
      a->closure = false;
      a->Bpos = false;
      a->Bneg = false;
      a->visited = false;
      a->guess = false;
      a->backtracked = false;
      a->forced = false;
      a->in_etrue_queue = false;
      a->in_efalse_queue = false;
      a->in_queue = false;
    }
}

void
Dcl::dcl ()
{
  Atom *a;
  for (;;)
    {
      // If an atom is explicitly set after it's on the queue then
      // this test misses the conflict
      if (program->conflict)
	break;
      // Propagate explicit truth. (Fitting Semantics)
      if (!program->equeue.empty ())
	{
	  a = program->equeue.pop ();
	  if ((a->Bneg | a->in_efalse_queue)
	      && (a->Bpos | a->in_etrue_queue))
	    {
	      a->in_etrue_queue = false;
	      a->in_efalse_queue = false;
	      break;
	    }
	  else if (a->in_efalse_queue)
	    {
	      if (a->Bneg == false)
		smodels->setToBFalse (a);
	    }
	  else if (a->Bpos == false)
	    smodels->setToBTrue (a);
	  a->in_etrue_queue = false;
	  a->in_efalse_queue = false;
	}
      else if (program->queue.empty ())
	return;
      // Find conflicts in order to guarantee soundness of smodels
      // This part prevents dcl from being incrementally linear.
      else if (propagateFalse ())
	break;
    }
  program->conflict = false;
  smodels->set_conflict ();
}

bool
Dcl::propagateFalse ()
{
  Atom *a;
  Follows *f;
  long i;
  tmpfalsetop = 0;
  // Propagate falsehood.
  while (!program->queue.empty ())
    {
      a = program->queue.pop ();
      a->source = 0;
      // If a->closure is false, then the descendants of a are
      // in the queue.
      if (a->closure == true && a->Bneg == false)
	{
	  for (f = a->pos; f != a->endUpper; f++)
	    f->r->propagateFalse (f);
	  a->closure = false;
	  tmpfalse[tmpfalsetop] = a;
	  tmpfalsetop++;
	}
      a->in_queue = false;
    }
  // Some atoms now false should be true.
  for (i = 0; i < tmpfalsetop; i++)
    if (tmpfalse[i]->headof)
      for (f = tmpfalse[i]->head; f != tmpfalse[i]->endHead; f++)
	if (f->r->isUpperActive ())
	  {
	    tmpfalse[i]->source = f->r;
	    tmpfalse[i]->queue_push ();
	    break;
	  }
  // Propagate truth to find them all.
  while (!program->queue.empty ())
    {
      a = program->queue.pop ();
      if (a->closure == false && a->Bneg == false)
	{
	  for (f = a->pos; f != a->endUpper; f++)
	    f->r->propagateTrue (f);
	  a->closure = true;
	}
      a->in_queue = false;
    }
  // Those atoms now false can be set to explicitly false.
  for (i = 0; i < tmpfalsetop; i++)
    {
      a = tmpfalse[i];
      if (a->closure == false)
	if (a->Bpos)
	  {
	    // Conflict found, we reset the f->upper variables that are
	    // not reset while backtracking.
	    for (; i < tmpfalsetop; i++)
	      {
		a = tmpfalse[i];
		if (a->closure == false)
		  {
		    for (f = a->pos; f != a->endUpper; f++)
		      f->r->backtrackUpper (f);
		    a->closure = true;
		  }
	      }
	    return true;
	  }
	else
	  {
	    smodels->setToBFalse (a);
	    // Rule::inactivate () changes upper and it shouldn't be
	    // changed for the atoms below endUpper.
	    for (f = a->pos; f != a->endUpper; f++)
	      f->r->backtrackUpper (f);
	  }
    }
  return false;
}

int
Dcl::path (Atom *a, Atom *b)
{
  if (a == b)
    return 1;
  Follows *f;
  Atom *c;
  for (Node *n = program->rules.head (); n; n = n->next)
    n->rule->visited = false;
  for (Node *n = program->atoms.head (); n; n = n->next)
    n->atom->visited = false;
  a->queue_push ();
  a->visited = true;
  while (!program->queue.empty ())
    {
      c = program->queue.pop ();
      c->in_queue = false;
      if (c == b)
	{
	  while (!program->queue.empty ())
	    {
	      c = program->queue.pop ();
	      c->in_queue = false;
	    }
	  return 1;
	}
      for (f = c->head; f != c->endHead; f++)
	if (!f->r->visited && !f->r->isInactive ())
	  f->r->search (c);
      for (f = c->pos; f != c->endPos; f++)
	if (!f->r->visited && !f->r->isInactive ())
	  f->r->search (c);
      for (f = c->neg; f != c->endNeg; f++)
	if (!f->r->visited && !f->r->isInactive ())
	  f->r->search (c);
    }
  return 0;
}

void
Dcl::reduce (bool strongly)
{
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      Atom *a = n->atom;
      a->reduce (strongly);
    }
  for (Node *n = program->rules.head (); n; n = n->next)
    {
      if (n->rule->isInactive ())
	continue;   // Nothing points at this rule now
      n->rule->reduce (strongly);
    }
}

void
Dcl::unreduce ()
{
  for (Node *n = program->atoms.head (); n; n = n->next)
    n->atom->unreduce ();
  for (Node *n = program->rules.head (); n; n = n->next)
    n->rule->unreduce ();
}

void
Dcl::print ()
{
  cout << "bdcl:" << endl << "True:";
  for (Node *n = program->atoms.head (); n; n = n->next)
    if (n->atom->closure == true)
      cout << ' ' << n->atom->atom_name ();
  cout << endl << "BTrue:";
  for (Node *n = program->atoms.head (); n; n = n->next)
    if (n->atom->Bpos)
      cout << ' ' << n->atom->atom_name ();
  cout << endl << "BFalse:";
  for (Node *n = program->atoms.head (); n; n = n->next)
    if (n->atom->Bneg)
      cout << ' ' << n->atom->atom_name ();
  cout << endl;
}
