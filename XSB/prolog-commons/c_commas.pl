
/* Calling the module commas for now, rather than c_commas, to support XSB */
:- module(c_commas,
	  [ comma_append/3,
	    comma_length/2,
	    comma_member/2,
	    comma_memberchk/2,
	    comma_to_list/2
	  ]).

%:- include(library('dialect/commons')).

:- comment(title,"Comma list utilities").

:- comment(author,"T. Swift").

:- comment(module,"This module implements very simple operations on comma lists.").

%----
comma_append(','(L,R),Cl,','(L,R1)):- !,
	comma_append(R,Cl,R1).
comma_append(true,Cl,Cl):- !.
comma_append(L,Cl,Out):- 
	(Cl == true -> Out = L ; Out = ','(L,Cl)).

%----

comma_length(','(_L,R),N1):- !,
	comma_length(R,N),
	N1 is N + 1.	
comma_length(true,0):- !.
comma_length(_,1).

%----
% warning: may bind variables.
comma_member(A,','(A,_)).
comma_member(A,','(_,R)):- 
	comma_member(A,R).
comma_member(A,A):- \+ (functor(A,',',_)).

%----
comma_memberchk(A,','(A,_)):- !.
comma_memberchk(A,','(_,R)):- 
	comma_memberchk(A,R).
comma_memberchk(A,A):- \+ (functor(A,',',_)).

%----

comma_to_list((One,Two),[One|Twol]):- !,
	comma_to_list(Two,Twol).
comma_to_list(One,[One]).

