
:- [altcdf].

:- import load_extensional_facts/1,
	  load_intensional_rules/1 from cdf_io.
:- import update_all_components/2 from cdf_comps_noshare.
:- import load_component/3 from cdf_comps_share.
:- import retractallExtTerm/1 from cdf_init_cdf.

reset:- 
	shell('rm -rf cdfcomp*').

updtest1:- 
	load_extensional_facts('cdftest1'),
	load_intensional_rules('cdftest1'),
	update_all_components('mytest/cdfcomp1',[action(create)]).

mkinit:- shell('cp cdftest1/initialization_file.P cdfcomp1/test').

loadtest1:- 
	load_component(testo1,'mytest/cdfcomp1',[]).

loadtest1a:- 
	load_component(test,'mytest/cdfcomp1',[]).

loadtest2:- 
	load_component(testo1,'mytest/cdfcomp1',[force(yes)]).

create_new_version :-
	assert(cdf_flags(dirty,test)),
	update_all_components('mytest/cdfcomp1',[action(in_place)]).

version_conflict :-
	load_component(test,'mytest/cdfcomp1',[version(0)]),
	load_component(testo1,'mytest/cdfcomp1',[version(1)]).

create_second_copy :-
	load_component(testo1,'mytest/cdfcomp1',[]),
	update_all_components('mytest/cdfcomp2',[action(create)]).

path_conflict :-
	load_component(test,'mytest/cdfcomp1',[]),
	load_component(testo1,'mytest/cdfcomp2',[]).

force_path_conflict :-
	load_component(test,'mytest/cdfcomp1',[]),
	load_component(testo1,'mytest/cdfcomp2',[action(union)]).

update_in_place_after_conflict :-
	load_component(testo1,'mytest/cdfcomp1',[]),
	load_component(testo1,'mytest/cdfcomp2',[action(union)]),
	%% should not update
	update_all_components('mytest/cdfcomp2',[action(in_place)]).

load_twice :-
	load_component(testo1,'mytest/cdfcomp1',[]),
	load_component(testo1,'mytest/cdfcomp1',[force(yes)]).
	
check_deps :-
	load_component(testo1,'mytest/cdfcomp2',[]),
	retractallExtTerm(isa_ext(oid(o1,testo1),cid(c1,test))),
	update_all_components('mytest/cdfcomp2',[action(in_place)]).



end_of_file.
