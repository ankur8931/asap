
:- if(current_prolog_flag(dialect,xsb)).
:- module(c_terms,
	  [ term_hash/2,		% @Term, -HashKey
	    term_hash/4,		% @Term, +Depth, +Range, -HashKey
	    term_variables/2,		% @Term, -Variables
	    term_variables/3,		% @Term, -Variables, +Tail
	    variant/2,			% @Term1, @Term2
	    subsumes/2,			% +Generic, @Specific
	    subsumes_chk/2,		% +Generic, @Specific
	    cyclic_term/1,		% @Term
	    acyclic_term/1,
	    term_variables/2,
	    genarg/3,			% ?int, +term, ?term
	    unifiable/3
%	    , (?=)/2
	  ]).
:- elseif(true).
:- module(c_terms,
	  [ term_hash/2,		% @Term, -HashKey
	    term_hash/4,		% @Term, +Depth, +Range, -HashKey
	    term_variables/2,		% @Term, -Variables
	    term_variables/3,		% @Term, -Variables, +Tail
	    variant/2,			% @Term1, @Term2
	    subsumes/2,			% +Generic, @Specific
	    subsumes_chk/2,		% +Generic, @Specific
	    cyclic_term/1,		% @Term
	    acyclic_term/1,
	    term_variables/2,
	    genarg/3,			% ?int, +term, ?term
	    unifiable/3,
	    (?=)/2
	  ]).
:- endif.

:- include(library('dialect/commons')).

term_variables(Term, Vars) :-
	listofvars(Term, Vars, []).

term_variables(Term, Vars,Tail) :-
	listofvars(Term, Vars, Tail).

listofvars(Term, Vh, Vt) :-
	(var(Term)
	 ->	Vh = [Term | Vt]
	 ;	Term =.. [_|Args],
		listofvars1(Args, Vh, Vt)
	).

listofvars1([], V, V).
listofvars1([T|Ts], Vh, Vt) :-
	listofvars(T, Vh, Vm),
	listofvars1(Ts, Vm, Vt).

