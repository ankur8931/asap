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
#include <float.h>
#include <limits.h>
#include <string.h>
#include "atomrule.h"
#include "program.h"
#include "api.h"
#include "read.h"

Read::Read (Program *p, Api *a)
  : program (p), api (a)
{
  atoms = 0;
  size = 0;
}

Read::~Read ()
{
  delete[] atoms;
}

void
Read::grow ()
{
  long sz = size*2;
  if (sz == 0)
    sz = 256;
  Atom **array = new Atom *[sz];
  long i;
  for (i = 0; i < size; i++)
    array[i] = atoms[i];
  size = sz;
  for (; i < size; i++)
    array[i] = 0;
  delete[] atoms;
  atoms = array;
}

Atom *
Read::getAtom (long n)
{
  while (n >= size)
    grow ();
  if (atoms[n] == 0)
    atoms[n] = api->new_atom ();
  return atoms[n];
}

int
Read::readBody (istream &f, long size, bool pos)
{
  long n;
  for (long i = 0; i < size; i++)
    {
      f >> n;
      if (!f.good () || n < 1)
	{
	  cerr << "atom out of bounds, line " << linenumber << endl;
	  return 1;
	}
      Atom *a = getAtom (n);
      api->add_body (a, pos);
    }
  return 0;
}

int
Read::addBasicRule (istream &f)
{
  long n;
  api->begin_rule (BASICRULE);
  // Rule head
  f >> n;
  if (!f.good () || n < 1)
    {
      cerr << "head atom out of bounds, line " << linenumber << endl;
      return 1;
    }
  Atom *a = getAtom (n);
  api->add_head (a);
  // Body
  f >> n;
  if (!f.good () || n < 0)
    {
      cerr << "total body size, line " << linenumber << endl;
      return 1;
    }
  long body = n;
  // Negative body
  f >> n;
  if (!f.good () || n < 0 || n > body)
    {
      cerr << "negative body size, line " << linenumber << endl;
      return 1;
    }
  if (readBody (f, n, false))
    return 1;
  // Positive body
  if (readBody (f, body-n, true))
    return 1;
  api->end_rule ();
  return 0;
}

int
Read::addConstraintRule (istream &f)
{
  long n;
  api->begin_rule (CONSTRAINTRULE);
  // Rule head
  f >> n;
  if (!f.good ())
    return 1;
  if (!f.good () || n < 1)
    {
      cerr << "head atom out of bounds, line " << linenumber << endl;
      return 1;
    }
  Atom *a = getAtom (n);
  api->add_head (a);
  // Body
  f >> n;
  if (!f.good () || n < 0)
    return 1;
  long body = n;
  // Negative body
  f >> n;
  if (!f.good () || n < 0 || n > body)
    return 1;
  long nbody = n;
  // Choose
  f >> n;
  if (!f.good () || n < 0 || n > body)
    return 1;
  api->set_atleast_body (n);
  if (readBody (f, nbody, false))
    return 1;
  // Positive body
  if (readBody (f, body-nbody, true))
    return 1;
  api->end_rule ();
  return 0;
}

int
Read::addGenerateRule (istream &f)
{
  long n;
  api->begin_rule (GENERATERULE);
  // Heads
  f >> n;
  if (!f.good () || n < 2)
    {
      cerr << "head size less than two, line " << linenumber << endl;
      return 1;
    }
  long heads = n;
  // choose
  f >> n;
  if (!f.good () || n <= 0 || n > heads-1)
    {
      cerr << "choose out of bounds, line " << linenumber << endl;
      return 1;
    }
  api->set_atleast_head (n);
  for (long i = 0; i < heads; i++)
    {
      f >> n;
      if (!f.good () || n < 1)
	{
	  cerr << "atom out of bounds, line " << linenumber << endl;
	  return 1;
	}
      Atom *a = getAtom (n);
      api->add_head (a);
    }
  // Body
  f >> n;
  if (!f.good () || n < 0)
    {
      cerr << "total body size, line " << linenumber << endl;
      return 1;
    }
  if (readBody (f, n, true))
    return 1;
  api->end_rule ();
  return 0;
}

int
Read::addChoiceRule (istream &f)
{
  long n;
  api->begin_rule (CHOICERULE);
  // Heads
  f >> n;
  if (!f.good () || n < 1)
    {
      cerr << "head size less than one, line " << linenumber << endl;
      return 1;
    }
  long heads = n;
  for (long i = 0; i < heads; i++)
    {
      f >> n;
      if (!f.good () || n < 1)
	{
	  cerr << "atom out of bounds, line " << linenumber << endl;
	  return 1;
	}
      Atom *a = getAtom (n);
      api->add_head (a);
    }
  // Body
  f >> n;
  if (!f.good () || n < 0)
    {
      cerr << "total body size, line " << linenumber << endl;
      return 1;
    }
  long body = n;
  // Negative body
  f >> n;
  if (!f.good () || n < 0 || n > body)
    {
      cerr << "negative body size, line " << linenumber << endl;
      return 1;
    }
  if (readBody (f, n, false))
    return 1;
  // Positive body
  if (readBody (f, body-n, true))
    return 1;
  api->end_rule ();
  return 0;
}

int
Read::addWeightRule (istream &f)
{
  long n;
  api->begin_rule (WEIGHTRULE);
  // Rule head
  f >> n;
  if (!f.good () || n < 1)
    {
      cerr << "head atom out of bounds, line " << linenumber << endl;
      return 1;
    }
  Atom *a = getAtom (n);
  api->add_head (a);
  Weight w;
# ifdef USEDOUBLE
  f.precision (DBL_DIG);
# endif
  // Atleast
  f >> w;
  if (!f.good ())
    return 1;
  api->set_atleast_weight (w);
  // Body
  f >> n;
  if (!f.good () || n < 0)
    return 1;
  long body = n;
  // Negative body
  f >> n;
  if (!f.good () || n < 0 || n > body)
    return 1;
  long nbody = n;
  if (readBody (f, nbody, false))
    return 1;
  // Positive body
  if (readBody (f, body-nbody, true))
    return 1;
  Weight sum = 0;
  for (long i = 0; i < body; i++)
    {
      f >> w;
      if (!f.good ())
	return 1;
      if (sum+w < sum)
	{
	  cerr << "sum of weights in weight rule too large,"
	       << " line " << linenumber << endl;
	  return 1;
	}
      sum += w;
      if (i < nbody)
	api->change_body (i, false, w);
      else
	api->change_body (i-nbody, true, w);
    }
  api->end_rule ();
  return 0;
}

int
Read::addOptimizeRule (istream &f)
{
  long n;
  api->begin_rule (OPTIMIZERULE);
  // Optimize
  f >> n;
  if (!f.good ())
    return 1;
  if (n)
    {
      cerr << "maximize statements are no longer accepted, line"
	   << linenumber << endl;
      return 1;
    }
  // Body
  f >> n;
  if (!f.good () || n < 0)
    return 1;
  long body = n;
  // Negative body
  f >> n;
  if (!f.good () || n < 0 || n > body)
    return 1;
  long nbody = n;
  if (readBody (f, nbody, false))
    return 1;
  // Positive body
  if (readBody (f, body-nbody, true))
    return 1;
  Weight w;
# ifdef USEDOUBLE
  f.precision (DBL_DIG);
# endif
  Weight sum = 0;
  for (long i = 0; i < body; i++)
    {
      f >> w;
      if (!f.good ())
	return 1;
      if (sum+w < sum)
	{
	  cerr << "sum of weights in optimize statement too large,"
	       << " line " << linenumber << endl;
	  return 1;
	}
      sum += w;
      if (i < nbody)
	api->change_body (i, false, w);
      else
	api->change_body (i-nbody, true, w);
    }
  api->end_rule ();
  return 0;
}

int
Read::read (istream &f)
{
  // Read rules.
  int type;
  bool stop = false;
  for (linenumber = 1; !stop; linenumber++)
    {
      // Rule Type
      f >> type;
      switch (type)
	{
	case ENDRULE:
	  stop = true;
	  break;
	case BASICRULE:
	  if (addBasicRule (f))
	    return 1;
	  break;
	case CONSTRAINTRULE:
	  if (addConstraintRule (f))
	    return 1;
	  break;
	case CHOICERULE:
	  if (addChoiceRule (f))
	    return 1;
	  break;
	case GENERATERULE:
	  if (addGenerateRule (f))
	    return 1;
	  break;
	case WEIGHTRULE:
	  if (addWeightRule (f))
	    return 1;
	  break;
	case OPTIMIZERULE:
	  if (addOptimizeRule (f))
	    return 1;
	  break;
	default:
	  return 1;
	}
    }
  // Read atom names.
  const int len = 1024;
  char s[len];
  long i;
  f.getline (s, len);  // Get newline
  if (!f.good ())
    {
      cerr << "expected atom names to begin on new line, line "
	<< linenumber << endl;
      return 1;
    }
  f >> i;
  f.getline (s, len);
  linenumber++;
  while (i)
    {
      if (!f.good ())
	{
	  cerr << "atom name too long or end of file, line "
	       << linenumber << endl;
	  return 1;
	}
      Atom *a = getAtom (i);
      if (*s)
	api->set_name (a, s+1);
      else
	api->set_name (a, 0);
      f >> i;
      f.getline (s, len);
      linenumber++;
    }
  // Read compute-statement
  f.getline (s, len);
  if (!f.good () || strcmp (s, "B+"))
    {
      cerr << "B+ expected, line " << linenumber << endl;
      return 1;
    }
  linenumber++;
  f >> i;
  linenumber++;
  while (i)
    {
      if (!f.good () || i < 1)
	{
	  cerr << "B+ atom out of bounds, line " << linenumber << endl;
	  return 1;
	}
      Atom *a = getAtom (i);
      api->set_compute (a, true);
      f >> i;
      linenumber++;
    }
  f.getline (s, len);  // Get newline.
  f.getline (s, len);
  if (!f.good () || strcmp (s, "B-"))
    {
      cerr << "B- expected, line " << linenumber << endl;
      return 1;
    }
  linenumber++;
  f >> i;
  linenumber++;
  while (i)
    {
      if (!f.good () || i < 1)
	{
	  cerr << "B- atom out of bounds, line " << linenumber << endl;
	  return 1;
	}
      Atom *a = getAtom (i);
      api->set_compute (a, false);
      f >> i;
      linenumber++;
    }
  f >> models;  // zero means all
  if (f.fail ())
    {
      cerr << "number of models expected, line " << linenumber << endl;
      return 1;
    }
  return 0;
}
