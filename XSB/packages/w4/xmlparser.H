%%% XML Parser ( Version 1.0/October 2002)
%%%
%%% (c) 2002 Carlos Viegas Damásio (cd@di.fct.unl.pt)
%%%
%%% Centro de Inteligência Artificial da Universidade Nova de Lisboa
%%% Quinta da Torre
%%% 2829-516 Caparica
%%% Portugal
%%%
%%% This module provides several interfaces for constructing a Prolog representation of
%%% XML documents. It follows almost completely the recommendation XML Information Sets 
%%% (http://www.w3.org/TR/xml-infoset). 
%%% 
%%% XML Namespaces and XML Base are fully supported.
%%%
%%%


:- import createDocument/1 from xmldom.

:- import setDocumentPhysicalStructures/3 from xmldom.
:- import setDocumentDTD/3 from xmldom.
:- import setDocumentStandalone/3 from xmldom.
:- import setDocumentEncoding/3 from xmldom.
:- import setDocumentVersion/3 from xmldom.
:- import setDocumentElement/3 from xmldom.
:- import setDocumentAllProcessed/3 from xmldom.
:- import setDocumentBaseURI/3 from xmldom.

:- import getDocumentBaseURI/2 from xmldom.
:- import getDocumentDefaultAttributes/4 from xmldom.

:- import appendDocumentChildren/3 from xmldom.


:- import createDocumentType/6 from xmldom.
:- import splitAttrib/6 from xmldom.

:- import createPhysicalStructures/1 from xmldom.

:- import getPhysicalStructuresParameterEntities/2 from xmldom.
:- import getPhysicalStructuresGeneralEntities/2 from xmldom.

:- import addPhysicalStructuresParameterEntity/4 from xmldom.
:- import addPhysicalStructuresNotation/4 from xmldom.
:- import addPhysicalStructuresGeneralEntity/6 from xmldom.


:- import isExternalEntity/1 from xmldom.
:- import isInternalEntity/1 from xmldom.
:- import getEntityReplacementText/2 from xmldom.


:- import createLogicalStructures/1 from xmldom.
:- import createLogicalStructuresAttributeList/3 from xmldom.
:- import addLogicalStructuresElementDeclaration/4 from xmldom.
:- import addLogicalStructuresAttributeDeclaration/4 from xmldom.


:- import createElementSpecification/3 from xmldom.
:- import createAttributeDeclaration/4 from xmldom.
:- import createNotation/5 from xmldom.

:- import createDocumentFragment/1 from xmldom.
:- import appendDocumentFragmentChild/3 from xmldom.

:- import createPI/4 from xmldom.

:- import createElementNS/6 from xmldom.
:- import setElementContent/3 from xmldom.
:- import setElementAttributes/4 from xmldom.

:- import createAttributeNS/6 from xmldom.

:- import createComment/2 from xmldom.

:- import createNamedMap/1 from xmldom.
:- import getFirstNamedMap/3 from xmldom.
:- import getRestNamedMap/2 from xmldom.


:- import setNamedItem/4 from xmldom.
:- import setNewNamedItem/4 from xmldom.
:- import setNamedItemIfNew/4 from xmldom.

:- import getNamedItem/3 from xmldom.

:- import removeNamedItem/4 from xmldom.


:- import parseIRIref/3 from iri.
:- import resolveIRIref/3 from iri.
:- import getIRIrefFragment/2 from iri.

:- import ucs2utf8/2 from utilities.


:- export xml_document/7.
:- export xml_declaration0/7.
:- export ncname/5.

