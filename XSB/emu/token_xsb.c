/* File:      token_xsb.c
** Author(s): Richard A. O'Keefe, Deeporn H. Beardsley, Baoqiu Cui,
**    	      C.R. Ramakrishnan, Neng-Fa Zhou, warren, swift
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
** 
** XSB is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XSB is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: token_xsb.c,v 1.48 2013/01/04 14:56:22 dwarren Exp $
** 
*/

#include "xsb_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "token_xsb.h"
#include "psc_xsb.h"
#include "subp.h"
#include "register.h"
#include "error_xsb.h"
#include "memory_xsb.h"
#include "io_builtins_xsb.h"
#include "cinterf.h"
#include "sig_xsb.h"

#define exit_if_null(x) {\
  if(x == NULL){\
   xsb_exit( "Malloc Failed !\n");\
  }\
}

#define Char unsigned char
#define AlphabetSize 256
 
#define InRange(X,L,U) ((unsigned)((X)-(L)) <= (unsigned)((U)-(L)))
#define IsLayout(X) InRange(InType(X), SPACE, EOLN)
 

/*  VERY IMPORTANT NOTE: I assume that the stdio library returns the value
    EOF when character input hits the end of the file, and that this value
    is actually the integer -1.  You will note the DigVal(), InType(), and
    OuType() macros below, and there is a ChType() macro used in crack().
    They all depend on this assumption.
*/
 
#define InType(c)       (intab.chtype+1)[c]
#define DigVal(c)       (digval+1)[c]

//Char outqt[EOFCH+1];   /* All the "+1" appear because of the EOF char */
 
struct CHARS
    {
        int     eolcom;       /* End-of-line comment, default % */
        int     endeol;       /* early terminator of eolcoms, default none */
        int     begcom;       /* In-line comment start, default / */
        int     astcom;       /* In-line comment second, default * */
        int     endcom;       /* In-line comment finish, default / */
        int     radix;        /* Radix character, default ' */
        int     dpoint;       /* Decimal point, default . */
        int     escape;       /* String escape character, default \ */
        int     termin;       /* Terminates a clause */
        char    chtype[AlphabetSize+1];
    };
 
struct CHARS intab =   /* Special character table */
    {
        '%',                  /* eolcom: end of line comments */
        -1,                   /* endeol: early end for eolcoms */
        '/',                  /* begcom: in-line comments */
        '*',                  /* astcom: in-line comments */
        '/',                  /* endcom: in-line comments */
        '\'',                 /* radix : radix separator */
        '.',                  /* dpoint: decimal point */
        '\\',		      /* escape: string escape character */
        '.',                  /* termin: ends clause, sign or solo */
    {
        EOFCH,                /* really the -1th element of the table: */
    /*  ^@      ^A      ^B      ^C      ^D      ^E      ^F      ^G      */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  ^H      ^I      ^J      ^K      ^L      ^M      ^N      ^O      */
        SPACE,  SPACE,  EOLN,   SPACE,  EOLN,   SPACE,  SPACE,  SPACE,
    /*  ^P      ^Q      ^R      ^S      ^T      ^U      ^V      ^W      */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  ^X      ^Y      ^Z      ^[      ^\      ^]      ^^      ^_      */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  sp      !       "       #       $       %       &       '       */
        SPACE,  NOBLE,  LISQT,  SIGN,   SIGN,  PUNCT,  SIGN,   ATMQT,
    /*  (       )       *       +       ,       -       .       /       */
        PUNCT,  PUNCT,  SIGN,   SIGN,   PUNCT,  SIGN,   SIGN,   SIGN,
    /*  0       1       2       3       4       5       6       7       */
        DIGIT,  DIGIT,  DIGIT,  DIGIT,  DIGIT,  DIGIT,  DIGIT,  DIGIT,
    /*  8       9       :       ;       <       =       >       ?       */
        DIGIT,  DIGIT,  SIGN,   PUNCT,  SIGN,   SIGN,   SIGN,   SIGN,
    /*  @       A       B       C       D       E       F       G       */
        SIGN,   UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,
    /*  H       I       J       K       L       M       N       O       */
        UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,
    /*  P       Q       R       S       T       U       V       W       */
        UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,
    /*  X       Y       Z       [       \       ]       ^       _       */
        UPPER,  UPPER,  UPPER,  PUNCT,  SIGN,   PUNCT,  SIGN,   BREAK,
    /*  `       a       b       c       d       e       f       g       */
        SIGN,   LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,
    /*  h       i       j       k       l       m       n       o       */
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,
    /*  p       q       r       s       t       u       v       w       */
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,
    /*  x       y       z       {       |       }       ~       ^?      */
        LOWER,  LOWER,  LOWER,  PUNCT,  PUNCT,  PUNCT,  SIGN,   SPACE,
    /*  128     129     130     131     132     133     134     135     */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  136     137     138     139     140     141     142     143     */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  144     145     146     147     148     149     150     151     */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  152     153     154     155     156     157     158     159     */
        SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,  SPACE,
    /*  NBSP    !-inv   cents   pounds  ching   yen     brobar  section */
        SPACE,  SIGN,   SIGN,   SIGN,   SIGN,   SIGN,   SIGN,   SIGN,
    /*  "accent copyr   -a ord  <<      nothook SHY     (reg)   ovbar   */
        SIGN,   SIGN,   LOWER,  SIGN,   SIGN,   SIGN,   SIGN,   SIGN,
    /*  degrees +/-     super 2 super 3 -       micron  pilcrow -       */
        SIGN,   SIGN,   LOWER,  LOWER,  SIGN,   SIGN,   SIGN,   SIGN,
    /*  ,       super 1 -o ord  >>      1/4     1/2     3/4     ?-inv   */
        SIGN,   LOWER,  LOWER,  SIGN,   SIGN,   SIGN,   SIGN,   SIGN,
    /*  `A      'A      ^A      ~A      "A      oA      AE      ,C      */
        UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,
    /*  `E      'E      ^E      "E      `I      'I      ^I      "I      */
        UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,
    /*  ETH     ~N      `O      'O      ^O      ~O      "O      x times */
        UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  SIGN,
    /*  /O      `U      'U      ^U      "U      'Y      THORN   ,B      */
        UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  UPPER,  LOWER,
    /*  `a      'a      ^a      ~a      "a      oa      ae      ,c      */
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,
    /*  `e      'e      ^e      "e      `i      'i      ^i      "i      */
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,
    /*  eth     ~n      `o      'o      ^o      ~o      "o      -:-     */
#ifdef  vms
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,
#else
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  SIGN,
#endif
    /*  /o      `u      'u      ^u      "u      'y      thorn  "y       */
#ifdef  vms
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  SPACE
#else
        LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER,  LOWER
#endif
}};
 
char digval[AlphabetSize+1] =
    {
        99,                     /* really the -1th element of the table */
    /*  ^@      ^A      ^B      ^C      ^D      ^E      ^F      ^G      */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  ^H      ^I      ^J      ^K      ^L      ^M      ^N      ^O      */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  ^P      ^Q      ^R      ^S      ^T      ^U      ^V      ^W      */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  ^X      ^Y      ^Z      ^[      ^\      ^]      ^^      ^_      */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  sp      !       "       #       $       %       &       '       */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  (       )       *       +       ,       -       .       /       */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  0       1       2       3       4       5       6       7       */
        0,      1,      2,      3,      4,      5,      6,      7,
    /*  8       9       :       ;       <       =       >       ?       */
        8,      9,      99,     99,     99,     99,     99,     99,
    /*  @       A       B       C       D       E       F       G       */
        99,     10,     11,     12,     13,     14,     15,     16,
    /*  H       I       J       K       L       M       N       O       */
        17,     18,     19,     20,     21,     22,     23,     24,
    /*  P       Q       R       S       T       U       V       W       */
        25,     26,     27,     28,     29,     30,     31,     32,
    /*  X       Y       Z       [       \       ]       ^       _       */
        33,     34,     35,     99,     99,     99,     99,     0,  /*NB*/
    /*  `       a       b       c       d       e       f       g       */
        99,     10,     11,     12,     13,     14,     15,     16,
    /*  h       i       j       k       l       m       n       o       */
        17,     18,     19,     20,     21,     22,     23,     24,
    /*  p       q       r       s       t       u       v       w       */
        25,     26,     27,     28,     29,     30,     31,     32,
    /*  x       y       z       {       |       }       ~       ^?      */
        33,     34,     35,     99,     99,     99,     99,     99,
    /*  128     129     130     131     132     133     134     135     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  136     137     138     139     140     141     142     143     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  144     145     146     147     148     149     150     151     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  152     153     154     155     156     157     158     159     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  160     161     162     163     164     165     166     167     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  168     169     170(-a) 171     172     173     174     175     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  176     177     178(2)  179(3)  180     181     182     183     */
        99,     99,     2,      3,      99,     99,     99,     99,
    /*  184     185(1)  186(-o) 187     188     189     190     191     */
        99,     1,      99,     99,     99,     99,     99,     99,
    /*  192     193     194     195     196     197     198     199     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  200     201     202     203     204     205     206     207     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  208     209     210     211     212     213     214     215     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  216     217     218     219     220     221     222     223     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  224     225     226     227     228     229     230     231     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  232     233     234     235     236     237     238     239     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  240     241     242     243     244     245     246     247     */
        99,     99,     99,     99,     99,     99,     99,     99,
    /*  248     249     250     251     252     253     254     255     */
        99,     99,     99,     99,     99,     99,     99,     99
    };

Integer input_file_position(FILE *curr_in, STRFILE *instr) {
  if (curr_in) {
    return ftell(curr_in);
  } else {
    return (instr->strptr - instr->strbase);
  }
}

char *input_file_name(FILE *curr_in, STRFILE *instr) {
  if (curr_in) {
    int xsb_filedes = unset_fileptr(curr_in);
    if (xsb_filedes < 0) return "unknown";
    else return open_files[xsb_filedes].file_name;
  } else {
    return "reading string";
  }
}

int intype(int c)
{
  return (intab.chtype+1)[c];
}

static void SyntaxError(CTXTdeclc char *description)
{
  char message[MAXBUFSIZE];

  //	xsb_abort("[TOKENIZER] Syntax error: %s", description);
  strcpy(message, "(In tokenizer) ");
  strncpy(message+15,description,(MAXBUFSIZE - 15));
  xsb_syntax_error(CTXTc message);
}
 

void unGetC(int d, FILE *card, STRFILE *instr)
{
  if (instr) {
    (instr)->strcnt++;
    (instr)->strptr--;
  }
  else ungetc(d, card);
}

/* code_page_1252[code_1252] = codepoint. */

int code_page_1252[256] =
   {
 /*	NUL	SOH	STX	ETX	EOT	ENQ	ACK	BEL    */
	0x0000, 0x0001,	0x0002,	0x0003,	0x0004, 0x0005,	0x0006,	0x0007,
 /*	BS	HT	LF	VT	FF	CR	SO	SI     */
	0x0008, 0x0009, 0x000A, 0x000B,	0x000C, 0x000D, 0x000E, 0x000F,
 /*	DLE	DC1	DC2	DC3	DC4	NAK	SYN	ETB    */
	0x0010,	0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
 /*	CAN	EM	SUB	ESC	FS	GS	RS	US      */
	0x0018, 0x0019,	0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
 /*	SP	!	"	#	$	%	&	'       */
 	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
 /*	(	)	*	+	,	-	.	/       */
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
 /* 	0	1	2	3	4	5	6	7	*/
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
 /*	8	9	:	;	<	=	>	?	*/
	0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
 /* 	@	A	B	C	D	E	F	G	*/
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
 /*	H	I	J	K	L	M	N	O	*/
	0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
 /* 	P	Q	R	S	T	U	V	W	*/
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
 /*	X	Y	Z	[	\	]	^	_	*/
	0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
 /* 	`	a	b	c	d	e	f	g	*/
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
 /*	h	i	j	k	l	m	n	o	*/
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
 /* 	p	q	r	s	t	u	v	w	*/
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
 /*	x	y	z	{	|	}	~	DEL	*/
	0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
 /* 	euro	??	qut	Å£	ë°Ë	Å•	Å¶	ë°Ï	*/
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 
 /*	è®	Å©	Å™	Å´	Å¨	??	ÅÆ	??	*/
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
 /*	?? 	ë°¿	Å≤	Å≥	Å¥	Åµ	Å∂	ï°±	*/
	0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
 /*	è∏	Åπ	Å∫	Åª	Åº	??	 Åæ	Åø	*/
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
 /* 	NBSP	Å°	Å¢	Å£	ë°Ë	Å•	Å¶	ë°Ï	*/
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
 /*	ë°ß	Å©	Å™	Å´	Å¨	SHY	ÅÆ	ÅØ	*/
	0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
 /* 	ë°„	ë°¿	Å≤	Å≥	Å¥	Åµ	Å∂	ï°±	*/
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
 /*	Å∏	Åπ	Å∫	Åª	Åº	ÅΩ	Åæ	Åø 	*/
	0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
 /* 	Å¿	Å¡	Å¬	Å√	Åƒ	Å≈	Å∆	Å«	*/
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
 /*	Å»	Å…	Å 	ÅÀ	ÅÃ	ÅÕ	ÅŒ	Åœ	*/
	0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
 /* 	Å–	Å—	Å“	Å”	Å‘	Å’	Å÷	ë°¡	*/
	0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
 /*	Åÿ	ÅŸ	Å⁄	Å€	Å‹	Å›	Åﬁ	Åﬂ	*/
	0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 
 /* 	ë®§	ë®¢	Å‚	Å„	Å‰	ÅÂ	è°	ÅÁ	*/
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
 /*	ë®®	ë®¶	ë®∫	ÅÎ	ë®¨	ë®™	ÅÓ	ÅÔ	*/
	0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
 /* 	Å	ÅÒ	ë®∞	ë®Æ	ÅÙ	Åı	Åˆ	ë°¬ */
	0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
 /*	Å¯	ë®¥	ë®≤	Å˚	ë®π	Å˝	Å˛	Åˇ 	*/
	0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
   };

/* hash table to invert encoding */
int code_page_1252_hash[256] = {0};

int w1252_char_to_codepoint(byte **s) {
  return code_page_1252[*((*(s))++)];
}

/* many are self-inversions in w1252, i.e., 00-7F and A0-FF; For
   others see if they are already in hash-invert table. if not find
   and add to hash-invert table.  Hash collisions would overwrite
   previous and be inefficient */

byte *w1252_codepoint_to_str(int codepoint, byte *s) {
  int hc, hind, i;
  if (codepoint <= 0xFF && code_page_1252[codepoint] == codepoint) {
    *s++ = codepoint;
    return s;
  } else {
    hc = codepoint % 255;
    hind = code_page_1252_hash[hc];
    if (code_page_1252[hind] == codepoint) {
      *s++ = hind;
      return s;
    }
    for (i=0x80; i < 0xA0; i++) { /* assuming 00-7F,A0-FF are self-inverts */
      if (code_page_1252[i] == codepoint) {
	code_page_1252_hash[hc] = i;
	*s++ = i;
	return s;
      }
    }
    *s++ = 255; // ERROR  (or maybe just delete and return s?)
    return s;  // error, not found!
  }
}

extern byte *utf8_codepoint_to_str(int, byte *);
/* nfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfz */
/* convert a code point to char array s, which has n remaining slots */
#define CODEPOINT_TO_UTF8_STR(code,s,n) {		 \
    if (code <= 127) {					 \
      n--;						 \
      if (n < 0) {					 \
        realloc_strbuff(CTXTc &strbuff, &s, &n);	 \
      }							 \
      *s++ = (char)code;				 \
    } else {						 \
      byte *s1;						 \
      if (n < 4){					 \
	realloc_strbuff(CTXTc &strbuff, &s, &n);	 \
      }							 \
      s1 = utf8_codepoint_to_str(code,s);		 \
      n -= (int)(s1-s);					 \
      s = s1;						 \
    }							 \
  }

byte *codepoint_to_str(int code, int charset, byte *s) {
  switch (charset) {
  case LATIN_1: 
    *s++ = code;
    return s;
  case UTF_8:
    return utf8_codepoint_to_str(code,s);
  case CP1252:
    return w1252_codepoint_to_str(code,s);
  default:
    printf("bad charset type in codepoint_to_str: %d\n",charset);
    return s;
  }
}

byte *utf8_codepoint_to_str(int code, byte *s){
  if (code<0x80) {
    *s++ = code;
  } else if (code<0x800){
    *s++ = 192+code/64;
    *s++ = 128+code%64;
  } else if (code<0x10000){
    *s++ = 224+code/4096;
    *s++ = 128+code/64%64;
    *s++ = 128+code%64;
  } else {
    *s++ = 240+code/262144;
    *s++ = 128+code/4096%64;
    *s++ = 128+code/64%64;
    *s++ = 128+code%64;
  }
  return s;
}

int char_to_codepoint(int charset, byte **s_ptr) {
  switch (charset) {
  case LATIN_1: 
    return *(*s_ptr++);
  case UTF_8:
    return utf8_char_to_codepoint(s_ptr);
  case CP1252:
    return w1252_char_to_codepoint(s_ptr);
  default:
    printf("bad charset type in char_to_codepoint: %d\n",charset);
    return 0;
  }
}

/* return the codepoint of the current utf-8 char, setting s_ptr to the beginning of the next char */
int utf8_char_to_codepoint(byte **s_ptr){
  byte *s;
  int c, b2, b3, b4;
  s = *s_ptr;
  c = *s++;
  
  if (c & 0x80){                      /* leading byte of a utf8 char? */
    if ((c & 0xe0) == 0xc0){          /* 110xxxxx */
      b2 = *s++;
      if ((b2 & 0xc0) == 0x80){       /* 110xxxxx 10xxxxxx */
	c = (((c & 0x1f) << 6) | (b2 & 0x3f));
      }  else {
	s--;
      }
    } else if ((c & 0xf0) == 0xe0){    /* 1110xxxx */
      b2 = *s++;
      if ((b2 & 0xc0) == 0x80){        /* 1110xxxx 10xxxxxx */
	b3 = *s++;
	if ((b3 & 0xc0) == 0x80){      /* 1110xxxx 10xxxxxx 10xxxxxx */
	  c = (((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
	} else {
	  s--;
	  s--;
	}
      } else {
	s--;
      }
    } else if ((c & 0xf8) == 0xf0){    /* 11110xxx */
      b2 = *s++;
      if ((b2 & 0xc0) == 0x80){        /* 11110xxx 10xxxxxx */
	b3 = *s++;
	if ((b3 & 0xc0) == 0x80){      /* 11110xxx 10xxxxxx 10xxxxxx */
	  b4 = *s++;
	  if ((b4 & 0xc0) == 0x80){    /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
	    c = (((c & 0xf) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
	  } else {
	    s--;
	    s--;
	    s--;
	  }
	} else {
	  s--;
	  s--;
	}
      } else {
	s--;
      }
    }
  }
  *s_ptr = s;
  return c;
}

extern int utf8_GetCode(FILE *, STRFILE *, int);

int GetCode(int charset, FILE *curr_in, STRFILE *instr) {
  int c;
  c = GetC(curr_in,instr);
  if (c < 0) return c;  /* if eof, return it */
  if (instr) { /* encoding in strings is always utf8 */
    return utf8_GetCode(curr_in, instr, c);
  }
  switch (charset) {
  case LATIN_1:
    return c;
  case UTF_8:   
    return utf8_GetCode(curr_in, instr, c);
  case CP1252:
    return code_page_1252[c];
  default: printf("ERROR: bad current_charset in GetCode: %d\n",charset);
    return 0;
  }
}

int GetCodeP(int io_port) {
  int c, charset;
  FILE *fptr; STRFILE *sfptr;
  io_port_to_fptrs(io_port,fptr,sfptr,charset);
  c = GetC(fptr,sfptr);
  if (c < 0) return c; /* eof */
  if (sfptr) { /* encoding in strings is always utf8 */
    return utf8_GetCode(fptr, sfptr, c);
  }
  switch (charset) {
  case LATIN_1:
    return c;
  case UTF_8:   
    return utf8_GetCode(fptr, sfptr, c);
  case CP1252:
    return code_page_1252[c];
  default: printf("ERROR: bad current_charset in GetCode: %d\n",charset);
    return 0;
  }
}



/* read a utf8 char whose leading byte is c */
int utf8_GetCode(FILE *curr_in, STRFILE *instr, int c){
  int b2,b3,b4;
  if (c < 0x80) return c;           /* ascii */
  if ((c & 0xe0) == 0xc0){          /* 110xxxxx */
    b2 = GetC(curr_in,instr);
    if ((b2 & 0xc0) == 0x80){       /* 110xxxxx 10xxxxxx */
      return (((c & 0x1f) << 6) | (b2 & 0x3f));
    } else {                        /* not utf8 char */
      if (b2>0) {unGetC((char)b2,curr_in,instr);}/* don't unget EOF */
    }
  } else if ((c & 0xf0) == 0xe0){    /* 1110xxxx */
    b2 = GetC(curr_in,instr);	
    if ((b2 & 0xc0) == 0x80){        /* 1110xxxx 10xxxxxx */
      b3 = GetC(curr_in,instr);	
      if ((b3 & 0xc0) == 0x80){      /* 1110xxxx 10xxxxxx 10xxxxxx */
	return (((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
      } else {
	if (b3>0) {unGetC((char)b3,curr_in,instr);}
	unGetC((char)b2,curr_in,instr);
      }
    } else {
      if (b2>0) {unGetC((char)b2,curr_in,instr);}
    }
  } else if ((c & 0xf8) == 0xf0){    /* 11110xxx */
    b2 = GetC(curr_in,instr);	
    if ((b2 & 0xc0) == 0x80){        /* 11110xxx 10xxxxxx */
      b3 = GetC(curr_in,instr);	
      if ((b3 & 0xc0) == 0x80){      /* 11110xxx 10xxxxxx 10xxxxxx */
	b4 = GetC(curr_in,instr);	
	if ((b4 & 0xc0) == 0x80){    /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
	  return (((c & 0xf) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
	} else {
	  if (b2>0) {unGetC((char)b4,curr_in,instr);}
	  unGetC((char)b3,curr_in,instr);
	  unGetC((char)b2,curr_in,instr);
	}
      } else {
	if (b3>0) {unGetC((char)b3,curr_in,instr);}
	unGetC((char)b2,curr_in,instr);
      }
    } else {
      if (b2>0) {unGetC((char)b2,curr_in,instr);}
    }
  }
  return c;
}

void write_string_code(FILE *fptr, int charset, byte *string) {
  if (charset == UTF_8) { /* internal, just print it */
    fprintf(fptr, "%s", string); 
  } else {
    while (*string != '\0') {
      PutCode(utf8_char_to_codepoint(&string),charset,fptr);
    }
  }
}

//#define strungetc(p) (((p)->strptr>(p)->strbase)? ((int)*(p)->strptr--,(int)*(p)->strcnt++): -1)
int strungetc(STRFILE * p) {
  if (p -> strptr > p -> strbase) {
    p -> strcnt++;
    return (int)*(p) -> strptr--;
  }
  else return -1;
}

extern int utf8_nchars(byte *);

int coding_nchars(int charset, byte *s) {
  switch (charset) {
  case LATIN_1:
  case CP1252:
    return (int)strlen((char *)s);
  case UTF_8:   
    return utf8_nchars(s);
  default: printf("ERROR: bad current_charset in coding_nchars: %d\n",charset);
    return 0;
  }
}

/* return the number of utf-8 chars in s. */
int utf8_nchars(byte *s){
  unsigned int c;
  int count = 0;
  c = *s++;
  while (c != '\0'){
    count++;
    if (c & 0x80){                      /* leading byte of a utf8 char? */
      if ((c & 0xe0) == 0xc0){          /* 110xxxxx */
	c = *s++;
	if ((c & 0xc0) == 0x80){        /* 110xxxxx 10xxxxxx */
	                                /* two bytes, but one char */
	} else {
	  s--;                          /* not utf8, couting bytes instead */
	}
      } else if ((c & 0xf0) == 0xe0){    /* 1110xxxx */
	c = *s++;
	if ((c & 0xc0) == 0x80){        /* 1110xxxx 10xxxxxx */
	  c = *s++;
	  if ((c & 0xc0) == 0x80){      /* 1110xxxx 10xxxxxx 10xxxxxx */
	                                /* three bytes, but one char */
	  } else {
	    s--;
	    s--;
	  }
	} else {
	  s--;
	}
      } else if ((c & 0xf8) == 0xf0){    /* 11110xxx */
	c = *s++;
	if ((c & 0xc0) == 0x80){        /* 11110xxx 10xxxxxx */
	  c = *s++;
	  if ((c & 0xc0) == 0x80){      /* 11110xxx 10xxxxxx 10xxxxxx */
	    c = *s++;
	    if ((c & 0xc0) == 0x80){    /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
	                                /* four bytes, but one char */
	    } else {
	      s--;
	      s--;
	      s--;
	    }
	  } else {
	      s--;
	      s--;
	  }
	} else {
	  s--;
	}
      }
    }
    c = *s++;
  }
  return count;
}

/* nfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfz */
#define UNICODE_ESCAPE_OFF FALSE
 
 
/*  GetToken() reads a single token from the input stream and returns
    its type, which is one of the following:

        TK_INT		-- an integer 
        TK_INTFUNC	-- an integer functor
        TK_VARFUNC	-- a HiLog variable( pair 
        TK_FUNC		-- an atom( pair
        TK_ATOM		-- an atom
        TK_VAR		-- a variable
        TK_PUNC		-- a single punctuation mark
      TK_HPUNC	-- punctuation ) followed by a ( in HiLog terms
        TK_LIST		-- a quoted list of character codes (in buffer)
        TK_STR		-- a quoted string
        TK_EOC		-- end of clause (normally '.\n').
        TK_EOF		-- signifies end-of-file.
	TK_REAL		-- a real, in double_v.
	TK_REALFUNC	-- a real, in double_v.

    In most of the above cases (except the last two), the text of the 
    token is in AtomStr.
    There are two questions: between which pairs of adjacent tokens is
    a space (a) necessary, (b) desirable?  There is an additional
    dummy token type used by the output routines, namely
        NOBLE		-- extra space is definitely not needed.
    I leave it as an exercise for the reader to answer question (a).
    Since this program is to produce output I find palatable (even if
    it isn't exactly what I'd write myself), extra spaces are ok.  In
    fact, the main use of this program is as an editor command, so it
    is normal to do a bit of manual post-processing.  Question (b) is
    the one to worry about then.  My answer is that a space is never
    written
        - after  PUNCT ( [ { |
        - before PUNCT ) ] } | , <ENDCL>
    is written after comma only sometimes, and is otherwise always
    written.  The variable lastput thus takes these values:

        ALPHA      -- put a space except before PUNCT
        SIGN       -- as alpha, but different so ENDCL knows to put a space.
        NOBLE      -- don't put a space
        ENDCL      -- just ended a clause
        EOFCH      -- at beginning of file
*/

#ifndef MULTI_THREAD
struct xsb_token_t res_str;
struct xsb_token_t *token = &res_str;

int     lastc = ' ';    /* previous character */
byte*   strbuff = NULL;             /* Pointer to token buffer; Will be
				       allocated on first call to GetToken */
int     strbuff_len = InitStrLen;   /* length of first allocation will be
				       InitStrLen; will be doubled on
				       subsequent overflows */
double  double_v;
Integer	rad_int;
#endif

char    tok2long[]      = "token too long";
char    eofinrem[]      = "end of file in comment";
char    badexpt[]       = "bad exponent";
char    badradix[]      = "radix not 0 or 2..36";
 
 
/*  read_character(FILE* card, STRFILE* instr, Char q)
    reads one character from a quoted atom, list, string, or character.
    Doubled quotes are read as single characters, otherwise a
    quote is returned as -1 and lastc is set to the next character.
    If the input syntax has character escapes, they are processed.
    Note that many more character escape sequences are accepted than
    are generated.  There is a divergence from C: \xhh sequences are
    two hexadecimal digits long, not three.
    Note that the \c and \<space> sequences combine to make a pretty
    way of continuing strings.  Do it like this:
        "This is a string, which \c
       \ has to be continued over \c
       \ several lines.\n".

    -- If encounters the EOF, then return -2. (Baoqiu, 2/16/1997)
*/
 
static int read_character(CTXTdeclc register FILE *card,
			  register STRFILE *instr, int charset,
			  register int q)
{
        register int c;
	char message[200];
 
        c = GetCode(charset,card,instr);
BACK:   if (c < 0) {
          if (c == EOF) { /* to mostly handle cygwin stdio.h bug ... */
READ_ERROR: 
	    if (!instr && ferror(card)) {
	      xsb_warn(CTXTc "[TOKENIZER] I/O error in file %s: %s at position %d\n",
		       input_file_name(card,instr),input_file_position(card,instr),
		       strerror(errno));
	    }
	    if (q < 0) {
	      snprintf(message,200,"end of file in character constant in %s",
		       input_file_name(card,instr));
	      SyntaxError(CTXTc message);
		//		return -2;		/* encounters EOF */
            } else {
	      snprintf(message,200, "end of file in %cquoted%c constant in %s", 
		       q, q, input_file_name(card,instr));
	      SyntaxError(CTXTc message);
		//		return -2;		/* encounters EOF */
            }
	  }
          else c = c & 0xff;  /* in which getc returns "signed" char? */
        }
        if (c == q) {
	    c = GetCode(charset,card,instr);
            if (c == q) return c;
            lastc = c;
            return -1;
        } else
        if (c != intab.escape) {
	  return c;
        }
        /*  If we get here, we have read the "\" of an escape sequence  */
        c = GetCode(charset,card,instr);
        switch (c) {
            case EOF:
	        if (!instr && ferror(card)) 
		  xsb_warn(CTXTc "[TOKENIZER] I/O error: %s at position %d in %s\n",
			   strerror(errno),input_file_position(card,instr),
			   input_file_name(card,instr));
		clearerr(card);
                goto READ_ERROR;
	    case 'a':		        /* alarm */
                return  '\a';
            case 'b':		        /* backspace */
                return  '\b';
            case 'f':		        /* formfeed */
                return '\f';
            case '\n':		      /* seeing a newline */
	      //	      while (IsLayout(c = GetCode(charset,card,instr)));
	        c = GetCode(charset,card,instr); // ignore it
                goto BACK;
	case '\r':  // newline for windows eol?
	        c = GetCode(charset,card,instr); // ignore it
		if (c == '\n') c = GetCode(charset,card,instr);
		goto BACK;
            case 'n':		        /* newline */
	        return '\n';
            case 'r':		        /* return */
                return '\r';
            case 's':		        // space
                return ' ';
            case 't':		        /* tab */
                return  '\t';
            case 'v': 		      /* vertical tab */
                return '\v';
            case 'x':		        /* hexadecimal */
                { int n = 0;
		  c = GetCode(charset,card,instr);
		  while (DigVal(c) < 16) {
		    n = n * 16 + DigVal(c);
		    c = GetCode(charset,card,instr);
		  }
		  if (c < 0) goto READ_ERROR;
		  if (c != '\\') {
		    unGetC(c, card, instr);
		    //  xsb_warn(CTXTc "Ill-formed \\xHEX\\ escape: %d (dec) at position %d in %s",
		    //	     n,input_file_position(card,instr),
		    //	     input_file_name(card,instr));
		  }
		  return n;
                }

   	    /* nfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfz */
	    case 'u':  case 'U':                     /* unicode escape */
	      {int n,i;
		  if (!UNICODE_ESCAPE_OFF) {
		    n = 0;
		    for (i=1; i<=4; i++){                /* \uxxxx */ 
		      c = GetCode(charset,card,instr);
		      if (DigVal(c) <= 15 && c != '_'){
			n = (n<<4) + DigVal(c);
		      } else if (c != '\\') {
			unGetC(c,card,instr);
			xsb_warn(CTXTc "Ill-formed \\u unicode escape: %d (dec) at position %d in %s",
				 n,input_file_position(card,instr),
				 input_file_name(card,instr));
			return n;
		      } else return n;
		    }
		    c = GetCode(charset,card,instr);
		    if (DigVal(c) <= 15 && c != '_'){      /* \uxxxxxxxx */
		      n = (n<<4) + DigVal(c);	
		      for (i=1; i<=3; i++){
			c = GetCode(charset,card,instr);
			if (DigVal(c) <= 15 && c != '_'){
			  n = (n<<4) + DigVal(c);
			} else if (c != '\\') {
			  unGetC(c,card,instr);
			  xsb_warn(CTXTc "Ill-formed \\u unicode escape: %d (dec) at position %d in %s",
				   n,input_file_position(card,instr),
				   input_file_name(card,instr));
			  return n;
			} else return n;
		      }
		    } else {
		      if (c>0 && c != '\\') {unGetC(c,card,instr);}	
		    }
		    return n;
		  } else {   // not UTF_8
		    (void) unGetC(c, card, instr);
		    return('\\');
		  }
	      }
   	    /* nfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfznfz */

	case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
	        { int n = 0;
		  do {
		    n = n * 8 + DigVal(c);
		    c = GetCode(charset,card,instr);
		  } while (DigVal(c) < 8);
		  //		  if (c < 0) goto READ_ERROR;
		  if (c != '\\') {
		    unGetC(c, card, instr);
		    //		    xsb_warn(CTXTc "Ill-formed \\OCTAL\\ escape: %d (dec) at position %d in %s",
		    //			     n,input_file_position(card,instr),
		    //			     input_file_name(card,instr));
		  }
		  return n;
                }
	    case '\\':			/* backslash */
	        return '\\';
	    case '\'':			/* single quote */
	        return '\'';
	    case '"':			/* double quote */
	        return '"';
	    case '`':			/* back quote */
	        return '`';
	    default:			/* return the \, not an escape */
	      (void) unGetC(c, card, instr);
	      return '\\';
        }
    }
 
/*  com0plain(card, instr, endeol)
    These comments have the form
        <eolcom> <char>* <newline>                      {PUNCT}
    or  <eolcom><eolcom> <char>* <newline>              {SIGN }
    depending on the classification of <eolcom>.  Note that we could
    handle ADA comments with no trouble at all.  There was a Pop-2
    dialect which had end-of-line comments using "!" where the comment
    could also be terminated by "!".  You could obtain the effect of
    including a "!" in the comment by doubling it, but what you had
    then was of course two comments.  The endeol parameter of this
    function allows the handling of comments like that which can be
    terminated either by a new-line character or an <endeol>, whichever
    comes first.  For ordinary purposes, endeol = -1 will do fine.
    When this is called, the initial <eolcom>s have been consumed.
    We return the first character after the comment.
    If the end of the source file is encountered, we do not treat it
    as an error, but quietly close the comment and return EOF as the
    "following" character.
 
*/
static int com0plain(register FILE *card,	/* source file */
       		     register STRFILE *instr,	/* source string, if non-NULL */
		     register int endeol)	/* The closing character "!" */
{
    register int c;
 
    while ((c = GetC(card,instr)) >= 0 && c != '\n' && c != endeol) ;
    if (c >= 0) c = GetC(card,instr);
    return c;
}
 
 
/*  The states in the next two functions are
        0       - after an uninteresting character
        1       - after an "astcom"
        2       - after a  "begcom"
    Assuming begcom = "(", astom = "#", endcom = ")",
    com2plain will accept "(#)" as a complete comment.  This can
    be changed by initialising the state to 0 rather than 1.
    The same is true of com2nest, which accepts "(#(#)#) as a
    complete comment.  Changing it would be rather harder.
    Fixing the bug where the closing <astcom> is copied if it is
    not an asterisk may entail rejecting "(#)".
*/
 
/*  com2plain(card, instr, stcom, endcom)
    handles PL/I-style comments, that is, comments which begin with
    a pair of characters <begcom><astcom> and end with a pair of
    chracters <astcom><endcom>, where nesting is not allowed.  For
    example, if we take begcom='(', astcom='*', endcom=')' as in
    Pascal, the comment "(* not a (* plain *)^ comment *) ends at
    the "^".
    For this kind of comment, it is perfectly sensible for any of
    the characters to be equal.  For example, if all three of the
    bracket characters are "#", then "## stuff ##" is a comment.
    When this is called, the initial <begcom><astcom> has been consumed.
*/
static int com2plain(register FILE *card,	/* source file */
		     register STRFILE *instr,	/* source string, if non-NULL */
		     int astcom,		/* The asterisk character "*" */
		     int endcom)		/* The closing character "/" */
{
        register int c;
        register int state;

        for (state = 0; (c = GetC(card,instr)) >= 0; ) {
            if (c == endcom && state) break;
            state = c == astcom;
        }
        if (c < 0) return 1; 
	else return 0; 
}

#ifndef MULTI_THREAD 
int token_too_long_warning = 1;
#endif

void realloc_strbuff(CTXTdeclc byte **pstrbuff, byte **ps, int *pn)
     /* Expand token buffer when needed.
      * pstrbuff: base address of current buffer
      * ps: tail of current buffer
      * pn: number of elements remaining in the current buffer
      * --  C.R., 7/27/97
     */
{ 
  byte *newbuff;

  newbuff = (byte *)mem_realloc(*pstrbuff, strbuff_len, strbuff_len * 2,OTHER_SPACE);
  exit_if_null(newbuff);
  if (token_too_long_warning) {
    xsb_warn(CTXTc "Extra-long token. Runaway string?");
    token_too_long_warning = 0;
  }

  if (*pstrbuff != newbuff) {
    /* Aha, base address has changed, so change s too*/
    *ps += newbuff - *pstrbuff;
  }
  *pstrbuff = newbuff;
  *pn += strbuff_len;
  strbuff_len *= 2;
  return;
}


struct xsb_token_t *GetToken(CTXTdeclc int io_port, int prevch)
{
        byte *s;
        register int c, d = 0;
        Integer oldv = 0, newv = 0; 
        int n;
	int charset;
	FILE *card;
	STRFILE *instr;

	//	printf("==> GetTOken \n");
	if (strbuff == NULL)
	  {
	    /* First call for GetToken, so allocate a buffer */
	    strbuff = (byte *)mem_alloc(strbuff_len,OTHER_SPACE);
	  }
	s = strbuff;
	n = strbuff_len;

        c = prevch; 
	if ((io_port < 0) && (io_port >= -MAXIOSTRS)) {
	  instr = strfileptr(io_port);
	  charset = UTF_8;
	  card = NULL;
	} else {
	  instr = NULL;
	  SET_FILEPTR_CHARSET(card,charset,io_port);
	}

START:
        switch (InType(c)) {
 
            case DIGIT:
                /*  The following kinds of numbers exist:
                      (1) unsigned decimal integers: d+
                      (2) unsigned based integers: d+Ro+[R]
                      (3) unsigned floats: d* [. d*] [e +/-] d+
                      (4) characters: 0Rc[R]
                    We allow underscores in numbers too, ignoring them.
                */
                do {
                    if (c != '_') *s++ = c;
                    c = GetCode(charset,card,instr);
                } while (InType(c) <= BREAK);
                if (c == intab.radix) {  
                    *s = 0;
                    for (d = 0, s = strbuff; (c = *s++);) {
		      d = d*10-'0'+c;
		    }
		    if (d == 1 || d > 36) {
		      SyntaxError(CTXTc badradix);
		      //		      token->type = TK_ERROR;
		      //		      return token;
		    }
                    if (d == 0) {
		      /*  0'c['] is a character code  */
		      d = read_character(CTXTc card, instr, charset, -1);
		      //		      sprintf(strbuff, "%d", d);
		      rad_int = d;
		      d = GetCode(charset,card,instr);
		      //		      rad_int = atoi(strbuff);
		      token->nextch = d == 
			intab.radix ? GetCode(charset,card,instr):d;
		      token->value = (char *)(&rad_int);
		      token->type = TK_INT;
		      return token;
		    }
		    /* handle non-0 radix */
		NONZERO_RADIX:      while (c = GetCode(charset,card,instr), DigVal(c) < d)
                        if (c != '_') {
			    oldv = newv;
			    newv = newv*d + DigVal(c);
			    if (newv < oldv || newv > MY_MAXINT) {
				xsb_error("Overflow in radix notation, returning float");
			        double_v = oldv*1.0*d + DigVal(c);
				while (c = GetCode(charset,card,instr), DigVal(c) < 99)
                        	    if (c != '_') 
					double_v = double_v*d + DigVal(c);
                    		if (c == intab.radix) 
				  c = GetCode(charset,card,instr);
                    		token->nextch = c;
				token->value = (char *)(&double_v);
				if (c == '(')	/* Modified for HiLog */	
					token->type = TK_REALFUNC;
				else
					token->type = TK_REAL;
				return token;
			    }
			}
		    rad_int = newv;
                    if (c == intab.radix) 
		      c = GetCode(charset,card,instr);
                    token->nextch = c;
		    token->value = (char *)(&rad_int);
                    if (c == '(')	/* Modified for HiLog */
			token->type = TK_INTFUNC;
                    else
			token->type = TK_INT;
                    return token;
                }
		else if (c == intab.dpoint) {
		  d = GetCode(charset,card,instr);
                    if (InType(d) == DIGIT) {
LAB_DECIMAL:                *s++ = '.';
                        do {
                            if (d != '_') *s++ = d;
                            d = GetCode(charset,card,instr);
                        } while (InType(d) <= BREAK);
                        if ((d | 32) == 'e') {
                            *s++ = 'E';
                            d = GetCode(charset,card,instr);
                            if (d == '-') *s++ = d, d = GetCode(charset,card,instr);
                            else if (d == '+') d = GetCode(charset,card,instr);
                            if (InType(d) > BREAK) {
				SyntaxError(CTXTc badexpt);
				//				token->type = TK_ERROR;
				//				return token;
			    }
                            do {
                                if (d != '_') *s++ = d;
                                d = GetCode(charset,card,instr);
                            } while (InType(d) <= BREAK);
                        }
                        c = d;
                        *s = 0;
			sscanf((char *)strbuff, "%lf", &double_v);
			token->nextch = c;
			token->value = (char *)(&double_v);
			if (c == '(')	/* Modified for HiLog */	
				token->type = TK_REALFUNC;
			else
				token->type = TK_REAL;
                        return token;
                    } else {
		        unGetC(d, card, instr);
                        /* c has not changed */
                    }
		}
		else if (c == 'b' || c == 'o' || c == 'x') {
		  int oc;
		  *s = 0;
		  for (d = 0, s = strbuff; (oc = *s++);)
		    d = d*10-'0'+oc;
		  if (d == 0) {
		    if (c == 'b') d = 2;
		    else if (c == 'o') d = 8;
		    else /*if (c == 'x')*/ d = 16;
		  } else {
		    token->nextch = c;
		    rad_int = d;
		    token->value = (char *)(&rad_int);
		    token->type = TK_INT;
		    return token;
		  }
		  goto NONZERO_RADIX;
		}
		token->nextch = c;
                *s = 0;
		for (rad_int = 0, s = strbuff; (c = *s++);) {
		  if (rad_int < MY_MAXINT/10 || 
		      (rad_int == MY_MAXINT/10 && (c-'0') <= MY_MAXINT % 10)) 
		    rad_int = rad_int*10-'0'+c;
		  else {
		    xsb_error("Overflow in integer, returning MAX_INT");
		    rad_int = MY_MAXINT;
		    break;
		  }		    
		  /*		  d = rad_int;
		  rad_int = rad_int*10-'0'+c;
		  if (rad_int < d || rad_int > MY_MAXINT) {
		    xsb_error("Overflow in integer, returning MAX_INT");
		    rad_int = MY_MAXINT;
		    break;
		    }*/
		}
		  //		rad_int = atoi(strbuff);
		token->value = (char *)(&rad_int);
		if (c == '(')	/* Modified for HiLog */
			token->type = TK_INTFUNC;
		else
			token->type = TK_INT;
                return token;
 
            case BREAK:        /* Modified for HiLog */
	      do {
                    if (--n < 0) {
		      realloc_strbuff(CTXTc &strbuff, &s, &n); 
		      }
                    *s++ = c, c = GetCode(charset,card,instr);
                } while (InType(c) <= LOWER);
                *s = 0;
                if (c == '(') {
                    token->nextch = c;
                    token->value = (char *)strbuff;
                    token->type = TK_VVARFUNC;
                    return token;
                } else {
		    token->nextch = c;
		    token->value = (char *)strbuff;
                    token->type = TK_VVAR;
                    return token;
                }
 
            case UPPER:         /* Modified for HiLog */
                do {
                    if (--n < 0) {
		      realloc_strbuff(CTXTc &strbuff, &s, &n);
		    }
                    *s++ = c, c = GetCode(charset,card,instr);
                } while (InType(c) <= LOWER);
                *s = 0;
                if (c == '(') {
                    token->nextch = c;
                    token->value = (char *)strbuff;
                    token->type = TK_VARFUNC; 
                    return token;
                } else {
	            token->nextch = c;
		    token->value = (char *)strbuff;
                    token->type = TK_VAR;
                    return token;
                }
 
            case LOWER:
                do {
                    if (--n < 0) {
		      realloc_strbuff(CTXTc &strbuff, &s, &n);
		    }
                    *s++ = c, c = GetCode(charset,card,instr);
                } while (InType(c) <= LOWER);
                *s = 0;
SYMBOL:         if (c == '(') {
		    token->nextch = c;
		    token->value = (char *)strbuff;
		    token->type = TK_FUNC;
		    return token;
                } else {
		    token->nextch = c;
		    token->value = (char *)strbuff;
		    token->type = TK_ATOM;
		    return token;
                }
 
            case SIGN:
	        *s = c, d = GetCode(charset,card,instr);
                if (c == intab.begcom && d == intab.astcom) {
ASTCOM:             if (com2plain(card, instr, d, intab.endcom)) {
			SyntaxError(CTXTc eofinrem);
			//			token->type = TK_ERROR;
			//			return token;
		    }
		    c = GetCode(charset,card,instr);
                    goto START;
                } else
                if (c == intab.dpoint && InType(d) == DIGIT) {
                    *s++ = '0';
                    goto LAB_DECIMAL;
                }
                while (InType(d) == SIGN) {
                    if (--n == 0) {
		      realloc_strbuff(CTXTc &strbuff, &s, &n);
		    }
                    *++s = d, d = GetCode(charset,card,instr);
                }
                *++s = 0;
                if ((InType(d)>=SPACE || d == '%') && c==intab.termin && strbuff[1]==0) {
		    token->nextch = d;
		    token->value = 0;
		    token->type = TK_EOC;
		    unGetC(d, card, instr); // dsw added ??
		    return token;       /* i.e. '.' followed by layout */
                }
                c = d;
                goto SYMBOL;
 
            case NOBLE:
                if (c == intab.termin) {
                    *s = 0;
		    token->nextch = ' ';
		    token->value = 0;
		    token->type = TK_EOC;
		    return token;
                } else
                if (c == intab.eolcom) {
                    c = com0plain(card, instr, intab.endeol);
                    goto START;
                }
                *s++ = c, *s = 0;
                c = GetCode(charset,card,instr);
                goto SYMBOL;
 
            case PUNCT:
                if (c == intab.termin) {
                    *s = 0;
		    token->nextch = ' ';
		    token->value = 0;
		    token->type = TK_EOC;
		    return token;
                } else
                if (c == intab.eolcom) {
                    c = com0plain(card, instr, intab.endeol);
                    goto START;
                }
                d = GetCode(charset,card,instr);
                if (c == intab.begcom && d == intab.astcom) goto ASTCOM;
		if ((c == '{' && d == '}') || (c == '[' && d == ']')) {
		  /* handle {}, and [] as tokens, so can be functors */
		  strbuff[0] = c; strbuff[1] = d; strbuff[2] = 0;
		  c = GetCode(charset,card,instr);
		  goto SYMBOL;
		}
 
              /*  If we arrive here, c is an ordinary punctuation mark  */
/*                  if (c == '(')  *s++ = ' '; */
		    /* In PSBProlog (as in most other Prologs) it was     */
		    /* necessary to distinguish between atom( and atom (  */
		    /* This was originally used for operators but it was  */
		    /* deleted by Jiyang - seems to cause no problem for  */
		    /* HiLog.                                             */
                *s++ = c, *s = 0;
		token->nextch = d;
		token->value = (char *)strbuff;
	   /*  In HiLog we need the following distinction so that we do not */
	   /*  recognize terms of the form f(p) (c,d) which are not HiLog   */
	   /*  terms as the same HiLog term as f(p)(c,d) which is a legal   */
           /*  HiLog term. All this mess is caused by the fact that this    */
       	   /*  scanner throws away all the spaces and we have no other way  */
           /*  of recognizing whether the next left parenthesis belongs to  */
           /*  the same term being read, (especially since it is not        */
           /*  desirable to keep the previous character read).              */
		if (c == ')' && d == '(')
		  token->type = TK_HPUNC;
		else
		  token->type = TK_PUNC;
                return token;
 
            case CHRQT:
                /*  `c[`] is read as an integer.
                    Eventually we should treat characters as a distinct
                    token type, so they can be generated on output.
                    If the character quote, atom quote, list quote,
                    or string quote is the radix character, we should
                    generate 0'x notation, otherwise `x`.
                */
	        d = read_character(CTXTc card, instr, charset, -1);
                sprintf((char *)strbuff, "%d", d);
                d = GetCode(charset,card,instr);
		rad_int = atoi((char *)strbuff);
                token->nextch = d == c ? GetCode(charset,card,instr) : d;
		token->value = (char *)(&rad_int);
		token->type = TK_INT;
                return token;
 
            case ATMQT:
	        while ((d = read_character(CTXTc card, instr, charset, c)) != -1) {
		  CODEPOINT_TO_UTF8_STR(d,s,n); /* nfz */
		}
                *s = 0;
                c = lastc;
                goto SYMBOL;

/**** this case deleted, messed up treatment of $, which was STRQT
            case STRQT:
                while ((d = read_character(CTXTc card, instr, charset, c)) >= 0) {
                    if (--n < 0) {
		      realloc_strbuff(CTXTc &strbuff, &s, &n);
		    }
                    *s++ = d;
                }
                *s = 0;
		token->nextch = lastc;
		token->value = strbuff;
		token->type = TK_STR;
                return token;
case deleted ****/

	    case LISQT: 
	        while ((d = read_character(CTXTc card, instr, charset, c)) != -1) {
		  CODEPOINT_TO_UTF8_STR(d,s,n);  /* nfz */
		}
		*s = 0;
		token->nextch = lastc;
		token->value = (char *)strbuff;
		token->type = TK_LIST;
                return token;

            case EOLN:
            case SPACE:
	      c = GetC(card,instr); // more efficient than GetCode
                goto START;
 
            case EOFCH:
	        if (!instr) {
		  if (ferror(card) && !(asynint_val & KEYINT_MARK)) 
		    xsb_warn(CTXTc "[TOKENIZER] I/O error: %s",strerror(errno));
		  clearerr(card);
	        }
		token->nextch = ' ';
		token->value = 0;
		token->type = TK_EOF;
                return token;

        }
        /* There is no way we can get here */
        xsb_abort("[Internal error] InType(%d)==%d\n", c, InType(c));
        /*NOTREACHED*/
	return FALSE; /* to pacify the compiler */
}

/* --- Testing routines (usually commented) --- 

| void main(int arc, char *argv[])
| {
|   FILE *card;
|   struct xsb_token_t *res;
| 
|   card = fopen(argv[1], "r");
|   if (!card) exit(1);
|   token->nextch = ' ';
|   do {
|     res = GetToken(CTXTc card, NULL, token->nextch); // need to fix if used
|     print_token(res->type, res->value);
|   } while (res->type != TK_EOF);
| } */

void print_token(int token_type, char *ptr)
{
  switch (token_type) {
  case TK_PUNC		: printf("P: %c ", *ptr); break;
  case TK_VARFUNC	: printf("VF: %s ", ptr); break;
  case TK_VAR		: printf("V: %s ", ptr); break;
  case TK_FUNC		: printf("F: %s ", ptr); break;
  case TK_INT		: printf("I: %" Intfmt " ", *(Integer *)ptr); break;
  case TK_ATOM		: printf("A: %s ", ptr); break;
  case TK_EOC		: printf("\nTK_EOC\n"); break;
  case TK_VVAR		: printf("VV: %s ", ptr); break;
  case TK_VVARFUNC	: printf("VVF: %s ", ptr); break;
  case TK_REAL		: printf("R: %f ", *(double *)ptr); break;
  case TK_EOF		: printf("\nTK_EOF\n"); break;
  case TK_STR		: printf("S: %s ", ptr); break;
  case TK_LIST		: printf("L: %s ", ptr); break;
  case TK_HPUNC		: printf("HP: %c ", *ptr); break;
  case TK_INTFUNC	: printf("IF: %" Intfmt " ", *(Integer *)ptr); break;
  case TK_REALFUNC	: printf("RF: %f ", *(double *)ptr); break;
  }
}

/* ----------------------------------------------- */
