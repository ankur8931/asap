:- export empty_db/1, assert_in_db/3, retractall_in_db/3, clause_in_db/3,
	call_in_db/2, load_in_db/3, load_in_db/2.
:- export union_db/3, dump_db/2.
:- export pure_call_in_db/2.

:- import close/1, writeln/2, write_canonical/2, '=..'/2,
   functor/3, open/3, throw/1, call/1, read/2
   from standard.

:- import findall/3 from setof.
:- import numbervars/3 from num_vars.
:- import copy_term/2 from basics.

