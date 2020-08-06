isa_ext(cid(c3,test),cid('CDF Classes',cdf)).
isa_ext(cid(c13,test),cid('CDF Classes',cdf)).
isa_ext(cid(c1,test),cid(c2,test)).
isa_ext(cid(c2,test),cid(c3,test)).

isa_ext(cid(f(a,X),cdfpt),cid(c11,test)).
isa_ext(cid(c11,test),cid(c12,test)).
isa_ext(cid(c12,test),cid(c13,test)).

isa_ext(rid(r1,test),rid(r2,test)).
isa_ext(rid(r2,test),rid(r3,test)).
isa_ext(rid(r3,test),rid('CDF Object-Object Relations',cdf)).
isa_ext(rid(r4,test),rid('CDF Object-Object Relations',cdf)).
isa_ext(cid(f(cid(c1,test),
                  cid(g(cid(c1,cdf),cid(c2,test)),
	             test)),
	         test),
    	     cid(c13,test)).

isa_ext(crid(s1,test),crid(s2,test)).
isa_ext(crid(s2,test),crid(s2,cdf)).
isa_ext(crid(s2,test),crid(s3,test)).
isa_ext(crid(s3,test),crid('CDF Class-Object Relations',cdf)).

isa_ext(oid(o1,testo1),cid(c1,test)).
isa_ext(oid(o2,test),cid(c2,test)).
isa_ext(oid(o3,test),cid(c3,test)).
isa_ext(oid(o11,test),cid(c11,test)).
isa_ext(oid(o12,test),cid(c12,test)).
isa_ext(oid(o13,test),cid(c13,test)).

isa_ext(rid(prod(rid(r2,test),rid(r2,test)),test),
	rid('CDF Object-Object Relations',cdf)).

isa_ext(crid(prod(crid(s2,test),crid(s2,test)),test),
	crid('CDF Class-Object Relations',cdf)).

hasAttr_ext(cid(c2,test),rid(r2,test),cid(c12,test)).
hasAttr_ext(cid(c2,test),rid(r2,test),oid(o12,test)).
hasAttr_ext(oid(o2,test),rid(r1,test),cid(c12,test)).
hasAttr_ext(oid(o2,test),rid(r4,test),cid(c12,test)).
hasAttr_ext(oid(o2,test),rid(r2,test),oid(o12,test)).
hasAttr_ext(oid(o2,test),rid(r4,test),oid(o12,test)).

hasAttr_ext(cid(c2,test),rid(prod(rid(r2,test),rid(r2,test)),test),cid(c12,test)).

classHasAttr_ext(cid(c2,test),crid(s2,test),cid(c12,test)).
classHasAttr_ext(cid(c2,test),crid(s2,test),oid(o12,test)).

classHasAttr_ext(crid(s2,test),crcrid('Name',test),oid('s2 cridname',test)).
classHasAttr_ext(rid(r2,test),crrid(s2,test),oid('rd ridname',test)).

classHasAttr_ext(cid(c2,test),
	              crid(prod(crid(s2,test),crid(s2,test)),test),cid(c12,test)).

allAttr_ext(cid(c2,test),rid(r2,test),cid(c12,test)).
allAttr_ext(cid(c2,test),rid(r2,test),oid(o12,test)).
allAttr_ext(oid(o2,test),rid(r1,test),cid(c12,test)).
allAttr_ext(oid(o2,test),rid(r4,test),cid(c12,test)).
allAttr_ext(oid(o2,test),rid(r2,test),oid(o12,test)).
allAttr_ext(oid(o2,test),rid(r4,test),oid(o12,test)).

allAttr_ext(cid(c2,test),rid(prod(rid(r2,test),rid(r2,test)),test),cid(c12,test)).

minAttr_ext(cid(c2,test),rid(r2,test),cid(c12,test),2).
minAttr_ext(cid(c2,test),rid(r2,test),oid(o12,test),2).
minAttr_ext(oid(o2,test),rid(r1,test),cid(c12,test),2).
minAttr_ext(oid(o2,test),rid(r4,test),cid(c12,test),2).
minAttr_ext(oid(o2,test),rid(r2,test),oid(o12,test),2).
minAttr_ext(oid(o2,test),rid(r4,test),oid(o12,test),2).

minAttr_ext(cid(c2,test),rid(prod(rid(r2,test),rid(r2,test)),test),cid(c12,test),2).

maxAttr_ext(cid(c2,test),rid(r2,test),cid(c12,test),2).
maxAttr_ext(cid(c2,test),rid(r2,test),oid(o12,test),2).
maxAttr_ext(oid(o2,test),rid(r1,test),cid(c12,test),2).
maxAttr_ext(oid(o2,test),rid(r4,test),cid(c12,test),2).
maxAttr_ext(oid(o2,test),rid(r2,test),oid(o12,test),2).
maxAttr_ext(oid(o2,test),rid(r4,test),oid(o12,test),2).

maxAttr_ext(cid(c2,test),rid(prod(rid(r2,test),rid(r2,test)),test),
	                                                         cid(c12,test),2).

end_of_file.
% nothing can be both a c12 and a c2.
ic_ext(','(cid(c12,test),cid(c2,test))).