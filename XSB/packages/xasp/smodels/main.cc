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
#include <stdlib.h>
#include <string.h>
#include "stable.h"

using namespace std;

void usage ()
{
  cerr << "Usage: smodels [number] [-w] [-restart] [-simple] [-seed number]"
       <<  "              [-nolookahead] [-internal] [-backjump]" << endl;
  cerr << "       The number determines the number of stable models computed."
       << endl
       << "       A zero indicates all." << endl
       << "       -w displays the well-founded model." << endl
       << "       -restart does restarts during the search." << endl
       << "       -simple uses a simple heuristic." << endl
       << "       -seed mixes the program." << endl
       << "       -nolookahead do not use lookahead." << endl
       << "       -internal prints a reduced program in internal form." << endl
       << "       -backjump does not always backtrack chronologically."
       << endl;
}

int main (int argc, char *argv[])
{
  long models = 0;
  int sm = 0;
  char *endptr;
  bool error = false;
  Stable stable;
  bool print_internal = false;
  for (int c = 1; c < argc && !error; c++)
    {
      if (argv[c][0] == '-')
	{
	  if (strcmp (&argv[c][1], "w") == 0)
	    stable.wellfounded = true;
	  else if (strcmp (&argv[c][1], "restart") == 0)
	    stable.restart = true;
	  else if (strcmp (&argv[c][1], "nolookahead") == 0)
	    stable.lookahead = false;
	  else if (strcmp (&argv[c][1], "simple") == 0)
	    stable.simple = true;
	  else if (strcmp (&argv[c][1], "backjump") == 0)
	    stable.backjumping = true;
	  else if (strcmp (&argv[c][1], "internal") == 0)
	    print_internal = true;
	  else if (strcmp (&argv[c][1], "seed") == 0)
	    {
	      c++;
	      if (c < argc)
		stable.seed = strtol (argv[c], &endptr, 0);
	      else
		error = true;
	      if (*endptr != '\0')
		error = true;
	    }
	  else
	    error = true;
	}
      else
	{
	  models = strtol (argv[c], &endptr, 0);
	  if (models < 0 || *endptr != '\0')
	    error = true;
	  sm = 1;
	}
    }
  if (error)
    {
      usage ();
      return 1;
    }
  if (!print_internal)
    cout << "smodels version 2.33. Reading...";
  int bad = stable.read(cin);
  if (!print_internal)
    cout << "done" << endl;
  if (bad)
    {
      cerr << "Error in input" << endl;
      return 1;
    }
  if (sm)
    stable.smodels.max_models = models;
  if (print_internal)
    stable.print_internal ();
  else
    stable.calculate ();
  return 0;
}
