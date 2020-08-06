/* File:      xasppkg.c
** Author(s): Luis Castro
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
** Copyright (C) ECRC, Germany, 1990
** 
** XSB is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XSB is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: xasppkg.c,v 1.11 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

#include <stdlib.h>
#include <stdio.h>
#include "smodels/smodels.h"
#ifndef SMODELS_H
#error "You need the .h and .o files from SModels in your directory"
#endif
#include "smodels/api.h"
#include "smodels/atomrule.h"
#include "xsb_config.h"

Atom **globatoms;

// These variables should not be global in the MT-engine.
// They will soon be moved to the th_context structure so that
// each thread can have its own Smodels instance.

#ifdef CYGWIN
#include "../../emu/context.h"
#else
#include "context.h"
#endif

//#define MULTI_THREAD 1

#ifdef MULTI_THREAD
#include "xasp.h"
#else
Smodels *smodels;
Api *api;
Atom **atoms;
int curatom, totatoms;
#endif

extern "C" void init(CTXTdecl)
{

  smodels = (new Smodels);
  
  api = (new Api(&(smodels)->program));
}

extern "C" void numberAtoms(CTXTdeclc int nAtoms)
{
  int i;

  atoms = (Atom **) malloc(sizeof(Atom*)*nAtoms);

  for (i=0; i<nAtoms; i++) 
    atoms[i] = api->new_atom();

  curatom = 0;
  totatoms=nAtoms;
}


extern "C" void atomName(CTXTdeclc char *name)
{ 
  api->set_name(atoms[curatom],name);
  curatom++;
//  Atom ** loc_atoms = atoms;
//  int loc_curatom = curatom;
//  api->set_name(globatoms[loc_curatom],name);
//  curatom++;
}

#ifdef CYGWIN
extern "C" void __assert(const char *a, int b, const char *c) { return; }
#endif

extern "C" void beginBasicRule(CTXTdecl)
{
  api->begin_rule(BASICRULE);
}

extern "C" void beginChoiceRule(CTXTdecl)
{
  api->begin_rule(CHOICERULE);
}

extern "C" void beginConstraintRule(CTXTdecl)
{
  api->begin_rule(CONSTRAINTRULE);
}

extern "C" void beginWeightRule(CTXTdecl)
{
  api->begin_rule(WEIGHTRULE);
}

extern "C" void addHead(CTXTdeclc int atomNum)
{
  api->add_head(atoms[atomNum-1]);
}

extern "C" void addWPosBody(CTXTdeclc int atomNum,Weight weight)
{
  api->add_body(atoms[atomNum-1],true,weight);
}

extern "C" void addPosBody(CTXTdeclc int atomNum)
{
  api->add_body(atoms[atomNum-1],true);
}

extern "C" void addWNegBody(CTXTdeclc int atomNum,Weight weight)
{
  api->add_body(atoms[atomNum-1],false,weight);
}

extern "C" void addNegBody(CTXTdeclc int atomNum)
{
  api->add_body(atoms[atomNum-1],false);
}

extern "C" void endRule(CTXTdecl)
{
  api->end_rule();
}

extern "C" void commitRules(CTXTdecl)
{
  api->done();
  smodels->init();
}

extern "C" void printProgram(CTXTdecl)
{
  smodels->program.print();
}

extern "C" int existsModel(CTXTdecl)
{
  //  printf("exists Model called\n");
  return smodels->model();
}

extern "C" void printAnswer(CTXTdecl)
{
  smodels->printAnswer();
}

extern "C" void close(CTXTdecl)
{
  delete(api);
  delete(smodels);
  free(atoms);
}

extern "C" int checkAtom(CTXTdeclc int atom)
{
  return atoms[atom-1]->Bpos;
}

extern "C" void setPosCompute(CTXTdeclc int atom)
{
  api->set_compute(atoms[atom-1],true);
}

extern "C" void setNegCompute(CTXTdeclc int atom)
{
  api->set_compute(atoms[atom-1],false);
}

extern "C" void resetPosCompute(CTXTdeclc int atom)
{
  api->reset_compute(atoms[atom-1],true);
}

extern "C" void resetNegCompute(CTXTdeclc int atom)
{
  api->reset_compute(atoms[atom-1],false);
}

extern "C" void remember(CTXTdecl )
{
  api->remember();
}

extern "C" void forget(CTXTdecl )
{
  api->forget();
}

extern "C" void setBody(CTXTdeclc long val)
{
  api->set_atleast_body(val);
}

extern "C" void setWeight(CTXTdeclc long val)
{
  api->set_atleast_weight(val);
}

extern "C" void setHead(CTXTdeclc long val)
{
  api->set_atleast_head(val);
}

extern "C" void wellfounded(CTXTdecl)
{
  smodels->wellfounded();
}

extern "C" int testPos(CTXTdeclc int atom)
{
  return smodels->testPos(atoms[atom-1]);
}

extern "C" int testNeg(CTXTdeclc int atom)
{
  return smodels->testNeg(atoms[atom-1]);
}

extern "C" void dummy1(int atom)
{
  printf("%s\n",atom);
}

extern "C" void dummy2(void)
{
  printf("%s\n","foo");
}

extern "C" int dummy3(int atom)
{
  printf("%s\n",atom);
  return 1;
}

extern "C" int dummy4(void)
{
  printf("%s\n","foo");
  return 1;
}
