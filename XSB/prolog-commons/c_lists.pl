:- module(c_lists,
	  [ member/2,
	    append/3,
	    flatten/2,
	    nth0/3,
	    nth1/3,
	    length/2,
	    memberchk/2,
	    reverse/2,
	    select/3,
	    permutation/2,
	    prefix/2,
	    suffix/2,
	    sublist/2,
	    sub_list/5,
	    last/2,			% List, Elem
	    list_concat/2,		% ListOfLists, List
	    is_list/1,
	    nextto/3,
	    remove_duplicates/2,
	    selectchk/3,
	    sort/2,
	    msort/2,
					% high order stuff
	    include/3,			% :Pred, +List, -Ok
	    exclude/3,			% :Pred. +List, -NotOk
	    partition/4,		% :Pred, +List, -Included, -Excluded
	    partition/5,		% :Pred, +List, ?Less, ?Equal, ?Greater
	    maplist/2,			% :Pred, +List
	    maplist/3,			% :Pred, ?List, ?List
	    maplist/4,			% :Pred, ?List, ?List, ?List
	    maplist/5			% :Pred, ?List, ?List, ?List, ?List
	  ]).
%:- include(library('dialect/commons')).

:- meta_predicate
	include(1, +, -),
	exclude(1, +, -),
	partition(1, +, -, -),
	partition(2, +, -, -, -),
	maplist(1, ?),
	maplist(2, ?, ?),
	maplist(3, ?, ?, ?),
	maplist(4, ?, ?, ?, ?).

:- if(\+current_predicate(member/2)).
member(X, [X|_]).
member(X, [_|T]) :-
	member(X, T).
:- endif.
