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
#ifdef USEDOUBLE
#include <math.h>
#endif
#include "atomrule.h"
#include "program.h"

using namespace std;

static Auxiliary pos_aux(true);

Atom::Atom (Program *p0)
  : p(p0)
{
  endHead = 0;
  endPos = 0;
  endNeg = 0;
  endUpper = 0;
  end = 0;
  headof = 0;
  head = 0;
  pos = 0;
  neg = 0;
  source = 0;
  posScore = 0;
  negScore = 0;
  wrong_choices = 0;
  name = 0;
  closure = false;
  Bpos = false;
  Bneg = false;
  visited = false;
  guess = false;
  isnant = false;
  dependsonTrue = false;
  dependsonFalse = false;
  computeTrue = false;
  computeFalse = false;
  backtracked = false;
  forced = false;
  in_etrue_queue = false;
  in_efalse_queue = false;
  in_queue = false;
  pos_tested = false;
  neg_tested = false;
}

Atom::~Atom ()
{
  delete[] head;
  delete[] name;
}

void
Atom::etrue_push ()
{
  if (!(in_etrue_queue | in_efalse_queue))
    p->equeue.push (this);
  if (Bneg | in_efalse_queue)
    p->conflict = true;
  in_etrue_queue = true;
}

void
Atom::efalse_push ()
{
  if (!(in_etrue_queue | in_efalse_queue))
    p->equeue.push (this);
  if (Bpos | in_etrue_queue)
    p->conflict = true;
  in_efalse_queue = true;
}

void
Atom::queue_push ()
{
  if (!in_queue)
    p->queue.push (this);
  in_queue = true;
}

void
Atom::backchainTrue ()
{
  // Find the only active rule.
  for (Follows *f = head; f != endHead; f++)
    if (!f->r->isInactive ())
      {
	f->r->backChainTrue ();
	break;
      }
}

void
Atom::backchainFalse ()
{
  for (Follows *f = head; f != endHead; f++)
    f->r->backChainFalse ();
}

//
// Note that since backchain depends on the truth values of the
// atoms in the body of the rules, we must set Bpos first.
// To keep everything safe we also handle inactivation before
// we fire any rules.
//
void
Atom::setBTrue ()
{
  Bpos = true;
  Follows *f;
  for (f = neg; f != endNeg; f++)
    f->r->inactivate (f);
  for (f = pos; f != endPos; f++)
    f->r->mightFire (f);
  // This atom could have been forced.
  if (headof == 1)
    backchainTrue ();
}

void
Atom::setBFalse ()
{
  Bneg = true;
  Follows *f;
  for (f = pos; f != endPos; f++)
    f->r->inactivate (f);
  for (f = neg; f != endNeg; f++)
    f->r->mightFire (f);
  closure = false;
  source = 0;
  // This atom could have been forced.
  if (headof)          // There are active rules.
    backchainFalse (); // Might backchain already backchained rules
		       // in mightFire.
}

void
Atom::setTrue ()
{
  for (Follows *f = pos; f != endPos; f++)
    f->r->mightFireInit (f);
  closure = true;
}

void
Atom::backtrackFromBTrue ()
{
  Follows *f;
  for (f = pos; f != endPos; f++)
    f->r->backtrackFromActive (f);
  for (f = neg; f != endNeg; f++)
    f->r->backtrackFromInactive (f);
  Bpos = false;
  closure = true;
}

void
Atom::backtrackFromBFalse ()
{
  Follows *f;
  for (f = neg; f != endNeg; f++)
    f->r->backtrackFromActive (f);
  for (f = pos; f != endPos; f++)
    f->r->backtrackFromInactive (f);
  Bneg = false;
  closure = true;
}

void
Atom::reduce_head ()
{
  Follows t;
  Follows *g = head;
  for (Follows *f = head; f != endHead; f++)
    {
      f->r->swapping (f, g);
      g->r->swapping (g, f);
      t = *g;
      *g = *f;
      *f = t;
      if (!g->r->isInactive ())
	g++;
    }
  endHead = g;
}

void
Atom::reduce_pbody (bool strong)
{
  Follows t;
  Follows *g = pos;
  for (Follows *f = pos; f != endPos; f++)
    {
      f->r->swapping (f, g);
      g->r->swapping (g, f);
      t = *g;
      *g = *f;
      *f = t;
      if ((strong && Bpos) || g->r->isInactive ())
	g->r->not_in_loop (g);
      else if (Bneg == false)
	g++;
    }
  endPos = g;
  endUpper = g;
}

void
Atom::reduce_nbody (bool)
{
  Follows t;
  Follows *g = neg;
  for (Follows *f = neg; f != endNeg; f++)
    {
      f->r->swapping (f, g);
      g->r->swapping (g, f);
      t = *g;
      *g = *f;
      *f = t;
      if (Bpos == false && Bneg == false && !g->r->isInactive ())
	g++;
    }
  endNeg = g;
}

void
Atom::reduce (bool strongly)
{
  reduce_head ();
  reduce_pbody (strongly);
  reduce_nbody (strongly);
}

void
Atom::unreduce ()
{
  endHead = pos;
  endPos = neg;
  endUpper = neg;
  endNeg = end;
  for (Follows *f = pos; f != endPos; f++)
    f->r->in_loop (f);
}

const char *
Atom::atom_name ()
{
  if (name)
    return name;
  else
    return "#noname#";
}

void
Atom::visit ()
{
  queue_push ();
  visited = true;
}


BasicRule::BasicRule ()
{
  head = 0;
  lit = 0;
  upper = 0;
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  inactive = 0;
  visited = false;
  type = BASICRULE;
}

BasicRule::~BasicRule ()
{
  delete[] nbody;  // pbody is allocated after nbody
}

bool
BasicRule::isInactive ()
{
  return inactive;
}

bool
BasicRule::isUpperActive ()
{
  return upper == 0 && !inactive;
}

bool
BasicRule::isFired ()
{
  return lit == 0;
}

void
BasicRule::inactivate (const Follows *f)
{
  if (f->aux)
    upper++;
  inactive++;
  if (inactive == 1)
    {
      Atom *b = head;
      b->headof--;
      if (b->Bneg == false)
	{
	  if (b->headof && (b->source == 0 || b->source == this))
	    b->queue_push ();
	  if (b->headof == 0)
	    b->efalse_push ();
	  else if (b->headof == 1 && b->Bpos)
	    b->backchainTrue ();
	}
    }
}

void
BasicRule::fireInit ()
{
  if (lit == 0)
    {
      head->etrue_push ();
      head->queue_push ();
      head->source = this;
    }
  else if (upper == 0)
    {
      head->queue_push ();
      if (head->source == 0)
	head->source = this;
    }
}

void
BasicRule::mightFireInit (const Follows *)
{
  upper--;
  if (upper == 0 && !inactive)
    {
      head->queue_push ();
      if (head->source == 0)
	head->source = this;
    }
}

void
BasicRule::unInit ()
{
  upper = pend-pbody;
  lit = end-nbody;
  inactive = 0;
}

void
BasicRule::mightFire (const Follows *)
{
  lit--;
  if (lit == 0 && head->Bpos == false)
    head->etrue_push ();
  else if (lit == 1 && !inactive && head->Bneg == true)
    backChainFalse ();
}

void
BasicRule::backChainTrue ()
{
  if (lit)
    {
      for (Atom **b = nbody; b != nend; b++)
	if ((*b)->Bneg == false)
	  (*b)->efalse_push ();
      for (Atom **b = pbody; b != pend; b++)
	if ((*b)->Bpos == false)
	  (*b)->etrue_push ();
    }
}

void
BasicRule::backChainFalse ()
{
  if (lit == 1 && !inactive)
    {
      for (Atom **b = nbody; b != nend; b++)
	if ((*b)->Bneg == false)
	  {
	    (*b)->etrue_push ();
	    return;
	  }
      for (Atom **b = pbody; b != pend; b++)
	if ((*b)->Bpos == false)
	  {
	    (*b)->efalse_push ();
	    return;
	  }
    }
}

void
BasicRule::backtrackFromActive (const Follows *)
{
  lit++;
}

void
BasicRule::backtrackFromInactive (const Follows *f)
{
  if (f->aux)
    upper--;
  inactive--;
  if (inactive == 0)
    head->headof++;
}

void
BasicRule::propagateFalse (const Follows *)
{
  upper++;
  if (upper == 1 && !inactive && head->closure != false &&
      (head->source == 0 || head->source == this))
    head->queue_push ();
}

void
BasicRule::propagateTrue (const Follows *)
{
  upper--;
  if (upper == 0 && !inactive && head->closure == false)
    {
      if (head->source == 0)
	head->source = this;
      head->queue_push ();
    }
}

void
BasicRule::backtrackUpper (const Follows *)
{
  upper--;
}

void
BasicRule::search (Atom *)
{
  if (!head->visited && head->Bneg == false)
    head->visit ();
  for (Atom **a = nbody; a != nend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  for (Atom **a = pbody; a != pend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  visited = true;
}

void
BasicRule::reduce (bool strongly)
{
  Atom **a;
  Atom **b;
  Atom *t;
  b = nbody;
  for (a = nbody; a != nend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((*b)->Bpos == false && (*b)->Bneg == false)
	b++;
    }
  nend = b;
  b = pbody;
  for (a = pbody; a != pend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((strongly == false || (*b)->Bpos == false)
	  && (*b)->Bneg == false)
	b++;
    }
  pend = b;
}

void
BasicRule::unreduce ()
{
  nend = pbody;
  pend = end;
}

void
BasicRule::setup ()
{
  head->head->r = this;
  head->head->aux = 0;
  head->head++;
  Atom **a;
  for (a = nbody; a != nend; a++)
    {
      (*a)->negScore--;
      (*a)->neg[(*a)->negScore].r = this;
      (*a)->neg[(*a)->negScore].aux = 0;
    }
  for (a = pbody; a != pend; a++)
    {
      (*a)->posScore--;
      (*a)->pos[(*a)->posScore].r = this;
      (*a)->pos[(*a)->posScore].aux = &pos_aux;
    }
}

void
BasicRule::init (Init *init)
{
  head = *init->head;
  head->headof++;
  long n = init->psize+init->nsize;
  if (n != 0)
    nbody = new Atom *[n];
  else
    nbody = 0;
  end = nbody+n;
  pend = end;
  lit = n;
  for (long i = 0; i < init->nsize; i++)
    {
      nbody[i] = init->nbody[i];
      nbody[i]->negScore++;
      nbody[i]->isnant = true;
    }
  nend = nbody + init->nsize;
  pbody = nend;
  upper = init->psize;
  for (long i = 0; i < init->psize; i++)
    {
      pbody[i] = init->pbody[i];
      pbody[i]->posScore++;
    }
}

void
BasicRule::print ()
{
  cout << head->atom_name ();
  if (nbody)
    cout << " :- ";
  Atom **a;
  int comma = 0;
  for (a = pbody; a != pend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name ();
      comma = 1;
    }
  for (a = nbody; a != nend; a++)
    {
      if (comma)
	cout << ", ";
      cout << "not " << (*a)->atom_name ();
      comma = 1;
    }
  cout << '.' << endl;
}

void
BasicRule::print_internal ()
{
  cout << (int)type << ' ' << head->posScore << ' '
       << (nend-nbody)+(pend-pbody) << ' ' << nend-nbody;
  head->negScore = 1;
  Atom **a;
  for (a = nbody; a != nend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;  // Mark as needed in compute statement
    }
  for (a = pbody; a != pend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  cout << endl;
}


ConstraintRule::ConstraintRule ()
{
  head = 0;
  lit = 0;
  upper = 0;
  lower = 0;
  atleast = 0;
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  inactive = 0;
  visited = false;
  type = CONSTRAINTRULE;
}

ConstraintRule::~ConstraintRule ()
{
  delete[] nbody;
}

bool
ConstraintRule::isInactive ()
{
  return inactive > 0;
}

bool
ConstraintRule::isUpperActive ()
{
  return upper <= 0 && inactive <= 0;
}

bool
ConstraintRule::isFired ()
{
  return lit <= 0 && inactive <= 0;
}

void
ConstraintRule::inactivate (const Follows *f)
{
  if (f->aux == 0)
    lower++;
  upper++;
  inactive++;
  Atom *b = head;
  if (inactive == 0 && b->headof == 1 && b->Bpos)
    backChainTrue ();
  if (inactive == 1)
    {
      b->headof--;
      if (b->Bneg == false)
	{
	  if (b->headof && (b->source == 0 || b->source == this))
	    b->queue_push ();
	  if (b->headof == 0)
	    b->efalse_push ();
	  else if (b->headof == 1 && b->Bpos)
	    b->backchainTrue ();
	}
    }
  else if (b->Bneg == false && (b->source == 0 || b->source == this)
	   && lower > 0)
    b->queue_push ();
}

void
ConstraintRule::fireInit ()
{
  if (lit <= 0)
    {
      head->etrue_push ();
      head->queue_push ();
      head->source = this;
    }
  else if (upper <= 0)
    {
      head->queue_push ();
      if (head->source == 0)
	head->source = this;
    }
}

void
ConstraintRule::mightFireInit (const Follows *)
{
  upper--;
  if (upper == 0 && inactive <= 0)
    {
      head->queue_push ();
      if (head->source == 0)
	head->source = this;
    }
}

void
ConstraintRule::unInit ()
{
  lit = atleast;
  upper = lit - (nend-nbody);
  lower = upper;
  inactive = lit - (end-nbody);
}

void
ConstraintRule::mightFire (const Follows *)
{
  lit--;
  if (lit == 0 && head->Bpos == false)
    head->etrue_push ();
  else if (lit == 1 && inactive <= 0 && head->Bneg == true)
    backChainFalse ();
}

void
ConstraintRule::backChainTrue ()
{
  if (inactive == 0 && lit > 0)
    {
      for (Atom **b = nbody; b != nend; b++)
	if ((*b)->Bneg == false && (*b)->Bpos == false)
	  (*b)->efalse_push ();
      for (Atom **b = pbody; b != pend; b++)
	if ((*b)->Bneg == false && (*b)->Bpos == false)
	  (*b)->etrue_push ();
    }
}

void
ConstraintRule::backChainFalse ()
{
  if (lit == 1 && inactive <= 0)
    {
      for (Atom **b = nbody; b != nend; b++)
	if ((*b)->Bneg == false && (*b)->Bpos == false)
	  (*b)->etrue_push ();
      for (Atom **b = pbody; b != pend; b++)
	if ((*b)->Bneg == false && (*b)->Bpos == false)
	  (*b)->efalse_push ();
    }
}

void
ConstraintRule::backtrackFromActive (const Follows *)
{
  lit++;
}

void
ConstraintRule::backtrackFromInactive (const Follows *f)
{
  if (f->aux == 0)
    lower--;
  upper--;
  inactive--;
  if (inactive == 0)
    head->headof++;
}

void
ConstraintRule::propagateFalse (const Follows *)
{
  upper++;
  if (lower > 0 && inactive <= 0 && head->closure != false &&
      (head->source == 0 || head->source == this))
    head->queue_push ();
}

void
ConstraintRule::propagateTrue (const Follows *)
{
  upper--;
  if (upper == 0 && inactive <= 0 && head->closure == false)
    {
      if (head->source == 0)
	head->source = this;
      head->queue_push ();
    }
}

void
ConstraintRule::backtrackUpper (const Follows *)
{
  upper--;
}

void
ConstraintRule::in_loop (Follows *f)
{
  if (f->aux == 0)
    lower++;
  f->aux = &pos_aux;
}

void
ConstraintRule::not_in_loop (Follows *f)
{
  if (f->aux)
    lower--;
  f->aux = 0;
}

void
ConstraintRule::search (Atom *)
{
  if (!head->visited && head->Bneg == false)
    head->visit ();
  for (Atom **a = nbody; a != nend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  for (Atom **a = pbody; a != pend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  visited = true;
}

void
ConstraintRule::reduce (bool strongly)
{
  Atom **a;
  Atom **b;
  Atom *t;
  b = nbody;
  for (a = nbody; a != nend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((*b)->Bpos == false && (*b)->Bneg == false)
	b++;
    }
  nend = b;
  b = pbody;
  for (a = pbody; a != pend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((strongly == false || (*b)->Bpos == false) &&
	  (*b)->Bneg == false)
	b++;
    }
  pend = b;
}

void
ConstraintRule::unreduce ()
{
  nend = pbody;
  pend = end;
}

void
ConstraintRule::setup ()
{
  head->head->r = this;
  head->head->aux = 0;
  head->head++;
  Atom **a;
  for (a = nbody; a != nend; a++)
    {
      (*a)->negScore--;
      (*a)->neg[(*a)->negScore].r = this;
      (*a)->neg[(*a)->negScore].aux = 0;
    }
  for (a = pbody; a != pend; a++)
    {
      (*a)->posScore--;
      (*a)->pos[(*a)->posScore].r = this;
      (*a)->pos[(*a)->posScore].aux = &pos_aux;
    }
}

void
ConstraintRule::init (Init *init)
{
  head = *init->head;
  head->headof++;
  long n = init->nsize + init->psize;
  if (n != 0)
    nbody = new Atom *[n];
  else
    nbody = 0;
  end = nbody+n;
  pend = end;
  atleast = init->atleast_body;
  lit = init->atleast_body;
  inactive = init->atleast_body - n;
  for (long i = 0; i < init->nsize; i++)
    {
      nbody[i] = init->nbody[i];
      nbody[i]->negScore++;
      nbody[i]->isnant = true;
    }
  nend = nbody + init->nsize;
  pbody = nend;
  upper = init->atleast_body - init->nsize;
  lower = upper;
  for (long i = 0; i < init->psize; i++)
    {
      pbody[i] = init->pbody[i];
      pbody[i]->posScore++;
    }
}

void
ConstraintRule::print ()
{
  cout << head->atom_name () << " :- ";
  Atom **a;
  int comma = 0;
  cout << atleast << " { ";
  for (a = pbody; a != pend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name ();
      comma = 1;
    }
  for (a = nbody; a != nend; a++)
    {
      if (comma)
	cout << ", ";
      cout << "not " << (*a)->atom_name ();
      comma = 1;
    }
  cout << " }";
  cout << '.' << endl;
}

void
ConstraintRule::print_internal ()
{
  cout << (int)type << ' ' << head->posScore << ' '
       << (nend-nbody)+(pend-pbody) << ' ' << nend-nbody;
  head->negScore = 1;
  cout << ' ' << lit;
  Atom **a;
  for (a = nbody; a != nend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;  // Mark as needed in compute statement
    }
  for (a = pbody; a != pend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  cout << endl;
}


GenerateRule::GenerateRule ()
{
  head = 0;
  hend = 0;
  pbody = 0;
  pend = 0;
  end = 0;
  neg = 0;
  pos = 0;
  upper = 0;
  inactivePos = 0;
  inactiveNeg = 0;
  atleast = 0;
  visited = false;
  type = GENERATERULE;
}

GenerateRule::~GenerateRule ()
{
  delete[] head;
}

bool
GenerateRule::isInactive ()
{
  return inactivePos || inactiveNeg > 0;
}

bool
GenerateRule::isUpperActive ()
{
  return upper == 0 && !isInactive ();
}

bool
GenerateRule::isFired ()
{
  return pos == 0 && neg <= 0;
}

void
GenerateRule::inactivate (const Follows *f)
{
  if (f->aux)
    {
      upper++;
      inactivePos++;
      if (!(inactivePos == 1 && inactiveNeg <= 0))
	return;
      inactivate ();
    }
  else
    {
      inactiveNeg++;
      if (!(inactiveNeg == 1 && inactivePos == 0))
	return;
      inactivate ();
    }
}

void
GenerateRule::inactivate ()
{
  for (Atom **h = head; h != hend; h++)
    {
      Atom *b = *h;
      b->headof--;
      if (b->Bneg == false)
	{
	  if (b->headof && (b->source == 0 || b->source == this))
	    b->queue_push ();
	  if (b->headof == 0)
	    b->efalse_push ();
	  else if (b->headof == 1 && b->Bpos)
	    b->backchainTrue ();
	}
    }
}

void
GenerateRule::fireInit ()
{
  if (pos == 0 && neg <= 0)
    for (Atom **h = head; h != hend; h++)
      {
	(*h)->etrue_push ();
	(*h)->queue_push ();
	(*h)->source = this;
      }
  else if (upper == 0)
    for (Atom **h = head; h != hend; h++)
      {
	(*h)->queue_push ();
	if ((*h)->source == 0)
	  (*h)->source = this;
      }
}

void
GenerateRule::mightFireInit (const Follows *)
{
  upper--;
  if (upper == 0 && !isInactive ())
    for (Atom **h = head; h != hend; h++)
      {
	(*h)->queue_push ();
	if ((*h)->source == 0)
	  (*h)->source = this;
      }
}

void
GenerateRule::unInit ()
{
  pos = pend-pbody;
  inactivePos = 0;
  upper = pos;
  inactiveNeg = -atleast;
  neg = (hend-head) - atleast;
}

void
GenerateRule::mightFire (const Follows *f)
{
  if (f->aux)
    pos--;
  else
    neg--;
  if (pos == 0 && neg == 0)
    {
      for (Atom **h = head; h != hend; h++)
	if ((*h)->Bneg == false && (*h)->Bpos == false)
	  (*h)->etrue_push ();
    }
  else if (pos == 1 && neg < 0 && !isInactive ())
    backChainFalse ();
}

void
GenerateRule::backChainTrue ()
{
  if (inactiveNeg != 0)
    return;
  Atom **b;
  if (neg)
    for (b = head; b != hend; b++)
      if ((*b)->Bneg == false && (*b)->Bpos == false)
	(*b)->efalse_push ();
  if (pos)
    for (b = pbody; b != pend; b++)
      if ((*b)->Bpos == false)
	(*b)->etrue_push ();
}

void
GenerateRule::backChainFalse ()
{
  if (pos == 1 && neg < 0)
    for (Atom **b = pbody; b != pend; b++)
      if ((*b)->Bpos == false)
	{
	  (*b)->efalse_push ();
	  break;
	}
}

void
GenerateRule::backtrackFromActive (const Follows *f)
{
  if (f->aux)
    pos++;
  else
    neg++;
}

void
GenerateRule::backtrackFromInactive (const Follows *f)
{
  if (f->aux)
    {
      upper--;
      inactivePos--;
      if (inactivePos == 0 && inactiveNeg <= 0)
	for (Atom **h = head; h != hend; h++)
	  (*h)->headof++;
    }
  else
    {
      inactiveNeg--;
      if (inactiveNeg == 0 && inactivePos == 0)
	for (Atom **h = head; h != hend; h++)
	  (*h)->headof++;
    }
}

void
GenerateRule::propagateFalse (const Follows *)
{
  upper++;
  if (upper == 1 && !isInactive ())
    {
      if (inactiveNeg < 0)
	{
	  for (Atom **h = head; h != hend; h++)
	    if ((*h)->closure != false &&
		((*h)->source == 0 || (*h)->source == this))
	      (*h)->queue_push ();
	}
      else
	for (Atom **h = head; h != hend; h++)
	  if ((*h)->closure != false && (*h)->Bpos &&
	      ((*h)->source == 0 || (*h)->source == this))
	    (*h)->queue_push ();
    }
}

void
GenerateRule::propagateTrue (const Follows *)
{
  upper--;
  if (upper == 0 && !isInactive ())
    {
      if (inactiveNeg < 0)
	{
	  for (Atom **h = head; h != hend; h++)
	    if ((*h)->closure == false)
	      {
		if ((*h)->source == 0)
		  (*h)->source = this;
		(*h)->queue_push ();
	      }
	}
      else
	for (Atom **h = head; h != hend; h++)
	  if ((*h)->closure == false && (*h)->Bpos)
	    {
	      if ((*h)->source == 0)
		(*h)->source = this;
	      (*h)->queue_push ();
	    }
    }
}

void
GenerateRule::backtrackUpper (const Follows *)
{
  upper--;
}

void
GenerateRule::search (Atom *)
{
  for (Atom **a = head; a != hend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  for (Atom **a = pbody; a != pend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  visited = true;
}

void
GenerateRule::reduce (bool strongly)
{
  Atom **a;
  Atom **b;
  Atom *t;
  b = head;
  for (a = head; a != hend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((*b)->Bpos == false && (*b)->Bneg == false)
	b++;
    }
  hend = b;
  b = pbody;
  for (a = pbody; a != pend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((strongly == false || (*b)->Bpos == false) &&
	  (*b)->Bneg == false)
	b++;
    }
  pend = b;
}

void
GenerateRule::unreduce ()
{
  hend = pbody;
  pend = end;
}

void
GenerateRule::setup ()
{
  Atom **a;
  for (a = head; a != hend; a++)
    {
      (*a)->head->r = this;
      (*a)->head++;
    }
  for (a = head; a != hend; a++)
    {
      (*a)->negScore--;
      (*a)->neg[(*a)->negScore].r = this;
      (*a)->neg[(*a)->negScore].aux = 0;
    }
  for (a = pbody; a != pend; a++)
    {
      (*a)->posScore--;
      (*a)->pos[(*a)->posScore].r = this;
      (*a)->pos[(*a)->posScore].aux = &pos_aux;
    }
}

void
GenerateRule::init (Init *init)
{
  head = new Atom *[init->hsize + init->psize];
  hend = head+init->hsize;
  pbody = hend;
  atleast = init->atleast_head;
  neg = init->hsize - init->atleast_head;
  inactiveNeg = -init->atleast_head;
  for (long i = 0; i < init->hsize; i++)
    {
      head[i] = init->head[i];
      head[i]->headof++;
      head[i]->negScore++;
      head[i]->isnant = true;
    }
  pend = pbody + init->psize;
  end = pend;
  pos = init->psize;
  upper = init->psize;
  inactivePos = 0;
  for (long i = 0; i < init->psize; i++)
    {
      pbody[i] = init->pbody[i];
      pbody[i]->posScore++;
    }
}

void
GenerateRule::print ()
{
  Atom **a;
  int comma = 0;
  cout << atleast << " {";
  for (a = head; a != hend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name ();
      comma = 1;
    }
  cout << '}';
  if (pbody != pend)
    cout << " :- ";
  comma = 0;
  for (a = pbody; a != pend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name ();
      comma = 1;
    }
  cout << '.' << endl;
}

void
GenerateRule::print_internal ()
{
  cout << (int)type << ' ' << hend-head;
  cout << ' ' << atleast;
  Atom **a;
  for (a = head; a != hend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;  // Mark as needed in compute statement
    }
  cout << ' ' << pend-pbody;
  for (a = pbody; a != pend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  cout << endl;
}


ChoiceRule::ChoiceRule ()
{
  head = 0;
  hend = 0;
  lit = 0;
  upper = 0;
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  inactive = 0;
  visited = false;
  type = CHOICERULE;
}

ChoiceRule::~ChoiceRule ()
{
  delete[] head;
}

bool
ChoiceRule::isInactive ()
{
  return inactive;
}

bool
ChoiceRule::isUpperActive ()
{
  return upper == 0 && !inactive;
}

bool
ChoiceRule::isFired ()
{
  return lit == 0;
}

void
ChoiceRule::inactivate (const Follows *f)
{
  if (f->aux)
    upper++;
  inactive++;
  if (inactive == 1)
    {
      for (Atom **h = head; h != hend; h++)
	{
	  Atom *b = *h;
	  b->headof--;
	  if (b->Bneg == false)
	    {
	      if (b->headof && (b->source == 0 || b->source == this))
		b->queue_push ();
	      if (b->headof == 0)
		b->efalse_push ();
	      else if (b->headof == 1 && b->Bpos)
		b->backchainTrue ();
	    }
	}
    }
}

void
ChoiceRule::fireInit ()
{
  if (lit == 0 || upper == 0)
    for (Atom **h = head; h != hend; h++)
      {
	(*h)->queue_push ();
	if ((*h)->source == 0)
	  (*h)->source = this;
      }
}

void
ChoiceRule::mightFireInit (const Follows *)
{
  upper--;
  if (upper == 0 && !inactive)
    for (Atom **h = head; h != hend; h++)
      {
	(*h)->queue_push ();
	if ((*h)->source == 0)
	  (*h)->source = this;
      }
}

void
ChoiceRule::unInit ()
{
  upper = pend-pbody;
  lit = end-nbody;
  inactive = 0;
}

void
ChoiceRule::mightFire (const Follows *)
{
  lit--;
}

void
ChoiceRule::backChainTrue ()
{
  if (lit)
    {
      for (Atom **b = nbody; b != nend; b++)
	if ((*b)->Bneg == false)
	  (*b)->efalse_push ();
      for (Atom **b = pbody; b != pend; b++)
	if ((*b)->Bpos == false)
	  (*b)->etrue_push ();
    }
}

void
ChoiceRule::backChainFalse ()
{
}

void
ChoiceRule::backtrackFromActive (const Follows *)
{
  lit++;
}

void
ChoiceRule::backtrackFromInactive (const Follows *f)
{
  if (f->aux)
    upper--;
  inactive--;
  if (inactive == 0)
    for (Atom **h = head; h != hend; h++)
      (*h)->headof++;
}

void
ChoiceRule::propagateFalse (const Follows *)
{
  upper++;
  if (upper == 1 && !inactive)
    for (Atom **h = head; h != hend; h++)
      if ((*h)->closure != false &&
	  ((*h)->source == 0 || (*h)->source == this))
	(*h)->queue_push ();
}

void
ChoiceRule::propagateTrue (const Follows *)
{
  upper--;
  if (upper == 0 && !inactive)
    for (Atom **h = head; h != hend; h++)
      if ((*h)->closure == false)
	{
	  if ((*h)->source == 0)
	    (*h)->source = this;
	  (*h)->queue_push ();
	}
}

void
ChoiceRule::backtrackUpper (const Follows *)
{
  upper--;
}

void
ChoiceRule::search (Atom *)
{
  for (Atom **h = head; h != hend; h++)
    if (!(*h)->visited && (*h)->Bneg == false)
      (*h)->visit ();
  for (Atom **a = nbody; a != nend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  for (Atom **a = pbody; a != pend; a++)
    if (!(*a)->visited && (*a)->Bneg == false)
      (*a)->visit ();
  visited = true;
}

void
ChoiceRule::reduce (bool strongly)
{
  Atom **a;
  Atom **b;
  Atom *t;
  b = head;
  for (a = head; a != hend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((strongly == false || (*b)->Bpos == false)
	  && (*b)->Bneg == false)
	b++;
    }
  hend = b;
  b = nbody;
  for (a = nbody; a != nend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((*b)->Bpos == false && (*b)->Bneg == false)
	b++;
    }
  nend = b;
  b = pbody;
  for (a = pbody; a != pend; a++)
    {
      t = *b;
      *b = *a;
      *a = t;
      if ((strongly == false || (*b)->Bpos == false)
	  && (*b)->Bneg == false)
	b++;
    }
  pend = b;
}

void
ChoiceRule::unreduce ()
{
  hend = nbody;
  nend = pbody;
  pend = end;
}

void
ChoiceRule::setup ()
{
  Atom **a;
  for (a = head; a != hend; a++)
    {
      (*a)->head->r = this;
      (*a)->head->aux = 0;
      (*a)->head++;
    }
  for (a = nbody; a != nend; a++)
    {
      (*a)->negScore--;
      (*a)->neg[(*a)->negScore].r = this;
      (*a)->neg[(*a)->negScore].aux = 0;
    }
  for (a = pbody; a != pend; a++)
    {
      (*a)->posScore--;
      (*a)->pos[(*a)->posScore].r = this;
      (*a)->pos[(*a)->posScore].aux = &pos_aux;
    }
}

void
ChoiceRule::init (Init *init)
{
  long n = init->hsize + init->psize + init->nsize;
  head = new Atom *[n];
  hend = head + init->hsize;
  end = head + n;
  pend = end;
  for (long i = 0; i < init->hsize; i++)
    {
      head[i] = init->head[i];
      head[i]->headof++;
      head[i]->isnant = true;  // Implicit
    }
  nbody = hend;
  lit = init->psize + init->nsize;
  for (long i = 0; i < init->nsize; i++)
    {
      nbody[i] = init->nbody[i];
      nbody[i]->negScore++;
      nbody[i]->isnant = true;
    }
  nend = nbody + init->nsize;
  pbody = nend;
  upper = init->psize;
  for (long i = 0; i < init->psize; i++)
    {
      pbody[i] = init->pbody[i];
      pbody[i]->posScore++;
    }
}

void
ChoiceRule::print ()
{
  Atom **a;
  bool comma = false;
  cout << "{ ";
  for (a = head; a != hend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name ();
      comma = true;
    }
  cout << " }";
  comma = false;
  if (pbody != pend || nbody != nend)
    cout << " :- ";
  for (a = pbody; a != pend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name ();
      comma = true;
    }
  for (a = nbody; a != nend; a++)
    {
      if (comma)
	cout << ", ";
      cout << "not " << (*a)->atom_name ();
      comma = true;
    }
  cout << '.' << endl;
}

void
ChoiceRule::print_internal ()
{
  if (head == hend)
    return;
  cout << (int)type << ' ' << hend-head;
  Atom **a;
  for (a = head; a != hend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;  // Mark as needed in compute statement
    }
  cout << ' ' << (nend-nbody)+(pend-pbody) << ' ' << nend-nbody;
  for (a = nbody; a != nend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  for (a = pbody; a != pend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  cout << endl;
}


WeightRule::WeightRule ()
{
  head = 0;
  bend = 0;
  body = 0;
  end = 0;
  aux = 0;
  reverse = 0;
  minweight = 0;
  maxweight = 0;
  upper_min = 0;
  lower_min = 0;
  atleast = 0;
  max = 0;
  min = 0;
  max_shadow = 0;
  min_shadow = 0;
  visited = false;
  type = WEIGHTRULE;
}

WeightRule::~WeightRule ()
{
  delete[] body;
  delete[] aux;
  delete[] reverse;
}

bool
WeightRule::isInactive ()
{
  return maxweight < atleast;
}

bool
WeightRule::isUpperActive ()
{
  return upper_min >= atleast;
}

bool
WeightRule::isFired ()
{
  return minweight >= atleast;
}

void
WeightRule::inactivate (const Follows *f)
{
  upper_min -= f->aux->weight;
  if (!f->aux->in_loop)
    lower_min -= f->aux->weight;
  bool inactive = (maxweight < atleast);
  maxweight -= f->aux->weight;
  if (minweight < atleast && !inactive)
    {
      if (maxweight < atleast)
	{
	  Atom *b = head;
	  b->headof--;
	  if (b->Bneg == false)
	    {
	      if (b->headof && (b->source == 0 || b->source == this))
		b->queue_push ();
	      if (b->headof == 0)
		b->efalse_push ();
	      else if (b->headof == 1 && b->Bpos)
		b->backchainTrue ();
	    }
	}
      else if (head->Bpos && head->headof == 1)
	backChainTrueShadow ();
    }
  if (maxweight >= atleast
      && lower_min < atleast
      && head->Bneg == false
      && (head->source == 0 || head->source == this))
    head->queue_push ();
}

void
WeightRule::fireInit ()
{
  if (minweight >= atleast)
    {
      head->etrue_push ();
      head->queue_push ();
      head->source = this;
    }
  else if (upper_min >= atleast && maxweight >= atleast)
    {
      head->queue_push ();
      if (head->source == 0)
	head->source = this;
    }
}

void
WeightRule::mightFireInit (const Follows *f)
{
  bool active = (upper_min >= atleast);
  upper_min += f->aux->weight;
  if (!active && upper_min >= atleast && maxweight >= atleast)
    {
      head->queue_push ();
      if (head->source == 0)
	head->source = this;
    }
}

void
WeightRule::unInit ()
{
  upper_min = 0;
  maxweight = 0;
  minweight = 0;
  for (Atom **a = body; a != bend; a++)
    {
      maxweight += aux[a-body].weight;
      if (!aux[a-body].positive)
	upper_min += aux[a-body].weight;
    }
  lower_min = upper_min;
}

void
WeightRule::mightFire (const Follows *f)
{
  bool frd = (minweight >= atleast);
  minweight += f->aux->weight;
  if (!frd && maxweight >= atleast)
    {
      if (minweight >= atleast)
	{
	  if (head->Bpos == false)
	    head->etrue_push ();
	}
      else if (head->Bneg)
	backChainFalseShadow ();
    }
}

Atom **
WeightRule::chainTrue (Atom **a)
{
  while (a != bend)
    {
      if ((*a)->Bpos || (*a)->Bneg)
	a++;
      else if (maxweight - aux[a-body].weight < atleast)
	{
	  if (aux[a-body].positive)
	    (*a)->etrue_push ();
	  else
	    (*a)->efalse_push ();
	  a++;
	}
      else
	break;
    }
  return a;
}

Atom **
WeightRule::chainFalse (Atom **a)
{
  while (a != bend)
    {
      if ((*a)->Bpos || (*a)->Bneg)
	a++;
      else if (minweight + aux[a-body].weight >= atleast)
	{
	  if (aux[a-body].positive)
	    (*a)->efalse_push ();
	  else
	    (*a)->etrue_push ();
	  a++;
	}
      else
	break;
    }
  return a;
}

void
WeightRule::backChainTrue ()
{
  if (minweight < atleast)
    {
      max_shadow = chainTrue (max);
      while (max != bend && ((*max)->Bpos || (*max)->Bneg))
	max++;
    }
}

void
WeightRule::backChainFalse ()
{
  if (maxweight >= atleast && minweight < atleast)
    {
      min_shadow = chainFalse (min);
      while (min != bend && ((*min)->Bpos || (*min)->Bneg))
	min++;
    }
}

void
WeightRule::backChainTrueShadow ()
{
  max_shadow = chainTrue (max_shadow);
  while (max != bend && ((*max)->Bpos || (*max)->Bneg))
    max++;
}

void
WeightRule::backChainFalseShadow ()
{
  min_shadow = chainFalse (min_shadow);
  while (min != bend && ((*min)->Bpos || (*min)->Bneg))
    min++;
}

void
WeightRule::backtrack (Atom **a)
{
  if (max > a)
    max = a;
  if (min > a)
    min = a;
  max_shadow = max;
  min_shadow = min;
}

void
WeightRule::backtrackFromActive (const Follows *f)
{
  minweight -= f->aux->weight;
  backtrack (f->aux->a);
}

void
WeightRule::backtrackFromInactive (const Follows *f)
{
  upper_min += f->aux->weight;
  if (!f->aux->in_loop)
    lower_min += f->aux->weight;
  bool inactive = (maxweight < atleast);
  maxweight += f->aux->weight;
  if (inactive && maxweight >= atleast)
    head->headof++;
  backtrack (f->aux->a);
}

void
WeightRule::propagateFalse (const Follows *f)
{
  bool active = (upper_min >= atleast);
  bool inactive = (maxweight < atleast);
  upper_min -= f->aux->weight;
  if (!inactive && active && lower_min < atleast &&
      head->closure != false &&
      (head->source == 0 || head->source == this))
    head->queue_push ();
}

void
WeightRule::propagateTrue (const Follows *f)
{
  bool active = (upper_min >= atleast);
  bool inactive = (maxweight < atleast);
  upper_min += f->aux->weight;
  if (!inactive && !active && upper_min >= atleast &&
      head->closure == false)
    {
      if (head->source == 0)
	head->source = this;
      head->queue_push ();
    }
}

void
WeightRule::backtrackUpper (const Follows *f)
{
  upper_min += f->aux->weight;
}

void
WeightRule::in_loop (Follows *f)
{
  if (!f->aux->in_loop)
    lower_min -= f->aux->weight;
  f->aux->in_loop = true;
}

void
WeightRule::not_in_loop (Follows *f)
{
  if (f->aux->in_loop)
    lower_min += f->aux->weight;
  f->aux->in_loop = false;
}

void
WeightRule::search (Atom *a)
{
  if (a->Bneg || (a->Bpos && a != head))
    return;
  if (!head->visited && head->Bneg == false)
    head->visit ();
  for (Atom **b = body; b != bend; b++)
    if (!(*b)->visited && (*b)->Bneg == false)
      (*b)->visit ();
  visited = true;
}

void
WeightRule::reduce (bool strongly)
{
  Atom **b = body;
  for (Atom **a = body; a != bend; a++)
    {
      swap (b-body, a-body);
      if (aux[b-body].positive)
	{
	  if ((strongly == false || (*b)->Bpos == false) &&
	      (*b)->Bneg == false)
	    b++;
	}
      else if ((*b)->Bpos == false && (*b)->Bneg == false)
	b++;
    }
  bend = b;
  backtrack (body);
}

void
WeightRule::unreduce ()
{
  bend = end;
  backtrack (body);
  sort ();
}

void
WeightRule::initWeight (Weight w)
{
  maxweight += w;
}

void
WeightRule::swap (long a, long b)
{
  Auxiliary tau = aux[a];
  aux[a] = aux[b];
  aux[b] = tau;
  aux[a].a = body+a;
  aux[b].a = body+b;
  Atom *ta = body[a];
  body[a] = body[b];
  body[b] = ta;
  if (reverse[a])
    reverse[a]->aux = aux+b;
  if (reverse[b])
    reverse[b]->aux = aux+a;
  Follows *tf = reverse[a];
  reverse[a] = reverse[b];
  reverse[b] = tf;
}

bool
WeightRule::smaller (long a, long b)
{
  if (aux[a].weight < aux[b].weight)
    return true;
  else
    return false;
}

void
WeightRule::heap (long k, long e)
{
  long a = 2*k+1;
  while (a < e)
    {
      if (smaller (a+1, a))
	a++;
      if (smaller (a, k))
	swap (a, k);
      else
	break;
      k = a;
      a = 2*k+1;
    }
  if (a == e && smaller (a, k))
    swap (a, k);
}

void
WeightRule::sort ()
{
  long i;
  long e = bend-body-1;
  for (i = (e-1)/2; i >= 0; i--)
    heap (i, e);
  i = e;
  while (i > 0)
    {
      swap (i, 0);
      i--;
      heap (0, i);
    }
}

void
WeightRule::swapping (Follows *f, Follows *g)
{
  if (f->aux)
    reverse[f->aux->a - body] = g;
}

void
WeightRule::setup ()
{
  head->head->r = this;
  head->head->aux = 0;
  head->head++;
  Atom **a;
  sort ();
  for (a = body; a != bend; a++)
    if (aux[a-body].positive)
      {
	(*a)->posScore--;
	(*a)->pos[(*a)->posScore].r = this;
	(*a)->pos[(*a)->posScore].aux = &aux[a-body];
	reverse[a-body] = (*a)->pos + (*a)->posScore;
	initWeight (aux[a-body].weight);
      }
    else
      {
	(*a)->negScore--;
	(*a)->neg[(*a)->negScore].r = this;
	(*a)->neg[(*a)->negScore].aux = &aux[a-body];
	reverse[a-body] = (*a)->neg + (*a)->negScore;
	upper_min += aux[a-body].weight;
	initWeight (aux[a-body].weight);
      }
  lower_min = upper_min;
}

void
WeightRule::init (Init *init)
{
  head = *init->head;
  head->headof++;
  atleast = init->atleast_weight;
  maxweight = 0;
  minweight = 0;
  long n = init->psize + init->nsize;
  if (n != 0)
    {
      body = new Atom *[n];
      aux = new Auxiliary[n];
      reverse = new Follows *[n];
    }
  else
    {
      body = 0;
      aux = 0;
      reverse = 0;
    }
  bend = body+n;
  end = bend;
  max = body;
  min = body;
  max_shadow = body;
  min_shadow = body;
  long i;
  for (i = 0; i < init->nsize; i++)
    {
      body[i] = init->nbody[i];
      body[i]->negScore++;
      body[i]->isnant = true;
      aux[i].a = body+i;
      aux[i].weight = init->nweight[i];
      aux[i].positive = false;
      aux[i].in_loop = false;
      reverse[i] = 0;
    }
  for (long j = 0; j < init->psize; j++, i++)
    {
      body[i] = init->pbody[j];
      body[i]->posScore++;
      aux[i].a = body+i;
      aux[i].weight = init->pweight[j];
      aux[i].positive = true;
      aux[i].in_loop = true;
      reverse[i] = 0;
    }
}

void
WeightRule::print ()
{
  cout << head->atom_name () << " :- { ";
  int comma = 0;
  for (Atom **a = body; a != bend; a++)
    if (aux[a-body].positive)
      {
	if (comma)
	  cout << ", ";
	cout << (*a)->atom_name () << " = " << aux[a-body].weight;
	comma = 1;
      }
    else
      {
	if (comma)
	  cout << ", ";
	cout << "not " << (*a)->atom_name () << " = "
	     << aux[a-body].weight;
	comma = 1;
      }
  cout << "} >=" << atleast << '.' << endl;
}

void
WeightRule::print_internal ()
{
# ifdef USEDOUBLE
  cout.precision (DBL_DIG);
# endif
  cout << (int)type << ' ' << head->posScore;
  head->negScore = 1;
  cout << ' ' << (minweight >= atleast ? WEIGHT_MIN : atleast-minweight) << ' ' << bend-body;
  long neg = 0;
  Atom **a;
  for (a = body; a != bend; a++)
    if (aux[a-body].positive == false)
      neg++;
  cout << ' ' << neg;
  for (a = body; a != bend; a++)
    if (aux[a-body].positive == false)
      {
	cout << ' ' << (*a)->posScore;
	(*a)->negScore = 1;
      }
  for (a = body; a != bend; a++)
    if (aux[a-body].positive == true)
      {
	cout << ' ' << (*a)->posScore;
	(*a)->negScore = 1;
      }
  for (a = body; a != bend; a++)
    if (aux[a-body].positive == false)
      cout << ' ' << aux[a-body].weight;
  for (a = body; a != bend; a++)
    if (aux[a-body].positive == true)
      cout << ' ' << aux[a-body].weight;
  cout << endl;
}


OptimizeRule::OptimizeRule ()
{
  nbody = 0;
  pbody = 0;
  nend = 0;
  pend = 0;
  end = 0;
  aux = 0;
  minweight = 0;
  minoptimum = 0;
  heuristic_best = 0;
  visited = false;
  type = OPTIMIZERULE;
  next = 0;
}

OptimizeRule::~OptimizeRule ()
{
  delete[] nbody;
  delete[] aux;
}

void
OptimizeRule::setOptimum ()
{
  minoptimum = minweight;
}

bool
OptimizeRule::isInactive ()
{
  return false;  // Reduce(Strongly|Weakly) needs this
}

bool
OptimizeRule::isUpperActive ()
{
  return false;
}

bool
OptimizeRule::isFired ()
{
  return true;
}

void
OptimizeRule::inactivate (const Follows *)
{
}

void
OptimizeRule::fireInit ()
{
}

void
OptimizeRule::mightFireInit (const Follows *)
{
}

void
OptimizeRule::unInit ()
{
  minweight = 0;
  minoptimum = WEIGHT_MAX;
}

void
OptimizeRule::mightFire (const Follows *f)
{
  minweight += f->aux->weight;
}

void
OptimizeRule::backChainTrue ()
{
}

void
OptimizeRule::backChainFalse ()
{
}

void
OptimizeRule::backtrackFromActive (const Follows *f)
{
  minweight -= f->aux->weight;
}

void
OptimizeRule::backtrackFromInactive (const Follows *)
{
}

void
OptimizeRule::propagateFalse (const Follows *)
{
}

void
OptimizeRule::propagateTrue (const Follows *)
{
}

void
OptimizeRule::backtrackUpper (const Follows *)
{
}

void
OptimizeRule::search (Atom *a)
{
  if (a->Bneg || a->Bpos)
    return;
  Atom **b;
  for (b = nbody; b != nend; b++)
    if (!(*b)->visited && (*b)->Bneg == false && (*b)->Bpos == false)
      (*b)->visit ();
  for (b = pbody; b != pend; b++)
    if (!(*b)->visited && (*b)->Bneg == false && (*b)->Bpos == false)
      (*b)->visit ();
  visited = true;
}

void
OptimizeRule::reduce (bool)
{
}

void
OptimizeRule::unreduce ()
{
}

void
OptimizeRule::setup ()
{
  Atom **a;
  for (a = nbody; a != nend; a++)
    {
      (*a)->negScore--;
      (*a)->neg[(*a)->negScore].r = this;
      (*a)->neg[(*a)->negScore].aux = &aux[a-nbody];
    }
  for (a = pbody; a != pend; a++)
    {
      (*a)->posScore--;
      (*a)->pos[(*a)->posScore].r = this;
      (*a)->pos[(*a)->posScore].aux = &aux[a-nbody];
    }
}

void
OptimizeRule::init (Init *init)
{
  minweight = 0;
  minoptimum = WEIGHT_MAX;
  long n = init->psize + init->nsize;
  if (n != 0)
    {
      nbody = new Atom *[n];
      aux = new Auxiliary[n];
    }
  else
    {
      nbody = 0;
      aux = 0;
    }
  end = nbody+n;
  long i;
  for (i = 0; i < init->nsize; i++)
    {
      nbody[i] = init->nbody[i];
      nbody[i]->negScore++;
      aux[i].weight = init->nweight[i];
    }
  nend = nbody + init->nsize;
  pbody = nend;
  pend = pbody + init->psize;
  for (long j = 0; j < init->psize; j++, i++)
    {
      pbody[j] = init->pbody[j];
      pbody[j]->posScore++;
      aux[i].weight = init->pweight[j];
    }
}

void
OptimizeRule::print ()
{
  cout << "minimize { ";
  Atom **a;
  int comma = 0;
  for (a = pbody; a != pend; a++)
    {
      if (comma)
	cout << ", ";
      cout << (*a)->atom_name () << " = " << aux[a-nbody].weight;
      comma = 1;
    }
  for (a = nbody; a != nend; a++)
    {
      if (comma)
	cout << ", ";
      cout << "not " << (*a)->atom_name () << " = " << aux[a-nbody].weight;
      comma = 1;
    }
  cout << " }" << endl;
}

void
OptimizeRule::print_internal ()
{
# ifdef USEDOUBLE
  cout.precision (DBL_DIG);
# endif
  cout << (int)type << ' ' << 0;
  cout << ' ' << (nend-nbody)+(pend-pbody) << ' ' << nend-nbody;
  Atom **a;
  for (a = nbody; a != nend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  for (a = pbody; a != pend; a++)
    {
      cout << ' ' << (*a)->posScore;
      (*a)->negScore = 1;
    }
  for (a = nbody; a != nend; a++)
    cout << ' ' << aux[a-nbody].weight;
  for (a = pbody; a != pend; a++)
    cout << ' ' << aux[a-nbody].weight;
  cout << endl;
}
