:- compiler_options([xpp_on]).
#include "socket_defs_xsb.h"

:- export tryN/5.

tryN(Attempts, Call, SuccessGoal, FailGoal, Ecode) :-
	Attempts > 0,
	(call(Call), Ecode == SOCK_OK, write('Success: '), call(SuccessGoal), !
	; sleep(3), Attempts1 is Attempts-1,
	  write('Retrying: '), writeln(Call),
	  tryN(Attempts1,Call,SuccessGoal,FailGoal,Ecode)
	).
	
tryN(Attempts, _Call, _SuccessGoal, FailGoal, _Ecode) :-
	Attempts < 1, write('Giving up: '), call(FailGoal), fail.
