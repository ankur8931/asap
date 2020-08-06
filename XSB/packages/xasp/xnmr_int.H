:- export init_smodels/1.
:- export atom_handle/2.
:- export in_all_stable_models/2.
:- export a_stable_model/0.
:- export in_current_stable_model/1.
:- export current_stable_model/1.
:- export print_current_stable_model/0.
:- export answer_set/1.
:- export full_answer_set/2.
:- export pstable_model/3.
:- export set_query_true/1.

:- import conset/2,coninc/1,conget/2 from gensym.
:- import new_trie/1, delete_trie/1, trie_intern/5 from intern.
:- import trie_interned/4 from intern.
:- import file_write/2 from xsb_writ.
:- import findall/3, excess_vars/4 from setof.
:- import (table)/1, warning/1, abort/1, integer/1, repeat/0, call/1
        from standard.
:- import get_calls/3, get_returns/3, table_state/2 from tables.
:- import (dynamic)/1, assert/1, retractall/1, retract/1 from assert.
:- import is_most_general_term/1, (index)/3 from standard.
:- import subsumes_chk/2 from subsumes.
:- import copy_term/2, append/3 from basics.

/*
:- import is/2, '=\='/2, '=:='/2, '<'/2, '=<'/2, '>'/2
	from standard.
*/

/*
:- import asserta/1 from assert.
:- import xsb_configuration/2 from xsb_configuration.
:- import str_cat/3 from string.

:- xsb_configuration(install_dir,Install_dir),
	str_cat(Install_dir,'/packages/sModels/',Smodels_dir),
	asserta(library_directory(Smodels_dir)).
*/

:- import smodelsInit/0, smodelsNumAtoms/1 from xasppkg.
:- import smodelsBeginBasicRule/0, smodelsAddHead/1 from xasppkg.
:- import smodelsAddPosBody/1, smodelsAddNegBody/1 from xasppkg.
:- import smodelsEndRule/0, smodelsCommitRules/0 from xasppkg.
:- import smodelsExistsModel/1 from xasppkg.
:- import smodelsCheckAtom/2 from xasppkg.
:- import smodelsSetPosCompute/1, smodelsSetNegCompute/1 from xasppkg.



