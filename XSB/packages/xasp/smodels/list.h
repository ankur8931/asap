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
#ifndef LIST_H
#define LIST_H

class Rule;
class Atom;

class Node
{
public:
  Node () { next = 0; rule = 0; };
  ~Node () {};

  Node *next;
  union {
    Rule *rule;
    Atom *atom;
  };
};

class List
{
public:
  List ();
  ~List () {};

  void push (Node *n);
  Node *head ();

private:
  Node list;
  Node *tail;
};

class AtomList : public List
{
public:
  AtomList () {};
  ~AtomList ();

  void push (Atom *);
};

class RuleList : public List
{
public:
  RuleList () {};
  ~RuleList ();

  void push (Rule *);
};


inline Node *
List::head ()
{
  return list.next;
}

inline void
List::push (Node *n)
{
  tail->next = n;
  tail = n;
}

inline void
AtomList::push (Atom *a)
{
  Node *n = new Node;
  n->atom = a;
  List::push (n);
}

inline void
RuleList::push (Rule *r)
{
  Node *n = new Node;
  n->rule = r;
  List::push (n);
}

#endif
