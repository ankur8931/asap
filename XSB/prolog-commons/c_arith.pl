:- module(c_arith,
	  [ between/3,			% +int, +int, ?int
	    succ/2			% int <-> int
	  ]).

% XSB succ/2 -- please replace if there is a better one to use.
% TLS: not putting in errors (they are caught by is/2).
succ(First,Second):- 
    (number(First) -> 
         Second is First + 1
      ; (number(Second) -> 
	  First is Second - 1
	 ; Second is First + 1)).
