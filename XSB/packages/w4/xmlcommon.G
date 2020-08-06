%%% Common Syntactic Constructs

% XML Characters
% [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF] /

xml_char( 16'9 ) :- !.
xml_char( 16'A ) :- !.
xml_char( 16'D ) :- !.
xml_char( Char ) :- 
	Char =< 16'D7FF, !, Char >= 16'20.
xml_char( Char ) :- 
        Char =< 16'FFFD, !, Char >= 16'E000.
xml_char( Char ) :- 
	  Char =< 16'10FFFF, !, Char >= 16'10000.
%xml_char( Char ) :-
%	error( 'Invalid XML character' ).

/*
xml_char( 16'9 ) :- !.
xml_char( 16'A ) :- !.
xml_char( 16'D ) :- !.
xml_char( Char ) :- 
	( Char =< 16'D7FF, !, Char >= 16'20 ;
        Char =< 16'FFFD, !, Char >= 16'E000 ;
	  Char =< 16'10FFFF, !, Char >= 16'10000;
	  error( 'Invalid XML character' )
	).
*/

% Whitespace characters, including the ones in XML 1.1
% [3]    S    ::=    (#x20 | #x9 | #xD | #xA | #x85 |#x2028)+ 

whitespace -->
	+ white_char, !,
	whitespace0.

whitespace0 -->
	+ white_char, !,
	whitespace0.
whitespace0 --> [].

white_char --> [16'09].
white_char --> [16'0A].
white_char --> [16'0D].
white_char --> [16'20].
white_char --> + other_blank(_).


% [4]  NCNameStartChar ::=  (Letter | '_') 	/*  An XML Name, minus the ":" */ 


ncnamestartchar( [Byte1|Rest], Rest ) --> + ncnamestartchar1byte( Byte1 ), !.
ncnamestartchar( [Byte1,Byte2|Rest], Rest ) ::= 
	+ ncnamestartchar2byte( Code ), !,
	{
		Byte1 is 16'C0 \/ ( Code >> 6 ),
		Byte2 is 16'80 \/ ( Code /\ 16'3F)
	}.
ncnamestartchar( [Byte1,Byte2,Byte3|Rest], Rest ) ::= 
	+ ncnamestartchar3byte( Code ), !,
	{
		Byte1 is 16'E0 \/ ( Code >> 12 ),
		Byte2 is 16'80 \/ ( ( Code >> 6 ) /\ 16'3F),
		Byte3 is 16'80 \/ ( Code /\ 16'3F)
	}.
ncnamestartchar( [Byte1,Byte2,Byte3,Byte4|Rest], Rest ) ::= 
	+ ncnamestartchar4byte( Code ), !,
	{
		Byte1 is 16'F0 \/ ( Code >> 18 ),
		Byte2 is 16'80 \/ ( ( Code >> 12 ) /\ 16'3F),
		Byte3 is 16'80 \/ ( ( Code >>  6 ) /\ 16'3F),
		Byte4 is 16'80 \/ ( Code /\ 16'3F)
	}.

% [5]     NCNameChar ::=  Letter | Digit | '.' | '-' | '_' | CombiningChar | Extender 

ncnamechar( [Byte1|Rest], Rest ) --> + ncnamechar1byte( Byte1 ), !.
ncnamechar( [Byte1,Byte2|Rest], Rest ) --> 
	+ ncnamechar2byte( Code ), !,
	{     Byte1 is 16'C0 \/ ( Code >> 6 ),
		Byte2 is 16'80 \/ ( Code /\ 16'3F)
	}.
ncnamechar( [Byte1,Byte2,Byte3|Rest], Rest ) -->
	+ ncnamechar3byte( Code ), !,
	{
		Byte1 is 16'E0 \/ ( Code >> 12 ),
		Byte2 is 16'80 \/ ( ( Code >> 6 ) /\ 16'3F),
		Byte3 is 16'80 \/ ( Code /\ 16'3F)
	}.
ncnamechar( [Byte1,Byte2,Byte3,Byte4|Rest], Rest ) -->
	+ ncnamechar4byte( Code ), !,
	{
		Byte1 is 16'F0 \/ ( Code >> 18 ),
		Byte2 is 16'80 \/ ( ( Code >> 12 ) /\ 16'3F),
		Byte3 is 16'80 \/ ( ( Code >>  6 ) /\ 16'3F),
		Byte4 is 16'80 \/ ( Code /\ 16'3F)
	}.



% [4]  NCName ::=  (Letter | '_') (NCNameChar)* /*  An XML Name, minus the ":" */ 

ncname( Name ) --> 
	ncnamestartchar(NameText,RestName), 
	rest_ncname(RestName), 
	{ atom_codes( Name, NameText ) }.

rest_ncname( NameText ) -->
	ncnamechar(NameText,RestText), !,
	rest_ncname( RestText ).
rest_ncname( [] ) --> [].


/* Not Used in XML Namespaces

% [5]    Name    ::=   NameStartChar NameChar*

name( Name ) -->
	namestartchar( NameText, RestName ),
	rest_name( RestName ),
	{atom_codes( Name, NameText )}.
rest_name(  NameText ) -->
	namechar( NameText, RestText ), !,
	rest_name( RestText ).
rest_name( [] ) --> [].


%% Include ":" in namestartchar and namechar.

namestartchar( [0':'|Rest], Rest ) ::= ":", !.
namestartchar( Name, Rest ) --> + ncnamestartchar( Name, Rest ).

namechar( [0':'|Rest], Rest ) ::= ":", !.
namechar( Name, Rest ) --> + ncnamechar( Name, Rest ).

*/

% Qualified names, from Namespaces recommendation
% [6]  QName ::=  (Prefix ':')? LocalPart 
% [7]  Prefix ::=  NCName 
% [8]  LocalPart ::=  NCName 


qname( QN ) --> ncname( QN1 ), rest_qname( QN1, QN ).

rest_qname( Prefix, qname( Prefix, LocalPart ) ) --> ":", !, ncname( LocalPart ).
rest_qname( LocalPart, qname( '', LocalPart ) ) --> [].



% [7]    Nmtoken    ::=    (NameChar)+ 
% [8]    Nmtokens    ::=    Nmtoken (S Nmtoken)* 


nmtoken( Nmtoken ) -->
	ncnamechar( NameText, RestName ),
	rest_ncname( RestName ),
	{atom_codes( Nmtoken, NameText )}.

% Not used anywhere in the grammar
/*
nmtokens( [Nmtoken|RestNmtokens] ) -->
	nmtoken( Nmtoken ), rest_nmtokens( RestNmtokens ).

rest_nmtokens( [Nmtoken|RestNmtokens] ) --> 
	whitespace, nmtoken( Nmtoken ), !,
	rest_nmtokens( RestNmtokens ).
rest_nmtokens( [] ) --> [].
*/


% [9]    EntityValue    ::=    '"' ([^%&"] | PEReference | Reference)* '"'  
%                         |    "'" ([^%&'] | PEReference | Reference)* "'" 


entity_value( PEs, ReplacementText-RestText ) --> """", !,
	entity_value( 0'", PEs, ReplacementText, RestText ).
entity_value( PEs, ReplacementText-RestText ) --> "'", !,
	entity_value( 0''', PEs, ReplacementText, RestText ).

entity_value( End, _, RestText, RestText ) --> [End], !.
entity_value( End, PEs, ReplacementText, RestText ) -->
	"%", !, 
	pe_reference( PEs, ReplacementText, ValueText ),
	entity_value( End, PEs, ValueText, RestText ).
entity_value( End, PEs, ReplacementText, RestText ) -->
	"&", !,
	reference_in_entity( ReplacementText, RefText ), 
	entity_value( End, PEs, RefText, RestText ).
entity_value( End, PEs, [Char|ReplacementText], RestText ) -->
	[Char], !, 
	{ xml_char( Char ) },								% Uncomment for fully conformant XML parser
	entity_value( End, PEs, ReplacementText, RestText ).

pe_reference( PEs, ReplacementText, RestText ) -->
	ncname( Name ), ";" , !,
	{ expand_pereference( Name, PEs, ReplacementText, RestText ) }. 
% pe_reference( _, _, _ ) --> error( 'Invalid PE reference' ).

% Entities referenced in entity_values are not expanded, but should parse correctly
reference_in_entity( ReplacementText, RestText ) --> 
	"#", !, char_ref( ReplacementText, RestText ).
reference_in_entity( [0'&|NameText], RestText ) -->
	ncnamestartchar( NameText, RestName ),
	rest_entity_name( RestName, RestText ).

rest_entity_name(  NameText, RestText ) -->
	ncnamechar( NameText, AuxText ), !,
	rest_entity_name( AuxText, RestText ).
rest_entity_name( [0';|RestText], RestText ) --> ";".


% Could be made more efficient by asserting the parameter entities...
expand_pereference( Name, PEs, ReplacementText, RestText ) :-
	getNamedItem( PEs, Name, Value ), !,
	copy_term( Value, ReplacementText - RestText ).

% [11]    SystemLiteral    ::=    ('"' [^"]* '"') | ("'" [^']* "'")  

system_literal( Literal ) ::= 
	"""", !, system_literal( 0'", LiteralCodes ), 
	{ parseIRIref(yes,LiteralCodes, Literal),
	  getIRIrefFragment( Literal, [] )				% System IDs cannot have fragment parts
      }.
system_literal( Literal ) ::= 
	"'", !, system_literal( 0''', LiteralCodes ), 
	{ parseIRIref(yes,LiteralCodes, Literal),
	  getIRIrefFragment( Literal, [] )				% System IDs cannot have fragment parts
	}. 

% The string returned is terminated with -1 in order to use the IRI parser afterwards.
system_literal( 0''', [-1] ) --> [0'''], !.
system_literal( 0'", [-1] ) --> [0'"], !.
system_literal( End, [Char|RestLiteral] ) -->
	[Char], !, 
	{ xml_char( Char ) },								% Uncomment for fully conformant XML parser
	system_literal( End, RestLiteral ).
%system_literal( _, _ ) -->
%	error( 'System Literal not properly closed' ).

% [12]    PubidLiteral    ::=    '"' PubidChar* '"' | "'" (PubidChar - "'")* "'" 

pubid_literal( Literal ) ::= 
	"""", !, pubid_literal_trim( 0'", LiteralCodes ), { name(Literal,LiteralCodes) }.
pubid_literal( Literal ) ::= 
	"'", !, pubid_literal_trim(  0''', LiteralCodes ), { name(Literal,LiteralCodes) }.
%pubid_literal( _ ) -->
%	error( 'Public Identifiers should start with '' or ".' ).

pubid_literal_trim( End, [] ) --> [End], !.
pubid_literal_trim( End, Literal ) --> [16'A], !, pubid_literal_trim( End, Literal ).
pubid_literal_trim( End, Literal ) --> [16'D], !, pubid_literal_trim( End, Literal ).
pubid_literal_trim( End, Literal ) --> [16'20], !, pubid_literal_trim( End, Literal ).
pubid_literal_trim( End, [Char|RestLiteral] ) -->
	[Char], !, { pubid_char( Char ) }, 
	pubid_literal( End, RestLiteral ).

pubid_literal( End, [] ) --> [End], !.
pubid_literal( End, Literal ) --> [16'A], !, pubid_literal_white( End, Literal ).
pubid_literal( End, Literal ) --> [16'D], !, pubid_literal_white( End, Literal ).
pubid_literal( End, Literal ) --> [16'20], !, pubid_literal_white( End, Literal ).
pubid_literal( End, [Char|RestLiteral] ) -->
	[Char], !, { pubid_char( Char ) }, 
	pubid_literal( End, RestLiteral ).

pubid_literal_white( End, [] ) --> [End], !.
pubid_literal_white( End, Literal ) --> [16'A], !, pubid_literal_white( End, Literal ).
pubid_literal_white( End, Literal ) --> [16'D], !, pubid_literal_white( End, Literal ).
pubid_literal_white( End, Literal ) --> [16'20], !, pubid_literal_white( End, Literal ).
pubid_literal_white( End, [16'20,Char|RestLiteral] ) -->
	[Char], !, { pubid_char( Char ) }, 
	pubid_literal( End, RestLiteral ).

 
% [13]    PubidChar    ::=    #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%] 

pubid_char( 16'20 ) :- !.
pubid_char( 16'D ) :- !.
pubid_char( 16'A ) :- !.
pubid_char( 0'- ) :- !.
pubid_char( 0'' ) :- !.
pubid_char( 0'( ) :- !.
pubid_char( 0') ) :- !.
pubid_char( 0'+ ) :- !.
pubid_char( 0', ) :- !.
pubid_char( 0'. ) :- !.
pubid_char( 0'/ ) :- !.
pubid_char( 0': ) :- !.
pubid_char( 0'= ) :- !.
pubid_char( 0'? ) :- !.
pubid_char( 0'; ) :- !.
pubid_char( 0'! ) :- !.
pubid_char( 0'* ) :- !.
pubid_char( 0'# ) :- !.
pubid_char( 0'@ ) :- !.
pubid_char( 0'$ ) :- !.
pubid_char( 0'_ ) :- !.
pubid_char( 0'% ) :- !.
pubid_char( Char ) :-
	Char >= 0'a, !, Char =< 0'z.
pubid_char( Char ) :-
	Char >= 0'A, !, Char =< 0'Z.
pubid_char( Char ) :-
	Char >= 0'0, !, Char =< 0'9.
%pubid_char( _ ) :- 
%	error( 'Invalid character in Public Identifier' ).

