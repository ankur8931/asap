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
#include <iomanip>
#include <stdlib.h>
#if defined _BSD_SOURCE || defined _SVID_SOURCE
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include "stable.h"

using namespace std;

Stable::Stable ()
  : api (&smodels.program),
    reader (&smodels.program, &api)
{
  wellfounded = false;
  lookahead = true;
  backjumping = false;
  restart = false;
  simple = false;
  seed = 1;
}

Stable::~Stable ()
{
}

int
Stable::read (istream &f)
{
  timer.start ();
  int r = reader.read (f);
  // Set up atoms
  api.done ();
  if (r == 0)
    {
      smodels.init ();
      smodels.max_models = reader.models;
    }
  return r;
}

void
Stable::print_internal ()
{
  char false_program[] = "1 1 0 0\n0\n0\nB+\n0\nB-\n1\n0\n1\n";
  if (wellfounded || !lookahead)
    {
      smodels.setup ();
      if (!smodels.conflict ())
	smodels.program.print_internal (smodels.max_models);
      else
	cout << false_program;
    }
  else
    {
      smodels.setup_with_lookahead ();
      if (!smodels.conflict ())
	{
	  smodels.improve ();
	  smodels.program.print_internal (smodels.max_models);
	}
      else
	cout << false_program;
    }
}

void
Stable::print_time ()
{
#if defined _BSD_SOURCE || defined _SVID_SOURCE
  struct rusage rusage;
  getrusage (RUSAGE_SELF, &rusage);
  cout << "Duration: " << rusage.ru_utime.tv_sec << '.'
       << setw(3) << setfill('0') << rusage.ru_utime.tv_usec/1000
       << endl;
#else
  cout << "Duration: " << timer.print () << endl;
#endif
}

void
Stable::calculate ()
{
  int r = 0;
  if (wellfounded)
    r = smodels.wellfounded ();
  else
    {
      if (seed != 1)
	{
	  srand (1);   // Reset
	  srand (seed);
	  smodels.shuffle ();
	}
      if (restart)
	r = smodels.smodels_restart (lookahead, simple);
      else
	r = smodels.smodels (lookahead, backjumping, simple);
    }
  if (r)
    cout << "True" << endl;
  else
    cout << "False" << endl;
  timer.stop ();
  print_time ();
  cout << "Number of choice points: " <<
    smodels.number_of_choice_points << endl;
  cout << "Number of wrong choices: " <<
    smodels.number_of_wrong_choices << endl;
  cout << "Number of atoms: " << smodels.program.number_of_atoms <<
    endl;
  cout << "Number of rules: " << smodels.program.number_of_rules <<
    endl;
  if (lookahead)
    {
      cout << "Number of picked atoms: " <<
	smodels.number_of_picked_atoms << endl;
      cout << "Number of forced atoms: " <<
	smodels.number_of_forced_atoms << endl;
      cout << "Number of truth assignments: " <<
	smodels.number_of_assignments << endl;
      cout << "Size of searchspace (removed): "
	   << smodels.size_of_searchspace << " ("
	   << smodels.number_of_denants << ')' << endl;
    }
  if (restart)
    cout << "Number of restarts: " <<
	smodels.number_of_restarts << endl;
  if (backjumping)
    cout << "Number of backjumps: " << smodels.number_of_backjumps <<
      endl;
}
