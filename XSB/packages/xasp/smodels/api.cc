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
#include <string.h>
#include <float.h>
#include <limits.h>
#include <assert.h>
#include "atomrule.h"
#include "program.h"
#include "tree.h"
#include "api.h"

Api::list::list ()
{
  top = 0;
  size = 32;
  atoms = new Atom *[size];
  weights = new Weight[size];
}

Api::list::~list ()
{
  delete[] atoms;
  delete[] weights;
}

void
Api::list::push (Atom *a, Weight w)
{
  if (top == size)
    grow ();
  atoms[top] = a;
  weights[top] = w;
  top++;
}

void
Api::list::reset ()
{
  top = 0;
}

void
Api::list::grow ()
{
  long sz = size*2;
  Atom **atom_array = new Atom *[sz];
  Weight *weight_array = new Weight[sz];
  for (int i = 0; i < size; i++)
    {
      atom_array[i] = atoms[i];
      weight_array[i] = weights[i];
    }
  size = sz;
  delete[] atoms;
  atoms = atom_array;
  delete[] weights;
  weights = weight_array;
}

Api::Api (Program *p)
  : program (p)
{
  tree = 0;
  pointer_to_tree = 0;
  init = new Init;
}

Api::~Api ()
{
  delete init;
  delete pointer_to_tree;
}

inline long
Api::size (list &l)
{
  return l.top;
}

Atom *
Api::new_atom ()
{
  Atom *a = new Atom (program);
  program->atoms.push (a);
  program->number_of_atoms++;
  return a;
}

void
Api::set_compute (Atom *a, bool pos)
{
  assert (a);
  if (pos)
    a->computeTrue = true;
  else
    a->computeFalse = true;
}

void
Api::reset_compute (Atom *a, bool pos)
{
  assert (a);
  if (pos)
    a->computeTrue = false;
  else
    a->computeFalse = false;
}

void
Api::set_name (Atom *a, const char *s)
{
  assert (a);
  if (a->name && tree)
    tree->remove (a);
  delete[] a->name;
  if (s)
    {
      a->name = strcpy (new char[strlen (s)+1], s);
      if (tree)
	tree->insert (a);
    }
  else
    a->name = 0;
}

void
Api::remember ()
{
  if (pointer_to_tree == 0)
    pointer_to_tree = new Tree;
  tree = pointer_to_tree;
}

void
Api::forget ()
{
  tree = 0;
}

Atom *
Api::get_atom (const char *name)
{
  if (pointer_to_tree)
    return pointer_to_tree->find (name);
  else
    return 0;
}

void
Api::begin_rule (RuleType t)
{
  type = t;
  atleast_weight = WEIGHT_MIN;
  atleast_body = 0;
  atleast_head = 1;
}

void
Api::set_init ()
{
  init->head = head.atoms;
  init->hsize = size (head);
  init->pbody = pbody.atoms;
  init->psize = size (pbody);
  init->pweight = pbody.weights;
  init->nbody = nbody.atoms;
  init->nweight = nbody.weights;
  init->nsize = size (nbody);
  init->atleast_weight = atleast_weight;
  init->atleast_body = atleast_body;
  init->atleast_head = atleast_head;
  init->maximize = maximize;
}

void
Api::end_rule ()
{
  set_init ();
  Rule *r = 0;
  switch (type)
    {
    case BASICRULE:
      assert (size (head) == 1);
      r = new BasicRule;
      break;
    case CONSTRAINTRULE:
      assert (size (head) == 1);
      r = new ConstraintRule;
      break;
    case GENERATERULE:
      assert (size (head) >= 2);
      r = new GenerateRule;
      break;
    case CHOICERULE:
      assert (size (head) >= 1);
      r = new ChoiceRule;
      break;
    case WEIGHTRULE:
      assert (size (head) == 1);
      r = new WeightRule;
      break;
    case OPTIMIZERULE:
      {
	OptimizeRule *o = new OptimizeRule;
	r = o;
	o->next = program->optimize;
	program->optimize = o;
	break;
      }
    default:
      break;
    }
  if (r)
    {
      program->rules.push (r);
      program->number_of_rules++;
      r->init (init);
    }
  pbody.reset ();
  nbody.reset ();
  head.reset ();
}

void
Api::add_head (Atom *a)
{
  assert (a);
  head.push (a);
}

void
Api::add_body (Atom *a, bool pos)
{
  assert (a);
  if (pos)
    pbody.push (a);
  else
    nbody.push (a);
}

void
Api::add_body (Atom *a, bool pos, Weight w)
{
#ifdef USEDOUBLE
  assert (w >= 0);
#endif
  assert (a);
  if (pos)
    pbody.push (a, w);
  else
    nbody.push (a, w);
}

void
Api::change_body (long i, bool pos, Weight w)
{
#ifdef USEDOUBLE
  assert (w >= 0);
#endif
  if (pos)
    {
      assert (0 <= i && i < size (pbody));
      pbody.weights[i] = w;
    }
  else
    {
      assert (0 <= i && i < size (nbody));
      nbody.weights[i] = w;
    }
}

void
Api::set_atleast_weight (Weight w)
{
  atleast_weight = w;
}

void
Api::set_atleast_body (long n)
{
  atleast_body = n;
}

void
Api::set_atleast_head (long n)
{
  atleast_head = n;
}

void
Api::copy (Api *api)
{
  for (Node *n = api->program->atoms.head (); n; n = n->next)
    {
      Atom *a = n->atom;
      Atom *na = new_atom ();
      set_name (na, a->name);
      a->copy = na;
      na->computeTrue = a->computeTrue;
      na->computeFalse = a->computeFalse;
    }
  for (Node *n = api->program->rules.head (); n; n = n->next)
    {
      Rule *rule = n->rule;
      switch (rule->type)
	{
	case BASICRULE:
	  {
	    BasicRule *r = (BasicRule *)rule;
	    begin_rule (r->type);
	    add_head (r->head->copy);
	    for (Atom **a = r->nbody; a != r->nend; a++)
	      add_body ((*a)->copy, false);
	    for (Atom **a = r->pbody; a != r->pend; a++)
	      add_body ((*a)->copy, true);
	    end_rule ();
	    break;
	  }
	case CONSTRAINTRULE:
	  {
	    ConstraintRule *r = (ConstraintRule *)rule;
	    begin_rule (r->type);
	    add_head (r->head->copy);
	    for (Atom **a = r->nbody; a != r->nend; a++)
	      add_body ((*a)->copy, false);
	    for (Atom **a = r->pbody; a != r->pend; a++)
	      add_body ((*a)->copy, true);
	    set_atleast_body (r->atleast);
	    end_rule ();
	    break;
	  }
	case GENERATERULE:
	  {
	    GenerateRule *r = (GenerateRule *)rule;
	    begin_rule (r->type);
	    for (Atom **a = r->head; a != r->hend; a++)
	      add_head ((*a)->copy);
	    for (Atom **a = r->pbody; a != r->pend; a++)
	      add_body ((*a)->copy, true);
	    set_atleast_head (r->atleast);
	    end_rule ();
	    break;
	  }
	case CHOICERULE:
	  {
	    ChoiceRule *r = (ChoiceRule *)rule;
	    begin_rule (r->type);
	    for (Atom **a = r->head; a != r->hend; a++)
	      add_head ((*a)->copy);
	    for (Atom **a = r->nbody; a != r->nend; a++)
	      add_body ((*a)->copy, false);
	    for (Atom **a = r->pbody; a != r->pend; a++)
	      add_body ((*a)->copy, true);
	    end_rule ();
	    break;
	  }
	case WEIGHTRULE:
	  {
	    WeightRule *r = (WeightRule *)rule;
	    begin_rule (r->type);
	    add_head (r->head->copy);
	    for (Atom **a = r->body; a != r->bend; a++)
	      add_body ((*a)->copy, r->aux[a-r->body].positive,
			r->aux[a-r->body].weight);
	    set_atleast_weight (r->atleast);
	    end_rule ();
	    break;
	  }
	case OPTIMIZERULE:
	  {
	    OptimizeRule *r = (OptimizeRule *)rule;
	    begin_rule (r->type);
	    for (Atom **a = r->nbody; a != r->nend; a++)
	      add_body ((*a)->copy, false, r->aux[a-r->nbody].weight);
	    for (Atom **a = r->pbody; a != r->pend; a++)
	      add_body ((*a)->copy, true, r->aux[a-r->nbody].weight);
	    end_rule ();
	    break;
	  }
	default:
	  break;
	}
    }
}

void
Api::done ()
{
  // Set up atoms
  for (Node *n = program->atoms.head (); n; n = n->next)
    {
      Atom *a = n->atom;
      a->source = 0;     // Not null if copied
      if (a->headof + a->posScore + a->negScore)
	{
	  a->head = new Follows[a->headof + a->posScore + a->negScore];
	  a->endHead = a->head + a->headof;
	  a->pos = a->endHead;
	  a->endPos = a->pos + a->posScore;
	  a->endUpper = a->endPos;
	  a->neg = a->endPos;
	  a->endNeg = a->neg + a->negScore;
	  a->end = a->endNeg;
	}
    }
  for (Node *n = program->rules.head (); n; n = n->next)
    n->rule->setup ();
  for (Node *n = program->atoms.head (); n; n = n->next)
    n->atom->head -= n->atom->headof;
}
