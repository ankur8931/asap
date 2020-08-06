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
#ifndef SMODELS_H
#define SMODELS_H

#include "program.h"
#include "dcl.h"
#include "stack.h"

class Atom;
class OptimizeRule;
class RestartNode;
class Restart;

class Smodels
{
public:
  Smodels ();
  virtual ~Smodels ();
  void init ();

  void setToBTrue (Atom *a);
  void setToBFalse (Atom *a);
  void pick (bool look=true, bool simple=false);
  void lazy_lookahead ();
  void simple_pick ();
  void expand ();
  void setup ();
  void setup_with_lookahead ();
  void revert ();
  bool conflict ();
  bool covered ();
  void set_conflict ();
  Atom *unwind ();
  void unwind_to (long);
  Atom *backtrack ();
  Atom *backjump ();
  void backtrack (bool jump);
  int smodels (bool look, bool jump, bool simple=false);
  int smodels_restart(bool look, bool simple=false);
  int model (bool look=true, bool jump=false, bool simple=false);
  int wellfounded ();
  RestartNode *restart(RestartNode *root, Restart &r);
  bool testPos (Atom *);
  bool testNeg (Atom *);
  void testScore (Atom *, long i);
  bool lookahead_no_heuristic ();
  void heuristic ();
  void optimize_heuristic ();
  void choose ();
  void lookahead ();
  void improve ();
  void reset_dependency ();
  void clear_dependency ();
  void add_dependency (Atom *, bool);
  void reset_queues ();
  void shuffle ();
  void print ();
  void printAnswer ();
  void printStack ();
  void printRule (Rule *);

  void removeAtom (long);
  void addAtom (Atom *);

  Dcl dcl;
  Program program;
  Atom **atom;           // The atoms in the program. We walk the
			 // array when we do lookahead or compute the
			 // heuristic.
  long atomtop;          // Atoms on and above atomtop are in B and
			 // can be ignored.
  unsigned long score;   // Heuristics...
  Stack depends;         // Holds atoms with dependson{True,False} true
  Stack stack;           // Holds the backtrack information (the atoms
			 // in B). When full we have a stable model.
  long setup_top;        // Top of stack after setup
  long guesses;          // The number of atoms on the stack that are
			 // choice points. If its zero and we try to
			 // backtrack, then we have exhausted the
			 // search space and we should --
  bool fail;             // fail.
  bool conflict_found;   // If we have found a conflict this is true
  unsigned long max_models; // The number of models we compute before we
			    // stop.
  unsigned long max_conflicts; // The number of conflicts before we restart.
  long level;            // Mustn't backjump below this stack level.
  long lasti;            // Speed up lookahead by starting where we
			 // last stopped doing lookahead.
  Atom *cnflct;          // Backjump...
  OptimizeRule *optimizerule; // List of optimizerules
  unsigned long hiscore1;// Most significant heuristic score
  unsigned long hiscore2;// Least significant heuristic score
  bool hi_is_positive;   // Most significant score is for positive literal
  long hi_index;         // Index of heuristic choice in array atom
  unsigned long wrong_choices; // For computing an estimated lower
			       // bound on the score.
  bool use_lookahead;
  unsigned long answer_number;
  unsigned long number_of_choice_points;
  unsigned long number_of_wrong_choices;
  unsigned long number_of_backjumps;
  unsigned long number_of_picked_atoms;
  unsigned long number_of_forced_atoms;
  unsigned long number_of_assignments;
  unsigned long size_of_searchspace;
  unsigned long number_of_denants;
  unsigned long number_of_restarts;
};

#endif
