% An example Look Up DGC
% Changed this one on November thr 7th.

:- start( example/1 ).
:- end( -1 ).

:- - digit/1.

example( Is ) ::= lf, !, example( Is ).
example( [] ) ::= [].
example( [I|Is] ) --> item( I ), !, lf, example( Is ).


item( I ) ::= !, number( I ).
item( I ) ::= name( I ).


number( N ) --> + digit(D), !, rest_digits( Ds ), { number_codes( N, [D|Ds] ) }.

rest_digits( [D|Ds] ) --> + digit( D ), !, rest_digits( Ds ).
rest_digits( [] ) ::= [].

digit( 0'0 ) --> "0".
digit( 0'1 ) --> "1".
digit( 0'2 ) --> "2".
digit( 0'3 ) --> "3".
digit( 0'4 ) --> "4".
digit( 0'5 ) --> "5".
digit( 0'6 ) --> "6".
digit( 0'7 ) --> "7".
digit( 0'8 ) --> "8".
digit( 0'9 ) --> "9".


name( N ) --> startchar(C), !, rest_name( Cs ), { atom_codes( N, [C|Cs] ) }.

rest_name( [C|Cs] ) --> namechar( C ), !, rest_name( Cs ).
rest_name( [] ) --> [].


startchar( C ) --> [[0'A-0'Z,0'a-0'z]]/[C], !.
namechar( D ) ::= + digit(D), !.
namechar( C ) ::= startchar(C).

lf --> [16'A].
lf --> [16'D].
