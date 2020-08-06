:- export build_xref/0, build_xref/1, dump_xref/0, dump_xref/1.
:- export print_pred/3, print_file/3.
:- export xld/0, xld/1, xhelp/1, xhelpd/1, xhelpf/1.

:- import list_directory/2, is_plain_file/1, is_directory/1  from shell.
:- import process_control/2 from shell.
:- import xpp_process_file/3 from parse.
:- import file_size/2 from file_io.
:- import '_$op'/3, op_type/2 from file_op.
:- import concat_atom/2 from string.
:- import append/3, member/2 from basics.
:- import file_read_canonical/3 from machine.
:- import expand_term/2 from standard.
:- import conget/2, coninc/1, coninc/2, conset/2 from gensym.

:- import xsb_defined/3 from usermod.
:- import xsb_used/5 from usermod.

:- import (dynamic)/1, retractall/1, assert/1 from assert.
:- import (index)/2, told/0, writeln/1, write_canonical/1, tell/1, nl/0,
   op/3, abort/1, functor/3, write/1, tab/1, atom_codes/2, catch/3, repeat/0,
   close/1, (;)/2 from standard.
:- import load_dync/1 from consult.
:- import cd/1, cwd/1 from shell.
:- import /*file_open/3, */ file_close/1 from file_io.
:- import file_read/2 from xsb_read.
:- import sort/2, findall/3 from setof.
:- import standard_symbol/3 from std_xsb.
