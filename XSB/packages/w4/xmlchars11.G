% The character ranges for XML Name productions

:- - other_blank/1.

:- - ncnamechar1byte/1.
:- - ncnamechar2byte/1.
:- - ncnamechar3byte/1.
:- - ncnamechar4byte/1.

:- - ncnamestartchar1byte/1.
:- - ncnamestartchar2byte/1.
:- - ncnamestartchar3byte/1.
:- - ncnamestartchar4byte/1.


other_blank( 16'85 ) --> [16'85].				% Extra white space char allowed in XML 1.1
other_blank( 16'2028 ) --> [16'2028].			% Extra white space char allowed in XML 1.1


% [4]    NCNameStartChar    :=   [A-Z] | "_" | [a-z] | [#xC0-#x02FF] |
%        [#x0370-#x037D] | [#x037F-#x2027] | [#x202A-#x218F] | [#x2800-#xD7FF] |
%        [#xE000-#xFDCF] | [#xFDF0-#xFFEF] | [#x10000-#x10FFFF]

ncnamestartchar1byte( 0'A ) --> [0'A], !.
ncnamestartchar1byte( 0'B ) --> [0'B], !.
ncnamestartchar1byte( 0'C ) --> [0'C], !.
ncnamestartchar1byte( 0'D ) --> [0'D], !.
ncnamestartchar1byte( 0'E ) --> [0'E], !.
ncnamestartchar1byte( 0'F ) --> [0'F], !.
ncnamestartchar1byte( 0'G ) --> [0'G], !.
ncnamestartchar1byte( 0'H ) --> [0'H], !.
ncnamestartchar1byte( 0'I ) --> [0'I], !.
ncnamestartchar1byte( 0'J ) --> [0'J], !.
ncnamestartchar1byte( 0'K ) --> [0'K], !.
ncnamestartchar1byte( 0'L ) --> [0'L], !.
ncnamestartchar1byte( 0'M ) --> [0'M], !.
ncnamestartchar1byte( 0'N ) --> [0'N], !.
ncnamestartchar1byte( 0'O ) --> [0'O], !.
ncnamestartchar1byte( 0'P ) --> [0'P], !.
ncnamestartchar1byte( 0'Q ) --> [0'Q], !.
ncnamestartchar1byte( 0'R ) --> [0'R], !.
ncnamestartchar1byte( 0'S ) --> [0'S], !.
ncnamestartchar1byte( 0'T ) --> [0'T], !.
ncnamestartchar1byte( 0'U ) --> [0'U], !.
ncnamestartchar1byte( 0'V ) --> [0'V], !.
ncnamestartchar1byte( 0'W ) --> [0'W], !.
ncnamestartchar1byte( 0'X ) --> [0'X], !.
ncnamestartchar1byte( 0'Y ) --> [0'Y], !.
ncnamestartchar1byte( 0'Z ) --> [0'Z], !.
ncnamestartchar1byte( 0'_ ) --> [0'_], !.
ncnamestartchar1byte( 0'a ) --> [0'a], !.
ncnamestartchar1byte( 0'b ) --> [0'b], !.
ncnamestartchar1byte( 0'c ) --> [0'c], !.
ncnamestartchar1byte( 0'd ) --> [0'd], !.
ncnamestartchar1byte( 0'e ) --> [0'e], !.
ncnamestartchar1byte( 0'f ) --> [0'f], !.
ncnamestartchar1byte( 0'g ) --> [0'g], !.
ncnamestartchar1byte( 0'h ) --> [0'h], !.
ncnamestartchar1byte( 0'i ) --> [0'i], !.
ncnamestartchar1byte( 0'j ) --> [0'j], !.
ncnamestartchar1byte( 0'k ) --> [0'k], !.
ncnamestartchar1byte( 0'l ) --> [0'l], !.
ncnamestartchar1byte( 0'm ) --> [0'm], !.
ncnamestartchar1byte( 0'n ) --> [0'n], !.
ncnamestartchar1byte( 0'o ) --> [0'o], !.
ncnamestartchar1byte( 0'p ) --> [0'p], !.
ncnamestartchar1byte( 0'q ) --> [0'q], !.
ncnamestartchar1byte( 0'r ) --> [0'r], !.
ncnamestartchar1byte( 0's ) --> [0's], !.
ncnamestartchar1byte( 0't ) --> [0't], !.
ncnamestartchar1byte( 0'u ) --> [0'u], !.
ncnamestartchar1byte( 0'v ) --> [0'v], !.
ncnamestartchar1byte( 0'w ) --> [0'w], !.
ncnamestartchar1byte( 0'x ) --> [0'x], !.
ncnamestartchar1byte( 0'y ) --> [0'y], !.
ncnamestartchar1byte( 0'z ) --> [0'z], !.


ncnamestartchar2byte( Code ) --> [Code], { Code =< 16'07FF, !, ns2byte( Code ) }.

ns2byte( Code ) :- Code =< 16'02FF, !, Code >= 16'C0.
ns2byte( Code ) :- Code =< 16'037D, !, Code >= 16'0370.
ns2byte( Code ) :- Code >= 16'037F, !.

ncnamestartchar3byte( Code ) --> [Code], { Code < 16'FFEF, !, ns3byte( Code ) }.

ns3byte( Code ) :- Code =< 16'D7FF, !, nsbyte31( Code ).
ns3byte( Code ) :- Code =< 16'FDCF, !, Code >= 16'E000.
ns3byte( Code ) :- Code >= 16'FDF0, !.

nsbyte31( Code ) :- Code =< 16'2027, !, Code >= 16'0800.
nsbyte31( Code ) :- Code =< 16'218F, !, Code >= 16'202A.
nsbyte31( Code ) :- Code >= 16'2800, !.


ncnamestartchar4byte( Code ) --> [Code], { Code < 16'10FFFF, !, Code >= 16'10000 }.


% [4a]    NCNameChar := NCNameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F]

ncnamechar1byte( 0'_ ) --> [0'_], !.
ncnamechar1byte( 0'. ) --> [0'.], !.
ncnamechar1byte( 0'- ) --> [0'-], !.
ncnamechar1byte( 0'0 ) --> [0'0], !.
ncnamechar1byte( 0'1 ) --> [0'1], !.
ncnamechar1byte( 0'2 ) --> [0'2], !.
ncnamechar1byte( 0'3 ) --> [0'3], !.
ncnamechar1byte( 0'4 ) --> [0'4], !.
ncnamechar1byte( 0'5 ) --> [0'5], !.
ncnamechar1byte( 0'6 ) --> [0'6], !.
ncnamechar1byte( 0'7 ) --> [0'7], !.
ncnamechar1byte( 0'8 ) --> [0'8], !.
ncnamechar1byte( 0'9 ) --> [0'9], !.
ncnamechar1byte( 0'A ) --> [0'A], !.
ncnamechar1byte( 0'B ) --> [0'B], !.
ncnamechar1byte( 0'C ) --> [0'C], !.
ncnamechar1byte( 0'D ) --> [0'D], !.
ncnamechar1byte( 0'E ) --> [0'E], !.
ncnamechar1byte( 0'F ) --> [0'F], !.
ncnamechar1byte( 0'G ) --> [0'G], !.
ncnamechar1byte( 0'H ) --> [0'H], !.
ncnamechar1byte( 0'I ) --> [0'I], !.
ncnamechar1byte( 0'J ) --> [0'J], !.
ncnamechar1byte( 0'K ) --> [0'K], !.
ncnamechar1byte( 0'L ) --> [0'L], !.
ncnamechar1byte( 0'M ) --> [0'M], !.
ncnamechar1byte( 0'N ) --> [0'N], !.
ncnamechar1byte( 0'O ) --> [0'O], !.
ncnamechar1byte( 0'P ) --> [0'P], !.
ncnamechar1byte( 0'Q ) --> [0'Q], !.
ncnamechar1byte( 0'R ) --> [0'R], !.
ncnamechar1byte( 0'S ) --> [0'S], !.
ncnamechar1byte( 0'T ) --> [0'T], !.
ncnamechar1byte( 0'U ) --> [0'U], !.
ncnamechar1byte( 0'V ) --> [0'V], !.
ncnamechar1byte( 0'W ) --> [0'W], !.
ncnamechar1byte( 0'X ) --> [0'X], !.
ncnamechar1byte( 0'Y ) --> [0'Y], !.
ncnamechar1byte( 0'Z ) --> [0'Z], !.
ncnamechar1byte( 0'a ) --> [0'a], !.
ncnamechar1byte( 0'b ) --> [0'b], !.
ncnamechar1byte( 0'c ) --> [0'c], !.
ncnamechar1byte( 0'd ) --> [0'd], !.
ncnamechar1byte( 0'e ) --> [0'e], !.
ncnamechar1byte( 0'f ) --> [0'f], !.
ncnamechar1byte( 0'g ) --> [0'g], !.
ncnamechar1byte( 0'h ) --> [0'h], !.
ncnamechar1byte( 0'i ) --> [0'i], !.
ncnamechar1byte( 0'j ) --> [0'j], !.
ncnamechar1byte( 0'k ) --> [0'k], !.
ncnamechar1byte( 0'l ) --> [0'l], !.
ncnamechar1byte( 0'm ) --> [0'm], !.
ncnamechar1byte( 0'n ) --> [0'n], !.
ncnamechar1byte( 0'o ) --> [0'o], !.
ncnamechar1byte( 0'p ) --> [0'p], !.
ncnamechar1byte( 0'q ) --> [0'q], !.
ncnamechar1byte( 0'r ) --> [0'r], !.
ncnamechar1byte( 0's ) --> [0's], !.
ncnamechar1byte( 0't ) --> [0't], !.
ncnamechar1byte( 0'u ) --> [0'u], !.
ncnamechar1byte( 0'v ) --> [0'v], !.
ncnamechar1byte( 0'w ) --> [0'w], !.
ncnamechar1byte( 0'x ) --> [0'x], !.
ncnamechar1byte( 0'y ) --> [0'y], !.
ncnamechar1byte( 0'z ) --> [0'z], !.

ncnamechar2byte( Code ) --> [Code], { Code < 16'800, !, n2byte( Code ) }.

n2byte( 16'B7 ) :- !.
n2byte( Code ) :- Code =< 16'037D, !, Code >= 16'C0.
n2byte( Code ) :- Code >= 16'037F, !.

ncnamechar3byte( Code ) --> [Code], { Code < 16'10000, !, n3byte( Code ) }.

n3byte( Code ) :- Code =< 16'D7FF, !, nbyte31( Code ).
n3byte( Code ) :- Code =< 16'FDCF, !, Code >= 16'E000.
n3byte( Code ) :- Code >= 16'FDF0, !.

nbyte31( Code ) :- Code =< 16'2027, !, Code >= 16'0800.
nbyte31( Code ) :- Code =< 16'218F, !, Code >= 16'202A.
nbyte31( Code ) :- Code >= 16'2800, !.


ncnamechar4byte( Code ) --> [Code], { Code < 16'10FFFF, !, Code >= 16'10000 }.
