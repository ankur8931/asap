% TEMPORAL REASONING
% thom fruehwirth ECRC 920721
% follows work by Itay Meiri AAAI 1991
% uses path concistency handler pc.chr
% 930908 updated and modified for new CHR version
% Christian Holzbaur mods for SICStus (e.g. delay -> block/when)
% XSB Version Barry Evans 2006

:- export
	transl/4,
	intersection/3,
	equality/2,
	shift_interval/3,
	bind_value/3,
	unique/1,
	universal/3,
	empty/2,
	path1/5,
	qtype_g/2,	% All the following terms are called by when/2
	unique_g/1,
	unblocked_empty/2,
	unblocked_universal/3,
	unblocked_equality/2,
	unblocked_intersection/3,
	unblocked_transl/4,
	unblocked_is_quant/1,
	unblocked_is_qual/1,
	unblocked_qualquant/3,
	unblocked_pi_ii/2,
	unblocked_ip_ii/2,
	unblocked_pp_pi/2,
	unblocked_pp_ii/2.

:- ensure_loaded(chr).
:- import load_chr/1 from chr_pp.
:- load_chr(timeConstraints).	% get compiled path consistency handler

:- import memberchk/2, member/2 from basics.
:- import sort/2, findall/3 from setof.

:- ensure_loaded(constraintLib).
:- import when/2 from constraintLib.

%:- ensure_loaded('iso8601.P').

:- import misc_error/1 from error_handler.

%% domain specific predicates ------------------------------------------------


inf(3.40282e38).
minf(-3.40282e38).
sup(1.0e-45).
msup(-1.0e-45).

path1(1,X,Y,[R],p-p):- check_pp(X,Y,R).
path1(1,X,Y,[R],p-i):- check_pi(R,X,Y).
path1(1,X,Y,[R],i-p):- check_ip(R,X,Y).
path1(1,X,Y,[R],i-i):- check_ii(X,Y,R) .


%:-block empty(-,-).
%
% empty(0,[]).
empty(N,L) :-
	when((nonvar(N),nonvar(L)),time:unblocked_empty(N,L)).

unblocked_empty(0,[]).

%:- block universal(-,?,?), universal(?,-,?), universal(?,?,-).
%
universal(N,L,T):-
	when((nonvar(N);nonvar(L);nonvar(T)),time:unblocked_universal(N,L,T)).

unblocked_universal(N,L,T):-
	(is_quantl(L) -> 
	        inf(Inf), minf(Minf),
		L=[A-B],(A=<Minf),(Inf=<B)
		; 
	T=p-p -> 		% 930212 to account for finite domains
		sort(L,[eq, gt, lt])
		;
		size(T,N)
	),
	!.

	size(i-i,13).
	size(p-p,3).
	size(p-i,5).
	size(i-p,5).
	size(s-s,5).

%:- block equality(?,-), equality(-,?).
%
equality(L,T) :-
	when((nonvar(L);nonvar(T)),time:unblocked_equality(L,T)).

% TLS: probably dont need number(E) for is_quall
unblocked_equality(L,i-i):- !, member(equals,L).
unblocked_equality(L,s-s):- !, member(eq,L).
unblocked_equality(L,p-p):-
	(is_quall(L) ->			% succeeds also if var-case: dirty!!
		member(E,L),(E=eq;number(E),E=:=0)	% 930212
		;
		member(A-B,L),
		(A=0,B=0 ; (A=<0),(0=<B))
	),
	!.


unique(L) :- when(ground(L), time:unique_g(L)).

unique_g([A-B]):- !,(A=:=B).
unique_g([_A]).

% 930212 for finite domains
bind_value(X,Y,[R]):- (R=V-_;R=V)-> number(V), (Y=:=X+V).

shift_interval(_X,[],[]).
shift_interval(X,[A-C|L1],[B-D|L2]):-
	!,
	number(A),
	number(C),
	number(X),
	B is A-X, D is C-X,
	shift_interval(X,L1,L2).

shift_interval(X,[A|L1],[B|L2]):-
	number(A),
	number(X),
	B is A-X,
	shift_interval(X,L1,L2).


%:- block intersection(-,?,?), intersection(?,-,?).
%
intersection(L1,L2,L3) :-
	when((nonvar(L1);nonvar(L2)),time:unblocked_intersection(L1,L2,L3)).

unblocked_intersection(L1,L2,L3):- qtype(L1,Q1),qtype(L2,Q2),
		((Q1==quall,Q2==quall) ->
			intersection1(L1,L2,L3)
			;
			qualquant(L1,Q1,LQ1),qualquant(L2,Q2,LQ2),
			interint(LQ1,LQ2,L3)
		),
		!. 

  intersection1([], _, []).
  intersection1([Head|L1tail], L2, L3) :-
        memberchk(Head, L2),
        !,
        L3 = [Head|L3tail],
        intersection1(L1tail, L2, L3tail).
  intersection1([_|L1tail], L2, L3) :-
        intersection1(L1tail, L2, L3).


  % interint([1-2,4-5,6-9],[2-3,3-11],L).
  interint([],_L,[]).
  interint(L,[],[]):- L=[_|_].
  interint([A|L1],[B|L2],L3):-
	(
	 isless(A,B) -> interint(L1,[B|L2],L3);
	 isless(B,A) -> interint([A|L1],L2,L3);
	 overlaps1(A,B,C) -> L3=[C|L3N],interint([A|L1],L2,L3N);
	 overlaps2(A,B,C) -> L3=[C|L3N],interint(L1,[B|L2],L3N)
	).

    isless(_A-B,C-_D):- (B<C).

    overlaps1(A-B,C-D,E-F):- 
	(B>=D),(C=<B),(A=<D), 
	my_max(A,C,E),my_min(B,D,F).
%	E is max(float(A),float(C)), F is min(float(B),float(D)).
    overlaps2(A-B,C-D,E-F):- 
	(D>=B),(C=<B),(A=<D), 
	my_max(A,C,E),my_min(B,D,F).
%	E is max(float(A),float(C)), F is min(float(B),float(D)).

	my_max(X,Y,Z):- (X>=Y),!,X=Z.
	my_max(_X,Y,Y).

	my_min(X,Y,Z):- (X=<Y),!,X=Z.
	my_min(_X,Y,Y).

%:- block transl(-,-,?,?), transl(-,?,-,?), transl(?,-,-,?).
%
transl(A,B,C,T) :-
	when((	(nonvar(A),nonvar(B));
			(nonvar(A),nonvar(C));
			(nonvar(B),nonvar(C))),time:unblocked_transl(A,B,C,T)).

unblocked_transl(A,B,C,T):- 
	qtype(A,QA),qtype(B,QB),qtype(C,QC),	
	(
	(T=p-p-p,(QA==quantl;QB==quantl;QC==quantl) ) ->  % at least one quantl
		qualquant(A,QA,A1),qualquant(B,QB,B1),qualquant(C,QC,C1),
		transl(A1,B1,C1,T,quantl)
		;
		quantqual(A,QA,A1),quantqual(B,QB,B1),quantqual(C,QC,C1),
		transl(A1,B1,C1,T,quall)
	),
	!.	

/*
  transl(L1,L2,L3,T,Q):- var(L3),!,
	time_setof(C,A^B^(member(A,L1),member(B,L2),trans(A,B,C,T,Q)),L3N),
	mergerel(L3N,L3,Q).
  transl(L1,L2,L3,T,Q):- var(L2),!,
	time_setof(B,A^C^(member(A,L1),member(C,L3),trans(A,B,C,T,Q)),L2N),
	mergerel(L2N,L2,Q).
  transl(L1,L2,L3,T,Q):- var(L1),!,
	time_setof(A,B^C^(member(B,L2),member(C,L3),trans(A,B,C,T,Q)),L1N),
	mergerel(L1N,L1,Q).
*/

  transl(L1,L2,L3,T,Q):- var(L3),!,
	time_setof(C,(member(A,L1),member(B,L2),trans(A,B,C,T,Q)),L3N),
	mergerel(L3N,L3,Q).
  transl(L1,L2,L3,T,Q):- var(L2),!,
	time_setof(B,(member(A,L1),member(C,L3),
%	              writeln(calling(trans(A,B,C,T,Q))),
		      trans(A,B,C,T,Q)),L2N),
	mergerel(L2N,L2,Q).
  transl(L1,L2,L3,T,Q):- var(L1),!,
	time_setof(A,(member(B,L2),member(C,L3),trans(A,B,C,T,Q)),L1N),
	mergerel(L1N,L1,Q).

	mergerel(L1,L2,Q):- 
		(Q==quantl -> mergerel(L1,L2) ; L1=L2),
		!.
	  mergerel([],[]).
	  mergerel([A-B,C-D|L1],L2):-
	        sup(Sup),
		(B+Sup>=C),		% +sup added 921029
		!,
		my_min(A,C,Min),		% min, max added 920129
		my_max(B,D,Max),
		mergerel([Min-Max|L1],L2).
	  mergerel([X|L1],[X|L2]):-
		mergerel(L1,L2).

     trans(A,B,C,s-s-s,quall):- !, 
		strans(A,B,C).
     trans(A,B,C,p-p-p,quall):- !, 
		prans(A,B,C).
     trans(A,B,C,p-p-p,quantl):- !, 
		qtrans(A,B,C).
     trans(A,B,C,U-V-W,quall):- !,
		itrans(U-V-W,A,B,C). 


time_setof(A,B,C):-
	findall(A,B,CTemp),
	sort(CTemp,C).

%% qualitative and quantitative constraints interaction
qtype(L,T) :- when(ground(L), time:qtype_g(L,T)).

qtype_g(L,quantl):- is_quantl(L).
qtype_g(L,quall):- is_quall(L).

 is_quantl([X|_]):- is_quant(X).
 is_quall([X|_]):- is_qual(X).

%  :- block is_quant(-).
is_quant(X) :-
	when(nonvar(X),time:unblocked_is_quant(X)).

unblocked_is_quant(_A-_B). % :- A1 is A,B1 is B,number(A1),number(B1).

%  :- block is_qual(-).
is_qual(X) :-
	when(nonvar(X),time:unblocked_is_qual(X)).

unblocked_is_qual(A):- atomic(A).	% single numbers are treated like atoms 930212

%   :- block qualquant(-,?,-).	% necessary?
qualquant(A,QA,A1):-
	when((nonvar(A),nonvar(A1)),time:unblocked_qualquant(A,QA,A1)).

unblocked_qualquant(A,QA,A1):- 	% hacked for var-case (== versus = below!)
	(QA==quall -> qualquant(A,A0),mergerel(A0,A1) ; QA=quantl -> A=A1).	% mergrel added 921029

%   :- block quantqual(-,?,-).	% necessary?   No!!!
quantqual(A,QA,A1):- 	% hacked for var-case (== versus = below!)
	(QA==quantl -> quantqual(A,A1) ; QA=quall -> A=A1).

%path(N,X,Y,L,p-p) +=> qualquant(L,LIN), sort(LIN,LI), path(N,X,Y,LI,p-p).

	qualquant([],[]).
	qualquant([A|L1],[B|L2]):-
		qualquant1(A,B),
		qualquant(L1,L2).

		qualquant1(lt,A-B):- !, sup(A), inf(B).
		qualquant1(eq,0-0):- !.
		qualquant1(gt,A-B):- !, minf(A), msup(B).
		% 930212 to treat single numbers 
		qualquant1(N,A-A):- A is N.	% 'is' used to catch type error

%path(N,X,Y,LI,p-p) +=> N>2 | 		% quick hack condition for termination
%	quantqual(LI,L), length(L,N1), path(N1,X,Y,L,p-p). 

	quantqual(LI,L):-
		findall(X,quantqual1(LI,X),L).

		quantqual1(LI,eq):- 
			once((member(I-J,LI), (I=<0),(0=<J))).
		quantqual1(LI,lt):-  
			once((member(_I-J,LI), (0<J))).
		quantqual1(LI,gt):-  
			once((member(I-_J,LI), (I<0))).

		% 930212 to treat single numbers 
		quantqual1(LI,N):-  
			once((member(N-M,LI), (N=:=M))).




% ALLENS INTERVALS ---------------------------------------------------------

:- ensure_loaded(allentable).	% get cons_tri/3 transitivity table for Allens intervals
:- import cons_tri/3 from allentable.

% [after, before, contains, during, equals, finished_by, finishes, meets, met_by, overlapped_by, overlaps, started_by, starts].
/*
	after
	before
	contains
	during
	equals
	finished_by
	finishes
	meets
	met_by
	overlapped_by
	overlaps
	started_by
	starts

*/

%930212
check_ii(X,Y,R):- interval_point(X,R,Y).	
% taken from jonathan lever
interval_point(_X-Y,before,U-_V):- ((Y < U)).
interval_point([_X,Y],before,[U,_V]):- ((Y < U)).
interval_point(X-_Y,after,_U-V):- ((V < X)).
interval_point([X,_Y],after,[_U,V]):- ((V < X)).
interval_point(_X-Y,meets,U-_V):- ((Y =:= U)).
interval_point([_X,Y],meets,[U,_V]):- ((Y =:= U)).
interval_point(X-_Y,met_by,_U-V):- ((V =:= X)).
interval_point([X,_Y],met_by,[_U,V]):- ((V =:= X)).
interval_point(X-Y,starts,U-V):- ((X =:= U, Y < V)).
interval_point([X,Y],starts,[U,V]):- ((X =:= U, Y < V)).
interval_point(X-Y,started_by,U-V):- ((X =:= U, V < Y)).
interval_point([X,Y],started_by,[U,V]):- ((X =:= U, V < Y)).
interval_point(X-Y,finishes,U-V):- ((Y =:= V, U < X)).
interval_point([X,Y],finishes,[U,V]):- ((Y =:= V, U < X)).
interval_point(X-Y,finished_by,_U-V):- ((Y =:= V, X < V)).
interval_point([X,Y],finished_by,[_U,V]):- ((Y =:= V, X < V)).
interval_point(X-Y,during,U-V):- ((U < X, Y < V)).
interval_point([X,Y],during,[U,V]):- ((U < X, Y < V)).
interval_point(X-Y,contains,U-V):- ((X < U, V < Y)).
interval_point([X,Y],contains,[U,V]):- ((X < U, V < Y)).
interval_point(X-Y,overlaps,U-V):- ((X < U, U < Y, Y < V)).
interval_point([X,Y],overlaps,[U,V]):- ((X < U, U < Y, Y < V)).
interval_point(X-Y,overlapped_by,U-V):- ((U < X, X < V, V < Y)).
interval_point([X,Y],overlapped_by,[U,V]):- ((U < X, X < V, V < Y)).
interval_point(X-Y,equals,U-V):- ((X =:= U,Y =:= V)).
interval_point([X,Y],equals,[U,V]):- ((X =:= U,Y =:= V)).

/* 
TLS: I'm not sure the original itrans/4 was working properly.  It made
a heavy use of when, and thus would return many extra variables when
called by time_setof.  However, it is called only by transl which
ensures that 2 out of the three input arguments are bound.  Thus, the
explicit instantiation check will prove much more efficient.
*/

itrans(U-V-W,A,B,C):-	
	nonvar(A),nonvar(B),!,
	encode(U-V,A,X),
	encode(V-W,B,Y),
	cons_tri(X,Y,Z),
	encode(U-W,C,Z).
itrans(U-V-W,A,B,C):-	
	nonvar(A),nonvar(C),!,
	encode(U-V,A,X),
	encode(U-W,C,Z),
	cons_tri(X,Y,Z),
	encode(V-W,B,Y).
itrans(U-V-W,A,B,C):-	
	nonvar(B),nonvar(C),!,
	encode(V-W,B,Y),
	encode(U-W,C,Z),
	cons_tri(X,Y,Z),
	encode(U-V,A,X).
itrans(U-V-W,A,B,C):-	
	trace,
	misc_error('Does not have proper instantiation'(itrans(U-V-W,A,B,C))).

/*
Old code.

itrans(U-V-W,A,B,C):-	
		encode(U-V,A,X),
		encode(V-W,B,Y),
		encode(U-W,C,Z), 
		cons_tri(X,Y,Z).

%:- block encode(?,-,-).
%
encode(T,A,B) :-
	when((nonvar(A),nonvar(B)),time:unblocked_encode(T,A,B)).

*/

encode(i-i,A,A):-!.
encode(p-i,A,B):-!,pi_ii(A,B).
encode(i-p,A,B):-!,ip_ii(A,B).
encode(p-p,A,B):-!,pp_ii(A,B).

% POINT ALGEBRA ---------------------------------------------------------------

%[le,eq,ge]

% 930212
check_pp(X,Y,A-B):- !, ((X+A<Y,Y<X+B)).
check_pp(X,Y,N):- number(N),!, (X+N=:=Y).
check_pp(_X,Y,T):- \+ member(T,[lt,eq,gt]),!, Y=T.
check_pp(X,Y,R):- ((number(X),number(Y))->check_ppn(X,Y,R);check_ppt(X,Y,R)).

check_ppn(X,Y,lt):- (X<Y).
check_ppn(X,Y,eq):- (X=:=Y).
check_ppn(X,Y,gt):- (X>Y).

check_ppt(X,Y,lt):- (X@<Y).
check_ppt(X,Y,eq):- (X=Y).
check_ppt(X,Y,gt):- (X@>Y).

prans(A,B,C):- (number(A);number(B);number(C)),!,qtrans(A-A,B-B,C-C).
prans(lt,lt,lt).
prans(lt,eq,lt).
prans(eq,lt,lt).
prans(eq,eq,eq).
prans(eq,gt,gt).
prans(gt,eq,gt).
prans(gt,gt,gt).

/* Original 
prans(le,le,le).
prans(le,eq,le).
prans(le,ge,le).
prans(le,ge,eq).
prans(le,ge,ge).
prans(eq,le,le).
prans(eq,eq,eq).
prans(eq,ge,ge).
prans(ge,le,le).
prans(ge,le,eq).
prans(ge,le,ge).
prans(ge,eq,ge).
prans(ge,ge,ge).
*/

% QUANTITATIVE  ---------------------------------------------------------

% [I1-I2,...In-1-In] ordered Ii=<Ii+1, comparison problem with reals (equality)

qtrans(A-B,C-D,E-F):- (	(var(A),var(B)) -> safe_is(A,E-D), safe_is(B,F-C) ;
			(var(C),var(D)) -> safe_is(C,E-B), safe_is(D,F-A) ;
			(var(E),var(F)) -> safe_is(E,A+C), safe_is(F,B+D)
			).
	safe_is(A,X-Y):-
	        inf(Inf),
		minf(Minf),
		sup(Sup),
		msup(Msup),
		(X=:=Minf,Y=:=Inf -> A is Minf
		;
		 X=:=Inf,Y=:=Minf -> A is Inf
		;
		 X=:=Msup,Y=:=Sup -> A is Msup
		;
		 X=:=Sup,Y=:=Msup -> A is Sup
		;
		 A is X-Y).
	safe_is(A,X+Y):-
	        inf(Inf),
		minf(Minf),
		sup(Sup),
		msup(Msup),
		(X=:=Inf,Y=:=Inf -> A is Inf
		;
		 X=:=Minf,Y=:=Minf -> A is Minf
		;
		 X=:=Sup,Y=:=Sup -> A is Sup
		;
		 X=:=Msup,Y=:=Msup -> A is Msup
		;
		 A is X+Y).




% POINT-INTERVAL ---------------------------------------------------------

% p-i [before,starts,during,finishes,after]
% i-p [after,started_by,contains,finished_by,before]

%930212
check_pi(before,X,A-_B):- ((X<A)).
check_pi(before,X,[A,_B]):- ((X<A)).
check_pi(starts,X,A-_B):- ((X=:=A)).
check_pi(starts,X,[A,_B]):- ((X=:=A)).
check_pi(during,W-X,Y-Z):- !, ((Y=<W,X=<Z)).		%% Allow for morphing p-i to i-i when collapsing Constraints
check_pi(during,X,A-B):- ((A<X,X<B)).
check_pi(during,X,[A,B]):- ((A<X,X<B)).
check_pi(finishes,X,_A-B):- ((X=:=B)).
check_pi(finishes,X,[_A,B]):- ((X=:=B)).
check_pi(after,X,_A-B):- ((B<X)).
check_pi(after,X,[_A,B]):- ((B<X)).



check_ip(after,A-_B,X):- number(X),((X<A)).
check_ip(after,[A,_B],X):- number(X),((X<A)).
check_ip(started_by,A-_B,X):- number(X),((X=:=A)).
check_ip(started_by,[A,_B],X):- number(X),((X=:=A)).
check_ip(contains,A-B,X):- number(X),((A<X,X<B)).
check_ip(contains,[A,B],X):- number(X),((A<X,X<B)).
check_ip(finished_by,_A-B,X):- number(X),((X=:=B)).
check_ip(finished_by,[_A,B],X):- number(X),((X=:=B)).
check_ip(before,_A-B,X):- number(X),((B<X)).
check_ip(before,[_A,B],X):- number(X),((B<X)).

% trans see itrans for INTERVAL

% pi_ii
%:- block pi_ii(-,-).
%
%pi_ii(A,B) :-
%	when((nonvar(A),nonvar(B)),time:unblocked_pi_ii(A,B)).

pi_ii(A,B):- 
	unblocked_pi_ii(A,B).

:- index(unblocked_pi_ii/2,[1,2]).
unblocked_pi_ii(before, before).
unblocked_pi_ii(before, meets).
unblocked_pi_ii(before, finished_by).
unblocked_pi_ii(before, contains).
unblocked_pi_ii(before, overlaps).
unblocked_pi_ii(starts, starts).
unblocked_pi_ii(starts, equals).
unblocked_pi_ii(starts, started_by).
unblocked_pi_ii(during, during).
unblocked_pi_ii(during, finishes).
unblocked_pi_ii(during, overlaped_by).
unblocked_pi_ii(finishes, met_by).
unblocked_pi_ii(after, after).

% ip_ii (inversion of pi_ii)
%:- block ip_ii(-,-).
%
%ip_ii(A,B) :-
%	when((nonvar(A),nonvar(B)),time:unblocked_ip_ii(A,B)).

ip_ii(A,B):- 
	unblocked_ip_ii(A,B).

:- index(unblocked_ip_ii/2,[1,2]).
unblocked_ip_ii(before, before).
unblocked_ip_ii(finished_by, meets).
unblocked_ip_ii(contains, contains).
unblocked_ip_ii(contains, overlaps).
unblocked_ip_ii(contains, finished_by).
unblocked_ip_ii(started_by, starts).
unblocked_ip_ii(started_by, equals).
unblocked_ip_ii(started_by, started_by).
unblocked_ip_ii(after, during).
unblocked_ip_ii(after, finishes).
unblocked_ip_ii(after, overlaped_by).
unblocked_ip_ii(after, met_by).
unblocked_ip_ii(after, after).

% pp_pi
%:- block pp_pi(-,-).
%
%pp_pi(A,B) :-
%	when((nonvar(A),nonvar(B)),time:unblocked_pp_pi(A,B)).

pp_pi(A,B) :-unblocked_pp_pi(A,B).

unblocked_pp_pi(le, before).
unblocked_pp_pi(eq, starts).
unblocked_pp_pi(ge, during).
unblocked_pp_pi(ge, finishes).
unblocked_pp_pi(ge, after).

% pp_ii
%:- block pp_ii(-,-).
%
%pp_ii(A,B) :-
%	when((nonvar(A),nonvar(B)),time:unblocked_pp_ii(A,B)).

pp_ii(A,B):- unblocked_pp_ii(A,B).

unblocked_pp_ii(A,B):- pp_pi(A,C),pi_ii(C,B).

% end of handler time.chr ----------------------------------------------------



% SET ALGEBRA ---------------------------------------------------------------
% 9212 with carmen in half an hour
% 930216 corrected by mapping on Allen

% [eq,sub,sup,dj,int]
% {equals, subset, superset, disjoint, intersecting}

% 930216 - not debugged - be careful with overlapping names
% equality, unique, universal have not been updated!!

% [le,ge] % [eq] already defined
% {less,greater} meaning for *all* pairs of elements of disjoint sets
% not really worked out - should collapse to point algebra for zero-length intv
% could use two relations on start- and end-points of interval (</<, =/<, </>...)
% [le/ge] pairs of point relations

strans(A,B,C):-
	ss_ii(A,X),ss_ii(B,Y),ss_ii(C,Z),
	itrans(i-i-i,X,Y,Z).

%%delay ss_ii(A,X) if var(A),var(X).
%
ss_ii(eq,equals).
ss_ii(sub,during).
ss_ii(sub,starts).
ss_ii(sub,finishes).
ss_ii(sup,contains).
ss_ii(sup,started_by).
ss_ii(sup,finished_by).
ss_ii(dj,before).
ss_ii(dj,after).
ss_ii(int,overlaps).
ss_ii(int,overlapped_by).
ss_ii(int,meets).
ss_ii(int,met_by).
%
%ss_ii(eq,equals).
ss_ii(le,before).
ss_ii(ge,after).
%
ss_ii(le/le, before).
ss_ii(le/le, overlaps).
ss_ii(le/le, meets).
ss_ii(le/eq, finished_by).
ss_ii(le/ge, during).
ss_ii(eq/le, starts).
ss_ii(eq/eq, equals).
ss_ii(eq/ge, started_by).
ss_ii(ge/le, contains).
ss_ii(ge/eq, finishes).
ss_ii(ge/ge, after).
ss_ii(ge/ge, overlapped_by).
ss_ii(ge/ge, met_by).
%
ss_ii(le/le/le, before).
ss_ii(le/le/ge, overlaps).
ss_ii(le/le/eq, meets).
%
ss_ii(ge/ge/ge, after).
ss_ii(ge/ge/le, overlapped_by).
ss_ii(ge/ge/eq, met_by).

/* 
% 9212
strans(eq, eq,  eq).
strans(eq, sub,  sub).
strans(eq, sup,  sup).
strans(eq, dj,  dj).
strans(eq, int,  int).
strans(sub, eq,  sub).
strans(sub, sub,  sub).
strans(sub, sup,  X):- member(X,[eq,sub,sup,int]).
strans(sub, dj,  X):- member(X,[eq,sub,sup,dj,int]).
strans(sub, int, X):- member(X,[eq,sub,sup,dj,int]).
strans(sup, eq,  sup).
strans(sup, sub,  X):- member(X,[eq,sub,sup,int]).
strans(sup, sup,  sup).
strans(sup, dj,  X):- member(X,[eq,sub,sup,dj,int]).
strans(sup, int,  X):- member(X,[eq,sub,sup,dj,int]).
strans(dj, eq,  dj).
strans(dj, sub,  X):- member(X,[eq,sub,sup,dj,int]).
strans(dj, sup,  dj).
strans(dj, dj,  X):- member(X,[eq,sub,sup,dj,int]).
strans(dj, int,  X):- member(X,[sup,dj,int]).
strans(int, eq,  int).
strans(int, sub, int).
strans(int, sub,  sup).
strans(int, sup, dj).
strans(int, sup, int).
strans(int, sup, sup).
strans(int, dj,  dj).
strans(int, dj,  int).
strans(int, dj,  sup).
strans(int, int,  X):- member(X,[eq,sub,sup,dj,int]).
*/
