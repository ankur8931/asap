:- op(760,yfx,(#<=>)),
op(750,xfy,(#=>)),
op(750,yfx,(#<=)),
	op(740,yfx,(#\/)),
op(730,yfx,(#\)),
	op(720,yfx,(#/\)),
	op(710, fy,(#\)),
	op(700,xfx,(#>)),
	op(700,xfx,(#<)),
	op(700,xfx,(#>=)),
	op(700,xfx,(#=<)),
	op(700,xfx,(#=)),
	op(700,xfx,(#\=)),
	op(700,xfx,(in)),
	op(550,xfx,(..)).

end_of_file.

myportray(V):- 
   get_attr(V,bounds,B),
   write(V),write(':'),attr_portray_hook(B,_).

attr_portray_hook(bounds(L,U,_),_) :-
	write(L..U).
