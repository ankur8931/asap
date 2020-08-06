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
#include <limits.h>
#include "atomrule.h"
#include "stack.h"
#include "smodels.h"
#include "dcl.h"

#define lowlink posScore
#define dfnumber negScore
#define marked dependsonTrue

class Improve
{
public:
  Improve (long size);
  ~Improve () {};
  void strong (Atom *);
  void removeFrom (Atom *a, Follows *f);
  void doStrong (Atom *a, Atom *b);

  long count;
  Stack stack;
};

Improve::Improve (long size)
{
  stack.Init (size);
  count = 0;
}

inline void
Improve::removeFrom (Atom *a, Follows *f)
{
  f->r->not_in_loop (f);
  a->endUpper--;
  Follows *g = a->endUpper;
  f->r->swapping (f, g);
  g->r->swapping (g, f);
  Follows t = *f;
  *f = *g;
  *g = t;
}

inline void
Improve::doStrong (Atom *a, Atom *b)
{
  if (!b->visited)
    {
      strong (b);
      if (a->lowlink > b->lowlink)
	a->lowlink = b->lowlink;
    }
  else if (!b->marked && a->lowlink > b->dfnumber)
    a->lowlink = b->dfnumber;
}

void
Improve::strong (Atom *a)
{
  Atom *b;
  a->visited = true;
  a->dfnumber = count;
  a->lowlink = count;
  count++;
  stack.push (a);
  for (Follows *f = a->pos; f != a->endUpper; f++)
    {
      bool remove = false;
      if (a->Bneg || f->r->isInactive ())
	continue;
      switch (f->r->type)
	{
	case BASICRULE:
	case CONSTRAINTRULE:
	case WEIGHTRULE:
	  b = ((HeadRule *)f->r)->head;
	  doStrong (a, b);
	  remove = b->marked;
	  break;
	case CHOICERULE:
	  {
	    ChoiceRule *cr = (ChoiceRule *)f->r;
	    remove = true;
	    for (Atom **h = cr->head; h != cr->hend; h++)
	      {
		doStrong (a, *h);
		if (!(*h)->marked)
		  remove = false;
	      }
	    break;
	  }
	case GENERATERULE:
	  {
	    GenerateRule *gr = (GenerateRule *)f->r;
	    remove = true;
	    for (Atom **h = gr->head; h != gr->hend; h++)
	      {
		doStrong (a, *h);
		if (!(*h)->marked)
		  remove = false;
	      }
	    break;
	  }
	case OPTIMIZERULE:
	case ENDRULE:
	  break;
	}
      if (remove)
	{
	  removeFrom (a, f);
	  f--;
	}
    }
  if (a->lowlink == a->dfnumber)
    do {
      b = stack.pop ();
      b->marked = true;
    } while (a != b);
}

void
Dcl::improve ()
{
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      n->atom->visited = false;
      n->atom->marked = false;
    }
  Improve i(program->number_of_atoms);
  for (Node *n = program->atoms.head (); n; n = n->next)
    if (!n->atom->visited)
      i.strong(n->atom);
}
