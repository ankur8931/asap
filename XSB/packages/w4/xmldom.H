%%% Document Object Model ( Version 1.0/October 2002)
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
%%% (http://www.w3.org/TR/xml-infoset). For the time being it lacks full XML Base support
%%% (it does not resolve relative references) even though the properties in the information items and
%%% corresponding methods are already supported. 
%%% 
%%% XML Namespaces are fully supported.
%%%
%%% The properties which involve referencing other information items 
%%% are not implemented. This is motivated by the fact that some Prolog system do not 
%%% give support for cyclic terms. However, we are planning to provide an extension to this
%%% module for the full recommendation.
%%% 
%%% In most situations unknown or no value properties are represented by empty lists.
%%% In order to maintain compatibility for subsequent versions of our parser, all applications
%%% should use the current predicates to create/extract properties from the information items 
%%%
%%% Some of the specification and predicate names were inspired by XML DOM Level 3.
%%%

% Document
:- export isDocument/1.
:- export createDocument/1.
:- export setDocumentChildren/3.
:- export setDocumentElement/3.
:- export setDocumentPhysicalStructures/3.
:- export setDocumentBaseURI/3.
:- export setDocumentVersion/3.
:- export setDocumentEncoding/3.
:- export setDocumentStandalone/3.
:- export setDocumentAllProcessed/3.
:- export setDocumentDTD/3.

:- export getDocumentChildren/2.
:- export getDocumentElement/2.
:- export getDocumentNotations/2.
:- export getDocumentUnparsedEntities/2.
:- export getDocumentBaseURI/2.
:- export getDocumentVersion/2.
:- export getDocumentEncoding/2.
:- export getDocumentStandalone/2.
:- export getDocumentAllProcessed/2.
:- export getDocumentDTD/2.
:- export getDocumentDefaultAttributes/4.

:- export appendDocumentChildren/3.

% Document Type

:- export isDocumentType/1.
:- export createDocumentType/6.
:- export getDocumentTypeQualifiedName/2.
:- export getDocumentTypePublicId/2.
:- export getDocumentTypeSystemId/2.
:- export getDocumentTypeElementDeclarations/2.
:- export getDocumentTypeAttributeDeclarations/2.
:- export getDocumentTypeChildren/2.
:- export getDocumentTypeLogicalStructs/2.

:- export getElementSpecificationFromDTD/3.
:- export getDefaultAttributesFromDTD/4.
:- export getAttributeDeclarationFromDTD/5.


:- export splitAttrib/6.


% Physical Structures

:- export isPhysicalStructures/1.
:- export createPhysicalStructures/1.
:- export setPhysicalStructuresParameterEntities/3.
:- export setPhysicalStructuresGeneralEntities/3.
:- export setPhysicalStructuresNotations/3.
:- export setPhysicalStructuresUnparsedEntities/3.
:- export getPhysicalStructuresParameterEntities/2.
:- export getPhysicalStructuresGeneralEntities/2.
:- export getPhysicalStructuresNotations/2.
:- export getPhysicalStructuresUnparsedEntities/2.

:- export addPhysicalStructuresParameterEntity/4.
:- export addPhysicalStructuresParsedEntity/4.
:- export addPhysicalStructuresUnparsedEntity/4.
:- export addPhysicalStructuresNotation/4.
:- export addPhysicalStructuresGeneralEntity/6.


% Entities

:- export isInternalEntity/1.
:- export isExternalEntity/1.
:- export isUnparsedEntity/1.

:- export createEntity/3.
:- export createExternalEntity/5.
:- export createUnparsedEntity/6.
:- export getEntityName/2.
:- export getEntityPublicId/2.
:- export getEntitySystemId/2.
:- export getEntityBaseURI/2.
:- export getEntityNotationName/2.
:- export getEntityReplacementText/2.

% Logical Structures

:- export isLogicalStructures/1.
:- export createLogicalStructures/1.
:- export createLogicalStructuresAttributeList/3.
:- export addLogicalStructuresElementDeclaration/4.
:- export addLogicalStructuresAttributeDeclaration/4.
:- export getLogicalStructuresElementDeclaration/3.
:- export getLogicalStructuresAttributeDeclarations/3.

% Element Specifications

:- export isElementSpecification/1.
:- export createElementSpecification/3.
:- export getElementSpecification/2.

% Attribute List Declarations

:- export isAttributeListDeclaration/1.
:- export getAttributeListDeclaration/2.
:- export createAttributeDeclaration/4.
:- export getAttributeDeclaration/3.
:- export getAttributeDeclaration/4.

% Notations

:- export isNotation/1.
:- export createNotation/5.
:- export getNotationName/2.
:- export getNotationPublicId/2.
:- export getNotationSystemId/2.
:- export getNotationBaseURI/2.

% Document Fragments

:- export isDocumentFragment/1.
:- export createDocumentFragment/1.
:- export appendDocumentFragmentChild/3.

% Elements

:- export isElement/1.
:- export createElementNS/6.
:- export setElementAttributes/4.
:- export setElementContent/3.

:- export getElementName/4.
:- export getElementChildren/2.
:- export getElementAttributes/3.
:- export getElementInScopeNamespaces/2.
:- export getElementBaseURI/2.
:- export getElementLang/2.

% Attributes

:- export isAttribute/1.
:- export createAttributeNS/6.
:- export getAttributeName/4.
:- export getAttributeValue/2.
:- export getAttributeSpecified/2.
:- export getAttributeType/2.

% Processing Instructions

:- export isPI/1.
:- export createPI/4.
:- export setPIBaseURI/3.
:- export getPITarget/2.
:- export getPIContent/2.
:- export getPIBaseURI/2.


% Comments

:- export isComment/1.
:- export createComment/2.
:- export getCommentContent/2.

% Character Data

:- export isCharData/1.
:- export isWhiteSpace/1.

:- export createCharData/2.
:- export getCharData/2.


% Auxiliary predicates

:- export isNodeList/1.
:- export createNodeList/1.
:- export isEmptyNodeList/1.
:- export getHeadNodeList/2.
:- export getTailNodeList/2.

:- export isNamedMap/1.
:- export createNamedMap/1.
:- export isEmptyNamedMap/1.
:- export getFirstNamedMap/2.
:- export getFirstNamedMap/3.
:- export getRestNamedMap/2.
:- export getNamedItem/3.

:- export setNamedItem/4.
:- export setNewNamedItem/4.
:- export setNamedItemIfNew/4.
:- export removeNamedItem/4.


:- export isQualifiedName/1.
:- export createQualifiedName/3.
:- export getPrefix/2.
:- export getLocalName/2.


