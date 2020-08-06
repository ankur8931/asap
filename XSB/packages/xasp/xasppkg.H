/* File:      xasppkg.H
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
** $Id: xasppkg.H,v 1.7 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

%:- compile_command 'make module'.
	
:- private_foreign_pred smodelsInit from init(void):void.
:- private_foreign_pred smodelsNumAtoms(+a) from numberAtoms(a:int):void.
:- private_foreign_pred smodelsAtomName(+a) from atomName(a:string):void.

:- private_foreign_pred smodelsBeginBasicRule from beginBasicRule(void):void.
:- private_foreign_pred smodelsBeginChoiceRule from beginChoiceRule(void):void.
:- private_foreign_pred smodelsBeginConstraintRule from beginConstraintRule(void):void.
:- private_foreign_pred smodelsBeginWeightRule from beginWeightRule(void):void.

:- private_foreign_pred smodelsAddHead(+a) from addHead(a:int):void.
:- private_foreign_pred smodelsAddPosBody(+a) from addPosBody(a:int):void.
:- private_foreign_pred smodelsAddNegBody(+a) from addNegBody(a:int):void.
:- private_foreign_pred smodelsAddWPosBody(+a,+w) from addWPosBody(a:int,w:int):void.
:- private_foreign_pred smodelsAddWNegBody(+a,+w) from addWNegBody(a:int,w:int):void.
:- private_foreign_pred smodelsEndRule from endRule(void):void.
:- private_foreign_pred smodelsCommitRules from commitRules(void):void.
:- private_foreign_pred smodelsPrintProgram from printProgram(void):void.
:- private_foreign_pred smodelsExistsModel(-retval) from existsModel(void):int.
:- private_foreign_pred smodelsPrintAnswer from printAnswer(void):void.
:- private_foreign_pred smodelsClose from close(void):void.
:- private_foreign_pred smodelsCheckAtom(+a,-retval) from checkAtom(a:int):int.
:- private_foreign_pred smodelsSetPosCompute(+a) from setPosCompute(a:int):void.
:- private_foreign_pred smodelsSetNegCompute(+a) from setNegCompute(a:int):void.
:- private_foreign_pred smodelsResetPosCompute(+a) from resetPosCompute(a:int):void.
:- private_foreign_pred smodelsResetNegCompute(+a) from resetNegCompute(a:int):void.
:- private_foreign_pred smodelsRemember from remember(void):void.
:- private_foreign_pred smodelsForget from forget(void):void.
:- private_foreign_pred smodelsSetBody(+a) from setBody(a:int):void.
:- private_foreign_pred smodelsSetWeight(+a) from setWeight(a:int):void.
:- private_foreign_pred smodelsSetHead(+a) from setHead(a:int):void.
:- private_foreign_pred smodelsWellFounded from wellfounded(void):void.
:- private_foreign_pred smodelsTestPos(+a,-retval) from testPos(a:int):int.
:- private_foreign_pred smodelsTestNeg(+a,-retval) from testNeg(a:int):int.
