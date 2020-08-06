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
%%% (http://www.w3.org/TR/xml-infoset). For the time being it lacks full XML Base support
%%% (it does not resolve relative references) even though the properties in the information items and
%%% corresponding methods are already supported. 
%%% 
%%% XML Namespaces are fully supported.
%%%
%%%


:- start( xml_document/3 ).
:- end( -1 ).


% Production	
% [22]    prolog    ::=    XMLDecl? Misc* (doctypedecl Misc*)? 

xml_document( BaseURI, Encoding, NewDoc ) -->
%	whitespace0,
	xml_declaration( BaseURI, Encoding, Standalone, Doc ),
	{ createDocumentFragment( NewFrag ) }, 
	misc0( BaseURI, NewFrag, AuxFrag1 ),
	doctypedecl0( BaseURI, Standalone, Physical, All, DTD ), 
	{ 
	  	( DTD \= [] -> appendDocumentFragmentChild( AuxFrag1, DTD, AuxFrag2 ),  
				   setDocumentDTD( Doc, DTD, AuxDoc1 ),
				   setDocumentPhysicalStructures( AuxDoc1, Physical, AuxDoc2 ),
				   setDocumentAllProcessed( AuxDoc2, All, AuxDoc )
	      ; 
				   AuxFrag2 = AuxFrag1, AuxDoc = Doc 
		)
	},
	misc0( BaseURI, AuxFrag2, Prolog ),
	{ createDocumentContext( AuxDoc, Physical, Cx ) },
	{ checkEntities( Cx ) },								% Verifies if internal entities are well-formed
	element( Cx, DocElement ),
	{ appendDocumentFragmentChild( Prolog, DocElement, Epilog ) }, 
	misc0( BaseURI, Epilog, Document ),
	{ appendDocumentChildren( AuxDoc, Document, AuxDoc3 ),
	  setDocumentElement( AuxDoc3, DocElement, NewDoc )
	}.

% [23]    XMLDecl    ::=    '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>' 

% The XML declaration is optional

xml_declaration( BaseURI, Encoding, Standalone, Doc ) -->
	xml_declaration0( Version, Encoding, Standalone ),
	{ createDocument( NewDocument ),
	  setDocumentBaseURI( NewDocument, BaseURI, EmptyDocument ),
	  setDocumentVersion( EmptyDocument, Version, AuxDoc1),
	  setDocumentEncoding( AuxDoc1, Encoding, AuxDoc2),
	  setDocumentStandalone( AuxDoc2, Standalone, Doc)
	}.

% If XML declaration is not provided, then both version and standalone have no value, as prescribed in
% XML Info Sets. The only recognized encodings are UTF-8 and UTF-16, and by default UTF-8 is used.	

xml_declaration0( Version, Encoding, Standalone ) -->
	"<?xml", !, version_info( Version ), encoding_decl0( Encoding ), sd_decl0( Standalone ), whitespace0, "?>".
xml_declaration0( [],'UTF-8',[]) -->  [], !.
xml_declaration0( [],'UTF-16',[]) --> [], !.


% [24]    VersionInfo    ::=    S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
version_info( Version ) -->
	whitespace, "version", eq, versionnumquote( Version ).

versionnumquote( Version ) ::= "'", !, versionnum( Version ), "'".
versionnumquote( Version ) ::= """", !, versionnum( Version ), """".

% [25]    Eq    ::=    S? '=' S? 

eq --> whitespace0, "=", whitespace0.

	
% [26]    VersionNum    ::=    ([a-zA-Z0-9_.:] | '-')+ 
versionnum( '1.0' ) ::= "1.0", !.

% [27]    Misc    ::=    Comment | PI | S 

%% Constructs a document fragment with the markup in the prolog or epilog.
%% A document fragment is implemented with difference lists

misc0( BaseURI, Frag, NewFrag ) -->
	+ comment( Comment ),
	{ appendDocumentFragmentChild( Frag, Comment, AuxFrag ) },
	misc0( BaseURI, AuxFrag, NewFrag ).
misc0( BaseURI, Frag, NewFrag ) -->
	+ pi( BaseURI, PI ),
	{ appendDocumentFragmentChild( Frag, PI, AuxFrag ) },
	misc0( BaseURI, AuxFrag, NewFrag ).
misc0( BaseURI, Frag, NewFrag ) -->
	whitespace, !,
	misc0( BaseURI, Frag, NewFrag ).
misc0( _, Frag, Frag ) --> [].



% [28]    doctypedecl    ::=    '<!DOCTYPE' S QName (S ExternalID)? S? ('[' (markupdecl | DeclSep)* ']' S?)? '>' 

doctypedecl0( BaseURI, Standalone, Physical, All, DTD ) --> 
	"<!DOCTYPE", !, 
	whitespace, 
	qname( Name ), 
	whitespace0, 
	externalID0( id(PublicId,SystemId), Processed ), 
	whitespace0, 
	internal0( BaseURI, Standalone, Processed, Logical, Physical, PIs, All ), 
	{ createDocumentType( Name, PublicId, SystemId, Logical, PIs, DTD ) },
	whitespace0, ">".
doctypedecl0( _, _, Physical, yes, [] ) --> [],
	{ createPhysicalStructures( Physical ) }.

 
% [39]    element    ::=    EmptyElemTag 
%                      |    STag content ETag 

element( Cx, Element ) -->
	"<",  + rest_element( Cx, Element ).

rest_element( Cx, Element ) -->
	qname(  QName ),
%	{ write( QName ), nl },
	{ createElementContext( Cx, QName, CxElem) },
	start_tag( CxElem, Attribs, NamespaceAttribs ),
	{ createElementFromContext( CxElem, Attribs, NamespaceAttribs, AuxElem, NewCx ) },
	element_content( NewCx, Content ), 
	{ setElementContent( AuxElem, Content, Element ) }.
	
element_content( Cx, Content ) --> ">", !,
	content( Cx, Content ),
	end_tag( Cx ).

element_content( Cx, [] ) --> "/>", !.


% Parsing of start tags and corresponding attributes	
% [40]    STag    ::=    '<' Name (S Attribute)* S? '>' [WFC: Unique Att Spec] 

start_tag( Cx, Attribs, NamespaceAttribs ) --> 
	+ rest_stag( Cx, Attribs, NamespaceAttribs ).
start_tag( Cx, Attribs, NamespaceAttribs ) ::=				% Should be ::= for better performance
	whitespace, !,
	listattribs(  Cx, Attribs, NamespaceAttribs ).
 
listattribs( Cx, Attribs, NamespaceAttribs ) --> 
	+ rest_stag( Cx, Attribs, NamespaceAttribs ).
listattribs( Cx, Attribs, NamespaceAttribs ) -->
	attribute( Cx, Name, Value ),
	{ splitAttrib( Name, Value, Attribs, RestAttribs, NamespaceAttribs, RestNamespaceAttribs ) }, !,
	start_tag( Cx, RestAttribs, RestNamespaceAttribs ).

rest_stag( _Cx, [], [] ) --> ? "/>", !.
rest_stag( _Cx, [], [] ) --> ? ">", !.


% [41]    Attribute    ::=    Name Eq AttValue 

attribute( Cx, Name, Value ) -->
	qname(  Name ),
	eq, 
	{ getContextEntities(Cx, GEs)	},
	attvalue( GEs, Value, [] ).

attvalue( GEs, Value, RestValue ) --> "'",  !, attvalue( apos, GEs, Value, RestValue ).
attvalue( GEs, Value, RestValue ) --> """", !, attvalue( quot, GEs, Value, RestValue ).

% Missing normalization of attribute values for non CDATA types

attvalue( _, _, _, _ ) --> "<", !, error( ' < not allowed in attribute value' ).
attvalue( apos, _, Rest, Rest ) --> "'", !.
attvalue( quot, _, Rest, Rest ) --> """", !.
% Handles expansion of entities in attribute values
attvalue( ient, _, Value, RestValue ) --> [-2,GEs], !,
	attvalue( ient, GEs, Value, RestValue ). 
attvalue( ient, _, Value, RestValue ) --> [-3,GEs], !,
	attvalue( apos, GEs, Value, RestValue ). 
attvalue( ient, _, Value, RestValue ) --> [-4,GEs], !,
	attvalue( quot, GEs, Value, RestValue ). 
attvalue( End, GEs, Value, RestValue ) --> "&", !, 
	reference_in_attribute( GEs, End, Value, RestValue ).
attvalue( End, GEs, [0' |Value], RestValue ) --> [16'9], !,
	attvalue( End, GEs, Value, RestValue ).
% Substitution of end of lines
attvalue( End, GEs, [0' |Value], RestValue ) --> [16'A], !,
	attvalue( End, GEs, Value, RestValue ).
attvalue( End, GEs, Value, RestValue ) --> [16'D], !,
	attvalue1( End, GEs, Value, RestValue ).
%attvalue( End, GEs, [0' |Value], RestValue ) --> [16'85], !,		% For XML 1.1
%	attvalue( End, GEs, Value, RestValue ).
%attvalue( End, GEs, [0' |Value], RestValue ) --> [16'2028], !,		% For XML 1.1
%	attvalue( End, GEs, Value, RestValue ).
attvalue( End, GEs, [Char|Value], RestValue ) --> [Char], !,
	{ xml_char( Char ) },							% Uncomment for fully conformant XML parser
	attvalue( End, GEs, Value, RestValue ).

attvalue1( End, GEs, [0' |Value], RestValue ) --> [16'A], !,
	attvalue( End, GEs, Value, RestValue ).
attvalue1( End, GEs, [0' |Value], RestValue ) --> [16'D], !,
	attvalue1( End, GEs, Value, RestValue ).
%attvalue1( End, GEs, [0' |Value], RestValue ) --> [16'85], !,		% For XML 1.1
%	attvalue( End, GEs, Value, RestValue ).
%attvalue1( End, GEs, [0' ,0' |Value], RestValue ) --> [16'2028], !,	% For XML 1.1
%	attvalue( End, GEs, Value, RestValue ).
attvalue1( End, GEs, Value, RestValue ) --> !, attvalue( End, GEs, Value, RestValue ).


reference_in_attribute( GEs, End, Value, RestValue ) --> "#", !, 
	char_ref( Value, AuxValue ),
	attvalue( End, GEs, AuxValue, RestValue ).
reference_in_attribute( Look, Rest, GEs, End, Value, RestValue, Out2, SOut2 ) :-
	entity_ref( Look, Rest, GEs, Entity,  NewGEs, Out1, SOut1 ), !,  % the expanded entity is removed in order to avoid cycles
	( isExternalEntity( Entity ) ->
		error('Reference in attribute to an external parsed entity not allowed!')
	;
		attrcharend( End, CharEnd ),
		getEntityReplacementText( Entity, [Look1|Rest1]-[CharEnd,GEs,Out1|SOut1] ),
	      attvalue( Look1, Rest1, ient, NewGEs, Value, RestValue, Out2, SOut2 ) 		
	).

% This puts a mark stating where the replacement text of an entity ends 
attrcharend( ient, -2 ) :- !.
attrcharend( apos, -3 ) :- !.
attrcharend( quot, -4 ) :- !.

% Parsing of content
% [43]    content    ::=    CharData? ((element | Reference | CDSect | PI | Comment) CharData?)* 

content( Cx, Content ) --> 
	[-2,GEs], { setContextEntities( Cx, GEs, NewCx ) }, !,
	content( NewCx, Content ).

content( Cx, Content ) --> 
	"<", !, markup_in_content( Cx, Content ).

content( Cx, Content ) --> 
	"&", !, reference_in_content( Cx, Content ).

content( Cx, Content ) --> !,
	chardata_in_content( CharData, RestChars, Whitespace ),
	content( Cx, AuxContent ),
	{ addchardata( Whitespace, Cx, CharData, RestChars, AuxContent, Content ) }.


addchardata( yes, Cx, CharData, RestChars, AuxContent, Content ) :- 
	getContextPreserveSpace( Cx, default ) -> 
		Content = AuxContent
	;
		mergewhitespace( CharData, RestChars, AuxContent, Content )		
	.	

addchardata( no, _, CharData, RestChars, AuxContent, Content ) :- !,
	mergechardata( CharData, RestChars, AuxContent, Content ).
	
mergechardata( [], [], Content, Content ) :- !.
mergechardata( CharData, RestChars, [pcdata(RestChars)|RestContent], [pcdata(CharData)|RestContent] ) :- !.
mergechardata( CharData, RestChars, [whitespace(RestChars)|RestContent], [pcdata(CharData)|RestContent] ) :- !.
mergechardata( CharData, [], Content, [pcdata(CharData)|Content] ).

mergewhitespace( [], [], Content, Content ) :- !.
mergewhitespace( CharData, RestChars, [pcdata(RestChars)|RestContent], [pcdata(CharData)|RestContent] ) :- !.
mergewhitespace( CharData, RestChars, [whitespace(RestChars)|RestContent], [whitespace(CharData)|RestContent] ) :- !.
mergewhitespace( CharData, [], Content, [whitespace(CharData)|Content] ).


% Reading character data, including end-of-line handling and whitespace elimination
chardata_in_content( CharData, CharData, yes ) --> ? "<", !.
chardata_in_content( CharData, CharData, yes ) --> ? "&", !.
chardata_in_content( CharData, CharData, yes ) --> ? [-2], !.
chardata_in_content( [16'9|CharData], RestChars, Whitespace ) --> 
	[16'9], !, chardata_in_content( CharData, RestChars, Whitespace ). 
chardata_in_content( [16'A|CharData], RestChars, Whitespace ) --> 
	[16'A], !, chardata_in_content( CharData, RestChars, Whitespace ). 
chardata_in_content( CharData, RestChars, Whitespace ) --> 
	[16'D], !, chardata_in_content1( CharData, RestChars, Whitespace ). 
chardata_in_content( [16'20|CharData], RestChars, Whitespace ) --> 
	[16'20], !, chardata_in_content( CharData, RestChars, Whitespace ). 
chardata_in_content( [0']|CharData], RestChars, no ) --> 
	"]", !, chardata_in_content2( CharData, RestChars ). 
%chardata_in_content( [16'A|CharData], RestChars, Whitespace ) --> 			% For XML 1.1
%	[16'85], !, chardata_in_content( CharData, RestChars, Whitespace ). 
%chardata_in_content( [16'A|CharData], RestChars, Whitespace ) --> 			% For XML 1.1
%	[16'2028], !, chardata_in_content( CharData, RestChars, Whitespace ). 
chardata_in_content( [Char|CharData], RestChars, no ) --> 
	[Char], !, 
	{ xml_char( Char ) },										% Uncomment for fully conformant XML parser
	chardata_in_content( CharData, RestChars, _ ). 


chardata_in_content1( [16'A|CharData], RestChars, Whitespace ) --> 
	[16'A], !, chardata_in_content( CharData, RestChars, Whitespace ). 
chardata_in_content1( CharData, RestChars, Whitespace ) --> 
	[16'D], !, chardata_in_content1( CharData, RestChars, Whitespace ). 
%chardata_in_content1( [16'A|CharData], RestChars, Whitespace ) --> 			% For XML 1.1
%	[16'85], !, chardata_in_content( CharData, RestChars, Whitespace ). 
%chardata_in_content1( [16'A,16'A|CharData], RestChars, Whitespace ) --> 		% For XML 1.1
%	[16'2028], !, chardata_in_content( CharData, RestChars, Whitespace ). 
chardata_in_content1( [16'A|CharData], RestChars, Whitespace ) --> !, chardata_in_content( CharData, RestChars, Whitespace ). 


% Takes care of the sequence ]]> in character data
chardata_in_content2( [0']|CharData], RestChars ) --> 
	"]", !, chardata_in_content3( CharData, RestChars ). 
chardata_in_content2( CharData, RestChars ) --> 
	!, chardata_in_content( CharData, RestChars, _ ). 

chardata_in_content3( _, _ ) --> 
	">", !, error( 'String ]]> not allowed in element content' ). 
chardata_in_content3( [0']|CharData], RestChars ) --> 
	"]", !, chardata_in_content3( CharData, RestChars ). 
chardata_in_content3( CharData, RestChars ) --> 
	!, chardata_in_content( CharData, RestChars, _ ). 


% Should I use document fragments instead of lists?
markup_in_content( _Cx, [] ) --> 
	"/", !. 
markup_in_content( Cx, [PI|RestContent] ) --> 
	"?", !,
	{ getContextBaseURI( Cx, BaseURI ) },
	pi_rest( BaseURI, PI ), !,
	content( Cx, RestContent ). 
markup_in_content( Cx, Content ) --> "!", !,
	picdata( Cx, Content ).
markup_in_content( Cx, [Element|RestContent] ) -->  !,
	rest_element( Cx, Element ), 
	content( Cx, RestContent ).

picdata( Cx, Content ) --> "[CDATA[", !,
	cdata_section( CharData, RestChars ),
	content( Cx, AuxContent ),
	{ mergechardata( CharData, RestChars, AuxContent, Content ) }.

picdata( Cx, [Comment|RestContent] ) --> "--", !,
	comment_rest( Comment ),
	content( Cx, RestContent ).


cdata_section( CharData, RestCharData ) -->
	"]", !, cdata_section1( CharData, RestCharData ).
cdata_section( [Char|CharData], RestCharData ) -->
	[Char], !, 
	{ xml_char( Char ) },							% Uncomment for fully conformant XML parser
	cdata_section( CharData, RestCharData ).

cdata_section1( CharData, RestCharData ) -->
	"]", !, cdata_section2( CharData, RestCharData ).
cdata_section1( [0'],Char|CharData], RestCharData ) -->
	[Char], !, 
	{ xml_char( Char ) },							% Uncomment for fully conformant XML parser
	cdata_section( CharData, RestCharData ).

cdata_section2( CharData, CharData ) -->
	">", !.
cdata_section2( [0']|CharData], RestCharData ) -->
	"]", !, cdata_section2( CharData, RestCharData ).
cdata_section2( [0'],0'],Char|CharData], RestCharData ) -->
	[Char], !, 
	{ xml_char( Char ) },							% Uncomment for fully conformant XML parser
	cdata_section( CharData, RestCharData ).

% End Tag: "</" has already been processed

end_tag( Cx ) -->
	{ getContextName( Cx, Name ) },
	qname( Name ),
	whitespace0, ">".


% [67]    Reference    ::=    EntityRef | CharRef 
%                             & has already been parsed.
%

reference_in_content( Cx, Content ) -->
	"#", !, char_ref( CharData, RestChars ),
	content( Cx, AuxContent ),
	{ mergechardata( CharData, RestChars, AuxContent, Content ) }.

reference_in_content( Look, Rest, Cx, Content, Out2, SOut2 ) :-
	getContextEntities( Cx, GEs ),
	entity_ref( Look, Rest, GEs, Entity, NewGEs, Out1, SOut1 ), !,	% the expanded entity is removed in order to avoid cycles
	( isExternalEntity( Entity ) ->
		Content = [Entity|RestContent],
		content( Out1, SOut1, Cx, RestContent, Out2, SOut2 )		
	;
	  	getEntityReplacementText( Entity, [Look1|Rest1] - [-2,GEs,Out1|SOut1] ),	% The -2 marks the end of the replacement text of an entity in content
		setContextEntities( Cx, NewGEs, NewCx ),
		!, content( Look1, Rest1, NewCx, Content, Out2, SOut2 )
	).


% [66]    CharRef    ::=    '&#' [0-9]+ ';'  
%                      |    '&#x' [0-9a-fA-F]+ ';' [WFC: Legal Character] 


char_ref( [Code|Rest], Rest ) ::=
	"x", !, hexadecimal_ref( Code ).
char_ref( [Code|Rest], Rest ) ::= !,
	decimal_ref( Code ).
%char_ref( _, _ ) -->
%	error( 'Expecting ''x'' or number' ).

hexadecimal_ref( Code ) -->
	+ hexadecimal_char( Value ),
	hexadecimal_ref( Value, Code ), 
	{ xml_char( Code ) }.

hexadecimal_ref( Code, Code ) ::= ";", !.
hexadecimal_ref( OldValue, Code ) ::=
	+ hexadecimal_char( Value ), 
	{ NewValue is OldValue*16 + Value }, 
	!, hexadecimal_ref( NewValue, Code ).

hexadecimal_char( 0 ) --> "0", !.
hexadecimal_char( 1 ) --> "1", !.
hexadecimal_char( 2 ) --> "2", !.
hexadecimal_char( 3 ) --> "3", !.
hexadecimal_char( 4 ) --> "4", !.
hexadecimal_char( 5 ) --> "5", !.
hexadecimal_char( 6 ) --> "6", !.
hexadecimal_char( 7 ) --> "7", !.
hexadecimal_char( 8 ) --> "8", !.
hexadecimal_char( 9 ) --> "9", !.
hexadecimal_char( 10 ) --> "A", !.
hexadecimal_char( 10 ) --> "a", !.
hexadecimal_char( 11 ) --> "B", !.
hexadecimal_char( 11 ) --> "b", !.
hexadecimal_char( 12 ) --> "C", !.
hexadecimal_char( 12 ) --> "c", !.
hexadecimal_char( 13 ) --> "D", !.
hexadecimal_char( 13 ) --> "d", !.
hexadecimal_char( 14 ) --> "E", !.
hexadecimal_char( 14 ) --> "e", !.
hexadecimal_char( 15 ) --> "F", !.
hexadecimal_char( 15 ) --> "f", !.

decimal_ref( Code ) -->
	+ decimal_char( Value ),
	decimal_ref( Value, Code ),
	{ xml_char( Code ) }.

decimal_ref( Code, Code ) ::= ";", !.
decimal_ref( OldValue, Code ) ::=
	+ decimal_char( Value ), 
	{ NewValue is OldValue*10 + Value }, 
	!, decimal_ref( NewValue, Code ).

decimal_char( 0 ) --> "0", !.
decimal_char( 1 ) --> "1", !.
decimal_char( 2 ) --> "2", !.
decimal_char( 3 ) --> "3", !.
decimal_char( 4 ) --> "4", !.
decimal_char( 5 ) --> "5", !.
decimal_char( 6 ) --> "6", !.
decimal_char( 7 ) --> "7", !.
decimal_char( 8 ) --> "8", !.
decimal_char( 9 ) --> "9", !.

% [68]    EntityRef    ::=    '&' Name ';' 
%                              & has already been parsed

entity_ref( GEs, Value, NewGEs ) -->
	ncname( Name ), 
	";", 
	{ expand_entity( Name, GEs, Value, NewGEs ) }.

% <!ENTITY lt     "&#38;#60;">
% <!ENTITY gt     "&#62;">
% <!ENTITY amp    "&#38;#38;">
% <!ENTITY apos   "&#39;">
% <!ENTITY quot   "&#34;">

expand_entity( lt, GEs, [0'&,0'#,0'6,0'0,0';|Rest]-Rest, GEs ) :- !.
expand_entity( gt, GEs, [0'>|Rest]-Rest, GEs ) :- !.
expand_entity( amp, GEs, [0'&,0'#,0'3,0'8,0';|Rest]-Rest, GEs ) :- !.
expand_entity( apos, GEs, [0''|Rest]-Rest, GEs ) :- !.
expand_entity( quot, GEs, [0'"|Rest]-Rest, GEs ) :- !.
expand_entity( Name, GEs, Value, NewGEs ) :- !,
	removeNamedItem( GEs, Name, Value, NewGEs ).


% [80]    EncodingDecl    ::=    S 'encoding' Eq ('"' EncName '"' | "'" EncName "'" )  
encoding_decl0( Encoding ) -->
	whitespace, 
	"encoding", !,
	eq,
	encnamequote( Encoding ).

encoding_decl0( 'UTF-8' )  --> [], !.
encoding_decl0( 'UTF-16' ) --> [], !.

encnamequote( Encoding ) ::= "'", !, encname( Encoding ), "'".
encnamequote( Encoding ) ::= """", !, encname( Encoding ), """".

% [81]    EncName    ::=    [A-Za-z] ([A-Za-z0-9._] | '-')* /* Encoding name contains only Latin characters */
% EncName is converted to upper case

encname( Encoding ) -->
	encstartchar( Char ),
	encnamerest( RestName ), !,
	{ atom_codes( Encoding, [Char|RestName] ), recognized_encodings( Encoding ) }.

encnamerest( [Char|RestName] ) -->
	+ encchar( Char ), !,encnamerest( RestName ).
encnamerest( [] ) --> [].

encchar( 0'. ) --> ".", !.
encchar( 0'_ ) --> "_", !.
encchar( 0'- ) --> "-", !.
encchar( Char ) --> [0'0-0'9]/[Char], !.
encchar( Char ) --> + encstartchar( Char ).

encstartchar( Char ) --> [0'A-0'Z]/[Char], !.
encstartchar( Char ) --> [0'a-0'z]/[C], !, { Char is C - 32 }.


recognized_encodings( 'US-ASCII' ) :- !.
recognized_encodings( 'ISO-8859-1' ) :- !.
recognized_encodings( 'UTF-8' ) :- !.
recognized_encodings( 'UTF-16' ) :- !.
recognized_encodings( 'UTF-16BE' ) :- !.
recognized_encodings( 'UTF-16LE' ) :- !.
recognized_encodings( 'UTF-32' ) :- !.
recognized_encodings( 'UTF-32BE' ) :- !.
recognized_encodings( 'UTF-32LE' ):- !.
recognized_encodings( Enc ) :-
	error( Enc ).


%[32]    SDDecl    ::=    S 'standalone' Eq (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no') '"'))

sd_decl0( STD ) -->
	whitespace,
	"standalone", !,
	eq,
	yes_or_no( STD ).

sd_decl0( [] ) --> [].

yes_or_no( yes ) ::= "'yes'", !.
yes_or_no( no ) ::= "'no'", !.

yes_or_no( yes ) ::= """yes""", !.
yes_or_no( no ) ::= """no""", !.

% [15]    Comment    ::=    '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 

comment( Comment ) ::= "<!--", !, comment_rest( Comment ).

comment_rest( Comment ) --> 
	comment_text( Text ),
	{ createComment( Text, Comment ) }.

comment_text( [] ) ::= 
	"--", !, comment_end.
comment_text( [Char|RestText] ) --> 
	[Char], !, 
	{ xml_char( Char ) },							% Uncomment for fully conformant XML parser
	comment_text( RestText ).
%comment_text( _ ) -->
%	error( 'Comment not ended.' ).	

comment_end --> ">", !.
%comment_end --> !, error( '-- cannot occur inside a comment' ).


% [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
% [17]    PITarget    ::=    Name - (('X' | 'x') ('M' | 'm') ('L' | 'l')) 

pi( BaseURI, PI ) ::= "<?", !, pi_rest( BaseURI, PI ).

pi_rest( BaseURI, PI ) -->
	pi_target( Target ), 
	pi_instruction( Content ),
	{ createPI( Target, Content, BaseURI, PI ) }.

pi_target( Target ) -->
	ncname( Target ),
	{\+ invalid_pi( Target ) }, !.
%pi_target( Target ) -->
%	error( 'Invalid target in processing instruction ' ).

invalid_pi( 'xml' ).
invalid_pi( 'xmL' ).
invalid_pi( 'xMl' ).
invalid_pi( 'xML' ).
invalid_pi( 'Xml' ).
invalid_pi( 'XmL' ).
invalid_pi( 'XMl' ).
invalid_pi( 'XML' ).

pi_instruction( [] ) ::= "?>", !.
pi_instruction( Instruction ) -->
	whitespace,
	pi_instruction_text( Instruction ).

pi_instruction_text( [] ) ::= "?>", !.
pi_instruction_text( [Char|RestInstruction] ) -->
	[Char], !, 
	{ xml_char( Char ) },							% Uncomment for fully conformant XML parser
	pi_instruction_text( RestInstruction ).
%pi_instruction_text( _ ) -->
%	error( 'Processing instruction not finished' ).



%%%%%%%%%%%%%%%%%%%%  Contexts %%%%%%%%%%%%%%%%%%
% creates the initialization context containing the current document, the xml:space default, initial
% namespaces declarations, base URI, xml:lang value, and entities
%
% cx(Document,xml:space,InScopeNamespaces, ElementName, BaseURI, xml:lang, Entities)

createDocumentContext( Document, Physical, cx(Document,[],InScope,[],BaseURI,[],Entities) ) :-
	createNamedMap( Empty ),
	getDocumentBaseURI( Document, BaseURI ),
	getPhysicalStructuresGeneralEntities( Physical, Entities ),
	setNamedItem( Empty, '', '', DefaultNS ),									% Default namespace not declared
	setNamedItem( DefaultNS, xml, 'http://www.w3.org/XML/1998/namespace', InScope ).		% The XML namespace

% The start tag of an element begins a new context
createElementContext( cx(Document,Preserve,InScope,_,BaseURI,Lang,Entities), Name, cx(Document,Preserve,InScope,Name,BaseURI,Lang,Entities) ) :- !.

getContextDocument( cx(Document,_,_,_,_,_,_), Document ).
getContextPreserveSpace( cx(_,Preserve,_,_,_,_,_), Preserve ).
getContextInScopeNamespaces( cx(_,_,InScope,_,_,_,_), InScope ).
getContextName( cx(_,_,_,Name,_,_,_), Name ).
getContextBaseURI( cx(_,_,_,_,BaseURI,_,_), BaseURI).
getContextLang( cx(_,_,_,_,_,Lang,_), Lang).
getContextEntities( cx(_,_,_,_,_,_,Entities), Entities).

setContextPreserveSpace( cx(Document,_,InScope,Name,BaseURI,Lang,Entities), Preserve, cx(Document,Preserve,InScope,Name,BaseURI,Lang,Entities) ).
setContextInScopeNamespaces( cx(Document,Preserve,_,Name,BaseURI,Lang,Entities), InScope, cx(Document,Preserve,InScope,Name,BaseURI,Lang,Entities) ).
setContextBaseURI( cx(Document,Preserve,InScope,Name,_,Lang,Entities), BaseURI, cx(Document,Preserve,InScope,Name,BaseURI,Lang,Entities) ).
setContextLang( cx(Document,Preserve,InScope,Name,BaseURI,_,Entities), Lang, cx(Document,Preserve,InScope,Name,BaseURI,Lang,Entities) ).
setContextEntities( cx(Document,Preserve,InScope,Name,BaseURI,Lang,_), Entities, cx(Document,Preserve,InScope,Name,BaseURI,Lang,Entities) ).

createElementFromContext( Cx, Attribs, NamespaceAttribs,  Element, NewCx ) :-
	% Obtains the information from the context to 

	getContextDocument( Cx, Document ),
	getContextInScopeNamespaces( Cx, InScope ),
	getContextName( Cx, ElemQName ),

	ElemQName = qname(ElemPrefix,ElemLocal),
	getDocumentDefaultAttributes( Document, ElemQName, DefaultAttribs, NamespaceDefaultAttribs ),
	createNamedMap( EmptyNAttribs ),
	addNamespaceAttributes( NamespaceAttribs, no, InScope, EmptyNAttribs, AuxInScope, AuxNAttribs ),
	addNamespaceAttributes( NamespaceDefaultAttribs, yes, AuxInScope, AuxNAttribs, NewInScope, NAttribs ),

	setContextInScopeNamespaces( Cx, NewInScope, NSCx ),
	createNamedMap( EmptyAttribs ),
	addAttributes( Attribs, no, NSCx, EmptyAttribs, AuxAttribs, AuxCx ),
	addAttributes( DefaultAttribs, yes, AuxCx, AuxAttribs, MAttribs, NewCx ),

	getContextBaseURI( NewCx, BaseURI ),
	getContextLang( NewCx, Lang ),
	createElementNS( ElemPrefix, ElemLocal, BaseURI, NewInScope, Lang, AuxElement ),
	setElementAttributes( AuxElement, MAttribs, NAttribs, Element ).
 

addNamespaceAttributes( [], _, InScope, NAttribs, InScope, NAttribs ).
addNamespaceAttributes( [Decl|RestDecl], Default, InScope, NAttribs, NewInScope, NewNAttribs ) :-
	declareNamespace( Decl, Default, InScope, NAttribs, AuxInScope, AuxNAttribs ), !,
	addNamespaceAttributes( RestDecl, Default, AuxInScope, AuxNAttribs, NewInScope, NewNAttribs ).

declareNamespace( qname( '', xmlns) = Value, Default, InScope, NameAttribs, NewInScope, NewNameAttribs ) :- !,
	ucs2utf8( Value, URIRefString ),
	atom_codes( URIRef, URIRefString ),
	createAttributeNS( '', xmlns, 'http://www.w3.org/2000/xmlns/', Value, Default, Attrib ),
	setNamedItem( InScope, '', URIRef, NewInScope ),
	addAttribute( Default, ename('http://www.w3.org/2000/xmlns/',''), Attrib, NameAttribs, NewNameAttribs ).
	
declareNamespace( qname( xmlns, Prefix) = Value, Default, InScope, NameAttribs, NewInScope, NewNameAttribs ) :- !,
	( Value = [] -> throw( xml( 'Namespace value cannot be empty', Prefix ) ) ; true ),
	ucs2utf8( Value, URIRefString ),
	atom_codes( URIRef, URIRefString ),
	createAttributeNS( xmlns, Prefix, 'http://www.w3.org/2000/xmlns/', Value, Default, Attrib ),
	setNamedItem( InScope, Prefix, URIRef, NewInScope ),
	addAttribute( Default, ename('http://www.w3.org/2000/xmlns/',Prefix), Attrib, NameAttribs, NewNameAttribs ).


addAttributes( [], _, Cx, Attribs, Attribs, Cx ).
addAttributes( [qname(Prefix,Local)=Value|RestDecl], Default, Cx, Attribs, NewAttribs, NewCx ) :-
	setAttribute( Prefix, Local, Value, Default, Cx, Attribs, AuxAttribs, AuxCx ), !,
	addAttributes( RestDecl, Default, AuxCx, AuxAttribs, NewAttribs, NewCx ).


% Take care of special XML attributes 
% Default namespaces do not apply to attributes
setAttribute( '', Local, Value, Default, Cx, Attribs, NewAttribs, Cx ) :- !,
	createAttributeNS( '', Local, '', Value, Default, Attrib ),
	addAttribute( Default, ename('',Local), Attrib, Attribs, NewAttribs ).

setAttribute( xml,space, Value, Default, Cx, Attribs, NewAttribs, NewCx ) :- !,
	( Value = "preserve" -> 
		setContextPreserveSpace( Cx, preserve, NewCx )
	; Value = "default" ->
		setContextPreserveSpace( Cx, default, NewCx )
	),
	createAttributeNS( xml, space, 'http://www.w3.org/XML/1998/namespace', Value, Default, Attrib ),
	addAttribute( Default, ename('http://www.w3.org/XML/1998/namespace',space), Attrib, Attribs, NewAttribs ).
	
setAttribute( xml,lang, Lang, Default, Cx, Attribs, NewAttribs, NewCx ) :- !,
	setContextLang( Cx, Lang, NewCx ),
	createAttributeNS( xml, lang, 'http://www.w3.org/XML/1998/namespace', Lang, Default, Attrib ),
	addAttribute( Default, ename('http://www.w3.org/XML/1998/namespace',lang), Attrib, Attribs, NewAttribs ).

setAttribute( xml,base, Base, Default, Cx, Attribs, NewAttribs, NewCx ) :- !,
	parseIRIref( no, Base, RelIRI ),
	getContextBaseURI( Cx, BaseIRI ),
	resolveIRIref( RelIRI, BaseIRI, AbsIRI ),
	setContextBaseURI( Cx, AbsIRI, NewCx ),
	createAttributeNS( xml, base, 'http://www.w3.org/XML/1998/namespace', Base, Default, Attrib ),
	addAttribute( Default, ename('http://www.w3.org/XML/1998/namespace',base), Attrib, Attribs, NewAttribs ).

% Ordinary attributes
setAttribute( Prefix,Local, Value, Default, Cx, Attribs, NewAttribs, Cx ) :- !,
	getContextInScopeNamespaces( Cx, InScope ),
	getNamespaceURI( Prefix, InScope, URI ),
	createAttributeNS( Prefix, Local, URI, Value, Default, Attrib ),
	addAttribute( Default, ename(URI,Local), Attrib, Attribs, NewAttribs ).

addAttribute( no, EName, Attrib, Attribs, NewAttribs ) :-  !,
	setNamedItemIfNew( Attribs, EName, Attrib, NewAttribs ).
addAttribute( yes, EName, Attrib, Attribs, NewAttribs ) :- !,
	setNewNamedItem( Attribs, EName, Attrib, NewAttribs ).


% getNamespaceURI( Prefix, _, Prefix ) :- !.

getNamespaceURI( Prefix, InScope, URI ) :-
	getNamedItem( InScope, Prefix, URI ).
%getNamespaceURI( Prefix, _, _ ) :-
%	error( 'Namespace not declared' ).

checkEntities( Cx ) :-
	getContextEntities( Cx, Ents ),
	checkEntities( Ents, Cx ).

checkEntities( Ents, Cx ) :-
	( getFirstNamedMap( Ents, Name, Value ) -> 
	  	checkInternalEntity( Cx, Name, Value ),
		getRestNamedMap( Ents, RestEnts ), !, 
		checkEntities( RestEnts, Cx )
	;
	   	true
	).

checkInternalEntity( _Cx, _Name, Entity ) :-
	\+ isInternalEntity( Entity ), !.
checkInternalEntity( Cx, _Name, Entity ) :-
	getEntityReplacementText( Entity, [LookUp|Rest]-[0'<,0'/,-1] ),
	content( LookUp, Rest, Cx, Content, -1, [] ), !.
%checkInternalEntity( _, Name, _ ) :- !,
%	error( 'General Parsed Entity has invalid content' ).

/*
error( Msg ) :-
	write( Msg ), !, fail.
error( Look1, Rest1, Msg, _, _ ) :-
	write( Msg ), !, fail.
*/

error( _ ) :- fail.
error( _, _, _, _, _ ) :- fail.

