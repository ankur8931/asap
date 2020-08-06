%% IRI References (version 1.0 / October 2002)
%%
%% This module implements the parsing of Internationalized Resource Identifies draft,
%% and RFC 2396 and RFC 2732.
%%
%% (c) 2002 Carlos Viegas Damásio (cd@di.fct.unl.pt)
%%
%% Centro de Inteligência Artificial da Universade Nova de Lisboa
%% Quinta da Torre
%% 2829-516 Caparica
%% Portugal
%%
%% If UserInfo is absent, then the parser does not check if IP addresses 
%% and hostnames are according to the grammar.
%%
%% IPv6 addresses are never checked.
%%

/* Grammar implemented (with a few changes) extracted from the IRI draft

    IRI-reference  = IRI / relative-IRI

    IRI            = scheme ":" ihier-part [ "?" iquery ] [ "#" ifragment ]

    absolute-IRI   = scheme ":" ihier-part [ "?" iquery ]

    relative-IRI   = ihier-part [ "?" iquery ] [ "#" ifragment ]
    
    ihier-part     = inet-path / iabs-path / irel-path

    inet-path      = "//" iauthority [ iabs-path ]

    iabs-path      = "/"  ipath-segments

    irel-path      = ipath-segments
    
    iauthority     = [ iuserinfo "@" ] ihost [ ":" port ]
    
    iuserinfo      = *( iunreserved / escaped / ";" /
                       ":" / "&" / "=" / "+" / "$" / "," )
                              
    ihost          = [ IPv6reference / IPv4address / ihostname ]
    
    ihostname      = idomainlabel iqualified

    iqualified     = *( "." idomainlabel ) [ "." ]

    idomainlabel   = <<See following production rules>>

    ipath-segments = isegment *( "/" isegment )

    isegment       = *ipchar
    
    ipchar         = iunreserved / escaped / ";" /
                     ":" / "@" / "&" / "=" / "+" / "$" / ","

    iquery         = *( ipchar / iprivate / "/" / "?" )

    ifragment      = *( ipchar / "/" / "?" )
    
    iric           = reserved / iunreserved / escaped

    iunreserved    = unreserved / ucschar

    ucschar        = %xA0-D7FF / %xF900-FDCF / %xFDF0-FFEF /
                   / %x10000-1FFFD / %x20000-2FFFD / %x30000-3FFFD
                   / %x40000-4FFFD / %x50000-5FFFD / %x60000-6FFFD
                   / %x70000-7FFFD / %x80000-8FFFD / %x90000-9FFFD
                   / %xA0000-AFFFD / %xB0000-BFFFD / %xC0000-CFFFD
                   / %xD0000-DFFFD / %xE1000-EFFFD

    iprivate       = %xE000-F8FF / %xF0000-FFFFD / %x100000-10FFFD                      

The 'idomainlabel' production rule is as follows: 
The value 'idomainlabel' is defined as a string of 'ucschar' obeying the following rules: 


a) 
Given a string of 'ucschar' values, the ToASCII operation [RFC3490] is performed on that string with the flag 
UseSTD3ASCIIRules set to TRUE and the flag AllowUnassigned set to FALSE for creating IRIs and set to TRUE otherwise. 
b) ToASCII is successful and results in a string conforming to 'domainlabel' (see below). 

The following are the same as [RFCYYYY]: 

    scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

    port          = *DIGIT

    domainlabel   = alphanum [ 0*61( alphanum | "-" ) alphanum ]

    alphanum      = ALPHA / DIGIT
    
    IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet

    dec-octet     = DIGIT                           ; 0-9
                  / ( %x31-39 DIGIT )               ; 10-99
                  / ( "1" 2DIGIT )                  ; 100-199
                  / ( "2" %x30-34 DIGIT )           ; 200-249
                  / ( "25" %x30-35 )                ; 250-255

    IPv6reference = "[" IPv6address "]"

    IPv6address   =                          6( h4 ":" ) ls32
                  /                     "::" 5( h4 ":" ) ls32
                  / [              h4 ] "::" 4( h4 ":" ) ls32
                  / [ *1( h4 ":" ) h4 ] "::" 3( h4 ":" ) ls32
                  / [ *2( h4 ":" ) h4 ] "::" 2( h4 ":" ) ls32
                  / [ *3( h4 ":" ) h4 ] "::"    h4 ":"   ls32
                  / [ *4( h4 ":" ) h4 ] "::"             ls32
                  / [ *5( h4 ":" ) h4 ] "::"             h4
                  / [ *6( h4 ":" ) h4 ] "::"

    h4            = 1*4HEXDIG

    ls32          = ( h4 ":" h4 ) / IPv4address

    reserved      = "/" / "?" / "#" / "[" / "]" / ";" /
                    ":" / "@" / "&" / "=" / "+" / "$" / ","

    unreserved    = ALPHA / DIGIT / mark

    mark          = "-" / "_" / "." / "!" / "~" / "*" / "'" /
                    "(" / ")"
                    
    escaped       = "%" HEXDIG HEXDIG

*/

:- export parse_iri_reference/5.

:- start( parse_iri_reference/1 ).
:- end( -1 ).

% Inline expanded productions:

:- - alphanum/1.
:- - upalpha/1.
:- - lowalpha/1.
:- - reserved/1.
:- - mark/1.
:- - ucschar/1.
:- - unreserved/1.
:- - ucsqchar/1.
:- - ihostname/1.
:- - ipv4address/1.
:- - ipv6reference/1.
:- - inet_path/2.
:- - iabs_path/1.



%    IRI-reference  = IRI / relative-IRI

parse_iri_reference( iriref( Scheme, Authority, Path, Query, Fragment ) ) -->
	absreliri0( Scheme, Authority, Path ), iquery0( Query ), ifragment0( Fragment ).


%    IRI            = scheme ":" ihier-part [ "?" iquery ] [ "#" ifragment ]
%    relative-IRI   = ihier-part [ "?" iquery ] [ "#" ifragment ]
  
absreliri0( [], [], [] ) ::= [], !.
absreliri0( [],  A,  P ) ::= !,  ihier_part( A, P ).
absreliri0(  S,  A,  P ) --> !, sch_rel( S, A, P ).


ifragment0( fragment(F) ) ::= "#", !, ifragment( F ).
ifragment0( [] )          ::= [], !.


ihier_part(  A, P ) --> + inet_path( A, P ), !.
ihier_part( [], P ) --> + iabs_path( P ), !.

iquery0( query(Q) ) --> "?", !, iquery( Q ).
iquery0( [] )    ::= [], !.


% Parses an absolute or rel_path IRI
sch_rel( S,   A, P ) --> alpha( Char ), !, rest_sch_rel( [Char|RestChars], RestChars, S, A, P ).
sch_rel( [], [], P ) --> ipchar( Chars, RestChars ), !, rest_irelpath( Chars, RestChars, P ).

rest_sch_rel(  S, [], scheme( S ), A, P )  --> ":", !, rest_absoluteIRI( A, P ).
rest_sch_rel( SChars, AuxChars, S, A, P ) --> alpha( Char ), !, { AuxChars = [Char|RestChars] }, rest_sch_rel( SChars, RestChars, S, A, P ).
rest_sch_rel( SChars, AuxChars, S, A, P ) --> digit( Char ), !, { AuxChars = [Char|RestChars] }, rest_sch_rel( SChars, RestChars, S, A, P ).
rest_sch_rel( SChars, AuxChars, S, A, P ) --> "+", !, { AuxChars = [ 0'+|RestChars]}, rest_sch_rel( SChars, RestChars, S, A, P ).
rest_sch_rel( SChars, AuxChars, S, A, P ) --> "-", !, { AuxChars = [ 0'-|RestChars]}, rest_sch_rel( SChars, RestChars, S, A, P ).
rest_sch_rel( SChars, AuxChars, S, A, P ) --> ".", !, { AuxChars = [ 0'.|RestChars]}, rest_sch_rel( SChars, RestChars, S, A, P ).
rest_sch_rel( SChars, AuxChars, [], [], P ) --> rest_irelpath( SChars, AuxChars, P ).

rest_irelpath( Seg1, RestSeg1, path( rel, [segment(Seg1)|RestSegs] ) )  --> isegment( RestSeg1 ), abs_path0( RestSegs ). 

abs_path0(  P ) ::= "/", !, ipath_segments( P ).
abs_path0(  [] ) ::= [], !.

rest_absoluteIRI( A, P ) ::= !, ihier_part( A, P ).
rest_absoluteIRI( [], path(rel,P) ) --> !, ipath_segments( P ).


%    inet-path      = "//" iauthority [ iabs-path ]
%    iabs-path      = "/"  ipath-segments

inet_path( A, P ) --> "//", !, iauthority0( A ), rest_inet_path(P).
iabs_path( path(abs,Segments ) ) --> "/", !, ipath_segments( Segments ).

rest_inet_path( path(abs,P) ) ::= "/", !, ipath_segments( P ).
rest_inet_path( [] ) ::= [], !.


% Unfortunately, it is not possible to construct a LL(1) parser for the authority part since '.'s are
% allowed in the userinfo and host parts.

iauthority0( authority( [], [], [] ) ) ::= [], !.
iauthority0( authority( UserInfo, Host, Port ) ) --> iauthority( Aut, Aut, UserInfo, Host, Port ).

iauthority( U, RestU, UserInfo, Host, Port) --> ? "%", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ? ";", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ? "&", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ? "=", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ? "+", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ? "$", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ? ",", !, iauthority_with_iuserinfo( U, RestU, UserInfo, Host, Port).
iauthority( U, RestU, UserInfo, Host, Port) --> ":", !, iuserinfo_or_port( U, RestU, D, D, UserInfo, Host, Port ).
iauthority( UserInfo, [], UserInfo, Host, Port) --> "@", !, ihost0( Host ), port0( Port ).
iauthority( U, [Char|RestU], UserInfo, Host, Port) --> iunreserved( Char ), !, iauthority( U, RestU, UserInfo, Host, Port).
iauthority( U, [], [], U, [] ) ::= [], !, {nonvar(U)}.		% U is a hostname!


iuserinfo_or_port( Host, [], Port, [], [], Host, Port ) ::= [], !.
iuserinfo_or_port( U, RestU, D, [Dig|RestD], UserInfo, Host, Port ) --> digit( Dig ), !, iuserinfo_or_port( U, RestU, D, RestD, UserInfo, Host, Port ).
iuserinfo_or_port( U, [0':|D], D, [], U, Host, Port ) --> "@", !, ihost0( Host ), port0( Port ).
iuserinfo_or_port( U, [0':|D], D, RestU, UserInfo, Host, Port ) --> iauthority_with_iuserinfo(U, RestU, UserInfo, Host, Port ). 

iauthority_with_iuserinfo( UserInfo, RestUserInfo, UserInfo, Host, Port ) --> iuserinfo( RestUserInfo ), "@", ihost0( Host ), port0( Port ).



%    iuserinfo      = *( iunreserved / escaped / ";" /
%                       ":" / "&" / "=" / "+" / "$" / "," )

iuserinfo( [0';|RestChars] ) --> ";", !, iuserinfo( RestChars ).
iuserinfo( [0':|RestChars] ) --> ":", !, iuserinfo( RestChars ).
iuserinfo( [0'&|RestChars] ) --> "&", !, iuserinfo( RestChars ).
iuserinfo( [0'=|RestChars] ) --> "=", !, iuserinfo( RestChars ).
iuserinfo( [0'+|RestChars] ) --> "+", !, iuserinfo( RestChars ).
iuserinfo( [0'$|RestChars] ) --> "$", !, iuserinfo( RestChars ).
iuserinfo( [0',|RestChars] ) --> ",", !, iuserinfo( RestChars ).
iuserinfo( Chars ) ::= escaped( Chars, RestChars ), !, iuserinfo( RestChars ).
iuserinfo( [Char|RestChars] ) --> iunreserved( Char ), !, iuserinfo( RestChars ).
iuserinfo( [] ) ::= [], !.


%    port          = *DIGIT
%

port0( [] ) ::= [], !.
port0( Port ) ::= ":", !, digits0( Port ).

digits0( [D|RestD] ) ::= + digit(D), !, digits0( RestD ).
digits0( [] ) ::= [], !.



ihost0( IPV6 )  --> + ipv6reference( IPV6 ), !.
%ihost0( IPV4 )  ::= + ipv4address( IPV4 ), !.
ihost0( Host )  --> + ihostname( Host ), !.
ihost0( [] ) ::= [], !.

ipv6reference( [0'[|IPV6] ) ::= "[", !, ipv6address( IPV6 ).

%    IPv6address   =                          6( h4 ":" ) ls32
%                  /                     "::" 5( h4 ":" ) ls32
%                  / [              h4 ] "::" 4( h4 ":" ) ls32
%                  / [ *1( h4 ":" ) h4 ] "::" 3( h4 ":" ) ls32
%                  / [ *2( h4 ":" ) h4 ] "::" 2( h4 ":" ) ls32
%                  / [ *3( h4 ":" ) h4 ] "::"    h4 ":"   ls32
%                  / [ *4( h4 ":" ) h4 ] "::"             ls32
%                  / [ *5( h4 ":" ) h4 ] "::"             h4
%                  / [ *6( h4 ":" ) h4 ] "::"

%    h4            = 1*4HEXDIG
%    ls32          = ( h4 ":" h4 ) / IPv4address

ipv6address( [V6|RestV6] ) --> v6char( V6 ), restipv6( RestV6 ). 

restipv6( [V6|RestV6] ) --> + v6char( V6 ), !, restipv6( RestV6 ).
restipv6( [0']] ) ::= "]", !.

v6char( 0': ) --> ":", !.
v6char( 0'. ) --> ".", !.
v6char( H ) --> + hexdig( H ), !.
 

%    IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
%    dec-octet     = DIGIT                           ; 0-9
%                  / ( %x31-39 DIGIT )               ; 10-99
%                  / ( "1" 2DIGIT )                  ; 100-199
%                  / ( "2" %x30-34 DIGIT )           ; 200-249
%                  / ( "25" %x30-35 )                ; 250-255

ipv4address( D1 ) --> dec_octet(D1,[0'.|D2]), ".", dec_octet(D2,[0'.|D3]), ".", dec_octet(D3,[0'.|D4]), ".", dec_octet(D4,[]).

dec_octet([0'0|R], R) ::= "0", !.
dec_octet([0'3|D], R) ::= "3", !, dec_octet0(D,R).
dec_octet([0'4|D], R) ::= "4", !, dec_octet0(D,R).
dec_octet([0'5|D], R) ::= "5", !, dec_octet0(D,R).
dec_octet([0'6|D], R) ::= "6", !, dec_octet0(D,R).
dec_octet([0'7|D], R) ::= "7", !, dec_octet0(D,R).
dec_octet([0'8|D], R) ::= "8", !, dec_octet0(D,R).
dec_octet([0'9|D], R) ::= "9", !, dec_octet0(D,R).

dec_octet([0'1|D], R) ::= "1", !, dec_octet1(D, R).
dec_octet([0'2|D], R) ::= "2", !, dec_octet2(D, R).

dec_octet0([D|R], R) --> + digit(D), !. 
dec_octet0(R,R) ::= [], !. 

dec_octet1( [D1|D2], R ) --> + digit( D1 ), !, dec_octet0(D2, R).
dec_octet1( R, R ) ::= [], !.

dec_octet2( [0'0|D], R ) --> "0", !, dec_octet0(D, R).
dec_octet2( [0'1|D], R ) --> "1", !, dec_octet0(D, R).
dec_octet2( [0'2|D], R ) --> "2", !, dec_octet0(D, R).
dec_octet2( [0'3|D], R ) --> "3", !, dec_octet0(D, R).
dec_octet2( [0'4|D], R ) --> "4", !, dec_octet0(D, R).
dec_octet2( [0'5|D], R ) --> "5", !, dec_octet25(D, R).
dec_octet2( [0'6|R], R ) --> "6", !.
dec_octet2( [0'7|R], R ) --> "7", !.
dec_octet2( [0'8|R], R ) --> "8", !.
dec_octet2( [0'9|R], R ) --> "9", !.
dec_octet2( R, R ) ::= [], !.


dec_octet25( [0'0|R], R ) --> "0", !.
dec_octet25( [0'1|R], R ) --> "1", !.
dec_octet25( [0'2|R], R ) --> "2", !.
dec_octet25( [0'3|R], R ) --> "3", !.
dec_octet25( [0'4|R], R ) --> "4", !.
dec_octet25( [0'5|R], R ) --> "5", !.
dec_octet25( R, R ) ::= [], !.

%    ihostname      = idomainlabel iqualified

%    iqualified     = *( "." idomainlabel ) [ "." ]


ihostname( HostName ) --> idomainlabel( HostName, Qualified ), iqualified( Qualified ).


iqualified( [] ) ::= [], !.
iqualified( [0'.|D] ) --> ".", !, restiqualified( D ).

restiqualified( Domain ) --> idomainlabel( Domain, RestQualified ), !, iqualified( RestQualified ).
restiqualified( [] ) ::= [], !.

idomainlabel( [Char|Domain], RestDomain ) --> iunreserved( Char ), !, restidomainlabel( Domain, RestDomain ).

restidomainlabel( [Char|Domain], RestDomain ) --> iunreserved( Char ), !, restidomainlabel( Domain, RestDomain ).
restidomainlabel( Domain, Domain ) ::= [], !.



%    ipath-segments = isegment *( "/" isegment )

ipath_segments( [segment(Seg)|RestSegs] ) -->
	isegment( Seg ), rest_isegments( RestSegs ).

rest_isegments( [] )   ::= [], !.
rest_isegments( [segment(Seg)|RestSegs] ) --> "/", !, isegment( Seg ), rest_isegments( RestSegs ).

%    isegment       = *ipchar

isegment( [] ) ::= [], !.
isegment( SegChars ) --> ipchar( SegChars, RestChars ), !, isegment( RestChars ).

%     ipchar         = iunreserved / escaped / ";" /
%                     ":" / "@" / "&" / "=" / "+" / "$" / ","


ipchar( [0';|RestChars], RestChars ) --> ";", !.
ipchar( [0':|RestChars], RestChars ) --> ":", !.
ipchar( [0'@|RestChars], RestChars ) --> "@", !.
ipchar( [0'&|RestChars], RestChars ) --> "&", !.
ipchar( [0'=|RestChars], RestChars ) --> "=", !.
ipchar( [0'+|RestChars], RestChars ) --> "+", !.
ipchar( [0'$|RestChars], RestChars ) --> "$", !.
ipchar( [0',|RestChars], RestChars ) --> ",", !.
ipchar( Chars, RestChars ) --> + escaped( Chars, RestChars ).
ipchar( [Char|RestChars], RestChars ) --> + iunreserved( Char ).

%   iunreserved    = unreserved / ucschar

iunreserved( Char ) --> + unreserved( Char ).
iunreserved( Char ) --> + ucschar( Char ).

%    ucschar        = %xA0-D7FF / %xF900-FDCF / %xFDF0-FFEF /
%                   / %x10000-1FFFD / %x20000-2FFFD / %x30000-3FFFD
%                   / %x40000-4FFFD / %x50000-5FFFD / %x60000-6FFFD
%                   / %x70000-7FFFD / %x80000-8FFFD / %x90000-9FFFD
%                   / %xA0000-AFFFD / %xB0000-BFFFD / %xC0000-CFFFD
%                   / %xD0000-DFFFD / %xE1000-EFFFD

ucschar( Char ) --> [Char], { Char =< 16'D7FF, !, Char >= 16'A0 }.
ucschar( Char ) --> [Char], { Char =< 16'FDCF, !, Char >= 16'F900 }.
ucschar( Char ) --> [Char], { Char =< 16'FFEF, !, Char >= 16'FDF0 }.
ucschar( Char ) --> [Char], { Char =< 16'EFFFD, !, Char >= 16'10000, Char /\ 16'FFFF =< 16'FFFD }.


%    iquery         = *( ipchar / iprivate / "/" / "?" )
%    iprivate       = %xE000-F8FF / %xF0000-FFFFD / %x100000-10FFFD                      

iquery( Query  ) --> iqchar( Query, RestQChars ), !, iquery( RestQChars ).
iquery( [] )     ::= [], !.

iqchar( [0';|RestChars], RestChars ) --> ";", !.
iqchar( [0':|RestChars], RestChars ) --> ":", !.
iqchar( [0'@|RestChars], RestChars ) --> "@", !.
iqchar( [0'&|RestChars], RestChars ) --> "&", !.
iqchar( [0'=|RestChars], RestChars ) --> "=", !.
iqchar( [0'+|RestChars], RestChars ) --> "+", !.
iqchar( [0'$|RestChars], RestChars ) --> "$", !.
iqchar( [0',|RestChars], RestChars ) --> ",", !.
iqchar( [0'/|RestChars], RestChars ) --> "/", !.
iqchar( [0'?|RestChars], RestChars ) --> "?", !.
iqchar( [Char|RestChars], RestChars ) --> + unreserved( Char ).
iqchar( [Char|RestChars], RestChars ) --> + ucsqchar( Char ).
iqchar( Chars, RestChars ) --> + escaped( Chars, RestChars ).

ucsqchar( Char ) --> [Char], { Char =< 16'FDCF, !, Char >= 16'A0 }.
ucsqchar( Char ) --> [Char], { Char =< 16'FFEF, !, Char >= 16'FDF0 }.
ucsqchar( Char ) --> [Char], { Char =< 16'10FFFD, !, Char >= 16'10000, Char /\ 16'FFFF =< 16'FFFD }.


% ifragment      = *( ipchar / "/" / "?" )

ifragment( Frag ) --> ifchar( Frag, RestChars ), !, ifragment( RestChars ).
ifragment( [] ) ::= [].

ifchar( [0'/|RestChars], RestChars ) --> "/", !.
ifchar( [0'?|RestChars], RestChars ) --> "?", !.
ifchar( Chars, RestChars ) --> + ipchar( Chars, RestChars ).


%    reserved      = "/" / "?" / "#" / "[" / "]" / ";" /
%                    ":" / "@" / "&" / "=" / "+" / "$" / ","

reserved( 0'/ ) --> "/", !.
reserved( 0'? ) --> "?", !.
reserved( 0'# ) --> "#", !.
reserved( 0'[ ) --> "[", !.			% added by RFC 2732
reserved( 0'] ) --> "]", !.			% added by RFC 2732
reserved( 0'; ) --> ";", !.
reserved( 0': ) --> ":", !.
reserved( 0'@ ) --> "@", !.
reserved( 0'& ) --> "&", !.
reserved( 0'= ) --> "=", !.
reserved( 0'+ ) --> "+", !.
reserved( 0'$ ) --> "$", !.
reserved( 0', ) --> ",", !.


%     unreserved    = ALPHA / DIGIT / mark

unreserved( Char ) --> + alphanum( Char ).
unreserved( Char ) --> + mark( Char ).

%    mark          = "-" / "_" / "." / "!" / "~" / "*" / "'" /
%                    "(" / ")"

mark( 0'- ) --> "-", !.
mark( 0'_ ) --> "_", !.
mark( 0'. ) --> ".", !.
mark( 0'! ) --> "!", !.
mark( 0'~ ) --> "~", !.
mark( 0'* ) --> "*", !.
mark( 0'' ) --> "'", !.
mark( 0'( ) --> "(", !.
mark( 0') ) --> ")", !.

%    escaped       = "%" HEXDIG HEXDIG

escaped( [0'%,H1,H2|RestChars], RestChars ) --> "%", !, hexdig( H1 ), hexdig( H2 ).

hexdig( Char ) --> + digit( Char ).
hexdig( 0'A ) --> "A", !.
hexdig( 0'B ) --> "B", !.
hexdig( 0'C ) --> "C", !.
hexdig( 0'D ) --> "D", !.
hexdig( 0'E ) --> "E", !.
hexdig( 0'F ) --> "F", !.
hexdig( 0'a ) --> "a", !.
hexdig( 0'b ) --> "b", !.
hexdig( 0'c ) --> "c", !.
hexdig( 0'd ) --> "d", !.
hexdig( 0'e ) --> "e", !.
hexdig( 0'f ) --> "f", !.


alphanum( Char ) --> + alpha( Char ).
alphanum( Char ) --> + digit( Char ).
alpha( Char ) --> + lowalpha( Char ).
alpha( Char ) --> + upalpha( Char ).

lowalpha( 0'a ) --> "a", !.
lowalpha( 0'b ) --> "b", !.
lowalpha( 0'c ) --> "c", !.
lowalpha( 0'd ) --> "d", !.
lowalpha( 0'e ) --> "e", !.
lowalpha( 0'f ) --> "f", !.
lowalpha( 0'g ) --> "g", !.
lowalpha( 0'h ) --> "h", !.
lowalpha( 0'i ) --> "i", !.
lowalpha( 0'j ) --> "j", !.
lowalpha( 0'k ) --> "k", !.
lowalpha( 0'l ) --> "l", !.
lowalpha( 0'm ) --> "m", !.
lowalpha( 0'n ) --> "n", !.
lowalpha( 0'o ) --> "o", !.
lowalpha( 0'p ) --> "p", !.
lowalpha( 0'q ) --> "q", !.
lowalpha( 0'r ) --> "r", !.
lowalpha( 0's ) --> "s", !.
lowalpha( 0't ) --> "t", !.
lowalpha( 0'u ) --> "u", !.
lowalpha( 0'v ) --> "v", !.
lowalpha( 0'w ) --> "w", !.
lowalpha( 0'x ) --> "x", !.
lowalpha( 0'y ) --> "y", !.
lowalpha( 0'z ) --> "z", !.

upalpha( 0'A ) --> "A", !.
upalpha( 0'B ) --> "B", !.
upalpha( 0'C ) --> "C", !.
upalpha( 0'D ) --> "D", !.
upalpha( 0'E ) --> "E", !.
upalpha( 0'F ) --> "F", !.
upalpha( 0'G ) --> "G", !.
upalpha( 0'H ) --> "H", !.
upalpha( 0'I ) --> "I", !.
upalpha( 0'J ) --> "J", !.
upalpha( 0'K ) --> "K", !.
upalpha( 0'L ) --> "L", !.
upalpha( 0'M ) --> "M", !.
upalpha( 0'N ) --> "N", !.
upalpha( 0'O ) --> "O", !.
upalpha( 0'P ) --> "P", !.
upalpha( 0'Q ) --> "Q", !.
upalpha( 0'R ) --> "R", !.
upalpha( 0'S ) --> "S", !.
upalpha( 0'T ) --> "T", !.
upalpha( 0'U ) --> "U", !.
upalpha( 0'V ) --> "V", !.
upalpha( 0'W ) --> "W", !.
upalpha( 0'X ) --> "X", !.
upalpha( 0'Y ) --> "Y", !.
upalpha( 0'Z ) --> "Z", !.

digit( 0'0  ) --> "0", !.
digit( 0'1 ) --> "1", !.
digit( 0'2 ) --> "2", !.
digit( 0'3 ) --> "3", !.
digit( 0'4 ) --> "4", !.
digit( 0'5 ) --> "5", !.
digit( 0'6 ) --> "6", !.
digit( 0'7 ) --> "7", !.
digit( 0'8 ) --> "8", !.
digit( 0'9 ) --> "9", !.

