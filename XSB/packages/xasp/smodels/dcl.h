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
#ifndef DCL_H
#define DCL_H

class Atom;
class Program;
class Smodels;

class Dcl
{
public:
  Dcl (Smodels *s);
  ~Dcl ();
  void init ();
  void setup ();
  void revert ();
  void dcl ();
  bool propagateFalse ();
  int path (Atom *a, Atom *b);
  void reduce (bool strongly);
  void unreduce ();
  void improve ();
  void denant ();
  void undenant ();
  void print ();

protected:
  Smodels * const smodels;
  Program * const program;

  Atom **tmpfalse;
  long tmpfalsetop;
};

#endif
