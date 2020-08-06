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
#ifndef ATOMRULE_H
#define ATOMRULE_H

#include "defines.h"

class Rule;
class Atom;
class Program;

struct Auxiliary
{
  Auxiliary (bool p = false) { positive = p; in_loop = p; };
  Weight weight; // Local weight
  bool positive : 1; // Is the literal an atom?
  bool in_loop : 1;  // Is the atom in a strongly connected component?
  Atom **a;          // Position in rule body
};

struct Follows
{
  Rule *r;
  Auxiliary *aux;
};

struct Init
{
  long hsize;
  long psize;
  long nsize;
  Atom **head;
  Atom **pbody;
  Atom **nbody;
  Weight *pweight;
  Weight *nweight;
  Weight atleast_weight;
  long atleast_body;
  long atleast_head;
  bool maximize;
};

class Atom
{
public:
  Atom (Program *p);
  ~Atom ();

  void setBTrue ();
  void setBFalse ();
  void setTrue ();
  void backtrackFromBTrue ();
  void backtrackFromBFalse ();
  void backchainTrue ();
  void backchainFalse ();
  void reduce_head ();
  void reduce_pbody (bool strong);
  void reduce_nbody (bool strong);
  void reduce (bool strongly);
  void unreduce ();
  const char *atom_name ();
  void etrue_push ();
  void efalse_push ();
  void queue_push ();
  void visit ();

  Program * const p;  // The program in which the atom appears
  Follows *endHead;   // Sentinel of the head array
  Follows *endPos;    // Sentinel of the pos array
  Follows *endNeg;    // Sentinel of the neg array
  Follows *endUpper;  // Sentinel of the upper array = pos array, used
		      // to localize and thereby speed up the upper
		      // closure computation
  Follows *end;       // The end of the array
  Follows *head;      // The rules whose heads are atom
  Follows *pos;       // The rules in which atom appears positively
		      // (allocated in head)
  Follows *neg;       // The rules in which atom appears negatively
		      // (allocated in head)
  union {
    Rule *source;     // The rule that keeps this atom in the upper closure
    Atom *copy;       // Only used when copying
  };
  long headof;        // The number of rules in the head array whose
		      // inactive counter is zero
  bool closure : 1;       // True if the atom is in the upper closure
  bool Bpos : 1;          // True if the atom is in B+
  bool Bneg : 1;          // True if the atom is in B-
  bool visited : 1;       // For use in backjumping and improvement of
		          // upper closure
  bool guess : 1;         // True if the atom currently is a guess
  bool isnant : 1;        // True if the atom is in NAnt(P)
  bool dependsonTrue : 1; // Used by lookahead
  bool dependsonFalse : 1;// Used by lookahead
  bool computeTrue : 1;   // Compute statement
  bool computeFalse : 1;  // Compute statement
  bool backtracked : 1;   // For use in printStack
  bool forced : 1;        // For use in printStack
  bool in_etrue_queue : 1;// Atom is in equeue, will get Bpos == true
  bool in_efalse_queue : 1;// Atom is in equeue, will get Bneg == true
  bool in_queue : 1;      // Atom is in queue
  bool pos_tested : 1;    // True if the atom been tested in lookahead
  bool neg_tested : 1;    // True if the atom been tested in lookahead
  unsigned long posScore; // Used in heuristic, in improvement of upper
		          // closure, and in print_internal
  unsigned long negScore; // Used in heuristic, in improvement of upper
		          // closure, and in print_internal
  unsigned long wrong_choices; // The number of wrong choices when
			       // this atom was chosen or backtracked.
  char *name;             // The name of the atom
};

class Rule
{
public:
  Rule () { visited = false; type = ENDRULE; };
  virtual ~Rule () {};

  virtual void inactivate (const Follows *) = 0;
  virtual void fireInit () = 0;
  virtual void mightFireInit (const Follows *) = 0;
  virtual void unInit () = 0;
  virtual void mightFire (const Follows *) = 0;
  virtual void backChainTrue () = 0;
  virtual void backChainFalse () = 0;
  virtual void backtrackFromActive (const Follows *) = 0;
  virtual void backtrackFromInactive (const Follows *) = 0;
  virtual void propagateFalse (const Follows *) = 0;
  virtual void propagateTrue (const Follows *) = 0;
  virtual void backtrackUpper (const Follows *) = 0;

  virtual bool isInactive () = 0;
  virtual bool isUpperActive () = 0;
  virtual void in_loop (Follows *) {};
  virtual void not_in_loop (Follows *) {};
  virtual void search (Atom *) = 0;
  virtual void reduce (bool strongly) = 0;
  virtual void unreduce () = 0;
  virtual void swapping (Follows *, Follows *) {};
  virtual void setup () = 0;
  virtual void init (Init *) = 0;
  virtual void print () {};
  virtual void print_internal () {};

  RuleType type;
  bool visited;      // For use in backjumping only
};

class HeadRule : public Rule
{
public:
  HeadRule () { head = 0; };
  virtual ~HeadRule () {};

  Atom *head;        // The head of the rule
};

class BasicRule : public HeadRule
{
public:
  BasicRule ();
  virtual ~BasicRule ();

  void inactivate (const Follows *);
  void fireInit ();
  void mightFireInit (const Follows *);
  void unInit ();
  void mightFire (const Follows *);
  void backChainTrue ();
  void backChainFalse ();
  void backtrackFromActive (const Follows *);
  void backtrackFromInactive (const Follows *);
  void propagateFalse (const Follows *);
  void propagateTrue (const Follows *);
  void backtrackUpper (const Follows *);

  bool isInactive ();
  bool isUpperActive ();
  bool isFired ();
  void search (Atom *);
  void reduce (bool strongly);
  void unreduce ();
  void setup ();
  void init (Init *);
  void print ();
  void print_internal ();

  Atom **nbody;      // The negative literals in the body
  Atom **pbody;      // The positive literals in the body (allocated
		     // in nbody)
  Atom **nend;       // Sentinel of the nbody array
  Atom **pend;       // Sentinel of the pbody array
  Atom **end;        // The end of the body array

  long lit;          // The number of negative literals in body
                     // that are not in B- plus the number of positive
                     // literals in body that are not in B+
  long upper;        // The number of positive literals in body that
		     // are in the upper closure
  long inactive;     // The number of positive literals in B- plus
		     // the number of negative literals in B+
};

class ConstraintRule : public HeadRule
{
public:
  ConstraintRule ();
  virtual ~ConstraintRule ();

  void inactivate (const Follows *);
  void fireInit ();
  void mightFireInit (const Follows *);
  void unInit ();
  void mightFire (const Follows *);
  void backChainTrue ();
  void backChainFalse ();
  void backtrackFromActive (const Follows *);
  void backtrackFromInactive (const Follows *);
  void propagateFalse (const Follows *);
  void propagateTrue (const Follows *);
  void backtrackUpper (const Follows *);

  bool isInactive ();
  bool isUpperActive ();
  bool isFired ();
  void in_loop (Follows *);
  void not_in_loop (Follows *);
  void search (Atom *);
  void reduce (bool strongly);
  void unreduce ();
  void setup ();
  void init (Init *);
  void print ();
  void print_internal ();

  Atom **nbody;      // The negative literals in the body
  Atom **pbody;      // The positive literals in the body (allocated
		     // in nbody)
  Atom **nend;       // Sentinel of the nbody array
  Atom **pend;       // Sentinel of the pbody array
  Atom **end;        // The end of the body array

  long lit;          // The number of negative literals in body
                     // that are not in B- plus the number of positive
                     // literals in body that are not in B+
  long upper;        // atleast-(nend-nbody) - the number of literals
                     // in body that are in the upper closure
  long lower;        // atleast - the number of negative
                     // literals in body that are in B-
  long inactive;     // The number of positive literals in B- plus
		     // the number of negative literals in B+
  long atleast;      // If atleast literals are in B then fire
};

class GenerateRule : public Rule
{
public:
  GenerateRule ();
  virtual ~GenerateRule ();

  void inactivate (const Follows *);
  void inactivate ();
  void fireInit ();
  void mightFireInit (const Follows *);
  void unInit ();
  void mightFire (const Follows *);
  void backChainTrue ();
  void backChainFalse ();
  void backtrackFromActive (const Follows *);
  void backtrackFromInactive (const Follows *);
  void propagateFalse (const Follows *);
  void propagateTrue (const Follows *);
  void backtrackUpper (const Follows *);

  bool isInactive ();
  bool isUpperActive ();
  bool isFired ();
  void search (Atom *);
  void reduce (bool strongly);
  void unreduce ();
  void setup ();
  void init (Init *);
  void print ();
  void print_internal ();

  Atom **head;       // The heads of the rule
  Atom **hend;       // Sentinel of the head array
  Atom **pbody;      // The atoms in the body (allocated in head)
  Atom **pend;       // Sentinel of the body array
  Atom **end;        // The end of the array

  long pos;          // The number of positive literals in body that
                     // are not in B+
  long upper;        // The number of positive literals in body that
		     // are in the upper closure
  long neg;          // The number of negative literals in body
                     // that are not in B-
  long inactivePos;  // The number of positive literals in B-
  long inactiveNeg;  // The number of negative literals (heads) in B+
  long atleast;
};

class ChoiceRule : public Rule
{
public:
  ChoiceRule ();
  virtual ~ChoiceRule ();

  void inactivate (const Follows *);
  void fireInit ();
  void mightFireInit (const Follows *);
  void unInit ();
  void mightFire (const Follows *);
  void backChainTrue ();
  void backChainFalse ();
  void backtrackFromActive (const Follows *);
  void backtrackFromInactive (const Follows *);
  void propagateFalse (const Follows *);
  void propagateTrue (const Follows *);
  void backtrackUpper (const Follows *);

  bool isInactive ();
  bool isUpperActive ();
  bool isFired ();
  void search (Atom *);
  void reduce (bool strongly);
  void unreduce ();
  void setup ();
  void init (Init *);
  void print ();
  void print_internal ();

  Atom **head;       // The heads of the rule
  Atom **hend;       // Sentinel of the head array
  Atom **nbody;      // The negative literals in the body (allocated
                     // in head)
  Atom **pbody;      // The positive literals in the body (allocated
		     // in head)
  Atom **nend;       // Sentinel of the nbody array
  Atom **pend;       // Sentinel of the pbody array
  Atom **end;        // The end of the body array
  long lit;          // The number of negative literals in body
                     // that are not in B- plus the number of positive
                     // literals in body that are not in B+
  long upper;        // The number of positive literals in body that
		     // are in the upper closure
  long inactive;     // The number of positive literals in B- plus
		     // the number of negative literals in B+
};

class WeightRule : public HeadRule
{
public:
  WeightRule ();
  virtual ~WeightRule ();

  void inactivate (const Follows *);
  void fireInit ();
  bool fired ();
  void mightFireInit (const Follows *);
  void unInit ();
  void mightFire (const Follows *);
  void backChainTrue ();
  void backChainFalse ();
  void backtrackFromActive (const Follows *);
  void backtrackFromInactive (const Follows *);
  void propagateFalse (const Follows *);
  void propagateTrue (const Follows *);
  void backtrackUpper (const Follows *);
  void backtrack (Atom **);

  bool isInactive ();
  bool isUpperActive ();
  bool isFired ();
  void in_loop (Follows *);
  void not_in_loop (Follows *);
  void search (Atom *);
  void reduce (bool strongly);
  void unreduce ();
  void initWeight (Weight);
  void setup ();
  void init (Init *);
  void swap (long, long);
  void heap (long, long);
  bool smaller (long, long);
  void sort ();
  void swapping (Follows *, Follows *);
  Atom **chainTrue (Atom **);
  Atom **chainFalse (Atom **);
  void backChainTrueShadow ();
  void backChainFalseShadow ();
  void print ();
  void print_internal ();

  Atom **bend;       // Sentinel of the body array
  Atom **body;       // The literals in the body
  Atom **end;        // The end of the array
  Auxiliary *aux;    // Holds among other things the local weights of
                     // the atoms
  Weight maxweight;
  Weight minweight;
  Weight upper_min;   // Used in upper closure
  Weight lower_min;
  Weight atleast;
  Atom **max;       // Position of the literal of largest absolute
  Atom **min;       // value of weight not in B, used in backchain
  Atom **max_shadow;
  Atom **min_shadow;
  Follows **reverse;
};

// This is really a metarule. It won't interact with the truth-values
// of the atoms.
class OptimizeRule : public Rule
{
public:
  OptimizeRule ();
  virtual ~OptimizeRule ();

  void setOptimum ();

  void inactivate (const Follows *);
  void fireInit ();
  void mightFireInit (const Follows *);
  void unInit ();
  void mightFire (const Follows *);
  void backChainTrue ();
  void backChainFalse ();
  void backtrackFromActive (const Follows *);
  void backtrackFromInactive (const Follows *);
  void propagateFalse (const Follows *);
  void propagateTrue (const Follows *);
  void backtrackUpper (const Follows *);

  bool isInactive ();
  bool isUpperActive ();
  bool isFired ();
  void search (Atom *);
  void reduce (bool strongly);
  void unreduce ();
  void setup ();
  void init (Init *);
  void print ();
  void print_internal ();

  Atom **nbody;      // The negative literals in the body
  Atom **pbody;      // The positive literals in the body (allocated
		     // in nbody)
  Atom **nend;       // Sentinel of the nbody array
  Atom **pend;       // Sentinel of the pbody array
  Atom **end;        // The end of the body array
  Auxiliary *aux;    // Holds the local weights of atoms in nbody
  Weight minweight;
  Weight minoptimum;
  Weight heuristic_best;
  OptimizeRule *next;
};

#endif
