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
#ifndef TREE_H
#define TREE_H

class Atom;

class Tree
{
public:
  Tree ();
  ~Tree ();

  Atom *find (const char *);
  void insert (Atom *);
  void remove (Atom *);

  void check_consistency ();

private:
  class Node
  {
  public:
    Node (Atom *k = 0, Node *l = 0, Node *r = 0);
    ~Node ();

    Atom *key;
    Node *left;
    Node *right;
  };

  Node *root;

  Node *splay (const char *, Node *);

  void check_consistency (Node *);
  void flush (Node *);

  int compare (const char *, Atom *);
};

#endif
