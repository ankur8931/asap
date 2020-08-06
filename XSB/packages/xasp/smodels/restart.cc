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
#include <assert.h>
#include "restart.h"

Restart::Restart()
{
  level = 0;
  current = 0;
}

unsigned long
Restart::restart_after()
{
  if (current > level)
    {
      current = 0;
      level++;
    }
  unsigned long r = 1 << 3+current++;
  if (r)
    return r;
  else
    return (unsigned long)(-1);
}

RestartNode::RestartNode()
{
  backtracked = false;
  pebble = false; // This path has been trodden
  length = 0;
  computed = 0;
  computed_positive = 0;
  stack_top = 0;
  parent = 0;
  child = 0;
  sibling = 0;
}

RestartNode::~RestartNode()
{
  delete[] computed;
  delete[] computed_positive;
}

static void make_nodes(RestartNode *parent, Atom **stack, Atom **end, long top)
{
  // Add children to the parent node
  Atom **a;
  for (a = stack; a < end; a++)
    if ((*a)->guess | (*a)->forced | (*a)->backtracked)
      break;
  for (; a < end;)
    {
      RestartNode *child = new RestartNode();
      child->stack_top = top+(a-stack);
      if ((*a)->guess == false)
	child->backtracked = true;
      child->length++; // To include atom *a
      Atom **b = a+1;
      for (; b < end; b++)
	{
	  if ((*b)->forced | (*b)->backtracked)
	    child->length++;
	  else if ((*b)->guess)
	      break;
	}
      child->computed = new Atom *[child->length];
      child->computed_positive = new bool[child->length];
      int i = 0;
      for (Atom **c = a; c < b; c++)
	{
	  if ((*c)->guess | (*c)->forced | (*c)->backtracked)
	    {
	      child->computed[i] = *c;
	      child->computed_positive[i] = (*c)->Bpos;
	      i++;
	    }
	}
      a = b;
      child->parent = parent;
      if (parent->child)
	{
	  parent->child->sibling = child;
	  child->sibling = parent->child;
	}
      else
	parent->child = child;
      parent = child;
    }
}

void
RestartNode::add_node(Atom **stack, Atom **end, long top)
{
  if (child == 0)
    make_nodes(this, stack, end, top);
  else
    {
      // Find the leaf node, and add the new node there
      RestartNode *n = child;
      for (Atom **a = stack; a < end; a++)
	{
	  if ((*a)->guess | (*a)->backtracked)
	    {
	      if (*a == n->computed[0])
		{
		  if ((*a)->Bpos != n->computed_positive[0])
		    {
		      if (n->sibling)
			n = n->sibling;
		      else
			{ // Add new nodes in the sibling
			  make_nodes(n->parent, a, end, top+(a-stack));
			  break;
			}
		    }
		  if (n->child)
		    n = n->child;
		  else
		    { // Add new nodes in the child
		      make_nodes(n, a, end, top+(a-stack));
		      break;
		    }
		}
	    }
	}
    }
}

RestartNode *find_restart(RestartNode *n)
{
  if (n->child)
    {
      if (n->child->sibling)
	{
	  if (n->child->pebble)
	    n = n->child->sibling;
	  else
	    n = n->child;
	  n->pebble = true;
	  n->sibling->pebble = false;
	  if (n->child)
	    n->stop = false;
	  else
	    n->stop = true;
	}
      else
	{
	  n = n->child;
	  n->pebble = true;
	  if (n->child == 0 || n->backtracked == false)
	    n->stop = true;
	  else
	    n->stop = false;
	}
      return n;
    }
  n->stop = true;
  return n;
}
