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
#include <stdio.h>
#include <limits.h>
#include "timer.h"

Timer::Timer ()
{
  sec = 0;
  msec = 0;
}

void
Timer::start ()
{
  timer = clock ();
}

void
Timer::stop ()
{
  clock_t ticks = clock () - timer;
  long s = ticks / CLOCKS_PER_SEC;
  sec += s;
  msec += (ticks - s*CLOCKS_PER_SEC)*1000/CLOCKS_PER_SEC;
  if (msec >= 1000)
    {
      msec -= 1000;
      sec++;
    }
}

void
Timer::reset ()
{
  sec = 0;
  msec = 0;
}

char *
Timer::print ()
{
  static char s[20];
  sprintf (s, "%ld.%0ld", sec, msec);
  return s;
}
