:- module(c_numlists,
	  [ max_list/2,
	    min_list/2,
	    sum_list/2,
	    num_list/3
	  ]).

:- if(\+(current_prolog_flag(dialect,xsb))).
% TLS: no include for now
:- include(library('dialect/commons')).
:- endif.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
max_list([H|T], Max) :-
	'$max_list1'(T, H, Max).

'$max_list1'([], Max, Max).
'$max_list1'([H|T], X, Max) :-
	H =< X, !,
	'$max_list1'(T, X, Max).
'$max_list1'([H|T], _, Max) :-
	'$max_list1'(T, H, Max).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
min_list([H|T], Min) :-
	'$min_list1'(T, H, Min).

'$min_list1'([], Min, Min).
'$min_list1'([H|T], X, Min) :-
	H >= X, !,
	'$min_list1'(T, X, Min).
'$min_list1'([H|T], _, Min) :-
	'$min_list1'(T, H, Min).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sum_list(L, Sum) :-
	'$sum_list1'(L, 0, Sum).

'$sum_list1'([], Sum, Sum).

'$sum_list1'([H|T], Sum0, Sum) :-
	Sum1 is H + Sum0,
	'$sum_list1'(T, Sum1, Sum).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
