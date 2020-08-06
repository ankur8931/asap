%%% Parsing of internal DTDs ( Version 1.0/October 2002)
%%%
%%% (c) 2002 Carlos Viegas Damásio (cd@di.fct.unl.pt)
%%%
%%% Centro de Inteligência Artificial da Universade Nova de Lisboa
%%% Quinta da Torre
%%% 2829-516 Caparica
%%% Portugal
%%%
%%% This module constructs a representation of XML internal DTDs
%%% There are no recommendations defining the information items for DTDs.
%%%
%%% Apparently, the current guidelines suggest the separation of logical structures
%%% from physical ones, which we follow in this implementation. Namely that the
%%% unparsed entitites and notation declararions should be members of the 
%%% document information item. Regarding internal entities, the XML INFO SET
%%% recommendation does not provide any clue where to store them. 
%%%
%%% Since this module implements only internal DTD it does not fully implement 
%%% expansion of parameter entities in the DTD, respecting the XML specification. 
%%% Thus, conditional sections were not implemented. 
%%% This is expected to be tackled in the near future.
%%%


% Main predicate for parsing internal DTDs

internal0( BaseURI, Standalone, ProcessedDTD, Logical, Physical, PIs, All ) -->
	{ createDTDContext( BaseURI, Standalone, ProcessedDTD, Cx ) },
	internal0( Cx, NewCx ),
	{
	  getLogicalStructsInDTDContext( NewCx, Logical ),
	  getPhysicalStructsInDTDContext( NewCx, Physical ),
	  getPIsInDTDContext( NewCx, PIs/[] ),
	  getAllProcessedInDTDContext( NewCx, All )
	}.

internal0( Cx, NewCx ) -->
	"[", !, internal_dtd( Cx, NewCx ).
internal0( Cx, Cx ) --> [].


% [28a]      DeclSep    ::=    PEReference | S [WFC: PE Between Declarations] 
%     
% [29]    markupdecl    ::=    elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment  [VC: Proper Declaration/PE Nesting] 
%    																	[WFC: PEs in Internal Subset] 


internal_dtd( Cx, Cx ) --> "]", !.

internal_dtd( Cx, NewCx ) -->  
	"%", !, 
	pereference_in_dtd( Cx, AuxCx ),
	internal_dtd( AuxCx, NewCx ).
	
internal_dtd( Cx, NewCx ) --> "<!", !,
	markupdecl( Cx, AuxCx  ),
	internal_dtd( AuxCx, NewCx  ).

internal_dtd( Cx, NewCx ) --> "<?", !,
	{ getDTDContextBaseURI( Cx, BaseURI ) },
	pi_rest( BaseURI, PI ), 
	{ appendPItoDTDContext( Cx, PI, AuxCx ) },
	internal_dtd( AuxCx, NewCx ).

internal_dtd( Cx, NewCx ) --> whitespace, !,
	internal_dtd( Cx, NewCx  ).


markupdecl( Cx, Cx ) --> "--", !,
	comment_rest( _ ).

% Declaration of Logical structures
markupdecl( Cx, NewCx ) --> 
	+ element_decl( Cx, NewCx ).

markupdecl( Cx, NewCx ) --> 
	+ attlist_decl( Cx, NewCx ).			% The current entities declared may be necessary for attribute default declarations!

% Declaration of Physical structures
markupdecl( Cx, NewCx ) --> 
	+ entity_decl( Cx, NewCx ).

markupdecl( Cx, NewCx ) --> 
	+ notation_decl( Cx, NewCx ).


% [45]    elementdecl    ::=    '<!ELEMENT' S QName S contentspec S? '>' [VC: Unique Element Type Declaration] 
% [46]    contentspec    ::=    'EMPTY' | 'ANY' | Mixed | children  

element_decl( Cx, NewCx ) -->
	"ELEMENT", !, whitespace, qname(Name), whitespace, contentspec( CSpec ), whitespace0, ">",
	{ createElementSpecificationInDTDContext( Cx, Name, CSpec, NewCx) }.

contentspec( empty ) --> 
	"EMPTY", !.
contentspec( any ) --> 
	"ANY", !.
contentspec( EC ) -->
	+ element_content_spec( EC ).


element_content_spec( EC ) -->
	"(", whitespace0, mixed_or_children( EC ).

mixed_or_children( MC )  ::=
	+ mixed( MC ).
mixed_or_children( C )-->
	+ children( C ).

	
% [47]    children    ::=    (choice | seq) ('?' | '*' | '+')? 
% [48]    cp    ::=    (QName | choice | seq) ('?' | '*' | '+')? 
% [49]    choice    ::=    '(' S? cp ( S? '|' S? cp )+ S? ')' /* */ 
%    										  /* */ 
%    										  [VC: Proper Group/PE Nesting] 
% [50]    seq    ::=    '(' S? cp ( S? ',' S? cp )* S? ')' 	  /* */ 
%                                                              [VC: Proper Group/PE Nesting] 


children( C ) --> choice_or_seq( CS ), opt_operator( CS, C ).

choice_or_seq( CS ) -->
	cp( CP ), whitespace0, rest_choice_or_seq( CP, CS ).

rest_choice_or_seq( CP, seq( [CP] ) ) --> ")", !.
rest_choice_or_seq( CP, choice( [CP,CP1|RestC] ) ) --> "|", whitespace0, cp( CP1 ), whitespace0, rest_choice( RestC ).
rest_choice_or_seq( CP,    seq( [CP,CP1|RestS] ) ) --> ",", whitespace0, cp( CP1 ), whitespace0, rest_seq( RestS ).

cp( CP ) --> "(", !, choice_or_seq( CS ), opt_operator( CS, CP ).
cp( CP ) --> qname(Name), opt_operator(Name, CP).

rest_choice( [] )         --> ")", !.
rest_choice( [CP|RestC] ) --> "|", whitespace0, cp( CP ), whitespace0, rest_choice( RestC ).

rest_seq( [] )            --> ")", !.
rest_seq( [CP|RestS] )    --> ",", whitespace0, cp(CP), whitespace0, rest_seq( RestS ).

opt_operator( EC, opt(EC) ) --> "?", !.
opt_operator( EC, times(EC) ) --> "*", !.
opt_operator( EC, plus(EC) ) --> "+", !.
opt_operator( EC, EC ) --> [].

% [51]    Mixed    ::=    '(' S? '#PCDATA' (S? '|' S? QName)* S? ')*'  
%                       | '(' S? '#PCDATA' S? ')'  [VC: Proper Group/PE Nesting] 
%                                                  [VC: No Duplicate Types] 

mixed( MC ) -->
	"#PCDATA", !, whitespace0, mixed_content( MC ).

mixed_content( times( seq(['#pcdata']) ) ) --> ")*", !.
mixed_content( '#pcdata' ) --> ")", !.
mixed_content( times( choice( ['#pcdata',Name|RestMC] ) ) ) --> 
	"|", whitespace0, qname(Name), whitespace0, rest_mixed_children( RestMC ).


rest_mixed_children( [] ) --> ")*", !.
rest_mixed_children( [Name|RestMC] ) --> "|", !, whitespace0, qname(Name), whitespace0, rest_mixed_children( RestMC ).


%  [52]    AttlistDecl    ::=    '<!ATTLIST' S QName AttDef* S? '>' 
%  [53]    AttDef         ::=    S (QName | NSAttName) S AttType S DefaultDecl 


attlist_decl( Cx, NewCx ) --> 
	"ATTLIST", !, 
	whitespace, 
	qname(Name),
	{ createAttributeListInDTDContext( Cx, Name, AuxCx ) }, 
	opt_attdefs(Name,AuxCx, NewCx ).

opt_attdefs(_,Cx, Cx) --> ">", !.
opt_attdefs(ElName,Cx,NewCx) --> whitespace, attdefs0(ElName,Cx,NewCx).

attdefs0(_,Cx,Cx) --> ">", !.
attdefs0(ElName,Cx,NewCx) -->
	qname( Name ),
	whitespace,
	atttype( Type ),
	whitespace,
	default_decl( Cx, Default ),
	{ createAttributeDeclarationInDTDContext(Cx,ElName, Name,Type,Default,AuxCx) }, !,
	opt_attdefs(ElName,AuxCx,NewCx).


% [54]    AttType           ::=    StringType | TokenizedType | EnumeratedType  
% [55]    StringType        ::=    'CDATA' 
% [56]    TokenizedType     ::=    'ID' [VC: ID] 
%                                       [VC: One ID per Element Type] 
%                                       [VC: ID Attribute Default] 
%                           |      'IDREF' [VC: IDREF] 
%                           |      'IDREFS' [VC: IDREF] 
%                           |      'ENTITY' [VC: Entity Name] 
%                           |      'ENTITIES' [VC: Entity Name] 
%                           |      'NMTOKEN' [VC: Name Token] 
%                           |      'NMTOKENS' [VC: Name Token] 


atttype( Type ) -->
	+ stringtype( Type ), !.
atttype( Type ) -->
	+ tokenizedtype( Type ), !.
atttype( Type ) -->
	+ enumeratedtype( Type ), !.


stringtype( cdata ) --> "CDATA".

% Order of clauses is important!
tokenizedtype( idrefs ) --> "IDREFS".
tokenizedtype( idref ) --> "IDREF".
tokenizedtype( id ) --> "ID".
tokenizedtype( entity ) --> "ENTITY".
tokenizedtype( entities ) --> "ENTITIES".
tokenizedtype( nmtokens ) --> "NMTOKENS".
tokenizedtype( nmtoken ) --> "NMTOKEN".

% [57]    EnumeratedType    ::=    NotationType | Enumeration  
% [58]    NotationType      ::=    'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'  [VC: Notation Attributes] 
%                                                                                     [VC: One Notation Per Element Type] 
%                                                                                     [VC: No Notation on Empty Element] 
% [59]    Enumeration       ::=    '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')' [VC: Enumeration] 


enumeratedtype( Notations ) -->
	+ notationtype( Notations ).
enumeratedtype( Enumeration ) -->
	+ enumeration( Enumeration ).
	
notationtype( notations( Notations ) ) -->
	"NOTATION", !, whitespace, "(", whitespace0, notation_attribs( Notations ).

notation_attribs( [Name|RestNotations] ) -->
	ncname(Name),whitespace0, rest_notation_attribs( RestNotations ).

rest_notation_attribs( [] ) --> ")", !.
rest_notation_attribs( [Name|RestNotations] ) --> "|", !, whitespace0, ncname(Name), whitespace0, rest_notation_attribs( RestNotations ).

enumeration( enum( EnumNmtokens ) ) -->
	"(", !, whitespace0, nmtokens_enum( EnumNmtokens ).

nmtokens_enum( [Name|RestNmtokens] ) -->
	nmtoken(Name),whitespace0, rest_nmtokens_enum( RestNmtokens ).

rest_nmtokens_enum( [] ) --> ")", !.
rest_nmtokens_enum( [Name|RestNmtokens] ) --> "|", !, whitespace0, nmtoken(Name), whitespace0, rest_nmtokens_enum(RestNmtokens).


% [60]    DefaultDecl    ::=    '#REQUIRED' | '#IMPLIED'  
%                        |      (('#FIXED' S)? AttValue) [VC: Required Attribute] 
%                                                        [VC: Attribute Default Legal] 
%                                                        [WFC: No < in Attribute Values] 
%                                                        [VC: Fixed Attribute Default] 

default_decl( _, required ) -->
	"#REQUIRED", !.
default_decl( _, implied ) -->
	"#IMPLIED", !.
default_decl( Cx, fixed(Value) ) -->
	"#FIXED", !, { getGeneralEntitiesInDTDContext(Cx,Ents) }, whitespace, attvalue(Ents,Value,[]).
default_decl( Cx, default(Value) ) -->
	{ getGeneralEntitiesInDTDContext(Cx,Ents) }, attvalue( Ents, Value, []).

% [70]    EntityDecl    ::=    GEDecl | PEDecl 
% [71]    GEDecl        ::=    '<!ENTITY' S Name S EntityDef S? '>' 
% [72]    PEDecl        ::=    '<!ENTITY' S '%' S Name S PEDef S? '>' 
% [73]    EntityDef     ::=    EntityValue | (ExternalID NDataDecl?) 
% [74]    PEDef         ::=    EntityValue | ExternalID 

entity_decl( Cx, NewCx ) -->
	"ENTITY", !, 
	whitespace, 
	entity_gepedecl( Cx, NewCx ),
	whitespace0,
	">".

 
entity_gepedecl( Cx, NewCx ) --> "%", !,
	whitespace,
	pedecl( Cx, Name, Value  ),
	{ 
	   addParameterEntityToDTDContext( Cx,  Name, Value, NewCx ) 
      }.

entity_gepedecl( Cx, NewCx ) --> 
	gedecl( Cx, Name, Value, Notation ),
	{ addGeneralEntityToDTDContext( Cx, Notation, Value, Name, NewCx ) }.

pedecl( Cx, Name, Value  ) -->
	ncname( Name ), 
	whitespace, 
	pedef( Cx, Value  ).
	
gedecl( Cx, Name, Value, Notation ) -->
	ncname( Name ), 
	whitespace, 
	entitydef( Cx, Value, Notation ).


pedef( _, ID ) ::= !,
	externalID( ID ).
pedef( Cx, ReplacementText ) -->
%	{ getParameterEntitiesInDTDContext( Cx, PEs ) }, !,		% Parameter Entities in Entity values not allowed in internal DTD subset
	entity_value( [], ReplacementText ).
	
entitydef( _, ID, Notation ) ::= !,
	externalID( ID ),
	ndatadecl0( Notation ).
entitydef( Cx, ReplacementText, [] ) --> 
%	{ getParameterEntitiesInDTDContext( Cx, PEs ) }, !,		% Parameter Entities in Entity values not allowed in internal DTD subset
	entity_value( [], ReplacementText ).


% [75]    ExternalID    ::=    'SYSTEM' S SystemLiteral 
%                            | 'PUBLIC' S PubidLiteral S SystemLiteral 

externalID0( ID, no ) -->
	externalID(  ID ), !.
externalID0( id([],[]), yes ) --> [].


externalID( ID ) -->
	+ system_id( ID ).
externalID( id(PubId,SystemId)  ) -->
	"PUBLIC", !, whitespace, pubid_literal(PubId), whitespace, system_literal(SystemId).

system_id( id([],SystemId) ) -->
	"SYSTEM", !, whitespace, system_literal(SystemId).

% [76]    NDataDecl    ::=    S 'NDATA' S Name [VC: Notation Declared] 

ndatadecl0( Notation ) -->
	whitespace, !,
	ndatadecl( Notation ).
ndatadecl0( [] ) --> [].

ndatadecl( NotationName ) --> "NDATA", !,
	whitespace,
	ncname( NotationName).
ndatadecl( [] ) --> [].


% [82]    NotationDecl    ::=    '<!NOTATION' S Name S (ExternalID | PublicID) S? '>' [VC: Unique Notation Name] 
% [83]        PublicID    ::=    'PUBLIC' S PubidLiteral  


notation_decl( Cx, NewCx ) -->
	"NOTATION", !,
	whitespace,
	ncname( Name ),
	whitespace,
	notation_identifier( id(PublicId,SystemId) ),
	whitespace0,
	">",
	{ createNotationInDTDContext(Cx, Name,PublicId,SystemId,NewCx) }.


notation_identifier( ID ) -->
	+ system_id( ID ).
notation_identifier( id(PubId,SystemId)  ) -->
	"PUBLIC", !, whitespace, pubid_literal(PubId), system_literal0(SystemId).


% Skips spaces, if existing, and then read the SystemIdentifier
system_literal0( SystemId ) -->
	whitespace, !, system_lit0( SystemId ), !.
system_literal0( [] ) --> [].

system_lit0( SystemId ) -->
	system_literal( SystemId ), !.
system_lit0( [] ) --> [].
	

% Expands the parameter entity reference and calls the internal_dtd production with the replacement text.
% It should be generalized in order to handle conditional sections
pereference_in_dtd( Cx, NewCx ) -->
	!, ncname( Name ), ";",
	{ expand_pereference( Cx, Name, [Lookahead|Text]-[0'],'$'], AuxCx ),
	  internal_dtd( Lookahead, Text, AuxCx, NewCx, '$', [] )
	}.



%%%%% Auxiliary predicates
%%%%% DTD contexts mantain the information

createDTDContext( BaseURI, Standalone, AllProcessed, dtdcx( BaseURI, Standalone, AllProcessed, Log, Phys, PIs/PIs ) ) :-
	createLogicalStructures( Log ),
	createPhysicalStructures( Phys ).

getAllProcessedInDTDContext( dtdcx( _, _, yes, _, _, _ ), yes ) :- !.
getAllProcessedInDTDContext( dtdcx( _, _, _, _, _, _ ), no ) :- !.
getDTDContextBaseURI( dtdcx( BaseURI, _, _, _, _, _ ), BaseURI ) :- !.
getLogicalStructsInDTDContext( dtdcx( _, _, _, Log, _, _), Log ).
getPhysicalStructsInDTDContext( dtdcx( _, _, _, _, Phys, _), Phys ).
getPIsInDTDContext( dtdcx( _, _, _, _, _, PIs ), PIs ).
getGeneralEntitiesInDTDContext( dtdcx( _, _, _, _, Phys, _), Ents ) :- !,
	getPhysicalStructuresGeneralEntities( Phys, Ents ).
%getParameterEntitiesInDTDContext( dtdcx( _, _, _, _, Phys, _), PEs ) :- !,
%	getPhysicalStructuresParameterEntities( Phys, PEs ).


createElementSpecificationInDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs ), Name, CSpec, dtdcx( BaseURI, St, All, NewLog, Phys, PIs ) ) :- !,
	createElementSpecification( Name, CSpec, ElDecl ),
	addLogicalStructuresElementDeclaration( Log, Name, ElDecl, NewLog ).

createAttributeListInDTDContext( dtdcx( BaseURI, St, skip, Log, Phys, PIs ), _, dtdcx( BaseURI, St, skip, Log, Phys, PIs ) ) :- !. 
createAttributeListInDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs ), ElName, dtdcx( BaseURI, St, All, NewLog, Phys, PIs ) ) :- !,
	createLogicalStructuresAttributeList( Log, ElName, NewLog ).

createAttributeDeclarationInDTDContext( dtdcx( BaseURI, St, skip, Log, Phys, PIs ), _, _, _, _, dtdcx( BaseURI, St, skip, Log, Phys, PIs ) ) :- !. 
createAttributeDeclarationInDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs ), ElName, Name, Type, Default, dtdcx( BaseURI, St, All, NewLog, Phys, PIs ) ) :- !,
	createAttributeDeclaration(Name,Type,Default,AttDecl),
	addLogicalStructuresAttributeDeclaration( Log, ElName, AttDecl, NewLog ).

addParameterEntityToDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs ),  Name, Value, dtdcx( BaseURI, St, All, Log, NewPhys, PIs ) ) :- !,
	addPhysicalStructuresParameterEntity( Phys, Name, Value, NewPhys ).

addGeneralEntityToDTDContext( dtdcx( BaseURI, St, skip, Log, Phys, PIs ), _, _, _, dtdcx( BaseURI, St, skip, Log, Phys, PIs ) ) :- !.
addGeneralEntityToDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs ), Notation, Value, Name, dtdcx( BaseURI, St, NewAll, Log, NewPhys, PIs ) ) :- !,
	( ( Value = id(_,_), All = yes ) -> NewAll = no ; NewAll = All ),
	addPhysicalStructuresGeneralEntity( Notation, Value, Name, BaseURI, Phys, NewPhys ).

createNotationInDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs ), Name, PublicId, SystemId, dtdcx( BaseURI, St, All, Log, NewPhys, PIs ) ) :- !,
	createNotation(Name,PublicId,SystemId, BaseURI, Notation),
	addPhysicalStructuresNotation( Phys, Name, Notation, NewPhys ).

appendPItoDTDContext( dtdcx( BaseURI, St, All, Log, Phys, PIs/[PI|RestPIs] ), PI, dtdcx( BaseURI, St, All, Log, Phys, PIs/RestPIs ) ).


expand_pereference( dtdcx( BaseURI, St, All, Log, Phys, PIs ), Name, Text, dtdcx( BaseURI, St, NewAll, Log, Phys, PIs ) ) :-
	getPhysicalStructuresParameterEntities( Phys, PEs ),
	getNamedItem( PEs, Name, Item ),
	( Item \= id(_,_) 
	-> 	copy_term( Item, Text ),
		NewAll = All 
	; 	Text = S - S,
		( St = yes -> NewAll = All ; NewAll = skip )
	), !.
expand_pereference( _, _, _, _ ) :-
	error( 'Unknown Parameter Entity' ).

