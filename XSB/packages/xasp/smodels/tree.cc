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
#include <string.h>
#include "atomrule.h"
#include "tree.h"

using namespace std;

Tree::Node::Node (Atom *k, Node *l, Node *r)
{
  key = k;
  left = l;
  right = r;
}

Tree::Node::~Node ()
{
}

Tree::Tree ()
{
  root = 0;
}

Tree::~Tree ()
{
  flush (root);
}

void
Tree::flush (Node *n)
{
  if (n == 0)
    return;
  flush (n->left);
  flush (n->right);
  delete n;
}

Tree::Node *
Tree::splay (const char *key, Node *root)
{
  Node n;
  Node *l = &n;
  Node *r = &n;
  Node *t;
  for (;;)
    {
      int c = compare (key, root->key);
      if (c < 0)
	{
	  if (root->left == 0)
	    break;
	  if (compare (key, root->left->key) < 0)
	    {
	      t = root->left;
	      root->left = t->right;
	      t->right = root;
	      root = t;
	      if (root->left == 0)
		break;
	    }
	  r->left = root;
	  r = root;
	  root = root->left;
        }
      else if (c > 0)
	{
	  if (root->right == 0)
	    break;
	  if (compare (key, root->right->key) > 0)
	    {
	      t = root->right;
	      root->right = t->left;
	      t->left = root;
	      root = t;
	      if (root->right == 0)
		break;
	    }
	  l->right = root;
	  l = root;
	  root = root->right;
        }
      else
	break;
    }
  l->right = root->left;
  r->left = root->right;
  root->left = n.right;
  root->right = n.left;
  return root;
}

Atom *
Tree::find (const char *key)
{
  if (root)
    {
      root = splay (key, root);
      if (compare (key, root->key) == 0)
	return root->key;
    }
  return 0;
}

void
Tree::insert (Atom *key)
{
  if (root == 0)
    {
      root = new Node (key);
      return;
    }
  root = splay (key->name, root);
  int c = compare (key->name, root->key);
  if (c < 0)
    {
      Node *t = root;
      root = new Node (key, root->left, root);
      t->left = 0;
    }
  else if (c > 0)
    {
      Node *t = root;
      root = new Node (key, root, root->right);
      t->right = 0;
    }
}

void
Tree::remove (Atom *key)
{
  if (root == 0)
    return;
  Node *t = splay (key->name, root);
  if (compare (key->name, t->key) == 0)
    {
      if (t->left == 0)
	root = t->right;
      else
	{
	  root = splay (key->name, t->left);
	  root->right = t->right;
	}
      delete t;
    }
  else
    root = t;
}

void
Tree::check_consistency ()
{
  check_consistency (root);
}

void
Tree::check_consistency (Node *n)
{
  if (n == 0)
    return;
  if (n->left && compare (n->left->key->name, n->key) > 0)
    {
      cerr << "error: key " << n->key << " smaller than key "
	   << n->left->key << endl;
      return;
    }
  if (n->right && compare (n->right->key->name, n->key) < 0)
    {
      cerr << "error: key " << n->key << " larger than key "
	   << n->right->key << endl;
      return;
    }
  check_consistency (n->left);
  check_consistency (n->right);
}

int
Tree::compare (const char *a, Atom *b)
{
  return strcmp (a, b->name);
}
