% The character ranges for XML Name productions

:- - ncnamechar1byte/1.
:- - ncnamechar2byte/1.
:- - ncnamechar3byte/1.

:- - ncnamestartchar1byte/1.
:- - ncnamestartchar2byte/1.
:- - ncnamestartchar3byte/1.


% [84]    Letter    ::=    BaseChar | Ideographic 
% [85]    BaseChar    ::=    [#x0041-#x005A] | [#x0061-#x007A] | [#x00C0-#x00D6] | [#x00D8-#x00F6] | [#x00F8-#x00FF] | [#x0100-#x0131] | [#x0134-#x013E] | [#x0141-#x0148] | [#x014A-#x017E] | [#x0180-#x01C3] | [#x01CD-#x01F0] | [#x01F4-#x01F5] | [#x01FA-#x0217] | [#x0250-#x02A8] | [#x02BB-#x02C1] | #x0386 | [#x0388-#x038A] | #x038C | [#x038E-#x03A1] | [#x03A3-#x03CE] | [#x03D0-#x03D6] | #x03DA | #x03DC | #x03DE | #x03E0 | [#x03E2-#x03F3] | [#x0401-#x040C] | [#x040E-#x044F] | [#x0451-#x045C] | [#x045E-#x0481] | [#x0490-#x04C4] | [#x04C7-#x04C8] | [#x04CB-#x04CC] | [#x04D0-#x04EB] | [#x04EE-#x04F5] | [#x04F8-#x04F9] | [#x0531-#x0556] | #x0559 | [#x0561-#x0586] | [#x05D0-#x05EA] | [#x05F0-#x05F2] | [#x0621-#x063A] | [#x0641-#x064A] | [#x0671-#x06B7] | [#x06BA-#x06BE] | [#x06C0-#x06CE] | [#x06D0-#x06D3] | #x06D5 | [#x06E5-#x06E6] | [#x0905-#x0939] | #x093D | [#x0958-#x0961] | [#x0985-#x098C] | [#x098F-#x0990] | [#x0993-#x09A8] | [#x09AA-#x09B0] | #x09B2 | [#x09B6-#x09B9] | [#x09DC-#x09DD] | [#x09DF-#x09E1] | [#x09F0-#x09F1] | [#x0A05-#x0A0A] | [#x0A0F-#x0A10] | [#x0A13-#x0A28] | [#x0A2A-#x0A30] | [#x0A32-#x0A33] | [#x0A35-#x0A36] | [#x0A38-#x0A39] | [#x0A59-#x0A5C] | #x0A5E | [#x0A72-#x0A74] | [#x0A85-#x0A8B] | #x0A8D | [#x0A8F-#x0A91] | [#x0A93-#x0AA8] | [#x0AAA-#x0AB0] | [#x0AB2-#x0AB3] | [#x0AB5-#x0AB9] | #x0ABD | #x0AE0 | [#x0B05-#x0B0C] | [#x0B0F-#x0B10] | [#x0B13-#x0B28] | [#x0B2A-#x0B30] | [#x0B32-#x0B33] | [#x0B36-#x0B39] | #x0B3D | [#x0B5C-#x0B5D] | [#x0B5F-#x0B61] | [#x0B85-#x0B8A] | [#x0B8E-#x0B90] | [#x0B92-#x0B95] | [#x0B99-#x0B9A] | #x0B9C | [#x0B9E-#x0B9F] | [#x0BA3-#x0BA4] | [#x0BA8-#x0BAA] | [#x0BAE-#x0BB5] | [#x0BB7-#x0BB9] | [#x0C05-#x0C0C] | [#x0C0E-#x0C10] | [#x0C12-#x0C28] | [#x0C2A-#x0C33] | [#x0C35-#x0C39] | [#x0C60-#x0C61] | [#x0C85-#x0C8C] | [#x0C8E-#x0C90] | [#x0C92-#x0CA8] | [#x0CAA-#x0CB3] | [#x0CB5-#x0CB9] | #x0CDE | [#x0CE0-#x0CE1] | [#x0D05-#x0D0C] | [#x0D0E-#x0D10] | [#x0D12-#x0D28] | [#x0D2A-#x0D39] | [#x0D60-#x0D61] | [#x0E01-#x0E2E] | #x0E30 | [#x0E32-#x0E33] | [#x0E40-#x0E45] | [#x0E81-#x0E82] | #x0E84 | [#x0E87-#x0E88] | #x0E8A | #x0E8D | [#x0E94-#x0E97] | [#x0E99-#x0E9F] | [#x0EA1-#x0EA3] | #x0EA5 | #x0EA7 | [#x0EAA-#x0EAB] | [#x0EAD-#x0EAE] | #x0EB0 | [#x0EB2-#x0EB3] | #x0EBD | [#x0EC0-#x0EC4] | [#x0F40-#x0F47] | [#x0F49-#x0F69] | [#x10A0-#x10C5] | [#x10D0-#x10F6] | #x1100 | [#x1102-#x1103] | [#x1105-#x1107] | #x1109 | [#x110B-#x110C] | [#x110E-#x1112] | #x113C | #x113E | #x1140 | #x114C | #x114E | #x1150 | [#x1154-#x1155] | #x1159 | [#x115F-#x1161] | #x1163 | #x1165 | #x1167 | #x1169 | [#x116D-#x116E] | [#x1172-#x1173] | #x1175 | #x119E | #x11A8 | #x11AB | [#x11AE-#x11AF] | [#x11B7-#x11B8] | #x11BA | [#x11BC-#x11C2] | #x11EB | #x11F0 | #x11F9 | [#x1E00-#x1E9B] | [#x1EA0-#x1EF9] | [#x1F00-#x1F15] | [#x1F18-#x1F1D] | [#x1F20-#x1F45] | [#x1F48-#x1F4D] | [#x1F50-#x1F57] | #x1F59 | #x1F5B | #x1F5D | [#x1F5F-#x1F7D] | [#x1F80-#x1FB4] | [#x1FB6-#x1FBC] | #x1FBE | [#x1FC2-#x1FC4] | [#x1FC6-#x1FCC] | [#x1FD0-#x1FD3] | [#x1FD6-#x1FDB] | [#x1FE0-#x1FEC] | [#x1FF2-#x1FF4] | [#x1FF6-#x1FFC] | #x2126 | [#x212A-#x212B] | #x212E | [#x2180-#x2182] | [#x3041-#x3094] | [#x30A1-#x30FA] | [#x3105-#x312C] | [#xAC00-#xD7A3]  
% [86]    Ideographic    ::=    [#x4E00-#x9FA5] | #x3007 | [#x3021-#x3029]  
% [87]    CombiningChar    ::=    [#x0300-#x0345] | [#x0360-#x0361] | [#x0483-#x0486] | [#x0591-#x05A1] | [#x05A3-#x05B9] | [#x05BB-#x05BD] | #x05BF | [#x05C1-#x05C2] |  [#x05C4 | [#x064B-#x0652] | #x0670 | [#x06D6-#x06DC] | [#x06DD-#x06DF] | [#x06E0-#x06E4] | [#x06E7-#x06E8] | [#x06EA-#x06ED] | [#x0901-#x0903] | #x093C | [#x093E-#x094C]| #x094D | [#x0951-#x0954] | [#x0962-#x0963] | [#x0981-#x0983] | #x09BC | #x09BE | #x09BF | [#x09C0-#x09C4] | [#x09C7-#x09C8] | [#x09CB-#x09CD] | #x09D7 | [#x09E2-#x09E3] | #x0A02 | #x0A3C | #x0A3E | #x0A3F | [#x0A40-#x0A42] | [#x0A47-#x0A48] | [#x0A4B-#x0A4D] | [#x0A70-#x0A71] | [#x0A81-#x0A83] | #x0ABC | [#x0ABE-#x0AC5] | [#x0AC7-#x0AC9] | [#x0ACB-#x0ACD] | [#x0B01-#x0B03] | #x0B3C | [#x0B3E-#x0B43] | [#x0B47-#x0B48] | [#x0B4B-#x0B4D] | [#x0B56-#x0B57] | [#x0B82-#x0B83] | [#x0BBE-#x0BC2] | [#x0BC6-#x0BC8] | [#x0BCA-#x0BCD] | #x0BD7 | [#x0C01-#x0C03] | [#x0C3E-#x0C44] | [#x0C46-#x0C48] | [#x0C4A-#x0C4D] | [#x0C55-#x0C56] | [#x0C82-#x0C83] | [#x0CBE-#x0CC4] | [#x0CC6-#x0CC8] | [#x0CCA-#x0CCD] | [#x0CD5-#x0CD6] | [#x0D02-#x0D03] | [#x0D3E-#x0D43] | [#x0D46-#x0D48] | [#x0D4A-#x0D4D] | #x0D57 | #x0E31 | [#x0E34-#x0E3A] | [#x0E47-#x0E4E] | #x0EB1 | [#x0EB4-#x0EB9] | [#x0EBB-#x0EBC] | [#x0EC8-#x0ECD] | [#x0F18-#x0F19] | #x0F35 | #x0F37 | #x0F39 | #x0F3E | #x0F3F | [#x0F71-#x0F84] | [#x0F86-#x0F8B] | [#x0F90-#x0F95] | #x0F97 | [#x0F99-#x0FAD] | [#x0FB1-#x0FB7] | #x0FB9 | [#x20D0-#x20DC] | #x20E1 | [#x302A-#x302F] | #x3099 | #x309A  
% [88]    Digit    ::=    [#x0030-#x0039] | [#x0660-#x0669] | [#x06F0-#x06F9] | [#x0966-#x096F] | [#x09E6-#x09EF] | [#x0A66-#x0A6F] | [#x0AE6-#x0AEF] | [#x0B66-#x0B6F] | [#x0BE7-#x0BEF] | [#x0C66-#x0C6F] | [#x0CE6-#x0CEF] | [#x0D66-#x0D6F] | [#x0E50-#x0E59] | [#x0ED0-#x0ED9] | [#x0F20-#x0F29]  
% [89]    Extender    ::=    #x00B7 | #x02D0 | #x02D1 | #x0387 | #x0640 | #x0E46 | #x0EC6 | #x3005 | [#x3031-#x3035] | [#x309D-#x309E] | [#x30FC-#x30FE]  


% [4]  NCNameStartChar ::=  (Letter | '_') 	/*  An XML Name, minus the ":" */ 


ncnamestartchar1byte( 0'_ ) --> [0'_], !.
ncnamestartchar1byte( 0'A ) --> [0'A], !.
ncnamestartchar1byte( 0'B ) --> [0'B], !.
ncnamestartchar1byte( 0'C ) --> [0'C], !.
ncnamestartchar1byte( 0'D ) --> [0'D], !.
ncnamestartchar1byte( 0'E ) --> [0'E], !.
ncnamestartchar1byte( 0'F ) --> [0'F], !.
ncnamestartchar1byte( 0'G ) --> [0'G], !.
ncnamestartchar1byte( 0'H ) --> [0'H], !.
ncnamestartchar1byte( 0'I ) --> [0'I], !.
ncnamestartchar1byte( 0'J ) --> [0'J], !.
ncnamestartchar1byte( 0'K ) --> [0'K], !.
ncnamestartchar1byte( 0'L ) --> [0'L], !.
ncnamestartchar1byte( 0'M ) --> [0'M], !.
ncnamestartchar1byte( 0'N ) --> [0'N], !.
ncnamestartchar1byte( 0'O ) --> [0'O], !.
ncnamestartchar1byte( 0'P ) --> [0'P], !.
ncnamestartchar1byte( 0'Q ) --> [0'Q], !.
ncnamestartchar1byte( 0'R ) --> [0'R], !.
ncnamestartchar1byte( 0'S ) --> [0'S], !.
ncnamestartchar1byte( 0'T ) --> [0'T], !.
ncnamestartchar1byte( 0'U ) --> [0'U], !.
ncnamestartchar1byte( 0'V ) --> [0'V], !.
ncnamestartchar1byte( 0'W ) --> [0'W], !.
ncnamestartchar1byte( 0'X ) --> [0'X], !.
ncnamestartchar1byte( 0'Y ) --> [0'Y], !.
ncnamestartchar1byte( 0'Z ) --> [0'Z], !.
ncnamestartchar1byte( 0'a ) --> [0'a], !.
ncnamestartchar1byte( 0'b ) --> [0'b], !.
ncnamestartchar1byte( 0'c ) --> [0'c], !.
ncnamestartchar1byte( 0'd ) --> [0'd], !.
ncnamestartchar1byte( 0'e ) --> [0'e], !.
ncnamestartchar1byte( 0'f ) --> [0'f], !.
ncnamestartchar1byte( 0'g ) --> [0'g], !.
ncnamestartchar1byte( 0'h ) --> [0'h], !.
ncnamestartchar1byte( 0'i ) --> [0'i], !.
ncnamestartchar1byte( 0'j ) --> [0'j], !.
ncnamestartchar1byte( 0'k ) --> [0'k], !.
ncnamestartchar1byte( 0'l ) --> [0'l], !.
ncnamestartchar1byte( 0'm ) --> [0'm], !.
ncnamestartchar1byte( 0'n ) --> [0'n], !.
ncnamestartchar1byte( 0'o ) --> [0'o], !.
ncnamestartchar1byte( 0'p ) --> [0'p], !.
ncnamestartchar1byte( 0'q ) --> [0'q], !.
ncnamestartchar1byte( 0'r ) --> [0'r], !.
ncnamestartchar1byte( 0's ) --> [0's], !.
ncnamestartchar1byte( 0't ) --> [0't], !.
ncnamestartchar1byte( 0'u ) --> [0'u], !.
ncnamestartchar1byte( 0'v ) --> [0'v], !.
ncnamestartchar1byte( 0'w ) --> [0'w], !.
ncnamestartchar1byte( 0'x ) --> [0'x], !.
ncnamestartchar1byte( 0'y ) --> [0'y], !.
ncnamestartchar1byte( 0'z ) --> [0'z], !.


ncnamestartchar2byte( Code ) --> [Code], { Code < 16'800, !, ns2byte( Code ) }.

ns2byte(Code) :- (Code =< 16'03F3  ','  !  ','  ns2byte0(Code)).
ns2byte(Code) :- ns2byte1(Code).
ns2byte0(Code) :- (Code =< 16'02A8  ','  !  ','  ns2byte00(Code)).
ns2byte0(Code) :- ns2byte01(Code).
ns2byte00(Code) :- (Code =< 16'0148  ','  !  ','  ns2byte000(Code)).
ns2byte00(Code) :- ns2byte001(Code).
ns2byte000(Code) :- (Code =< 16'00F6  ','  !  ','  ns2byte0000(Code)).
ns2byte000(Code) :- ns2byte0001(Code).
ns2byte0000(Code) :- (Code =< 16'00D6  ','  !  ','  ns2byte00000(Code)).
ns2byte0000(Code) :- ns2byte00001(Code).
ns2byte00000(Code) :- (Code =< 16'00D6  ','  !  ','  Code >= 16'00C0).
ns2byte00001(Code) :- (Code =< 16'00F6  ','  !  ','  Code >= 16'00D8).
ns2byte0001(Code) :- (Code =< 16'0131  ','  !  ','  ns2byte00010(Code)).
ns2byte0001(Code) :- ns2byte00011(Code).
ns2byte00010(Code) :- (Code =< 16'0131  ','  !  ','  Code >= 16'00F8).
ns2byte00011(Code) :- (Code =< 16'013E  ','  !  ','  ns2byte000110(Code)).
ns2byte00011(Code) :- ns2byte000111(Code).
ns2byte000110(Code) :- (Code =< 16'013E  ','  !  ','  Code >= 16'0134).
ns2byte000111(Code) :- (Code =< 16'0148  ','  !  ','  Code >= 16'0141).
ns2byte001(Code) :- (Code =< 16'01F0  ','  !  ','  ns2byte0010(Code)).
ns2byte001(Code) :- ns2byte0011(Code).
ns2byte0010(Code) :- (Code =< 16'017E  ','  !  ','  ns2byte00100(Code)).
ns2byte0010(Code) :- ns2byte00101(Code).
ns2byte00100(Code) :- (Code =< 16'017E  ','  !  ','  Code >= 16'014A).
ns2byte00101(Code) :- (Code =< 16'01C3  ','  !  ','  ns2byte001010(Code)).
ns2byte00101(Code) :- ns2byte001011(Code).
ns2byte001010(Code) :- (Code =< 16'01C3  ','  !  ','  Code >= 16'0180).
ns2byte001011(Code) :- (Code =< 16'01F0  ','  !  ','  Code >= 16'01CD).
ns2byte0011(Code) :- (Code =< 16'01F5  ','  !  ','  ns2byte00110(Code)).
ns2byte0011(Code) :- ns2byte00111(Code).
ns2byte00110(Code) :- (Code =< 16'01F5  ','  !  ','  Code >= 16'01F4).
ns2byte00111(Code) :- (Code =< 16'0217  ','  !  ','  ns2byte001110(Code)).
ns2byte00111(Code) :- ns2byte001111(Code).
ns2byte001110(Code) :- (Code =< 16'0217  ','  !  ','  Code >= 16'01FA).
ns2byte001111(Code) :- (Code =< 16'02A8  ','  !  ','  Code >= 16'0250).
ns2byte01(Code) :- (Code =< 16'03CE  ','  !  ','  ns2byte010(Code)).
ns2byte01(Code) :- ns2byte011(Code).
ns2byte010(Code) :- (Code =< 16'038A  ','  !  ','  ns2byte0100(Code)).
ns2byte010(Code) :- ns2byte0101(Code).
ns2byte0100(Code) :- (Code =< 16'02C1  ','  !  ','  ns2byte01000(Code)).
ns2byte0100(Code) :- ns2byte01001(Code).
ns2byte01000(Code) :- (Code =< 16'02C1  ','  !  ','  Code >= 16'02BB).
ns2byte01001(Code) :- (Code =< 16'0386  ','  !  ','  ns2byte010010(Code)).
ns2byte01001(Code) :- ns2byte010011(Code).
ns2byte010010(Code) :- (Code =< 16'0386  ','  !  ','  Code >= 16'0386).
ns2byte010011(Code) :- (Code =< 16'038A  ','  !  ','  Code >= 16'0388).
ns2byte0101(Code) :- (Code =< 16'038C  ','  !  ','  ns2byte01010(Code)).
ns2byte0101(Code) :- ns2byte01011(Code).
ns2byte01010(Code) :- (Code =< 16'038C  ','  !  ','  Code >= 16'038C).
ns2byte01011(Code) :- (Code =< 16'03A1  ','  !  ','  ns2byte010110(Code)).
ns2byte01011(Code) :- ns2byte010111(Code).
ns2byte010110(Code) :- (Code =< 16'03A1  ','  !  ','  Code >= 16'038E).
ns2byte010111(Code) :- (Code =< 16'03CE  ','  !  ','  Code >= 16'03A3).
ns2byte011(Code) :- (Code =< 16'03DC  ','  !  ','  ns2byte0110(Code)).
ns2byte011(Code) :- ns2byte0111(Code).
ns2byte0110(Code) :- (Code =< 16'03D6  ','  !  ','  ns2byte01100(Code)).
ns2byte0110(Code) :- ns2byte01101(Code).
ns2byte01100(Code) :- (Code =< 16'03D6  ','  !  ','  Code >= 16'03D0).
ns2byte01101(Code) :- (Code =< 16'03DA  ','  !  ','  ns2byte011010(Code)).
ns2byte01101(Code) :- ns2byte011011(Code).
ns2byte011010(Code) :- (Code =< 16'03DA  ','  !  ','  Code >= 16'03DA).
ns2byte011011(Code) :- (Code =< 16'03DC  ','  !  ','  Code >= 16'03DC).
ns2byte0111(Code) :- (Code =< 16'03DE  ','  !  ','  ns2byte01110(Code)).
ns2byte0111(Code) :- ns2byte01111(Code).
ns2byte01110(Code) :- (Code =< 16'03DE  ','  !  ','  Code >= 16'03DE).
ns2byte01111(Code) :- (Code =< 16'03E0  ','  !  ','  ns2byte011110(Code)).
ns2byte01111(Code) :- ns2byte011111(Code).
ns2byte011110(Code) :- (Code =< 16'03E0  ','  !  ','  Code >= 16'03E0).
ns2byte011111(Code) :- (Code =< 16'03F3  ','  !  ','  Code >= 16'03E2).
ns2byte1(Code) :- (Code =< 16'0556  ','  !  ','  ns2byte10(Code)).
ns2byte1(Code) :- ns2byte11(Code).
ns2byte10(Code) :- (Code =< 16'04C4  ','  !  ','  ns2byte100(Code)).
ns2byte10(Code) :- ns2byte101(Code).
ns2byte100(Code) :- (Code =< 16'044F  ','  !  ','  ns2byte1000(Code)).
ns2byte100(Code) :- ns2byte1001(Code).
ns2byte1000(Code) :- (Code =< 16'040C  ','  !  ','  ns2byte10000(Code)).
ns2byte1000(Code) :- ns2byte10001(Code).
ns2byte10000(Code) :- (Code =< 16'040C  ','  !  ','  Code >= 16'0401).
ns2byte10001(Code) :- (Code =< 16'044F  ','  !  ','  Code >= 16'040E).
ns2byte1001(Code) :- (Code =< 16'045C  ','  !  ','  ns2byte10010(Code)).
ns2byte1001(Code) :- ns2byte10011(Code).
ns2byte10010(Code) :- (Code =< 16'045C  ','  !  ','  Code >= 16'0451).
ns2byte10011(Code) :- (Code =< 16'0481  ','  !  ','  ns2byte100110(Code)).
ns2byte10011(Code) :- ns2byte100111(Code).
ns2byte100110(Code) :- (Code =< 16'0481  ','  !  ','  Code >= 16'045E).
ns2byte100111(Code) :- (Code =< 16'04C4  ','  !  ','  Code >= 16'0490).
ns2byte101(Code) :- (Code =< 16'04EB  ','  !  ','  ns2byte1010(Code)).
ns2byte101(Code) :- ns2byte1011(Code).
ns2byte1010(Code) :- (Code =< 16'04C8  ','  !  ','  ns2byte10100(Code)).
ns2byte1010(Code) :- ns2byte10101(Code).
ns2byte10100(Code) :- (Code =< 16'04C8  ','  !  ','  Code >= 16'04C7).
ns2byte10101(Code) :- (Code =< 16'04CC  ','  !  ','  ns2byte101010(Code)).
ns2byte10101(Code) :- ns2byte101011(Code).
ns2byte101010(Code) :- (Code =< 16'04CC  ','  !  ','  Code >= 16'04CB).
ns2byte101011(Code) :- (Code =< 16'04EB  ','  !  ','  Code >= 16'04D0).
ns2byte1011(Code) :- (Code =< 16'04F5  ','  !  ','  ns2byte10110(Code)).
ns2byte1011(Code) :- ns2byte10111(Code).
ns2byte10110(Code) :- (Code =< 16'04F5  ','  !  ','  Code >= 16'04EE).
ns2byte10111(Code) :- (Code =< 16'04F9  ','  !  ','  ns2byte101110(Code)).
ns2byte10111(Code) :- ns2byte101111(Code).
ns2byte101110(Code) :- (Code =< 16'04F9  ','  !  ','  Code >= 16'04F8).
ns2byte101111(Code) :- (Code =< 16'0556  ','  !  ','  Code >= 16'0531).
ns2byte11(Code) :- (Code =< 16'064A  ','  !  ','  ns2byte110(Code)).
ns2byte11(Code) :- ns2byte111(Code).
ns2byte110(Code) :- (Code =< 16'05EA  ','  !  ','  ns2byte1100(Code)).
ns2byte110(Code) :- ns2byte1101(Code).
ns2byte1100(Code) :- (Code =< 16'0559  ','  !  ','  ns2byte11000(Code)).
ns2byte1100(Code) :- ns2byte11001(Code).
ns2byte11000(Code) :- (Code =< 16'0559  ','  !  ','  Code >= 16'0559).
ns2byte11001(Code) :- (Code =< 16'0586  ','  !  ','  ns2byte110010(Code)).
ns2byte11001(Code) :- ns2byte110011(Code).
ns2byte110010(Code) :- (Code =< 16'0586  ','  !  ','  Code >= 16'0561).
ns2byte110011(Code) :- (Code =< 16'05EA  ','  !  ','  Code >= 16'05D0).
ns2byte1101(Code) :- (Code =< 16'05F2  ','  !  ','  ns2byte11010(Code)).
ns2byte1101(Code) :- ns2byte11011(Code).
ns2byte11010(Code) :- (Code =< 16'05F2  ','  !  ','  Code >= 16'05F0).
ns2byte11011(Code) :- (Code =< 16'063A  ','  !  ','  ns2byte110110(Code)).
ns2byte11011(Code) :- ns2byte110111(Code).
ns2byte110110(Code) :- (Code =< 16'063A  ','  !  ','  Code >= 16'0621).
ns2byte110111(Code) :- (Code =< 16'064A  ','  !  ','  Code >= 16'0641).
ns2byte111(Code) :- (Code =< 16'06CE  ','  !  ','  ns2byte1110(Code)).
ns2byte111(Code) :- ns2byte1111(Code).
ns2byte1110(Code) :- (Code =< 16'06B7  ','  !  ','  ns2byte11100(Code)).
ns2byte1110(Code) :- ns2byte11101(Code).
ns2byte11100(Code) :- (Code =< 16'06B7  ','  !  ','  Code >= 16'0671).
ns2byte11101(Code) :- (Code =< 16'06BE  ','  !  ','  ns2byte111010(Code)).
ns2byte11101(Code) :- ns2byte111011(Code).
ns2byte111010(Code) :- (Code =< 16'06BE  ','  !  ','  Code >= 16'06BA).
ns2byte111011(Code) :- (Code =< 16'06CE  ','  !  ','  Code >= 16'06C0).
ns2byte1111(Code) :- (Code =< 16'06D3  ','  !  ','  ns2byte11110(Code)).
ns2byte1111(Code) :- ns2byte11111(Code).
ns2byte11110(Code) :- (Code =< 16'06D3  ','  !  ','  Code >= 16'06D0).
ns2byte11111(Code) :- (Code =< 16'06D5  ','  !  ','  ns2byte111110(Code)).
ns2byte11111(Code) :- ns2byte111111(Code).
ns2byte111110(Code) :- (Code =< 16'06D5  ','  !  ','  Code >= 16'06D5).
ns2byte111111(Code) :- (Code =< 16'06E6  ','  !  ','  Code >= 16'06E5).


ncnamestartchar3byte( Code ) --> [Code], { Code < 16'10000, !, ns3byte( Code ) }.


ns3byte(Code) :- (Code =< 16'0E97  ','  !  ','  ns3byte0(Code)).
ns3byte(Code) :- ns3byte1(Code).
ns3byte0(Code) :- (Code =< 16'0B5D  ','  !  ','  ns3byte00(Code)).
ns3byte0(Code) :- ns3byte01(Code).
ns3byte00(Code) :- (Code =< 16'0A39  ','  !  ','  ns3byte000(Code)).
ns3byte00(Code) :- ns3byte001(Code).
ns3byte000(Code) :- (Code =< 16'09B9  ','  !  ','  ns3byte0000(Code)).
ns3byte000(Code) :- ns3byte0001(Code).
ns3byte0000(Code) :- (Code =< 16'098C  ','  !  ','  ns3byte00000(Code)).
ns3byte0000(Code) :- ns3byte00001(Code).
ns3byte00000(Code) :- (Code =< 16'093D  ','  !  ','  ns3byte000000(Code)).
ns3byte00000(Code) :- ns3byte000001(Code).
ns3byte000000(Code) :- (Code =< 16'0939  ','  !  ','  ns3byte0000000(Code)).
ns3byte000000(Code) :- ns3byte0000001(Code).
ns3byte0000000(Code) :- (Code =< 16'0939  ','  !  ','  Code >= 16'0905).
ns3byte0000001(Code) :- (Code =< 16'093D  ','  !  ','  Code >= 16'093D).
ns3byte000001(Code) :- (Code =< 16'0961  ','  !  ','  ns3byte0000010(Code)).
ns3byte000001(Code) :- ns3byte0000011(Code).
ns3byte0000010(Code) :- (Code =< 16'0961  ','  !  ','  Code >= 16'0958).
ns3byte0000011(Code) :- (Code =< 16'098C  ','  !  ','  Code >= 16'0985).
ns3byte00001(Code) :- (Code =< 16'09A8  ','  !  ','  ns3byte000010(Code)).
ns3byte00001(Code) :- ns3byte000011(Code).
ns3byte000010(Code) :- (Code =< 16'0990  ','  !  ','  ns3byte0000100(Code)).
ns3byte000010(Code) :- ns3byte0000101(Code).
ns3byte0000100(Code) :- (Code =< 16'0990  ','  !  ','  Code >= 16'098F).
ns3byte0000101(Code) :- (Code =< 16'09A8  ','  !  ','  Code >= 16'0993).
ns3byte000011(Code) :- (Code =< 16'09B0  ','  !  ','  ns3byte0000110(Code)).
ns3byte000011(Code) :- ns3byte0000111(Code).
ns3byte0000110(Code) :- (Code =< 16'09B0  ','  !  ','  Code >= 16'09AA).
ns3byte0000111(Code) :- (Code =< 16'09B2  ','  !  ','  ns3byte00001110(Code)).
ns3byte0000111(Code) :- ns3byte00001111(Code).
ns3byte00001110(Code) :- (Code =< 16'09B2  ','  !  ','  Code >= 16'09B2).
ns3byte00001111(Code) :- (Code =< 16'09B9  ','  !  ','  Code >= 16'09B6).
ns3byte0001(Code) :- (Code =< 16'0A10  ','  !  ','  ns3byte00010(Code)).
ns3byte0001(Code) :- ns3byte00011(Code).
ns3byte00010(Code) :- (Code =< 16'09E1  ','  !  ','  ns3byte000100(Code)).
ns3byte00010(Code) :- ns3byte000101(Code).
ns3byte000100(Code) :- (Code =< 16'09DD  ','  !  ','  ns3byte0001000(Code)).
ns3byte000100(Code) :- ns3byte0001001(Code).
ns3byte0001000(Code) :- (Code =< 16'09DD  ','  !  ','  Code >= 16'09DC).
ns3byte0001001(Code) :- (Code =< 16'09E1  ','  !  ','  Code >= 16'09DF).
ns3byte000101(Code) :- (Code =< 16'09F1  ','  !  ','  ns3byte0001010(Code)).
ns3byte000101(Code) :- ns3byte0001011(Code).
ns3byte0001010(Code) :- (Code =< 16'09F1  ','  !  ','  Code >= 16'09F0).
ns3byte0001011(Code) :- (Code =< 16'0A0A  ','  !  ','  ns3byte00010110(Code)).
ns3byte0001011(Code) :- ns3byte00010111(Code).
ns3byte00010110(Code) :- (Code =< 16'0A0A  ','  !  ','  Code >= 16'0A05).
ns3byte00010111(Code) :- (Code =< 16'0A10  ','  !  ','  Code >= 16'0A0F).
ns3byte00011(Code) :- (Code =< 16'0A30  ','  !  ','  ns3byte000110(Code)).
ns3byte00011(Code) :- ns3byte000111(Code).
ns3byte000110(Code) :- (Code =< 16'0A28  ','  !  ','  ns3byte0001100(Code)).
ns3byte000110(Code) :- ns3byte0001101(Code).
ns3byte0001100(Code) :- (Code =< 16'0A28  ','  !  ','  Code >= 16'0A13).
ns3byte0001101(Code) :- (Code =< 16'0A30  ','  !  ','  Code >= 16'0A2A).
ns3byte000111(Code) :- (Code =< 16'0A33  ','  !  ','  ns3byte0001110(Code)).
ns3byte000111(Code) :- ns3byte0001111(Code).
ns3byte0001110(Code) :- (Code =< 16'0A33  ','  !  ','  Code >= 16'0A32).
ns3byte0001111(Code) :- (Code =< 16'0A36  ','  !  ','  ns3byte00011110(Code)).
ns3byte0001111(Code) :- ns3byte00011111(Code).
ns3byte00011110(Code) :- (Code =< 16'0A36  ','  !  ','  Code >= 16'0A35).
ns3byte00011111(Code) :- (Code =< 16'0A39  ','  !  ','  Code >= 16'0A38).
ns3byte001(Code) :- (Code =< 16'0AB9  ','  !  ','  ns3byte0010(Code)).
ns3byte001(Code) :- ns3byte0011(Code).
ns3byte0010(Code) :- (Code =< 16'0A8D  ','  !  ','  ns3byte00100(Code)).
ns3byte0010(Code) :- ns3byte00101(Code).
ns3byte00100(Code) :- (Code =< 16'0A5E  ','  !  ','  ns3byte001000(Code)).
ns3byte00100(Code) :- ns3byte001001(Code).
ns3byte001000(Code) :- (Code =< 16'0A5C  ','  !  ','  ns3byte0010000(Code)).
ns3byte001000(Code) :- ns3byte0010001(Code).
ns3byte0010000(Code) :- (Code =< 16'0A5C  ','  !  ','  Code >= 16'0A59).
ns3byte0010001(16'0A5E) :- !.
ns3byte001001(Code) :- (Code =< 16'0A74  ','  !  ','  ns3byte0010010(Code)).
ns3byte001001(Code) :- ns3byte0010011(Code).
ns3byte0010010(Code) :- (Code =< 16'0A74  ','  !  ','  Code >= 16'0A72).
ns3byte0010011(Code) :- (Code =< 16'0A8B  ','  !  ','  ns3byte00100110(Code)).
ns3byte0010011(Code) :- ns3byte00100111(Code).
ns3byte00100110(Code) :- (Code =< 16'0A8B  ','  !  ','  Code >= 16'0A85).
ns3byte00100111(16'0A8D) :- !.
ns3byte00101(Code) :- (Code =< 16'0AA8  ','  !  ','  ns3byte001010(Code)).
ns3byte00101(Code) :- ns3byte001011(Code).
ns3byte001010(Code) :- (Code =< 16'0A91  ','  !  ','  ns3byte0010100(Code)).
ns3byte001010(Code) :- ns3byte0010101(Code).
ns3byte0010100(Code) :- (Code =< 16'0A91  ','  !  ','  Code >= 16'0A8F).
ns3byte0010101(Code) :- (Code =< 16'0AA8  ','  !  ','  Code >= 16'0A93).
ns3byte001011(Code) :- (Code =< 16'0AB0  ','  !  ','  ns3byte0010110(Code)).
ns3byte001011(Code) :- ns3byte0010111(Code).
ns3byte0010110(Code) :- (Code =< 16'0AB0  ','  !  ','  Code >= 16'0AAA).
ns3byte0010111(Code) :- (Code =< 16'0AB3  ','  !  ','  ns3byte00101110(Code)).
ns3byte0010111(Code) :- ns3byte00101111(Code).
ns3byte00101110(Code) :- (Code =< 16'0AB3  ','  !  ','  Code >= 16'0AB2).
ns3byte00101111(Code) :- (Code =< 16'0AB9  ','  !  ','  Code >= 16'0AB5).
ns3byte0011(Code) :- (Code =< 16'0B28  ','  !  ','  ns3byte00110(Code)).
ns3byte0011(Code) :- ns3byte00111(Code).
ns3byte00110(Code) :- (Code =< 16'0AE0  ','  !  ','  ns3byte001100(Code)).
ns3byte00110(Code) :- ns3byte001101(Code).
ns3byte001100(Code) :- (Code =< 16'0ABD  ','  !  ','  ns3byte0011000(Code)).
ns3byte001100(Code) :- ns3byte0011001(Code).
ns3byte0011000(16'0ABD) :- !.
ns3byte0011001(16'0AE0) :- !.
ns3byte001101(Code) :- (Code =< 16'0B0C  ','  !  ','  ns3byte0011010(Code)).
ns3byte001101(Code) :- ns3byte0011011(Code).
ns3byte0011010(Code) :- (Code =< 16'0B0C  ','  !  ','  Code >= 16'0B05).
ns3byte0011011(Code) :- (Code =< 16'0B10  ','  !  ','  ns3byte00110110(Code)).
ns3byte0011011(Code) :- ns3byte00110111(Code).
ns3byte00110110(Code) :- (Code =< 16'0B10  ','  !  ','  Code >= 16'0B0F).
ns3byte00110111(Code) :- (Code =< 16'0B28  ','  !  ','  Code >= 16'0B13).
ns3byte00111(Code) :- (Code =< 16'0B33  ','  !  ','  ns3byte001110(Code)).
ns3byte00111(Code) :- ns3byte001111(Code).
ns3byte001110(Code) :- (Code =< 16'0B30  ','  !  ','  ns3byte0011100(Code)).
ns3byte001110(Code) :- ns3byte0011101(Code).
ns3byte0011100(Code) :- (Code =< 16'0B30  ','  !  ','  Code >= 16'0B2A).
ns3byte0011101(Code) :- (Code =< 16'0B33  ','  !  ','  Code >= 16'0B32).
ns3byte001111(Code) :- (Code =< 16'0B39  ','  !  ','  ns3byte0011110(Code)).
ns3byte001111(Code) :- ns3byte0011111(Code).
ns3byte0011110(Code) :- (Code =< 16'0B39  ','  !  ','  Code >= 16'0B36).
ns3byte0011111(Code) :- (Code =< 16'0B3D  ','  !  ','  ns3byte00111110(Code)).
ns3byte0011111(Code) :- ns3byte00111111(Code).
ns3byte00111110(16'0B3D) :- !.
ns3byte00111111(Code) :- (Code =< 16'0B5D  ','  !  ','  Code >= 16'0B5C).
ns3byte01(Code) :- (Code =< 16'0C90  ','  !  ','  ns3byte010(Code)).
ns3byte01(Code) :- ns3byte011(Code).
ns3byte010(Code) :- (Code =< 16'0BAA  ','  !  ','  ns3byte0100(Code)).
ns3byte010(Code) :- ns3byte0101(Code).
ns3byte0100(Code) :- (Code =< 16'0B95  ','  !  ','  ns3byte01000(Code)).
ns3byte0100(Code) :- ns3byte01001(Code).
ns3byte01000(Code) :- (Code =< 16'0B8A  ','  !  ','  ns3byte010000(Code)).
ns3byte01000(Code) :- ns3byte010001(Code).
ns3byte010000(Code) :- (Code =< 16'0B61  ','  !  ','  ns3byte0100000(Code)).
ns3byte010000(Code) :- ns3byte0100001(Code).
ns3byte0100000(Code) :- (Code =< 16'0B61  ','  !  ','  Code >= 16'0B5F).
ns3byte0100001(Code) :- (Code =< 16'0B8A  ','  !  ','  Code >= 16'0B85).
ns3byte010001(Code) :- (Code =< 16'0B90  ','  !  ','  ns3byte0100010(Code)).
ns3byte010001(Code) :- ns3byte0100011(Code).
ns3byte0100010(Code) :- (Code =< 16'0B90  ','  !  ','  Code >= 16'0B8E).
ns3byte0100011(Code) :- (Code =< 16'0B95  ','  !  ','  Code >= 16'0B92).
ns3byte01001(Code) :- (Code =< 16'0B9C  ','  !  ','  ns3byte010010(Code)).
ns3byte01001(Code) :- ns3byte010011(Code).
ns3byte010010(Code) :- (Code =< 16'0B9A  ','  !  ','  ns3byte0100100(Code)).
ns3byte010010(Code) :- ns3byte0100101(Code).
ns3byte0100100(Code) :- (Code =< 16'0B9A  ','  !  ','  Code >= 16'0B99).
ns3byte0100101(16'0B9C) :- !.
ns3byte010011(Code) :- (Code =< 16'0B9F  ','  !  ','  ns3byte0100110(Code)).
ns3byte010011(Code) :- ns3byte0100111(Code).
ns3byte0100110(Code) :- (Code =< 16'0B9F  ','  !  ','  Code >= 16'0B9E).
ns3byte0100111(Code) :- (Code =< 16'0BA4  ','  !  ','  ns3byte01001110(Code)).
ns3byte0100111(Code) :- ns3byte01001111(Code).
ns3byte01001110(Code) :- (Code =< 16'0BA4  ','  !  ','  Code >= 16'0BA3).
ns3byte01001111(Code) :- (Code =< 16'0BAA  ','  !  ','  Code >= 16'0BA8).
ns3byte0101(Code) :- (Code =< 16'0C28  ','  !  ','  ns3byte01010(Code)).
ns3byte0101(Code) :- ns3byte01011(Code).
ns3byte01010(Code) :- (Code =< 16'0BB9  ','  !  ','  ns3byte010100(Code)).
ns3byte01010(Code) :- ns3byte010101(Code).
ns3byte010100(Code) :- (Code =< 16'0BB5  ','  !  ','  ns3byte0101000(Code)).
ns3byte010100(Code) :- ns3byte0101001(Code).
ns3byte0101000(Code) :- (Code =< 16'0BB5  ','  !  ','  Code >= 16'0BAE).
ns3byte0101001(Code) :- (Code =< 16'0BB9  ','  !  ','  Code >= 16'0BB7).
ns3byte010101(Code) :- (Code =< 16'0C0C  ','  !  ','  ns3byte0101010(Code)).
ns3byte010101(Code) :- ns3byte0101011(Code).
ns3byte0101010(Code) :- (Code =< 16'0C0C  ','  !  ','  Code >= 16'0C05).
ns3byte0101011(Code) :- (Code =< 16'0C10  ','  !  ','  ns3byte01010110(Code)).
ns3byte0101011(Code) :- ns3byte01010111(Code).
ns3byte01010110(Code) :- (Code =< 16'0C10  ','  !  ','  Code >= 16'0C0E).
ns3byte01010111(Code) :- (Code =< 16'0C28  ','  !  ','  Code >= 16'0C12).
ns3byte01011(Code) :- (Code =< 16'0C39  ','  !  ','  ns3byte010110(Code)).
ns3byte01011(Code) :- ns3byte010111(Code).
ns3byte010110(Code) :- (Code =< 16'0C33  ','  !  ','  ns3byte0101100(Code)).
ns3byte010110(Code) :- ns3byte0101101(Code).
ns3byte0101100(Code) :- (Code =< 16'0C33  ','  !  ','  Code >= 16'0C2A).
ns3byte0101101(Code) :- (Code =< 16'0C39  ','  !  ','  Code >= 16'0C35).
ns3byte010111(Code) :- (Code =< 16'0C61  ','  !  ','  ns3byte0101110(Code)).
ns3byte010111(Code) :- ns3byte0101111(Code).
ns3byte0101110(Code) :- (Code =< 16'0C61  ','  !  ','  Code >= 16'0C60).
ns3byte0101111(Code) :- (Code =< 16'0C8C  ','  !  ','  ns3byte01011110(Code)).
ns3byte0101111(Code) :- ns3byte01011111(Code).
ns3byte01011110(Code) :- (Code =< 16'0C8C  ','  !  ','  Code >= 16'0C85).
ns3byte01011111(Code) :- (Code =< 16'0C90  ','  !  ','  Code >= 16'0C8E).
ns3byte011(Code) :- (Code =< 16'0D61  ','  !  ','  ns3byte0110(Code)).
ns3byte011(Code) :- ns3byte0111(Code).
ns3byte0110(Code) :- (Code =< 16'0CE1  ','  !  ','  ns3byte01100(Code)).
ns3byte0110(Code) :- ns3byte01101(Code).
ns3byte01100(Code) :- (Code =< 16'0CB3  ','  !  ','  ns3byte011000(Code)).
ns3byte01100(Code) :- ns3byte011001(Code).
ns3byte011000(Code) :- (Code =< 16'0CA8  ','  !  ','  ns3byte0110000(Code)).
ns3byte011000(Code) :- ns3byte0110001(Code).
ns3byte0110000(Code) :- (Code =< 16'0CA8  ','  !  ','  Code >= 16'0C92).
ns3byte0110001(Code) :- (Code =< 16'0CB3  ','  !  ','  Code >= 16'0CAA).
ns3byte011001(Code) :- (Code =< 16'0CB9  ','  !  ','  ns3byte0110010(Code)).
ns3byte011001(Code) :- ns3byte0110011(Code).
ns3byte0110010(Code) :- (Code =< 16'0CB9  ','  !  ','  Code >= 16'0CB5).
ns3byte0110011(Code) :- (Code =< 16'0CDE  ','  !  ','  ns3byte01100110(Code)).
ns3byte0110011(Code) :- ns3byte01100111(Code).
ns3byte01100110(16'0CDE) :- !.
ns3byte01100111(Code) :- (Code =< 16'0CE1  ','  !  ','  Code >= 16'0CE0).
ns3byte01101(Code) :- (Code =< 16'0D10  ','  !  ','  ns3byte011010(Code)).
ns3byte01101(Code) :- ns3byte011011(Code).
ns3byte011010(Code) :- (Code =< 16'0D0C  ','  !  ','  ns3byte0110100(Code)).
ns3byte011010(Code) :- ns3byte0110101(Code).
ns3byte0110100(Code) :- (Code =< 16'0D0C  ','  !  ','  Code >= 16'0D05).
ns3byte0110101(Code) :- (Code =< 16'0D10  ','  !  ','  Code >= 16'0D0E).
ns3byte011011(Code) :- (Code =< 16'0D28  ','  !  ','  ns3byte0110110(Code)).
ns3byte011011(Code) :- ns3byte0110111(Code).
ns3byte0110110(Code) :- (Code =< 16'0D28  ','  !  ','  Code >= 16'0D12).
ns3byte0110111(Code) :- (Code =< 16'0D39  ','  !  ','  ns3byte01101110(Code)).
ns3byte0110111(Code) :- ns3byte01101111(Code).
ns3byte01101110(Code) :- (Code =< 16'0D39  ','  !  ','  Code >= 16'0D2A).
ns3byte01101111(Code) :- (Code =< 16'0D61  ','  !  ','  Code >= 16'0D60).
ns3byte0111(Code) :- (Code =< 16'0E82  ','  !  ','  ns3byte01110(Code)).
ns3byte0111(Code) :- ns3byte01111(Code).
ns3byte01110(Code) :- (Code =< 16'0E30  ','  !  ','  ns3byte011100(Code)).
ns3byte01110(Code) :- ns3byte011101(Code).
ns3byte011100(Code) :- (Code =< 16'0E2E  ','  !  ','  ns3byte0111000(Code)).
ns3byte011100(Code) :- ns3byte0111001(Code).
ns3byte0111000(Code) :- (Code =< 16'0E2E  ','  !  ','  Code >= 16'0E01).
ns3byte0111001(16'0E30) :- !.
ns3byte011101(Code) :- (Code =< 16'0E33  ','  !  ','  ns3byte0111010(Code)).
ns3byte011101(Code) :- ns3byte0111011(Code).
ns3byte0111010(Code) :- (Code =< 16'0E33  ','  !  ','  Code >= 16'0E32).
ns3byte0111011(Code) :- (Code =< 16'0E45  ','  !  ','  ns3byte01110110(Code)).
ns3byte0111011(Code) :- ns3byte01110111(Code).
ns3byte01110110(Code) :- (Code =< 16'0E45  ','  !  ','  Code >= 16'0E40).
ns3byte01110111(Code) :- (Code =< 16'0E82  ','  !  ','  Code >= 16'0E81).
ns3byte01111(Code) :- (Code =< 16'0E88  ','  !  ','  ns3byte011110(Code)).
ns3byte01111(Code) :- ns3byte011111(Code).
ns3byte011110(Code) :- (Code =< 16'0E84  ','  !  ','  ns3byte0111100(Code)).
ns3byte011110(Code) :- ns3byte0111101(Code).
ns3byte0111100(16'0E84) :- !.
ns3byte0111101(Code) :- (Code =< 16'0E88  ','  !  ','  Code >= 16'0E87).
ns3byte011111(Code) :- (Code =< 16'0E8A  ','  !  ','  ns3byte0111110(Code)).
ns3byte011111(Code) :- ns3byte0111111(Code).
ns3byte0111110(16'0E8A) :- !.
ns3byte0111111(Code) :- (Code =< 16'0E8D  ','  !  ','  ns3byte01111110(Code)).
ns3byte0111111(Code) :- ns3byte01111111(Code).
ns3byte01111110(16'0E8D) :- !.
ns3byte01111111(Code) :- (Code =< 16'0E97  ','  !  ','  Code >= 16'0E94).
ns3byte1(Code) :- (Code =< 16'11AB  ','  !  ','  ns3byte10(Code)).
ns3byte1(Code) :- ns3byte11(Code).
ns3byte10(Code) :- (Code =< 16'110C  ','  !  ','  ns3byte100(Code)).
ns3byte10(Code) :- ns3byte101(Code).
ns3byte100(Code) :- (Code =< 16'0EBD  ','  !  ','  ns3byte1000(Code)).
ns3byte100(Code) :- ns3byte1001(Code).
ns3byte1000(Code) :- (Code =< 16'0EA7  ','  !  ','  ns3byte10000(Code)).
ns3byte1000(Code) :- ns3byte10001(Code).
ns3byte10000(Code) :- (Code =< 16'0EA3  ','  !  ','  ns3byte100000(Code)).
ns3byte10000(Code) :- ns3byte100001(Code).
ns3byte100000(Code) :- (Code =< 16'0E9F  ','  !  ','  ns3byte1000000(Code)).
ns3byte100000(Code) :- ns3byte1000001(Code).
ns3byte1000000(Code) :- (Code =< 16'0E9F  ','  !  ','  Code >= 16'0E99).
ns3byte1000001(Code) :- (Code =< 16'0EA3  ','  !  ','  Code >= 16'0EA1).
ns3byte100001(Code) :- (Code =< 16'0EA5  ','  !  ','  ns3byte1000010(Code)).
ns3byte100001(Code) :- ns3byte1000011(Code).
ns3byte1000010(16'0EA5) :- !.
ns3byte1000011(16'0EA7) :- !.
ns3byte10001(Code) :- (Code =< 16'0EAE  ','  !  ','  ns3byte100010(Code)).
ns3byte10001(Code) :- ns3byte100011(Code).
ns3byte100010(Code) :- (Code =< 16'0EAB  ','  !  ','  ns3byte1000100(Code)).
ns3byte100010(Code) :- ns3byte1000101(Code).
ns3byte1000100(Code) :- (Code =< 16'0EAB  ','  !  ','  Code >= 16'0EAA).
ns3byte1000101(Code) :- (Code =< 16'0EAE  ','  !  ','  Code >= 16'0EAD).
ns3byte100011(Code) :- (Code =< 16'0EB0  ','  !  ','  ns3byte1000110(Code)).
ns3byte100011(Code) :- ns3byte1000111(Code).
ns3byte1000110(16'0EB0) :- !.
ns3byte1000111(Code) :- (Code =< 16'0EB3  ','  !  ','  ns3byte10001110(Code)).
ns3byte1000111(Code) :- ns3byte10001111(Code).
ns3byte10001110(Code) :- (Code =< 16'0EB3  ','  !  ','  Code >= 16'0EB2).
ns3byte10001111(16'0EBD) :- !.
ns3byte1001(Code) :- (Code =< 16'10F6  ','  !  ','  ns3byte10010(Code)).
ns3byte1001(Code) :- ns3byte10011(Code).
ns3byte10010(Code) :- (Code =< 16'0F47  ','  !  ','  ns3byte100100(Code)).
ns3byte10010(Code) :- ns3byte100101(Code).
ns3byte100100(Code) :- (Code =< 16'0EC4  ','  !  ','  ns3byte1001000(Code)).
ns3byte100100(Code) :- ns3byte1001001(Code).
ns3byte1001000(Code) :- (Code =< 16'0EC4  ','  !  ','  Code >= 16'0EC0).
ns3byte1001001(Code) :- (Code =< 16'0F47  ','  !  ','  Code >= 16'0F40).
ns3byte100101(Code) :- (Code =< 16'0F69  ','  !  ','  ns3byte1001010(Code)).
ns3byte100101(Code) :- ns3byte1001011(Code).
ns3byte1001010(Code) :- (Code =< 16'0F69  ','  !  ','  Code >= 16'0F49).
ns3byte1001011(Code) :- (Code =< 16'10C5  ','  !  ','  ns3byte10010110(Code)).
ns3byte1001011(Code) :- ns3byte10010111(Code).
ns3byte10010110(Code) :- (Code =< 16'10C5  ','  !  ','  Code >= 16'10A0).
ns3byte10010111(Code) :- (Code =< 16'10F6  ','  !  ','  Code >= 16'10D0).
ns3byte10011(Code) :- (Code =< 16'1103  ','  !  ','  ns3byte100110(Code)).
ns3byte10011(Code) :- ns3byte100111(Code).
ns3byte100110(Code) :- (Code =< 16'1100  ','  !  ','  ns3byte1001100(Code)).
ns3byte100110(Code) :- ns3byte1001101(Code).
ns3byte1001100(16'1100) :- !.
ns3byte1001101(Code) :- (Code =< 16'1103  ','  !  ','  Code >= 16'1102).
ns3byte100111(Code) :- (Code =< 16'1107  ','  !  ','  ns3byte1001110(Code)).
ns3byte100111(Code) :- ns3byte1001111(Code).
ns3byte1001110(Code) :- (Code =< 16'1107  ','  !  ','  Code >= 16'1105).
ns3byte1001111(Code) :- (Code =< 16'1109  ','  !  ','  ns3byte10011110(Code)).
ns3byte1001111(Code) :- ns3byte10011111(Code).
ns3byte10011110(16'1109) :- !.
ns3byte10011111(Code) :- (Code =< 16'110C  ','  !  ','  Code >= 16'110B).
ns3byte101(Code) :- (Code =< 16'1161  ','  !  ','  ns3byte1010(Code)).
ns3byte101(Code) :- ns3byte1011(Code).
ns3byte1010(Code) :- (Code =< 16'114C  ','  !  ','  ns3byte10100(Code)).
ns3byte1010(Code) :- ns3byte10101(Code).
ns3byte10100(Code) :- (Code =< 16'113C  ','  !  ','  ns3byte101000(Code)).
ns3byte10100(Code) :- ns3byte101001(Code).
ns3byte101000(Code) :- (Code =< 16'1112  ','  !  ','  ns3byte1010000(Code)).
ns3byte101000(Code) :- ns3byte1010001(Code).
ns3byte1010000(Code) :- (Code =< 16'1112  ','  !  ','  Code >= 16'110E).
ns3byte1010001(16'113C) :- !.
ns3byte101001(Code) :- (Code =< 16'113E  ','  !  ','  ns3byte1010010(Code)).
ns3byte101001(Code) :- ns3byte1010011(Code).
ns3byte1010010(16'113E) :- !.
ns3byte1010011(Code) :- (Code =< 16'1140  ','  !  ','  ns3byte10100110(Code)).
ns3byte1010011(Code) :- ns3byte10100111(Code).
ns3byte10100110(16'1140) :- !.
ns3byte10100111(16'114C) :- !.
ns3byte10101(Code) :- (Code =< 16'1150  ','  !  ','  ns3byte101010(Code)).
ns3byte10101(Code) :- ns3byte101011(Code).
ns3byte101010(Code) :- (Code =< 16'114E  ','  !  ','  ns3byte1010100(Code)).
ns3byte101010(Code) :- ns3byte1010101(Code).
ns3byte1010100(16'114E) :- !.
ns3byte1010101(16'1150) :- !.
ns3byte101011(Code) :- (Code =< 16'1155  ','  !  ','  ns3byte1010110(Code)).
ns3byte101011(Code) :- ns3byte1010111(Code).
ns3byte1010110(Code) :- (Code =< 16'1155  ','  !  ','  Code >= 16'1154).
ns3byte1010111(Code) :- (Code =< 16'1159  ','  !  ','  ns3byte10101110(Code)).
ns3byte1010111(Code) :- ns3byte10101111(Code).
ns3byte10101110(16'1159) :- !.
ns3byte10101111(Code) :- (Code =< 16'1161  ','  !  ','  Code >= 16'115F).
ns3byte1011(Code) :- (Code =< 16'116E  ','  !  ','  ns3byte10110(Code)).
ns3byte1011(Code) :- ns3byte10111(Code).
ns3byte10110(Code) :- (Code =< 16'1165  ','  !  ','  ns3byte101100(Code)).
ns3byte10110(Code) :- ns3byte101101(Code).
ns3byte101100(Code) :- (Code =< 16'1163  ','  !  ','  ns3byte1011000(Code)).
ns3byte101100(Code) :- ns3byte1011001(Code).
ns3byte1011000(16'1163) :- !.
ns3byte1011001(16'1165) :- !.
ns3byte101101(Code) :- (Code =< 16'1167  ','  !  ','  ns3byte1011010(Code)).
ns3byte101101(Code) :- ns3byte1011011(Code).
ns3byte1011010(16'1167) :- !.
ns3byte1011011(Code) :- (Code =< 16'1169  ','  !  ','  ns3byte10110110(Code)).
ns3byte1011011(Code) :- ns3byte10110111(Code).
ns3byte10110110(16'1169) :- !.
ns3byte10110111(Code) :- (Code =< 16'116E  ','  !  ','  Code >= 16'116D).
ns3byte10111(Code) :- (Code =< 16'1175  ','  !  ','  ns3byte101110(Code)).
ns3byte10111(Code) :- ns3byte101111(Code).
ns3byte101110(Code) :- (Code =< 16'1173  ','  !  ','  ns3byte1011100(Code)).
ns3byte101110(Code) :- ns3byte1011101(Code).
ns3byte1011100(Code) :- (Code =< 16'1173  ','  !  ','  Code >= 16'1172).
ns3byte1011101(16'1175) :- !.
ns3byte101111(Code) :- (Code =< 16'119E  ','  !  ','  ns3byte1011110(Code)).
ns3byte101111(Code) :- ns3byte1011111(Code).
ns3byte1011110(16'119E) :- !.
ns3byte1011111(Code) :- (Code =< 16'11A8  ','  !  ','  ns3byte10111110(Code)).
ns3byte1011111(Code) :- ns3byte10111111(Code).
ns3byte10111110(16'11A8) :- !.
ns3byte10111111(16'11AB) :- !.
ns3byte11(Code) :- (Code =< 16'1FB4  ','  !  ','  ns3byte110(Code)).
ns3byte11(Code) :- ns3byte111(Code).
ns3byte110(Code) :- (Code =< 16'1EF9  ','  !  ','  ns3byte1100(Code)).
ns3byte110(Code) :- ns3byte1101(Code).
ns3byte1100(Code) :- (Code =< 16'11C2  ','  !  ','  ns3byte11000(Code)).
ns3byte1100(Code) :- ns3byte11001(Code).
ns3byte11000(Code) :- (Code =< 16'11B8  ','  !  ','  ns3byte110000(Code)).
ns3byte11000(Code) :- ns3byte110001(Code).
ns3byte110000(Code) :- (Code =< 16'11AF  ','  !  ','  ns3byte1100000(Code)).
ns3byte110000(Code) :- ns3byte1100001(Code).
ns3byte1100000(Code) :- (Code =< 16'11AF  ','  !  ','  Code >= 16'11AE).
ns3byte1100001(Code) :- (Code =< 16'11B8  ','  !  ','  Code >= 16'11B7).
ns3byte110001(Code) :- (Code =< 16'11BA  ','  !  ','  ns3byte1100010(Code)).
ns3byte110001(Code) :- ns3byte1100011(Code).
ns3byte1100010(16'11BA) :- !.
ns3byte1100011(Code) :- (Code =< 16'11C2  ','  !  ','  Code >= 16'11BC).
ns3byte11001(Code) :- (Code =< 16'11F0  ','  !  ','  ns3byte110010(Code)).
ns3byte11001(Code) :- ns3byte110011(Code).
ns3byte110010(Code) :- (Code =< 16'11EB  ','  !  ','  ns3byte1100100(Code)).
ns3byte110010(Code) :- ns3byte1100101(Code).
ns3byte1100100(16'11EB) :- !.
ns3byte1100101(16'11F0) :- !.
ns3byte110011(Code) :- (Code =< 16'11F9  ','  !  ','  ns3byte1100110(Code)).
ns3byte110011(Code) :- ns3byte1100111(Code).
ns3byte1100110(16'11F9) :- !.
ns3byte1100111(Code) :- (Code =< 16'1E9B  ','  !  ','  ns3byte11001110(Code)).
ns3byte1100111(Code) :- ns3byte11001111(Code).
ns3byte11001110(Code) :- (Code =< 16'1E9B  ','  !  ','  Code >= 16'1E00).
ns3byte11001111(Code) :- (Code =< 16'1EF9  ','  !  ','  Code >= 16'1EA0).
ns3byte1101(Code) :- (Code =< 16'1F57  ','  !  ','  ns3byte11010(Code)).
ns3byte1101(Code) :- ns3byte11011(Code).
ns3byte11010(Code) :- (Code =< 16'1F1D  ','  !  ','  ns3byte110100(Code)).
ns3byte11010(Code) :- ns3byte110101(Code).
ns3byte110100(Code) :- (Code =< 16'1F15  ','  !  ','  ns3byte1101000(Code)).
ns3byte110100(Code) :- ns3byte1101001(Code).
ns3byte1101000(Code) :- (Code =< 16'1F15  ','  !  ','  Code >= 16'1F00).
ns3byte1101001(Code) :- (Code =< 16'1F1D  ','  !  ','  Code >= 16'1F18).
ns3byte110101(Code) :- (Code =< 16'1F45  ','  !  ','  ns3byte1101010(Code)).
ns3byte110101(Code) :- ns3byte1101011(Code).
ns3byte1101010(Code) :- (Code =< 16'1F45  ','  !  ','  Code >= 16'1F20).
ns3byte1101011(Code) :- (Code =< 16'1F4D  ','  !  ','  ns3byte11010110(Code)).
ns3byte1101011(Code) :- ns3byte11010111(Code).
ns3byte11010110(Code) :- (Code =< 16'1F4D  ','  !  ','  Code >= 16'1F48).
ns3byte11010111(Code) :- (Code =< 16'1F57  ','  !  ','  Code >= 16'1F50).
ns3byte11011(Code) :- (Code =< 16'1F5B  ','  !  ','  ns3byte110110(Code)).
ns3byte11011(Code) :- ns3byte110111(Code).
ns3byte110110(Code) :- (Code =< 16'1F59  ','  !  ','  ns3byte1101100(Code)).
ns3byte110110(Code) :- ns3byte1101101(Code).
ns3byte1101100(16'1F59) :- !.
ns3byte1101101(16'1F5B) :- !.
ns3byte110111(Code) :- (Code =< 16'1F5D  ','  !  ','  ns3byte1101110(Code)).
ns3byte110111(Code) :- ns3byte1101111(Code).
ns3byte1101110(16'1F5D) :- !.
ns3byte1101111(Code) :- (Code =< 16'1F7D  ','  !  ','  ns3byte11011110(Code)).
ns3byte1101111(Code) :- ns3byte11011111(Code).
ns3byte11011110(Code) :- (Code =< 16'1F7D  ','  !  ','  Code >= 16'1F5F).
ns3byte11011111(Code) :- (Code =< 16'1FB4  ','  !  ','  Code >= 16'1F80).
ns3byte111(Code) :- (Code =< 16'2126  ','  !  ','  ns3byte1110(Code)).
ns3byte111(Code) :- ns3byte1111(Code).
ns3byte1110(Code) :- (Code =< 16'1FD3  ','  !  ','  ns3byte11100(Code)).
ns3byte1110(Code) :- ns3byte11101(Code).
ns3byte11100(Code) :- (Code =< 16'1FBE  ','  !  ','  ns3byte111000(Code)).
ns3byte11100(Code) :- ns3byte111001(Code).
ns3byte111000(Code) :- (Code =< 16'1FBC  ','  !  ','  ns3byte1110000(Code)).
ns3byte111000(Code) :- ns3byte1110001(Code).
ns3byte1110000(Code) :- (Code =< 16'1FBC  ','  !  ','  Code >= 16'1FB6).
ns3byte1110001(16'1FBE) :- !.
ns3byte111001(Code) :- (Code =< 16'1FC4  ','  !  ','  ns3byte1110010(Code)).
ns3byte111001(Code) :- ns3byte1110011(Code).
ns3byte1110010(Code) :- (Code =< 16'1FC4  ','  !  ','  Code >= 16'1FC2).
ns3byte1110011(Code) :- (Code =< 16'1FCC  ','  !  ','  ns3byte11100110(Code)).
ns3byte1110011(Code) :- ns3byte11100111(Code).
ns3byte11100110(Code) :- (Code =< 16'1FCC  ','  !  ','  Code >= 16'1FC6).
ns3byte11100111(Code) :- (Code =< 16'1FD3  ','  !  ','  Code >= 16'1FD0).
ns3byte11101(Code) :- (Code =< 16'1FEC  ','  !  ','  ns3byte111010(Code)).
ns3byte11101(Code) :- ns3byte111011(Code).
ns3byte111010(Code) :- (Code =< 16'1FDB  ','  !  ','  ns3byte1110100(Code)).
ns3byte111010(Code) :- ns3byte1110101(Code).
ns3byte1110100(Code) :- (Code =< 16'1FDB  ','  !  ','  Code >= 16'1FD6).
ns3byte1110101(Code) :- (Code =< 16'1FEC  ','  !  ','  Code >= 16'1FE0).
ns3byte111011(Code) :- (Code =< 16'1FF4  ','  !  ','  ns3byte1110110(Code)).
ns3byte111011(Code) :- ns3byte1110111(Code).
ns3byte1110110(Code) :- (Code =< 16'1FF4  ','  !  ','  Code >= 16'1FF2).
ns3byte1110111(Code) :- (Code =< 16'1FFC  ','  !  ','  ns3byte11101110(Code)).
ns3byte1110111(Code) :- ns3byte11101111(Code).
ns3byte11101110(Code) :- (Code =< 16'1FFC  ','  !  ','  Code >= 16'1FF6).
ns3byte11101111(16'2126) :- !.
ns3byte1111(Code) :- (Code =< 16'3029  ','  !  ','  ns3byte11110(Code)).
ns3byte1111(Code) :- ns3byte11111(Code).
ns3byte11110(Code) :- (Code =< 16'212E  ','  !  ','  ns3byte111100(Code)).
ns3byte11110(Code) :- ns3byte111101(Code).
ns3byte111100(Code) :- (Code =< 16'212B  ','  !  ','  ns3byte1111000(Code)).
ns3byte111100(Code) :- ns3byte1111001(Code).
ns3byte1111000(Code) :- (Code =< 16'212B  ','  !  ','  Code >= 16'212A).
ns3byte1111001(16'212E) :- !.
ns3byte111101(Code) :- (Code =< 16'2182  ','  !  ','  ns3byte1111010(Code)).
ns3byte111101(Code) :- ns3byte1111011(Code).
ns3byte1111010(Code) :- (Code =< 16'2182  ','  !  ','  Code >= 16'2180).
ns3byte1111011(Code) :- (Code =< 16'3007  ','  !  ','  ns3byte11110110(Code)).
ns3byte1111011(Code) :- ns3byte11110111(Code).
ns3byte11110110(16'3007) :- !.
ns3byte11110111(Code) :- (Code =< 16'3029  ','  !  ','  Code >= 16'3021).
ns3byte11111(Code) :- (Code =< 16'30FA  ','  !  ','  ns3byte111110(Code)).
ns3byte11111(Code) :- ns3byte111111(Code).
ns3byte111110(Code) :- (Code =< 16'3094  ','  !  ','  ns3byte1111100(Code)).
ns3byte111110(Code) :- ns3byte1111101(Code).
ns3byte1111100(Code) :- (Code =< 16'3094  ','  !  ','  Code >= 16'3041).
ns3byte1111101(Code) :- (Code =< 16'30FA  ','  !  ','  Code >= 16'30A1).
ns3byte111111(Code) :- (Code =< 16'312C  ','  !  ','  ns3byte1111110(Code)).
ns3byte111111(Code) :- ns3byte1111111(Code).
ns3byte1111110(Code) :- (Code =< 16'312C  ','  !  ','  Code >= 16'3105).
ns3byte1111111(Code) :- (Code =< 16'9FA5  ','  !  ','  ns3byte11111110(Code)).
ns3byte1111111(Code) :- ns3byte11111111(Code).
ns3byte11111110(Code) :- (Code =< 16'9FA5  ','  !  ','  Code >= 16'4E00).
ns3byte11111111(Code) :- (Code =< 16'D7A3  ','  !  ','  Code >= 16'AC00).


ncnamechar1byte( 0'_ ) --> [0'_], !.
ncnamechar1byte( 0'. ) --> [0'.], !.
ncnamechar1byte( 0'- ) --> [0'-], !.
ncnamechar1byte( 0'0 ) --> [0'0], !.
ncnamechar1byte( 0'1 ) --> [0'1], !.
ncnamechar1byte( 0'2 ) --> [0'2], !.
ncnamechar1byte( 0'3 ) --> [0'3], !.
ncnamechar1byte( 0'4 ) --> [0'4], !.
ncnamechar1byte( 0'5 ) --> [0'5], !.
ncnamechar1byte( 0'6 ) --> [0'6], !.
ncnamechar1byte( 0'7 ) --> [0'7], !.
ncnamechar1byte( 0'8 ) --> [0'8], !.
ncnamechar1byte( 0'9 ) --> [0'9], !.
ncnamechar1byte( 0'A ) --> [0'A], !.
ncnamechar1byte( 0'B ) --> [0'B], !.
ncnamechar1byte( 0'C ) --> [0'C], !.
ncnamechar1byte( 0'D ) --> [0'D], !.
ncnamechar1byte( 0'E ) --> [0'E], !.
ncnamechar1byte( 0'F ) --> [0'F], !.
ncnamechar1byte( 0'G ) --> [0'G], !.
ncnamechar1byte( 0'H ) --> [0'H], !.
ncnamechar1byte( 0'I ) --> [0'I], !.
ncnamechar1byte( 0'J ) --> [0'J], !.
ncnamechar1byte( 0'K ) --> [0'K], !.
ncnamechar1byte( 0'L ) --> [0'L], !.
ncnamechar1byte( 0'M ) --> [0'M], !.
ncnamechar1byte( 0'N ) --> [0'N], !.
ncnamechar1byte( 0'O ) --> [0'O], !.
ncnamechar1byte( 0'P ) --> [0'P], !.
ncnamechar1byte( 0'Q ) --> [0'Q], !.
ncnamechar1byte( 0'R ) --> [0'R], !.
ncnamechar1byte( 0'S ) --> [0'S], !.
ncnamechar1byte( 0'T ) --> [0'T], !.
ncnamechar1byte( 0'U ) --> [0'U], !.
ncnamechar1byte( 0'V ) --> [0'V], !.
ncnamechar1byte( 0'W ) --> [0'W], !.
ncnamechar1byte( 0'X ) --> [0'X], !.
ncnamechar1byte( 0'Y ) --> [0'Y], !.
ncnamechar1byte( 0'Z ) --> [0'Z], !.
ncnamechar1byte( 0'a ) --> [0'a], !.
ncnamechar1byte( 0'b ) --> [0'b], !.
ncnamechar1byte( 0'c ) --> [0'c], !.
ncnamechar1byte( 0'd ) --> [0'd], !.
ncnamechar1byte( 0'e ) --> [0'e], !.
ncnamechar1byte( 0'f ) --> [0'f], !.
ncnamechar1byte( 0'g ) --> [0'g], !.
ncnamechar1byte( 0'h ) --> [0'h], !.
ncnamechar1byte( 0'i ) --> [0'i], !.
ncnamechar1byte( 0'j ) --> [0'j], !.
ncnamechar1byte( 0'k ) --> [0'k], !.
ncnamechar1byte( 0'l ) --> [0'l], !.
ncnamechar1byte( 0'm ) --> [0'm], !.
ncnamechar1byte( 0'n ) --> [0'n], !.
ncnamechar1byte( 0'o ) --> [0'o], !.
ncnamechar1byte( 0'p ) --> [0'p], !.
ncnamechar1byte( 0'q ) --> [0'q], !.
ncnamechar1byte( 0'r ) --> [0'r], !.
ncnamechar1byte( 0's ) --> [0's], !.
ncnamechar1byte( 0't ) --> [0't], !.
ncnamechar1byte( 0'u ) --> [0'u], !.
ncnamechar1byte( 0'v ) --> [0'v], !.
ncnamechar1byte( 0'w ) --> [0'w], !.
ncnamechar1byte( 0'x ) --> [0'x], !.
ncnamechar1byte( 0'y ) --> [0'y], !.
ncnamechar1byte( 0'z ) --> [0'z], !.

ncnamechar2byte( Code ) --> [Code], { Code < 16'800, !, n2byte( Code ) }.
ncnamechar3byte( Code ) --> [Code], { Code < 16'10000, !, n3byte( Code ) }.


n2byte(Code) :- (Code =< 16'044F  ','  !  ','  n2byte0(Code)).
n2byte(Code) :- n2byte1(Code).
n2byte0(Code) :- (Code =< 16'02D0  ','  !  ','  n2byte00(Code)).
n2byte0(Code) :- n2byte01(Code).
n2byte00(Code) :- (Code =< 16'0148  ','  !  ','  n2byte000(Code)).
n2byte00(Code) :- n2byte001(Code).
n2byte000(Code) :- (Code =< 16'00F6  ','  !  ','  n2byte0000(Code)).
n2byte000(Code) :- n2byte0001(Code).
n2byte0000(Code) :- (Code =< 16'00B7  ','  !  ','  n2byte00000(Code)).
n2byte0000(Code) :- n2byte00001(Code).
n2byte00000(16'00B7) :- !.
n2byte00001(Code) :- (Code =< 16'00D6  ','  !  ','  n2byte000010(Code)).
n2byte00001(Code) :- n2byte000011(Code).
n2byte000010(Code) :- (Code =< 16'00D6  ','  !  ','  Code >= 16'00C0).
n2byte000011(Code) :- (Code =< 16'00F6  ','  !  ','  Code >= 16'00D8).
n2byte0001(Code) :- (Code =< 16'0131  ','  !  ','  n2byte00010(Code)).
n2byte0001(Code) :- n2byte00011(Code).
n2byte00010(Code) :- (Code =< 16'00FF  ','  !  ','  n2byte000100(Code)).
n2byte00010(Code) :- n2byte000101(Code).
n2byte000100(Code) :- (Code =< 16'00FF  ','  !  ','  Code >= 16'00F8).
n2byte000101(Code) :- (Code =< 16'0131  ','  !  ','  Code >= 16'0100).
n2byte00011(Code) :- (Code =< 16'013E  ','  !  ','  n2byte000110(Code)).
n2byte00011(Code) :- n2byte000111(Code).
n2byte000110(Code) :- (Code =< 16'013E  ','  !  ','  Code >= 16'0134).
n2byte000111(Code) :- (Code =< 16'0148  ','  !  ','  Code >= 16'0141).
n2byte001(Code) :- (Code =< 16'01F5  ','  !  ','  n2byte0010(Code)).
n2byte001(Code) :- n2byte0011(Code).
n2byte0010(Code) :- (Code =< 16'01C3  ','  !  ','  n2byte00100(Code)).
n2byte0010(Code) :- n2byte00101(Code).
n2byte00100(Code) :- (Code =< 16'017E  ','  !  ','  n2byte001000(Code)).
n2byte00100(Code) :- n2byte001001(Code).
n2byte001000(Code) :- (Code =< 16'017E  ','  !  ','  Code >= 16'014A).
n2byte001001(Code) :- (Code =< 16'01C3  ','  !  ','  Code >= 16'0180).
n2byte00101(Code) :- (Code =< 16'01F0  ','  !  ','  n2byte001010(Code)).
n2byte00101(Code) :- n2byte001011(Code).
n2byte001010(Code) :- (Code =< 16'01F0  ','  !  ','  Code >= 16'01CD).
n2byte001011(Code) :- (Code =< 16'01F5  ','  !  ','  Code >= 16'01F4).
n2byte0011(Code) :- (Code =< 16'02A8  ','  !  ','  n2byte00110(Code)).
n2byte0011(Code) :- n2byte00111(Code).
n2byte00110(Code) :- (Code =< 16'0217  ','  !  ','  n2byte001100(Code)).
n2byte00110(Code) :- n2byte001101(Code).
n2byte001100(Code) :- (Code =< 16'0217  ','  !  ','  Code >= 16'01FA).
n2byte001101(Code) :- (Code =< 16'02A8  ','  !  ','  Code >= 16'0250).
n2byte00111(Code) :- (Code =< 16'02C1  ','  !  ','  n2byte001110(Code)).
n2byte00111(Code) :- n2byte001111(Code).
n2byte001110(Code) :- (Code =< 16'02C1  ','  !  ','  Code >= 16'02BB).
n2byte001111(16'02D0) :- !.
n2byte01(Code) :- (Code =< 16'03CE  ','  !  ','  n2byte010(Code)).
n2byte01(Code) :- n2byte011(Code).
n2byte010(Code) :- (Code =< 16'0361  ','  !  ','  n2byte0100(Code)).
n2byte010(Code) :- n2byte0101(Code).
n2byte0100(Code) :- (Code =< 16'02D1  ','  !  ','  n2byte01000(Code)).
n2byte0100(Code) :- n2byte01001(Code).
n2byte01000(16'02D1) :- !.
n2byte01001(Code) :- (Code =< 16'0345  ','  !  ','  n2byte010010(Code)).
n2byte01001(Code) :- n2byte010011(Code).
n2byte010010(Code) :- (Code =< 16'0345  ','  !  ','  Code >= 16'0300).
n2byte010011(Code) :- (Code =< 16'0361  ','  !  ','  Code >= 16'0360).
n2byte0101(Code) :- (Code =< 16'038C  ','  !  ','  n2byte01010(Code)).
n2byte0101(Code) :- n2byte01011(Code).
n2byte01010(Code) :- (Code =< 16'038A  ','  !  ','  n2byte010100(Code)).
n2byte01010(Code) :- n2byte010101(Code).
n2byte010100(Code) :- (Code =< 16'038A  ','  !  ','  Code >= 16'0386).
n2byte010101(16'038C) :- !.
n2byte01011(Code) :- (Code =< 16'03A1  ','  !  ','  n2byte010110(Code)).
n2byte01011(Code) :- n2byte010111(Code).
n2byte010110(Code) :- (Code =< 16'03A1  ','  !  ','  Code >= 16'038E).
n2byte010111(Code) :- (Code =< 16'03CE  ','  !  ','  Code >= 16'03A3).
n2byte011(Code) :- (Code =< 16'03DE  ','  !  ','  n2byte0110(Code)).
n2byte011(Code) :- n2byte0111(Code).
n2byte0110(Code) :- (Code =< 16'03DA  ','  !  ','  n2byte01100(Code)).
n2byte0110(Code) :- n2byte01101(Code).
n2byte01100(Code) :- (Code =< 16'03D6  ','  !  ','  n2byte011000(Code)).
n2byte01100(Code) :- n2byte011001(Code).
n2byte011000(Code) :- (Code =< 16'03D6  ','  !  ','  Code >= 16'03D0).
n2byte011001(16'03DA) :- !.
n2byte01101(Code) :- (Code =< 16'03DC  ','  !  ','  n2byte011010(Code)).
n2byte01101(Code) :- n2byte011011(Code).
n2byte011010(16'03DC) :- !.
n2byte011011(16'03DE) :- !.
n2byte0111(Code) :- (Code =< 16'03F3  ','  !  ','  n2byte01110(Code)).
n2byte0111(Code) :- n2byte01111(Code).
n2byte01110(Code) :- (Code =< 16'03E0  ','  !  ','  n2byte011100(Code)).
n2byte01110(Code) :- n2byte011101(Code).
n2byte011100(16'03E0) :- !.
n2byte011101(Code) :- (Code =< 16'03F3  ','  !  ','  Code >= 16'03E2).
n2byte01111(Code) :- (Code =< 16'040C  ','  !  ','  n2byte011110(Code)).
n2byte01111(Code) :- n2byte011111(Code).
n2byte011110(Code) :- (Code =< 16'040C  ','  !  ','  Code >= 16'0401).
n2byte011111(Code) :- (Code =< 16'044F  ','  !  ','  Code >= 16'040E).
n2byte1(Code) :- (Code =< 16'05BD  ','  !  ','  n2byte10(Code)).
n2byte1(Code) :- n2byte11(Code).
n2byte10(Code) :- (Code =< 16'04EB  ','  !  ','  n2byte100(Code)).
n2byte10(Code) :- n2byte101(Code).
n2byte100(Code) :- (Code =< 16'0486  ','  !  ','  n2byte1000(Code)).
n2byte100(Code) :- n2byte1001(Code).
n2byte1000(Code) :- (Code =< 16'045C  ','  !  ','  n2byte10000(Code)).
n2byte1000(Code) :- n2byte10001(Code).
n2byte10000(Code) :- (Code =< 16'045C  ','  !  ','  Code >= 16'0451).
n2byte10001(Code) :- (Code =< 16'0481  ','  !  ','  n2byte100010(Code)).
n2byte10001(Code) :- n2byte100011(Code).
n2byte100010(Code) :- (Code =< 16'0481  ','  !  ','  Code >= 16'045E).
n2byte100011(Code) :- (Code =< 16'0486  ','  !  ','  Code >= 16'0483).
n2byte1001(Code) :- (Code =< 16'04C8  ','  !  ','  n2byte10010(Code)).
n2byte1001(Code) :- n2byte10011(Code).
n2byte10010(Code) :- (Code =< 16'04C4  ','  !  ','  n2byte100100(Code)).
n2byte10010(Code) :- n2byte100101(Code).
n2byte100100(Code) :- (Code =< 16'04C4  ','  !  ','  Code >= 16'0490).
n2byte100101(Code) :- (Code =< 16'04C8  ','  !  ','  Code >= 16'04C7).
n2byte10011(Code) :- (Code =< 16'04CC  ','  !  ','  n2byte100110(Code)).
n2byte10011(Code) :- n2byte100111(Code).
n2byte100110(Code) :- (Code =< 16'04CC  ','  !  ','  Code >= 16'04CB).
n2byte100111(Code) :- (Code =< 16'04EB  ','  !  ','  Code >= 16'04D0).
n2byte101(Code) :- (Code =< 16'0559  ','  !  ','  n2byte1010(Code)).
n2byte101(Code) :- n2byte1011(Code).
n2byte1010(Code) :- (Code =< 16'04F9  ','  !  ','  n2byte10100(Code)).
n2byte1010(Code) :- n2byte10101(Code).
n2byte10100(Code) :- (Code =< 16'04F5  ','  !  ','  n2byte101000(Code)).
n2byte10100(Code) :- n2byte101001(Code).
n2byte101000(Code) :- (Code =< 16'04F5  ','  !  ','  Code >= 16'04EE).
n2byte101001(Code) :- (Code =< 16'04F9  ','  !  ','  Code >= 16'04F8).
n2byte10101(Code) :- (Code =< 16'0556  ','  !  ','  n2byte101010(Code)).
n2byte10101(Code) :- n2byte101011(Code).
n2byte101010(Code) :- (Code =< 16'0556  ','  !  ','  Code >= 16'0531).
n2byte101011(16'0559) :- !.
n2byte1011(Code) :- (Code =< 16'05A1  ','  !  ','  n2byte10110(Code)).
n2byte1011(Code) :- n2byte10111(Code).
n2byte10110(Code) :- (Code =< 16'0586  ','  !  ','  n2byte101100(Code)).
n2byte10110(Code) :- n2byte101101(Code).
n2byte101100(Code) :- (Code =< 16'0586  ','  !  ','  Code >= 16'0561).
n2byte101101(Code) :- (Code =< 16'05A1  ','  !  ','  Code >= 16'0591).
n2byte10111(Code) :- (Code =< 16'05B9  ','  !  ','  n2byte101110(Code)).
n2byte10111(Code) :- n2byte101111(Code).
n2byte101110(Code) :- (Code =< 16'05B9  ','  !  ','  Code >= 16'05A3).
n2byte101111(Code) :- (Code =< 16'05BD  ','  !  ','  Code >= 16'05BB).
n2byte11(Code) :- (Code =< 16'0652  ','  !  ','  n2byte110(Code)).
n2byte11(Code) :- n2byte111(Code).
n2byte110(Code) :- (Code =< 16'05C4  ','  !  ','  n2byte1100(Code)).
n2byte110(Code) :- n2byte1101(Code).
n2byte1100(Code) :- (Code =< 16'05BF  ','  !  ','  n2byte11000(Code)).
n2byte1100(Code) :- n2byte11001(Code).
n2byte11000(16'05BF) :- !.
n2byte11001(Code) :- (Code =< 16'05C2  ','  !  ','  n2byte110010(Code)).
n2byte11001(Code) :- n2byte110011(Code).
n2byte110010(Code) :- (Code =< 16'05C2  ','  !  ','  Code >= 16'05C1).
n2byte110011(16'05C4) :- !.
n2byte1101(Code) :- (Code =< 16'05F2  ','  !  ','  n2byte11010(Code)).
n2byte1101(Code) :- n2byte11011(Code).
n2byte11010(Code) :- (Code =< 16'05EA  ','  !  ','  n2byte110100(Code)).
n2byte11010(Code) :- n2byte110101(Code).
n2byte110100(Code) :- (Code =< 16'05EA  ','  !  ','  Code >= 16'05D0).
n2byte110101(Code) :- (Code =< 16'05F2  ','  !  ','  Code >= 16'05F0).
n2byte11011(Code) :- (Code =< 16'063A  ','  !  ','  n2byte110110(Code)).
n2byte11011(Code) :- n2byte110111(Code).
n2byte110110(Code) :- (Code =< 16'063A  ','  !  ','  Code >= 16'0621).
n2byte110111(Code) :- (Code =< 16'0652  ','  !  ','  Code >= 16'0640).
n2byte111(Code) :- (Code =< 16'06CE  ','  !  ','  n2byte1110(Code)).
n2byte111(Code) :- n2byte1111(Code).
n2byte1110(Code) :- (Code =< 16'06B7  ','  !  ','  n2byte11100(Code)).
n2byte1110(Code) :- n2byte11101(Code).
n2byte11100(Code) :- (Code =< 16'0669  ','  !  ','  n2byte111000(Code)).
n2byte11100(Code) :- n2byte111001(Code).
n2byte111000(Code) :- (Code =< 16'0669  ','  !  ','  Code >= 16'0660).
n2byte111001(Code) :- (Code =< 16'06B7  ','  !  ','  Code >= 16'0670).
n2byte11101(Code) :- (Code =< 16'06BE  ','  !  ','  n2byte111010(Code)).
n2byte11101(Code) :- n2byte111011(Code).
n2byte111010(Code) :- (Code =< 16'06BE  ','  !  ','  Code >= 16'06BA).
n2byte111011(Code) :- (Code =< 16'06CE  ','  !  ','  Code >= 16'06C0).
n2byte1111(Code) :- (Code =< 16'06E8  ','  !  ','  n2byte11110(Code)).
n2byte1111(Code) :- n2byte11111(Code).
n2byte11110(Code) :- (Code =< 16'06D3  ','  !  ','  n2byte111100(Code)).
n2byte11110(Code) :- n2byte111101(Code).
n2byte111100(Code) :- (Code =< 16'06D3  ','  !  ','  Code >= 16'06D0).
n2byte111101(Code) :- (Code =< 16'06E8  ','  !  ','  Code >= 16'06D5).
n2byte11111(Code) :- (Code =< 16'06ED  ','  !  ','  n2byte111110(Code)).
n2byte11111(Code) :- n2byte111111(Code).
n2byte111110(Code) :- (Code =< 16'06ED  ','  !  ','  Code >= 16'06EA).
n2byte111111(Code) :- (Code =< 16'06F9  ','  !  ','  Code >= 16'06F0).

n3byte(Code) :- (Code =< 16'0D48  ','  !  ','  n3byte0(Code)).
n3byte(Code) :- n3byte1(Code).
n3byte0(Code) :- (Code =< 16'0B30  ','  !  ','  n3byte00(Code)).
n3byte0(Code) :- n3byte01(Code).
n3byte00(Code) :- (Code =< 16'0A30  ','  !  ','  n3byte000(Code)).
n3byte00(Code) :- n3byte001(Code).
n3byte000(Code) :- (Code =< 16'09BC  ','  !  ','  n3byte0000(Code)).
n3byte000(Code) :- n3byte0001(Code).
n3byte0000(Code) :- (Code =< 16'0983  ','  !  ','  n3byte00000(Code)).
n3byte0000(Code) :- n3byte00001(Code).
n3byte00000(Code) :- (Code =< 16'094D  ','  !  ','  n3byte000000(Code)).
n3byte00000(Code) :- n3byte000001(Code).
n3byte000000(Code) :- (Code =< 16'0903  ','  !  ','  n3byte0000000(Code)).
n3byte000000(Code) :- n3byte0000001(Code).
n3byte0000000(Code) :- (Code =< 16'0903  ','  !  ','  Code >= 16'0901).
n3byte0000001(Code) :- (Code =< 16'0939  ','  !  ','  n3byte00000010(Code)).
n3byte0000001(Code) :- n3byte00000011(Code).
n3byte00000010(Code) :- (Code =< 16'0939  ','  !  ','  Code >= 16'0905).
n3byte00000011(Code) :- (Code =< 16'094D  ','  !  ','  Code >= 16'093C).
n3byte000001(Code) :- (Code =< 16'0963  ','  !  ','  n3byte0000010(Code)).
n3byte000001(Code) :- n3byte0000011(Code).
n3byte0000010(Code) :- (Code =< 16'0954  ','  !  ','  n3byte00000100(Code)).
n3byte0000010(Code) :- n3byte00000101(Code).
n3byte00000100(Code) :- (Code =< 16'0954  ','  !  ','  Code >= 16'0951).
n3byte00000101(Code) :- (Code =< 16'0963  ','  !  ','  Code >= 16'0958).
n3byte0000011(Code) :- (Code =< 16'096F  ','  !  ','  n3byte00000110(Code)).
n3byte0000011(Code) :- n3byte00000111(Code).
n3byte00000110(Code) :- (Code =< 16'096F  ','  !  ','  Code >= 16'0966).
n3byte00000111(Code) :- (Code =< 16'0983  ','  !  ','  Code >= 16'0981).
n3byte00001(Code) :- (Code =< 16'09A8  ','  !  ','  n3byte000010(Code)).
n3byte00001(Code) :- n3byte000011(Code).
n3byte000010(Code) :- (Code =< 16'098C  ','  !  ','  n3byte0000100(Code)).
n3byte000010(Code) :- n3byte0000101(Code).
n3byte0000100(Code) :- (Code =< 16'098C  ','  !  ','  Code >= 16'0985).
n3byte0000101(Code) :- (Code =< 16'0990  ','  !  ','  n3byte00001010(Code)).
n3byte0000101(Code) :- n3byte00001011(Code).
n3byte00001010(Code) :- (Code =< 16'0990  ','  !  ','  Code >= 16'098F).
n3byte00001011(Code) :- (Code =< 16'09A8  ','  !  ','  Code >= 16'0993).
n3byte000011(Code) :- (Code =< 16'09B2  ','  !  ','  n3byte0000110(Code)).
n3byte000011(Code) :- n3byte0000111(Code).
n3byte0000110(Code) :- (Code =< 16'09B0  ','  !  ','  n3byte00001100(Code)).
n3byte0000110(Code) :- n3byte00001101(Code).
n3byte00001100(Code) :- (Code =< 16'09B0  ','  !  ','  Code >= 16'09AA).
n3byte00001101(16'09B2) :- !.
n3byte0000111(Code) :- (Code =< 16'09B9  ','  !  ','  n3byte00001110(Code)).
n3byte0000111(Code) :- n3byte00001111(Code).
n3byte00001110(Code) :- (Code =< 16'09B9  ','  !  ','  Code >= 16'09B6).
n3byte00001111(16'09BC) :- !.
n3byte0001(Code) :- (Code =< 16'09DD  ','  !  ','  n3byte00010(Code)).
n3byte0001(Code) :- n3byte00011(Code).
n3byte00010(Code) :- (Code =< 16'09C4  ','  !  ','  n3byte000100(Code)).
n3byte00010(Code) :- n3byte000101(Code).
n3byte000100(Code) :- (Code =< 16'09BE  ','  !  ','  n3byte0001000(Code)).
n3byte000100(Code) :- n3byte0001001(Code).
n3byte0001000(16'09BE) :- !.
n3byte0001001(Code) :- (Code =< 16'09BF  ','  !  ','  n3byte00010010(Code)).
n3byte0001001(Code) :- n3byte00010011(Code).
n3byte00010010(16'09BF) :- !.
n3byte00010011(Code) :- (Code =< 16'09C4  ','  !  ','  Code >= 16'09C0).
n3byte000101(Code) :- (Code =< 16'09CD  ','  !  ','  n3byte0001010(Code)).
n3byte000101(Code) :- n3byte0001011(Code).
n3byte0001010(Code) :- (Code =< 16'09C8  ','  !  ','  n3byte00010100(Code)).
n3byte0001010(Code) :- n3byte00010101(Code).
n3byte00010100(Code) :- (Code =< 16'09C8  ','  !  ','  Code >= 16'09C7).
n3byte00010101(Code) :- (Code =< 16'09CD  ','  !  ','  Code >= 16'09CB).
n3byte0001011(Code) :- (Code =< 16'09D7  ','  !  ','  n3byte00010110(Code)).
n3byte0001011(Code) :- n3byte00010111(Code).
n3byte00010110(16'09D7) :- !.
n3byte00010111(Code) :- (Code =< 16'09DD  ','  !  ','  Code >= 16'09DC).
n3byte00011(Code) :- (Code =< 16'0A02  ','  !  ','  n3byte000110(Code)).
n3byte00011(Code) :- n3byte000111(Code).
n3byte000110(Code) :- (Code =< 16'09E3  ','  !  ','  n3byte0001100(Code)).
n3byte000110(Code) :- n3byte0001101(Code).
n3byte0001100(Code) :- (Code =< 16'09E3  ','  !  ','  Code >= 16'09DF).
n3byte0001101(Code) :- (Code =< 16'09F1  ','  !  ','  n3byte00011010(Code)).
n3byte0001101(Code) :- n3byte00011011(Code).
n3byte00011010(Code) :- (Code =< 16'09F1  ','  !  ','  Code >= 16'09E6).
n3byte00011011(16'0A02) :- !.
n3byte000111(Code) :- (Code =< 16'0A10  ','  !  ','  n3byte0001110(Code)).
n3byte000111(Code) :- n3byte0001111(Code).
n3byte0001110(Code) :- (Code =< 16'0A0A  ','  !  ','  n3byte00011100(Code)).
n3byte0001110(Code) :- n3byte00011101(Code).
n3byte00011100(Code) :- (Code =< 16'0A0A  ','  !  ','  Code >= 16'0A05).
n3byte00011101(Code) :- (Code =< 16'0A10  ','  !  ','  Code >= 16'0A0F).
n3byte0001111(Code) :- (Code =< 16'0A28  ','  !  ','  n3byte00011110(Code)).
n3byte0001111(Code) :- n3byte00011111(Code).
n3byte00011110(Code) :- (Code =< 16'0A28  ','  !  ','  Code >= 16'0A13).
n3byte00011111(Code) :- (Code =< 16'0A30  ','  !  ','  Code >= 16'0A2A).
n3byte001(Code) :- (Code =< 16'0A91  ','  !  ','  n3byte0010(Code)).
n3byte001(Code) :- n3byte0011(Code).
n3byte0010(Code) :- (Code =< 16'0A4D  ','  !  ','  n3byte00100(Code)).
n3byte0010(Code) :- n3byte00101(Code).
n3byte00100(Code) :- (Code =< 16'0A39  ','  !  ','  n3byte001000(Code)).
n3byte00100(Code) :- n3byte001001(Code).
n3byte001000(Code) :- (Code =< 16'0A33  ','  !  ','  n3byte0010000(Code)).
n3byte001000(Code) :- n3byte0010001(Code).
n3byte0010000(Code) :- (Code =< 16'0A33  ','  !  ','  Code >= 16'0A32).
n3byte0010001(Code) :- (Code =< 16'0A36  ','  !  ','  n3byte00100010(Code)).
n3byte0010001(Code) :- n3byte00100011(Code).
n3byte00100010(Code) :- (Code =< 16'0A36  ','  !  ','  Code >= 16'0A35).
n3byte00100011(Code) :- (Code =< 16'0A39  ','  !  ','  Code >= 16'0A38).
n3byte001001(Code) :- (Code =< 16'0A42  ','  !  ','  n3byte0010010(Code)).
n3byte001001(Code) :- n3byte0010011(Code).
n3byte0010010(Code) :- (Code =< 16'0A3C  ','  !  ','  n3byte00100100(Code)).
n3byte0010010(Code) :- n3byte00100101(Code).
n3byte00100100(16'0A3C) :- !.
n3byte00100101(Code) :- (Code =< 16'0A42  ','  !  ','  Code >= 16'0A3E).
n3byte0010011(Code) :- (Code =< 16'0A48  ','  !  ','  n3byte00100110(Code)).
n3byte0010011(Code) :- n3byte00100111(Code).
n3byte00100110(Code) :- (Code =< 16'0A48  ','  !  ','  Code >= 16'0A47).
n3byte00100111(Code) :- (Code =< 16'0A4D  ','  !  ','  Code >= 16'0A4B).
n3byte00101(Code) :- (Code =< 16'0A74  ','  !  ','  n3byte001010(Code)).
n3byte00101(Code) :- n3byte001011(Code).
n3byte001010(Code) :- (Code =< 16'0A5C  ','  !  ','  n3byte0010100(Code)).
n3byte001010(Code) :- n3byte0010101(Code).
n3byte0010100(Code) :- (Code =< 16'0A5C  ','  !  ','  Code >= 16'0A59).
n3byte0010101(Code) :- (Code =< 16'0A5E  ','  !  ','  n3byte00101010(Code)).
n3byte0010101(Code) :- n3byte00101011(Code).
n3byte00101010(16'0A5E) :- !.
n3byte00101011(Code) :- (Code =< 16'0A74  ','  !  ','  Code >= 16'0A66).
n3byte001011(Code) :- (Code =< 16'0A8B  ','  !  ','  n3byte0010110(Code)).
n3byte001011(Code) :- n3byte0010111(Code).
n3byte0010110(Code) :- (Code =< 16'0A83  ','  !  ','  n3byte00101100(Code)).
n3byte0010110(Code) :- n3byte00101101(Code).
n3byte00101100(Code) :- (Code =< 16'0A83  ','  !  ','  Code >= 16'0A81).
n3byte00101101(Code) :- (Code =< 16'0A8B  ','  !  ','  Code >= 16'0A85).
n3byte0010111(Code) :- (Code =< 16'0A8D  ','  !  ','  n3byte00101110(Code)).
n3byte0010111(Code) :- n3byte00101111(Code).
n3byte00101110(16'0A8D) :- !.
n3byte00101111(Code) :- (Code =< 16'0A91  ','  !  ','  Code >= 16'0A8F).
n3byte0011(Code) :- (Code =< 16'0ACD  ','  !  ','  n3byte00110(Code)).
n3byte0011(Code) :- n3byte00111(Code).
n3byte00110(Code) :- (Code =< 16'0AB3  ','  !  ','  n3byte001100(Code)).
n3byte00110(Code) :- n3byte001101(Code).
n3byte001100(Code) :- (Code =< 16'0AA8  ','  !  ','  n3byte0011000(Code)).
n3byte001100(Code) :- n3byte0011001(Code).
n3byte0011000(Code) :- (Code =< 16'0AA8  ','  !  ','  Code >= 16'0A93).
n3byte0011001(Code) :- (Code =< 16'0AB0  ','  !  ','  n3byte00110010(Code)).
n3byte0011001(Code) :- n3byte00110011(Code).
n3byte00110010(Code) :- (Code =< 16'0AB0  ','  !  ','  Code >= 16'0AAA).
n3byte00110011(Code) :- (Code =< 16'0AB3  ','  !  ','  Code >= 16'0AB2).
n3byte001101(Code) :- (Code =< 16'0AC5  ','  !  ','  n3byte0011010(Code)).
n3byte001101(Code) :- n3byte0011011(Code).
n3byte0011010(Code) :- (Code =< 16'0AB9  ','  !  ','  n3byte00110100(Code)).
n3byte0011010(Code) :- n3byte00110101(Code).
n3byte00110100(Code) :- (Code =< 16'0AB9  ','  !  ','  Code >= 16'0AB5).
n3byte00110101(Code) :- (Code =< 16'0AC5  ','  !  ','  Code >= 16'0ABC).
n3byte0011011(Code) :- (Code =< 16'0AC9  ','  !  ','  n3byte00110110(Code)).
n3byte0011011(Code) :- n3byte00110111(Code).
n3byte00110110(Code) :- (Code =< 16'0AC9  ','  !  ','  Code >= 16'0AC7).
n3byte00110111(Code) :- (Code =< 16'0ACD  ','  !  ','  Code >= 16'0ACB).
n3byte00111(Code) :- (Code =< 16'0B03  ','  !  ','  n3byte001110(Code)).
n3byte00111(Code) :- n3byte001111(Code).
n3byte001110(Code) :- (Code =< 16'0AE0  ','  !  ','  n3byte0011100(Code)).
n3byte001110(Code) :- n3byte0011101(Code).
n3byte0011100(16'0AE0) :- !.
n3byte0011101(Code) :- (Code =< 16'0AEF  ','  !  ','  n3byte00111010(Code)).
n3byte0011101(Code) :- n3byte00111011(Code).
n3byte00111010(Code) :- (Code =< 16'0AEF  ','  !  ','  Code >= 16'0AE6).
n3byte00111011(Code) :- (Code =< 16'0B03  ','  !  ','  Code >= 16'0B01).
n3byte001111(Code) :- (Code =< 16'0B10  ','  !  ','  n3byte0011110(Code)).
n3byte001111(Code) :- n3byte0011111(Code).
n3byte0011110(Code) :- (Code =< 16'0B0C  ','  !  ','  n3byte00111100(Code)).
n3byte0011110(Code) :- n3byte00111101(Code).
n3byte00111100(Code) :- (Code =< 16'0B0C  ','  !  ','  Code >= 16'0B05).
n3byte00111101(Code) :- (Code =< 16'0B10  ','  !  ','  Code >= 16'0B0F).
n3byte0011111(Code) :- (Code =< 16'0B28  ','  !  ','  n3byte00111110(Code)).
n3byte0011111(Code) :- n3byte00111111(Code).
n3byte00111110(Code) :- (Code =< 16'0B28  ','  !  ','  Code >= 16'0B13).
n3byte00111111(Code) :- (Code =< 16'0B30  ','  !  ','  Code >= 16'0B2A).
n3byte01(Code) :- (Code =< 16'0C10  ','  !  ','  n3byte010(Code)).
n3byte01(Code) :- n3byte011(Code).
n3byte010(Code) :- (Code =< 16'0B9A  ','  !  ','  n3byte0100(Code)).
n3byte010(Code) :- n3byte0101(Code).
n3byte0100(Code) :- (Code =< 16'0B5D  ','  !  ','  n3byte01000(Code)).
n3byte0100(Code) :- n3byte01001(Code).
n3byte01000(Code) :- (Code =< 16'0B43  ','  !  ','  n3byte010000(Code)).
n3byte01000(Code) :- n3byte010001(Code).
n3byte010000(Code) :- (Code =< 16'0B33  ','  !  ','  n3byte0100000(Code)).
n3byte010000(Code) :- n3byte0100001(Code).
n3byte0100000(Code) :- (Code =< 16'0B33  ','  !  ','  Code >= 16'0B32).
n3byte0100001(Code) :- (Code =< 16'0B39  ','  !  ','  n3byte01000010(Code)).
n3byte0100001(Code) :- n3byte01000011(Code).
n3byte01000010(Code) :- (Code =< 16'0B39  ','  !  ','  Code >= 16'0B36).
n3byte01000011(Code) :- (Code =< 16'0B43  ','  !  ','  Code >= 16'0B3C).
n3byte010001(Code) :- (Code =< 16'0B4D  ','  !  ','  n3byte0100010(Code)).
n3byte010001(Code) :- n3byte0100011(Code).
n3byte0100010(Code) :- (Code =< 16'0B48  ','  !  ','  n3byte01000100(Code)).
n3byte0100010(Code) :- n3byte01000101(Code).
n3byte01000100(Code) :- (Code =< 16'0B48  ','  !  ','  Code >= 16'0B47).
n3byte01000101(Code) :- (Code =< 16'0B4D  ','  !  ','  Code >= 16'0B4B).
n3byte0100011(Code) :- (Code =< 16'0B57  ','  !  ','  n3byte01000110(Code)).
n3byte0100011(Code) :- n3byte01000111(Code).
n3byte01000110(Code) :- (Code =< 16'0B57  ','  !  ','  Code >= 16'0B56).
n3byte01000111(Code) :- (Code =< 16'0B5D  ','  !  ','  Code >= 16'0B5C).
n3byte01001(Code) :- (Code =< 16'0B83  ','  !  ','  n3byte010010(Code)).
n3byte01001(Code) :- n3byte010011(Code).
n3byte010010(Code) :- (Code =< 16'0B61  ','  !  ','  n3byte0100100(Code)).
n3byte010010(Code) :- n3byte0100101(Code).
n3byte0100100(Code) :- (Code =< 16'0B61  ','  !  ','  Code >= 16'0B5F).
n3byte0100101(Code) :- (Code =< 16'0B6F  ','  !  ','  n3byte01001010(Code)).
n3byte0100101(Code) :- n3byte01001011(Code).
n3byte01001010(Code) :- (Code =< 16'0B6F  ','  !  ','  Code >= 16'0B66).
n3byte01001011(Code) :- (Code =< 16'0B83  ','  !  ','  Code >= 16'0B82).
n3byte010011(Code) :- (Code =< 16'0B90  ','  !  ','  n3byte0100110(Code)).
n3byte010011(Code) :- n3byte0100111(Code).
n3byte0100110(Code) :- (Code =< 16'0B8A  ','  !  ','  n3byte01001100(Code)).
n3byte0100110(Code) :- n3byte01001101(Code).
n3byte01001100(Code) :- (Code =< 16'0B8A  ','  !  ','  Code >= 16'0B85).
n3byte01001101(Code) :- (Code =< 16'0B90  ','  !  ','  Code >= 16'0B8E).
n3byte0100111(Code) :- (Code =< 16'0B95  ','  !  ','  n3byte01001110(Code)).
n3byte0100111(Code) :- n3byte01001111(Code).
n3byte01001110(Code) :- (Code =< 16'0B95  ','  !  ','  Code >= 16'0B92).
n3byte01001111(Code) :- (Code =< 16'0B9A  ','  !  ','  Code >= 16'0B99).
n3byte0101(Code) :- (Code =< 16'0BC2  ','  !  ','  n3byte01010(Code)).
n3byte0101(Code) :- n3byte01011(Code).
n3byte01010(Code) :- (Code =< 16'0BA4  ','  !  ','  n3byte010100(Code)).
n3byte01010(Code) :- n3byte010101(Code).
n3byte010100(Code) :- (Code =< 16'0B9C  ','  !  ','  n3byte0101000(Code)).
n3byte010100(Code) :- n3byte0101001(Code).
n3byte0101000(16'0B9C) :- !.
n3byte0101001(Code) :- (Code =< 16'0B9F  ','  !  ','  n3byte01010010(Code)).
n3byte0101001(Code) :- n3byte01010011(Code).
n3byte01010010(Code) :- (Code =< 16'0B9F  ','  !  ','  Code >= 16'0B9E).
n3byte01010011(Code) :- (Code =< 16'0BA4  ','  !  ','  Code >= 16'0BA3).
n3byte010101(Code) :- (Code =< 16'0BB5  ','  !  ','  n3byte0101010(Code)).
n3byte010101(Code) :- n3byte0101011(Code).
n3byte0101010(Code) :- (Code =< 16'0BAA  ','  !  ','  n3byte01010100(Code)).
n3byte0101010(Code) :- n3byte01010101(Code).
n3byte01010100(Code) :- (Code =< 16'0BAA  ','  !  ','  Code >= 16'0BA8).
n3byte01010101(Code) :- (Code =< 16'0BB5  ','  !  ','  Code >= 16'0BAE).
n3byte0101011(Code) :- (Code =< 16'0BB9  ','  !  ','  n3byte01010110(Code)).
n3byte0101011(Code) :- n3byte01010111(Code).
n3byte01010110(Code) :- (Code =< 16'0BB9  ','  !  ','  Code >= 16'0BB7).
n3byte01010111(Code) :- (Code =< 16'0BC2  ','  !  ','  Code >= 16'0BBE).
n3byte01011(Code) :- (Code =< 16'0BD7  ','  !  ','  n3byte010110(Code)).
n3byte01011(Code) :- n3byte010111(Code).
n3byte010110(Code) :- (Code =< 16'0BC8  ','  !  ','  n3byte0101100(Code)).
n3byte010110(Code) :- n3byte0101101(Code).
n3byte0101100(Code) :- (Code =< 16'0BC8  ','  !  ','  Code >= 16'0BC6).
n3byte0101101(Code) :- (Code =< 16'0BCD  ','  !  ','  n3byte01011010(Code)).
n3byte0101101(Code) :- n3byte01011011(Code).
n3byte01011010(Code) :- (Code =< 16'0BCD  ','  !  ','  Code >= 16'0BCA).
n3byte01011011(16'0BD7) :- !.
n3byte010111(Code) :- (Code =< 16'0C03  ','  !  ','  n3byte0101110(Code)).
n3byte010111(Code) :- n3byte0101111(Code).
n3byte0101110(Code) :- (Code =< 16'0BEF  ','  !  ','  n3byte01011100(Code)).
n3byte0101110(Code) :- n3byte01011101(Code).
n3byte01011100(Code) :- (Code =< 16'0BEF  ','  !  ','  Code >= 16'0BE7).
n3byte01011101(Code) :- (Code =< 16'0C03  ','  !  ','  Code >= 16'0C01).
n3byte0101111(Code) :- (Code =< 16'0C0C  ','  !  ','  n3byte01011110(Code)).
n3byte0101111(Code) :- n3byte01011111(Code).
n3byte01011110(Code) :- (Code =< 16'0C0C  ','  !  ','  Code >= 16'0C05).
n3byte01011111(Code) :- (Code =< 16'0C10  ','  !  ','  Code >= 16'0C0E).
n3byte011(Code) :- (Code =< 16'0CB3  ','  !  ','  n3byte0110(Code)).
n3byte011(Code) :- n3byte0111(Code).
n3byte0110(Code) :- (Code =< 16'0C56  ','  !  ','  n3byte01100(Code)).
n3byte0110(Code) :- n3byte01101(Code).
n3byte01100(Code) :- (Code =< 16'0C39  ','  !  ','  n3byte011000(Code)).
n3byte01100(Code) :- n3byte011001(Code).
n3byte011000(Code) :- (Code =< 16'0C28  ','  !  ','  n3byte0110000(Code)).
n3byte011000(Code) :- n3byte0110001(Code).
n3byte0110000(Code) :- (Code =< 16'0C28  ','  !  ','  Code >= 16'0C12).
n3byte0110001(Code) :- (Code =< 16'0C33  ','  !  ','  n3byte01100010(Code)).
n3byte0110001(Code) :- n3byte01100011(Code).
n3byte01100010(Code) :- (Code =< 16'0C33  ','  !  ','  Code >= 16'0C2A).
n3byte01100011(Code) :- (Code =< 16'0C39  ','  !  ','  Code >= 16'0C35).
n3byte011001(Code) :- (Code =< 16'0C48  ','  !  ','  n3byte0110010(Code)).
n3byte011001(Code) :- n3byte0110011(Code).
n3byte0110010(Code) :- (Code =< 16'0C44  ','  !  ','  n3byte01100100(Code)).
n3byte0110010(Code) :- n3byte01100101(Code).
n3byte01100100(Code) :- (Code =< 16'0C44  ','  !  ','  Code >= 16'0C3E).
n3byte01100101(Code) :- (Code =< 16'0C48  ','  !  ','  Code >= 16'0C46).
n3byte0110011(Code) :- (Code =< 16'0C4D  ','  !  ','  n3byte01100110(Code)).
n3byte0110011(Code) :- n3byte01100111(Code).
n3byte01100110(Code) :- (Code =< 16'0C4D  ','  !  ','  Code >= 16'0C4A).
n3byte01100111(Code) :- (Code =< 16'0C56  ','  !  ','  Code >= 16'0C55).
n3byte01101(Code) :- (Code =< 16'0C83  ','  !  ','  n3byte011010(Code)).
n3byte01101(Code) :- n3byte011011(Code).
n3byte011010(Code) :- (Code =< 16'0C61  ','  !  ','  n3byte0110100(Code)).
n3byte011010(Code) :- n3byte0110101(Code).
n3byte0110100(Code) :- (Code =< 16'0C61  ','  !  ','  Code >= 16'0C60).
n3byte0110101(Code) :- (Code =< 16'0C6F  ','  !  ','  n3byte01101010(Code)).
n3byte0110101(Code) :- n3byte01101011(Code).
n3byte01101010(Code) :- (Code =< 16'0C6F  ','  !  ','  Code >= 16'0C66).
n3byte01101011(Code) :- (Code =< 16'0C83  ','  !  ','  Code >= 16'0C82).
n3byte011011(Code) :- (Code =< 16'0C90  ','  !  ','  n3byte0110110(Code)).
n3byte011011(Code) :- n3byte0110111(Code).
n3byte0110110(Code) :- (Code =< 16'0C8C  ','  !  ','  n3byte01101100(Code)).
n3byte0110110(Code) :- n3byte01101101(Code).
n3byte01101100(Code) :- (Code =< 16'0C8C  ','  !  ','  Code >= 16'0C85).
n3byte01101101(Code) :- (Code =< 16'0C90  ','  !  ','  Code >= 16'0C8E).
n3byte0110111(Code) :- (Code =< 16'0CA8  ','  !  ','  n3byte01101110(Code)).
n3byte0110111(Code) :- n3byte01101111(Code).
n3byte01101110(Code) :- (Code =< 16'0CA8  ','  !  ','  Code >= 16'0C92).
n3byte01101111(Code) :- (Code =< 16'0CB3  ','  !  ','  Code >= 16'0CAA).
n3byte0111(Code) :- (Code =< 16'0CE1  ','  !  ','  n3byte01110(Code)).
n3byte0111(Code) :- n3byte01111(Code).
n3byte01110(Code) :- (Code =< 16'0CC8  ','  !  ','  n3byte011100(Code)).
n3byte01110(Code) :- n3byte011101(Code).
n3byte011100(Code) :- (Code =< 16'0CB9  ','  !  ','  n3byte0111000(Code)).
n3byte011100(Code) :- n3byte0111001(Code).
n3byte0111000(Code) :- (Code =< 16'0CB9  ','  !  ','  Code >= 16'0CB5).
n3byte0111001(Code) :- (Code =< 16'0CC4  ','  !  ','  n3byte01110010(Code)).
n3byte0111001(Code) :- n3byte01110011(Code).
n3byte01110010(Code) :- (Code =< 16'0CC4  ','  !  ','  Code >= 16'0CBE).
n3byte01110011(Code) :- (Code =< 16'0CC8  ','  !  ','  Code >= 16'0CC6).
n3byte011101(Code) :- (Code =< 16'0CD6  ','  !  ','  n3byte0111010(Code)).
n3byte011101(Code) :- n3byte0111011(Code).
n3byte0111010(Code) :- (Code =< 16'0CCD  ','  !  ','  n3byte01110100(Code)).
n3byte0111010(Code) :- n3byte01110101(Code).
n3byte01110100(Code) :- (Code =< 16'0CCD  ','  !  ','  Code >= 16'0CCA).
n3byte01110101(Code) :- (Code =< 16'0CD6  ','  !  ','  Code >= 16'0CD5).
n3byte0111011(Code) :- (Code =< 16'0CDE  ','  !  ','  n3byte01110110(Code)).
n3byte0111011(Code) :- n3byte01110111(Code).
n3byte01110110(16'0CDE) :- !.
n3byte01110111(Code) :- (Code =< 16'0CE1  ','  !  ','  Code >= 16'0CE0).
n3byte01111(Code) :- (Code =< 16'0D10  ','  !  ','  n3byte011110(Code)).
n3byte01111(Code) :- n3byte011111(Code).
n3byte011110(Code) :- (Code =< 16'0D03  ','  !  ','  n3byte0111100(Code)).
n3byte011110(Code) :- n3byte0111101(Code).
n3byte0111100(Code) :- (Code =< 16'0CEF  ','  !  ','  n3byte01111000(Code)).
n3byte0111100(Code) :- n3byte01111001(Code).
n3byte01111000(Code) :- (Code =< 16'0CEF  ','  !  ','  Code >= 16'0CE6).
n3byte01111001(Code) :- (Code =< 16'0D03  ','  !  ','  Code >= 16'0D02).
n3byte0111101(Code) :- (Code =< 16'0D0C  ','  !  ','  n3byte01111010(Code)).
n3byte0111101(Code) :- n3byte01111011(Code).
n3byte01111010(Code) :- (Code =< 16'0D0C  ','  !  ','  Code >= 16'0D05).
n3byte01111011(Code) :- (Code =< 16'0D10  ','  !  ','  Code >= 16'0D0E).
n3byte011111(Code) :- (Code =< 16'0D39  ','  !  ','  n3byte0111110(Code)).
n3byte011111(Code) :- n3byte0111111(Code).
n3byte0111110(Code) :- (Code =< 16'0D28  ','  !  ','  n3byte01111100(Code)).
n3byte0111110(Code) :- n3byte01111101(Code).
n3byte01111100(Code) :- (Code =< 16'0D28  ','  !  ','  Code >= 16'0D12).
n3byte01111101(Code) :- (Code =< 16'0D39  ','  !  ','  Code >= 16'0D2A).
n3byte0111111(Code) :- (Code =< 16'0D43  ','  !  ','  n3byte01111110(Code)).
n3byte0111111(Code) :- n3byte01111111(Code).
n3byte01111110(Code) :- (Code =< 16'0D43  ','  !  ','  Code >= 16'0D3E).
n3byte01111111(Code) :- (Code =< 16'0D48  ','  !  ','  Code >= 16'0D46).
n3byte1(Code) :- (Code =< 16'1159  ','  !  ','  n3byte10(Code)).
n3byte1(Code) :- n3byte11(Code).
n3byte10(Code) :- (Code =< 16'0F29  ','  !  ','  n3byte100(Code)).
n3byte10(Code) :- n3byte101(Code).
n3byte100(Code) :- (Code =< 16'0E97  ','  !  ','  n3byte1000(Code)).
n3byte100(Code) :- n3byte1001(Code).
n3byte1000(Code) :- (Code =< 16'0E4E  ','  !  ','  n3byte10000(Code)).
n3byte1000(Code) :- n3byte10001(Code).
n3byte10000(Code) :- (Code =< 16'0D61  ','  !  ','  n3byte100000(Code)).
n3byte10000(Code) :- n3byte100001(Code).
n3byte100000(Code) :- (Code =< 16'0D4D  ','  !  ','  n3byte1000000(Code)).
n3byte100000(Code) :- n3byte1000001(Code).
n3byte1000000(Code) :- (Code =< 16'0D4D  ','  !  ','  Code >= 16'0D4A).
n3byte1000001(Code) :- (Code =< 16'0D57  ','  !  ','  n3byte10000010(Code)).
n3byte1000001(Code) :- n3byte10000011(Code).
n3byte10000010(16'0D57) :- !.
n3byte10000011(Code) :- (Code =< 16'0D61  ','  !  ','  Code >= 16'0D60).
n3byte100001(Code) :- (Code =< 16'0E2E  ','  !  ','  n3byte1000010(Code)).
n3byte100001(Code) :- n3byte1000011(Code).
n3byte1000010(Code) :- (Code =< 16'0D6F  ','  !  ','  n3byte10000100(Code)).
n3byte1000010(Code) :- n3byte10000101(Code).
n3byte10000100(Code) :- (Code =< 16'0D6F  ','  !  ','  Code >= 16'0D66).
n3byte10000101(Code) :- (Code =< 16'0E2E  ','  !  ','  Code >= 16'0E01).
n3byte1000011(Code) :- (Code =< 16'0E3A  ','  !  ','  n3byte10000110(Code)).
n3byte1000011(Code) :- n3byte10000111(Code).
n3byte10000110(Code) :- (Code =< 16'0E3A  ','  !  ','  Code >= 16'0E30).
n3byte10000111(Code) :- (Code =< 16'0E4E  ','  !  ','  Code >= 16'0E40).
n3byte10001(Code) :- (Code =< 16'0E84  ','  !  ','  n3byte100010(Code)).
n3byte10001(Code) :- n3byte100011(Code).
n3byte100010(Code) :- (Code =< 16'0E59  ','  !  ','  n3byte1000100(Code)).
n3byte100010(Code) :- n3byte1000101(Code).
n3byte1000100(Code) :- (Code =< 16'0E59  ','  !  ','  Code >= 16'0E50).
n3byte1000101(Code) :- (Code =< 16'0E82  ','  !  ','  n3byte10001010(Code)).
n3byte1000101(Code) :- n3byte10001011(Code).
n3byte10001010(Code) :- (Code =< 16'0E82  ','  !  ','  Code >= 16'0E81).
n3byte10001011(16'0E84) :- !.
n3byte100011(Code) :- (Code =< 16'0E8A  ','  !  ','  n3byte1000110(Code)).
n3byte100011(Code) :- n3byte1000111(Code).
n3byte1000110(Code) :- (Code =< 16'0E88  ','  !  ','  n3byte10001100(Code)).
n3byte1000110(Code) :- n3byte10001101(Code).
n3byte10001100(Code) :- (Code =< 16'0E88  ','  !  ','  Code >= 16'0E87).
n3byte10001101(16'0E8A) :- !.
n3byte1000111(Code) :- (Code =< 16'0E8D  ','  !  ','  n3byte10001110(Code)).
n3byte1000111(Code) :- n3byte10001111(Code).
n3byte10001110(16'0E8D) :- !.
n3byte10001111(Code) :- (Code =< 16'0E97  ','  !  ','  Code >= 16'0E94).
n3byte1001(Code) :- (Code =< 16'0EB9  ','  !  ','  n3byte10010(Code)).
n3byte1001(Code) :- n3byte10011(Code).
n3byte10010(Code) :- (Code =< 16'0EA5  ','  !  ','  n3byte100100(Code)).
n3byte10010(Code) :- n3byte100101(Code).
n3byte100100(Code) :- (Code =< 16'0E9F  ','  !  ','  n3byte1001000(Code)).
n3byte100100(Code) :- n3byte1001001(Code).
n3byte1001000(Code) :- (Code =< 16'0E9F  ','  !  ','  Code >= 16'0E99).
n3byte1001001(Code) :- (Code =< 16'0EA3  ','  !  ','  n3byte10010010(Code)).
n3byte1001001(Code) :- n3byte10010011(Code).
n3byte10010010(Code) :- (Code =< 16'0EA3  ','  !  ','  Code >= 16'0EA1).
n3byte10010011(16'0EA5) :- !.
n3byte100101(Code) :- (Code =< 16'0EAB  ','  !  ','  n3byte1001010(Code)).
n3byte100101(Code) :- n3byte1001011(Code).
n3byte1001010(Code) :- (Code =< 16'0EA7  ','  !  ','  n3byte10010100(Code)).
n3byte1001010(Code) :- n3byte10010101(Code).
n3byte10010100(16'0EA7) :- !.
n3byte10010101(Code) :- (Code =< 16'0EAB  ','  !  ','  Code >= 16'0EAA).
n3byte1001011(Code) :- (Code =< 16'0EAE  ','  !  ','  n3byte10010110(Code)).
n3byte1001011(Code) :- n3byte10010111(Code).
n3byte10010110(Code) :- (Code =< 16'0EAE  ','  !  ','  Code >= 16'0EAD).
n3byte10010111(Code) :- (Code =< 16'0EB9  ','  !  ','  Code >= 16'0EB0).
n3byte10011(Code) :- (Code =< 16'0EC6  ','  !  ','  n3byte100110(Code)).
n3byte10011(Code) :- n3byte100111(Code).
n3byte100110(Code) :- (Code =< 16'0EBD  ','  !  ','  n3byte1001100(Code)).
n3byte100110(Code) :- n3byte1001101(Code).
n3byte1001100(Code) :- (Code =< 16'0EBD  ','  !  ','  Code >= 16'0EBB).
n3byte1001101(Code) :- (Code =< 16'0EC4  ','  !  ','  n3byte10011010(Code)).
n3byte1001101(Code) :- n3byte10011011(Code).
n3byte10011010(Code) :- (Code =< 16'0EC4  ','  !  ','  Code >= 16'0EC0).
n3byte10011011(16'0EC6) :- !.
n3byte100111(Code) :- (Code =< 16'0ED9  ','  !  ','  n3byte1001110(Code)).
n3byte100111(Code) :- n3byte1001111(Code).
n3byte1001110(Code) :- (Code =< 16'0ECD  ','  !  ','  n3byte10011100(Code)).
n3byte1001110(Code) :- n3byte10011101(Code).
n3byte10011100(Code) :- (Code =< 16'0ECD  ','  !  ','  Code >= 16'0EC8).
n3byte10011101(Code) :- (Code =< 16'0ED9  ','  !  ','  Code >= 16'0ED0).
n3byte1001111(Code) :- (Code =< 16'0F19  ','  !  ','  n3byte10011110(Code)).
n3byte1001111(Code) :- n3byte10011111(Code).
n3byte10011110(Code) :- (Code =< 16'0F19  ','  !  ','  Code >= 16'0F18).
n3byte10011111(Code) :- (Code =< 16'0F29  ','  !  ','  Code >= 16'0F20).
n3byte101(Code) :- (Code =< 16'10F6  ','  !  ','  n3byte1010(Code)).
n3byte101(Code) :- n3byte1011(Code).
n3byte1010(Code) :- (Code =< 16'0F8B  ','  !  ','  n3byte10100(Code)).
n3byte1010(Code) :- n3byte10101(Code).
n3byte10100(Code) :- (Code =< 16'0F39  ','  !  ','  n3byte101000(Code)).
n3byte10100(Code) :- n3byte101001(Code).
n3byte101000(Code) :- (Code =< 16'0F35  ','  !  ','  n3byte1010000(Code)).
n3byte101000(Code) :- n3byte1010001(Code).
n3byte1010000(16'0F35) :- !.
n3byte1010001(Code) :- (Code =< 16'0F37  ','  !  ','  n3byte10100010(Code)).
n3byte1010001(Code) :- n3byte10100011(Code).
n3byte10100010(16'0F37) :- !.
n3byte10100011(16'0F39) :- !.
n3byte101001(Code) :- (Code =< 16'0F69  ','  !  ','  n3byte1010010(Code)).
n3byte101001(Code) :- n3byte1010011(Code).
n3byte1010010(Code) :- (Code =< 16'0F47  ','  !  ','  n3byte10100100(Code)).
n3byte1010010(Code) :- n3byte10100101(Code).
n3byte10100100(Code) :- (Code =< 16'0F47  ','  !  ','  Code >= 16'0F3E).
n3byte10100101(Code) :- (Code =< 16'0F69  ','  !  ','  Code >= 16'0F49).
n3byte1010011(Code) :- (Code =< 16'0F84  ','  !  ','  n3byte10100110(Code)).
n3byte1010011(Code) :- n3byte10100111(Code).
n3byte10100110(Code) :- (Code =< 16'0F84  ','  !  ','  Code >= 16'0F71).
n3byte10100111(Code) :- (Code =< 16'0F8B  ','  !  ','  Code >= 16'0F86).
n3byte10101(Code) :- (Code =< 16'0FAD  ','  !  ','  n3byte101010(Code)).
n3byte10101(Code) :- n3byte101011(Code).
n3byte101010(Code) :- (Code =< 16'0F95  ','  !  ','  n3byte1010100(Code)).
n3byte101010(Code) :- n3byte1010101(Code).
n3byte1010100(Code) :- (Code =< 16'0F95  ','  !  ','  Code >= 16'0F90).
n3byte1010101(Code) :- (Code =< 16'0F97  ','  !  ','  n3byte10101010(Code)).
n3byte1010101(Code) :- n3byte10101011(Code).
n3byte10101010(16'0F97) :- !.
n3byte10101011(Code) :- (Code =< 16'0FAD  ','  !  ','  Code >= 16'0F99).
n3byte101011(Code) :- (Code =< 16'0FB9  ','  !  ','  n3byte1010110(Code)).
n3byte101011(Code) :- n3byte1010111(Code).
n3byte1010110(Code) :- (Code =< 16'0FB7  ','  !  ','  n3byte10101100(Code)).
n3byte1010110(Code) :- n3byte10101101(Code).
n3byte10101100(Code) :- (Code =< 16'0FB7  ','  !  ','  Code >= 16'0FB1).
n3byte10101101(16'0FB9) :- !.
n3byte1010111(Code) :- (Code =< 16'10C5  ','  !  ','  n3byte10101110(Code)).
n3byte1010111(Code) :- n3byte10101111(Code).
n3byte10101110(Code) :- (Code =< 16'10C5  ','  !  ','  Code >= 16'10A0).
n3byte10101111(Code) :- (Code =< 16'10F6  ','  !  ','  Code >= 16'10D0).
n3byte1011(Code) :- (Code =< 16'113C  ','  !  ','  n3byte10110(Code)).
n3byte1011(Code) :- n3byte10111(Code).
n3byte10110(Code) :- (Code =< 16'1107  ','  !  ','  n3byte101100(Code)).
n3byte10110(Code) :- n3byte101101(Code).
n3byte101100(Code) :- (Code =< 16'1100  ','  !  ','  n3byte1011000(Code)).
n3byte101100(Code) :- n3byte1011001(Code).
n3byte1011000(16'1100) :- !.
n3byte1011001(Code) :- (Code =< 16'1103  ','  !  ','  n3byte10110010(Code)).
n3byte1011001(Code) :- n3byte10110011(Code).
n3byte10110010(Code) :- (Code =< 16'1103  ','  !  ','  Code >= 16'1102).
n3byte10110011(Code) :- (Code =< 16'1107  ','  !  ','  Code >= 16'1105).
n3byte101101(Code) :- (Code =< 16'110C  ','  !  ','  n3byte1011010(Code)).
n3byte101101(Code) :- n3byte1011011(Code).
n3byte1011010(Code) :- (Code =< 16'1109  ','  !  ','  n3byte10110100(Code)).
n3byte1011010(Code) :- n3byte10110101(Code).
n3byte10110100(16'1109) :- !.
n3byte10110101(Code) :- (Code =< 16'110C  ','  !  ','  Code >= 16'110B).
n3byte1011011(Code) :- (Code =< 16'1112  ','  !  ','  n3byte10110110(Code)).
n3byte1011011(Code) :- n3byte10110111(Code).
n3byte10110110(Code) :- (Code =< 16'1112  ','  !  ','  Code >= 16'110E).
n3byte10110111(16'113C) :- !.
n3byte10111(Code) :- (Code =< 16'114C  ','  !  ','  n3byte101110(Code)).
n3byte10111(Code) :- n3byte101111(Code).
n3byte101110(Code) :- (Code =< 16'113E  ','  !  ','  n3byte1011100(Code)).
n3byte101110(Code) :- n3byte1011101(Code).
n3byte1011100(16'113E) :- !.
n3byte1011101(Code) :- (Code =< 16'1140  ','  !  ','  n3byte10111010(Code)).
n3byte1011101(Code) :- n3byte10111011(Code).
n3byte10111010(16'1140) :- !.
n3byte10111011(16'114C) :- !.
n3byte101111(Code) :- (Code =< 16'1150  ','  !  ','  n3byte1011110(Code)).
n3byte101111(Code) :- n3byte1011111(Code).
n3byte1011110(Code) :- (Code =< 16'114E  ','  !  ','  n3byte10111100(Code)).
n3byte1011110(Code) :- n3byte10111101(Code).
n3byte10111100(16'114E) :- !.
n3byte10111101(16'1150) :- !.
n3byte1011111(Code) :- (Code =< 16'1155  ','  !  ','  n3byte10111110(Code)).
n3byte1011111(Code) :- n3byte10111111(Code).
n3byte10111110(Code) :- (Code =< 16'1155  ','  !  ','  Code >= 16'1154).
n3byte10111111(16'1159) :- !.
n3byte11(Code) :- (Code =< 16'1F5D  ','  !  ','  n3byte110(Code)).
n3byte11(Code) :- n3byte111(Code).
n3byte110(Code) :- (Code =< 16'11BA  ','  !  ','  n3byte1100(Code)).
n3byte110(Code) :- n3byte1101(Code).
n3byte1100(Code) :- (Code =< 16'1173  ','  !  ','  n3byte11000(Code)).
n3byte1100(Code) :- n3byte11001(Code).
n3byte11000(Code) :- (Code =< 16'1165  ','  !  ','  n3byte110000(Code)).
n3byte11000(Code) :- n3byte110001(Code).
n3byte110000(Code) :- (Code =< 16'1161  ','  !  ','  n3byte1100000(Code)).
n3byte110000(Code) :- n3byte1100001(Code).
n3byte1100000(Code) :- (Code =< 16'1161  ','  !  ','  Code >= 16'115F).
n3byte1100001(Code) :- (Code =< 16'1163  ','  !  ','  n3byte11000010(Code)).
n3byte1100001(Code) :- n3byte11000011(Code).
n3byte11000010(16'1163) :- !.
n3byte11000011(16'1165) :- !.
n3byte110001(Code) :- (Code =< 16'1169  ','  !  ','  n3byte1100010(Code)).
n3byte110001(Code) :- n3byte1100011(Code).
n3byte1100010(Code) :- (Code =< 16'1167  ','  !  ','  n3byte11000100(Code)).
n3byte1100010(Code) :- n3byte11000101(Code).
n3byte11000100(16'1167) :- !.
n3byte11000101(16'1169) :- !.
n3byte1100011(Code) :- (Code =< 16'116E  ','  !  ','  n3byte11000110(Code)).
n3byte1100011(Code) :- n3byte11000111(Code).
n3byte11000110(Code) :- (Code =< 16'116E  ','  !  ','  Code >= 16'116D).
n3byte11000111(Code) :- (Code =< 16'1173  ','  !  ','  Code >= 16'1172).
n3byte11001(Code) :- (Code =< 16'11A8  ','  !  ','  n3byte110010(Code)).
n3byte11001(Code) :- n3byte110011(Code).
n3byte110010(Code) :- (Code =< 16'1175  ','  !  ','  n3byte1100100(Code)).
n3byte110010(Code) :- n3byte1100101(Code).
n3byte1100100(16'1175) :- !.
n3byte1100101(Code) :- (Code =< 16'119E  ','  !  ','  n3byte11001010(Code)).
n3byte1100101(Code) :- n3byte11001011(Code).
n3byte11001010(16'119E) :- !.
n3byte11001011(16'11A8) :- !.
n3byte110011(Code) :- (Code =< 16'11AF  ','  !  ','  n3byte1100110(Code)).
n3byte110011(Code) :- n3byte1100111(Code).
n3byte1100110(Code) :- (Code =< 16'11AB  ','  !  ','  n3byte11001100(Code)).
n3byte1100110(Code) :- n3byte11001101(Code).
n3byte11001100(16'11AB) :- !.
n3byte11001101(Code) :- (Code =< 16'11AF  ','  !  ','  Code >= 16'11AE).
n3byte1100111(Code) :- (Code =< 16'11B8  ','  !  ','  n3byte11001110(Code)).
n3byte1100111(Code) :- n3byte11001111(Code).
n3byte11001110(Code) :- (Code =< 16'11B8  ','  !  ','  Code >= 16'11B7).
n3byte11001111(16'11BA) :- !.
n3byte1101(Code) :- (Code =< 16'1F15  ','  !  ','  n3byte11010(Code)).
n3byte1101(Code) :- n3byte11011(Code).
n3byte11010(Code) :- (Code =< 16'11F0  ','  !  ','  n3byte110100(Code)).
n3byte11010(Code) :- n3byte110101(Code).
n3byte110100(Code) :- (Code =< 16'11C2  ','  !  ','  n3byte1101000(Code)).
n3byte110100(Code) :- n3byte1101001(Code).
n3byte1101000(Code) :- (Code =< 16'11C2  ','  !  ','  Code >= 16'11BC).
n3byte1101001(Code) :- (Code =< 16'11EB  ','  !  ','  n3byte11010010(Code)).
n3byte1101001(Code) :- n3byte11010011(Code).
n3byte11010010(16'11EB) :- !.
n3byte11010011(16'11F0) :- !.
n3byte110101(Code) :- (Code =< 16'1E9B  ','  !  ','  n3byte1101010(Code)).
n3byte110101(Code) :- n3byte1101011(Code).
n3byte1101010(Code) :- (Code =< 16'11F9  ','  !  ','  n3byte11010100(Code)).
n3byte1101010(Code) :- n3byte11010101(Code).
n3byte11010100(16'11F9) :- !.
n3byte11010101(Code) :- (Code =< 16'1E9B  ','  !  ','  Code >= 16'1E00).
n3byte1101011(Code) :- (Code =< 16'1EF9  ','  !  ','  n3byte11010110(Code)).
n3byte1101011(Code) :- n3byte11010111(Code).
n3byte11010110(Code) :- (Code =< 16'1EF9  ','  !  ','  Code >= 16'1EA0).
n3byte11010111(Code) :- (Code =< 16'1F15  ','  !  ','  Code >= 16'1F00).
n3byte11011(Code) :- (Code =< 16'1F4D  ','  !  ','  n3byte110110(Code)).
n3byte11011(Code) :- n3byte110111(Code).
n3byte110110(Code) :- (Code =< 16'1F1D  ','  !  ','  n3byte1101100(Code)).
n3byte110110(Code) :- n3byte1101101(Code).
n3byte1101100(Code) :- (Code =< 16'1F1D  ','  !  ','  Code >= 16'1F18).
n3byte1101101(Code) :- (Code =< 16'1F45  ','  !  ','  n3byte11011010(Code)).
n3byte1101101(Code) :- n3byte11011011(Code).
n3byte11011010(Code) :- (Code =< 16'1F45  ','  !  ','  Code >= 16'1F20).
n3byte11011011(Code) :- (Code =< 16'1F4D  ','  !  ','  Code >= 16'1F48).
n3byte110111(Code) :- (Code =< 16'1F59  ','  !  ','  n3byte1101110(Code)).
n3byte110111(Code) :- n3byte1101111(Code).
n3byte1101110(Code) :- (Code =< 16'1F57  ','  !  ','  n3byte11011100(Code)).
n3byte1101110(Code) :- n3byte11011101(Code).
n3byte11011100(Code) :- (Code =< 16'1F57  ','  !  ','  Code >= 16'1F50).
n3byte11011101(16'1F59) :- !.
n3byte1101111(Code) :- (Code =< 16'1F5B  ','  !  ','  n3byte11011110(Code)).
n3byte1101111(Code) :- n3byte11011111(Code).
n3byte11011110(16'1F5B) :- !.
n3byte11011111(16'1F5D) :- !.
n3byte111(Code) :- (Code =< 16'2126  ','  !  ','  n3byte1110(Code)).
n3byte111(Code) :- n3byte1111(Code).
n3byte1110(Code) :- (Code =< 16'1FD3  ','  !  ','  n3byte11100(Code)).
n3byte1110(Code) :- n3byte11101(Code).
n3byte11100(Code) :- (Code =< 16'1FBC  ','  !  ','  n3byte111000(Code)).
n3byte11100(Code) :- n3byte111001(Code).
n3byte111000(Code) :- (Code =< 16'1F7D  ','  !  ','  n3byte1110000(Code)).
n3byte111000(Code) :- n3byte1110001(Code).
n3byte1110000(Code) :- (Code =< 16'1F7D  ','  !  ','  Code >= 16'1F5F).
n3byte1110001(Code) :- (Code =< 16'1FB4  ','  !  ','  n3byte11100010(Code)).
n3byte1110001(Code) :- n3byte11100011(Code).
n3byte11100010(Code) :- (Code =< 16'1FB4  ','  !  ','  Code >= 16'1F80).
n3byte11100011(Code) :- (Code =< 16'1FBC  ','  !  ','  Code >= 16'1FB6).
n3byte111001(Code) :- (Code =< 16'1FC4  ','  !  ','  n3byte1110010(Code)).
n3byte111001(Code) :- n3byte1110011(Code).
n3byte1110010(Code) :- (Code =< 16'1FBE  ','  !  ','  n3byte11100100(Code)).
n3byte1110010(Code) :- n3byte11100101(Code).
n3byte11100100(16'1FBE) :- !.
n3byte11100101(Code) :- (Code =< 16'1FC4  ','  !  ','  Code >= 16'1FC2).
n3byte1110011(Code) :- (Code =< 16'1FCC  ','  !  ','  n3byte11100110(Code)).
n3byte1110011(Code) :- n3byte11100111(Code).
n3byte11100110(Code) :- (Code =< 16'1FCC  ','  !  ','  Code >= 16'1FC6).
n3byte11100111(Code) :- (Code =< 16'1FD3  ','  !  ','  Code >= 16'1FD0).
n3byte11101(Code) :- (Code =< 16'1FF4  ','  !  ','  n3byte111010(Code)).
n3byte11101(Code) :- n3byte111011(Code).
n3byte111010(Code) :- (Code =< 16'1FDB  ','  !  ','  n3byte1110100(Code)).
n3byte111010(Code) :- n3byte1110101(Code).
n3byte1110100(Code) :- (Code =< 16'1FDB  ','  !  ','  Code >= 16'1FD6).
n3byte1110101(Code) :- (Code =< 16'1FEC  ','  !  ','  n3byte11101010(Code)).
n3byte1110101(Code) :- n3byte11101011(Code).
n3byte11101010(Code) :- (Code =< 16'1FEC  ','  !  ','  Code >= 16'1FE0).
n3byte11101011(Code) :- (Code =< 16'1FF4  ','  !  ','  Code >= 16'1FF2).
n3byte111011(Code) :- (Code =< 16'20DC  ','  !  ','  n3byte1110110(Code)).
n3byte111011(Code) :- n3byte1110111(Code).
n3byte1110110(Code) :- (Code =< 16'1FFC  ','  !  ','  n3byte11101100(Code)).
n3byte1110110(Code) :- n3byte11101101(Code).
n3byte11101100(Code) :- (Code =< 16'1FFC  ','  !  ','  Code >= 16'1FF6).
n3byte11101101(Code) :- (Code =< 16'20DC  ','  !  ','  Code >= 16'20D0).
n3byte1110111(Code) :- (Code =< 16'20E1  ','  !  ','  n3byte11101110(Code)).
n3byte1110111(Code) :- n3byte11101111(Code).
n3byte11101110(16'20E1) :- !.
n3byte11101111(16'2126) :- !.
n3byte1111(Code) :- (Code =< 16'3035  ','  !  ','  n3byte11110(Code)).
n3byte1111(Code) :- n3byte11111(Code).
n3byte11110(Code) :- (Code =< 16'2182  ','  !  ','  n3byte111100(Code)).
n3byte11110(Code) :- n3byte111101(Code).
n3byte111100(Code) :- (Code =< 16'212B  ','  !  ','  n3byte1111000(Code)).
n3byte111100(Code) :- n3byte1111001(Code).
n3byte1111000(Code) :- (Code =< 16'212B  ','  !  ','  Code >= 16'212A).
n3byte1111001(Code) :- (Code =< 16'212E  ','  !  ','  n3byte11110010(Code)).
n3byte1111001(Code) :- n3byte11110011(Code).
n3byte11110010(16'212E) :- !.
n3byte11110011(Code) :- (Code =< 16'2182  ','  !  ','  Code >= 16'2180).
n3byte111101(Code) :- (Code =< 16'3007  ','  !  ','  n3byte1111010(Code)).
n3byte111101(Code) :- n3byte1111011(Code).
n3byte1111010(Code) :- (Code =< 16'3005  ','  !  ','  n3byte11110100(Code)).
n3byte1111010(Code) :- n3byte11110101(Code).
n3byte11110100(16'3005) :- !.
n3byte11110101(16'3007) :- !.
n3byte1111011(Code) :- (Code =< 16'302F  ','  !  ','  n3byte11110110(Code)).
n3byte1111011(Code) :- n3byte11110111(Code).
n3byte11110110(Code) :- (Code =< 16'302F  ','  !  ','  Code >= 16'3021).
n3byte11110111(Code) :- (Code =< 16'3035  ','  !  ','  Code >= 16'3031).
n3byte11111(Code) :- (Code =< 16'30FA  ','  !  ','  n3byte111110(Code)).
n3byte11111(Code) :- n3byte111111(Code).
n3byte111110(Code) :- (Code =< 16'309A  ','  !  ','  n3byte1111100(Code)).
n3byte111110(Code) :- n3byte1111101(Code).
n3byte1111100(Code) :- (Code =< 16'3094  ','  !  ','  n3byte11111000(Code)).
n3byte1111100(Code) :- n3byte11111001(Code).
n3byte11111000(Code) :- (Code =< 16'3094  ','  !  ','  Code >= 16'3041).
n3byte11111001(Code) :- (Code =< 16'309A  ','  !  ','  Code >= 16'3099).
n3byte1111101(Code) :- (Code =< 16'309E  ','  !  ','  n3byte11111010(Code)).
n3byte1111101(Code) :- n3byte11111011(Code).
n3byte11111010(Code) :- (Code =< 16'309E  ','  !  ','  Code >= 16'309D).
n3byte11111011(Code) :- (Code =< 16'30FA  ','  !  ','  Code >= 16'30A1).
n3byte111111(Code) :- (Code =< 16'312C  ','  !  ','  n3byte1111110(Code)).
n3byte111111(Code) :- n3byte1111111(Code).
n3byte1111110(Code) :- (Code =< 16'30FE  ','  !  ','  n3byte11111100(Code)).
n3byte1111110(Code) :- n3byte11111101(Code).
n3byte11111100(Code) :- (Code =< 16'30FE  ','  !  ','  Code >= 16'30FC).
n3byte11111101(Code) :- (Code =< 16'312C  ','  !  ','  Code >= 16'3105).
n3byte1111111(Code) :- (Code =< 16'9FA5  ','  !  ','  n3byte11111110(Code)).
n3byte1111111(Code) :- n3byte11111111(Code).
n3byte11111110(Code) :- (Code =< 16'9FA5  ','  !  ','  Code >= 16'4E00).
n3byte11111111(Code) :- (Code =< 16'D7A3  ','  !  ','  Code >= 16'AC00).
