// Copyright 1999 by Patrik Simons
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
#include <limits.h>
#include "atomrule.h"
#include "stack.h"
#include "smodels.h"
#include "dcl.h"

#define lowlink posScore
#define dfnumber negScore
#define marked dependsonTrue
#define realnant dependsonFalse


class Denant
{
public:
  Denant (long size);
  ~Denant () {};
  void strong (Atom *);
  void do_strong (Atom *a, Atom *b, bool negatively);
  void do_strong (Atom *a, Rule *r, bool negatively);

  long count;
  long size_of_searchspace;
  long number_of_denants;
  Stack stack;
};

Denant::Denant (long size)
{
  stack.Init (size);
  count = 0;
  size_of_searchspace = 0;
  number_of_denants = 0;
}

inline void
Denant::do_strong (Atom *a, Atom *b, bool negatively)
{
  if (!b->visited)
    {
      strong (b);
      if (a->lowlink >= b->lowlink)
	{
	  a->lowlink = b->lowlink;
	  a->realnant |= negatively;
	}
    }
  else if (!b->marked && a->lowlink >= b->dfnumber)
    {
      a->lowlink = b->dfnumber;
      a->realnant |= negatively;
    }
}

void
Denant::do_strong (Atom *a, Rule *r, bool negatively)
{
  if (r->isInactive ())
    return;
  switch (r->type)
    {
    case BASICRULE:
    case CONSTRAINTRULE:
    case WEIGHTRULE:
      {
	Atom *b = ((HeadRule *)r)->head;
	do_strong (a, b, negatively);
	break;
      }
    case CHOICERULE:
      {
	ChoiceRule *cr = (ChoiceRule *)r;
	for (Atom **h = cr->head; h != cr->hend; h++)
	  do_strong (a, *h, negatively);
	break;
      }
    case GENERATERULE:
      {
	GenerateRule *gr = (GenerateRule *)r;
	for (Atom **h = gr->head; h != gr->hend; h++)
	  do_strong (a, *h, negatively);
	break;
      }
    case OPTIMIZERULE:
    case ENDRULE:
      break;
    }
}

void
Denant::strong (Atom *a)
{
  a->visited = true;
  a->dfnumber = count;
  a->lowlink = count;
  count++;
  if (a->Bneg || a->Bpos)
    {
      a->marked = true;
      return;
    }
  stack.push (a);
  for (Follows *f = a->pos; f != a->endPos; f++)
    do_strong (a, f->r, false);
  for (Follows *f = a->neg; f != a->endNeg; f++)
    do_strong (a, f->r, true);
  for (Follows *f = a->head; !a->realnant && f != a->endHead; f++)
    if ((f->r->type == CHOICERULE || f->r->type == GENERATERULE) &&
	!f->r->isInactive ())
      a->realnant = true;
  if (a->lowlink == a->dfnumber)
    {
      Atom *b;
      do {
	b = stack.pop ();
	b->marked = true;
	if (b->isnant)
	  {
	    if (b->realnant == false)
	      {
		b->isnant = false;
		number_of_denants++;
	      }
	    else
	      size_of_searchspace++;
	  }
      } while (a != b);
    }
}

// Only the atoms that appear negatively in a loop can influence the
// stable models.
void
Dcl::denant ()
{
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      n->atom->visited = false;
      n->atom->marked = false;
      n->atom->realnant = false;
    }
  Denant i(program->number_of_atoms);
  for (Node *n = program->atoms.head (); n; n = n->next)
    if (!n->atom->visited)
      i.strong (n->atom);
  smodels->size_of_searchspace = i.size_of_searchspace;
  smodels->number_of_denants = i.number_of_denants;
}

void
Dcl::undenant ()
{
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      Atom *a = n->atom;
      for (Follows *f = a->neg; !a->isnant && f != a->endNeg; f++)
	if (f->r->type != OPTIMIZERULE)
	  a->isnant = true;
      for (Follows *f = a->head; !a->isnant && f != a->endHead; f++)
	if (f->r->type == CHOICERULE || f->r->type == GENERATERULE)
	  a->isnant = true;
    }
}
