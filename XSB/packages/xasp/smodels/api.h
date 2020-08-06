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
#ifndef API_H
#define API_H
#include "defines.h"

class Program;
class Atom;
class Tree;
struct Init;

class Api
{
public:
  Api (Program *);
  virtual ~Api ();

  void begin_rule (RuleType);  // First call begin_rule
  void add_head (Atom *);      // Then call these
  void add_body (Atom *, bool pos);         // Add atom to body of rule
  void add_body (Atom *, bool pos, Weight); // Add atom with weight to body
  void change_body (long i, bool pos, Weight); // Change weight in body
  void set_atleast_weight (Weight);   // Weight rule
  void set_atleast_body (long);       // Constraint rule
  void set_atleast_head (long);       // Generate rule
  void end_rule ();                   // Finally, end with end_rule

  virtual Atom *new_atom ();        // Create new atom
  void set_compute (Atom *, bool);  // Compute statement
  void reset_compute (Atom *, bool);  // Compute statement
  void set_name (Atom *, const char *);
  void remember ();  // Remember the set_name calls after this
  void forget ();    // Forget the set_name calls after this
  Atom *get_atom (const char *); // get_atom only works for the
                                 // set_name calls that have
                                 // been remembered
  void copy (Api *);             // Make a copy of the program
  void done ();  // Call done after the program is completely defined
		 // and before you begin to compute

protected:
  void set_init ();

  Program * const program;
  RuleType type;
  class list {
  public:
    list ();
    ~list ();
    void push (Atom *a, Weight w = 0);
    void reset ();
    void grow ();

    Atom **atoms;
    Weight *weights;
    int top;
    int size;
  };
  long size (list &);
  list pbody;
  list nbody;
  list head;
  Weight atleast_weight;
  long atleast_body;
  long atleast_head;
  bool maximize;
  Tree *tree;
  Tree *pointer_to_tree;
  Init *init;
};

#endif
