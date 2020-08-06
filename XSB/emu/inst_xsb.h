/* File:      inst_xsb.h
** Author(s): Warren, Swift, Xu, Sagonas, Freire, Johnson, Rao
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
** Copyright (C) ECRC, Germany, 1990
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
** $Id: inst_xsb.h,v 1.45 2013-05-06 21:10:24 dwarren Exp $
** 
*/


#ifndef __XSB_INSTS__

#define __XSB_INSTS__

extern void init_inst_table(void);

/************************************************************************/
/*	The following are operand types of instructions.		*/
/************************************************************************/

#define A    1	/* a byte of integer (for arity, size, builtin-num, etc) */
#define V    2	/* variable offset */
#define R    3	/* register number */
#define S    4	/* structure symbol */
#define C    5	/* constant symbol */
#define L    6	/* label (address) */
#define G    7	/* string */
#define N    8	/* number (integer) */
#define I    9	/* 2nd & 3rd arguments of switchonbound */
#define P   10	/* pad */
#define X   11	/* not present */
#define PP  12	/* double pad */
#define PPP 13	/* triple pad */
#define PPR 14  /* = PP + R; for switchonterm and switchonbound */
#define T   15  /* tabletry (tip pointer) */
#define RRR 16  /* = R + R + R; for switchon3bound */
#define F   17  /* floating point number */
#define B   18  /* tagged (boxed) integer number */
#define PRR 19  /* = P + R + R; for fun_test_ne */
#define D   20  /* double float number 8 bytes (32-bits) generalize.. */
#define H   21  /* internstr cell */

/************************************************************************/
/*	Macros to fetch the instructions/operands.			*/
/************************************************************************/

#define inst_name(C)		((char *)inst_table[C][0])

#define cell_opcode(C)		(*(pb)(C))

#define cell_opregaddr1(C)	(rreg+((pb)(cell))[1])
#define cell_opregaddr2(C)	(rreg+((pb)(cell))[2])
#define cell_opregaddr3(C)	(rreg+((pb)(cell))[3])
#define cell_opregaddrn(C,N)	(rreg+((pb)(cell))[N])

#define cell_opreg1(C)		(cell(opregaddr1(C)))
#define cell_opreg2(C)		(cell(opregaddr2(C)))
#define cell_opreg3(C)		(cell(opregaddr3(C)))
#define cell_opregn(C)		(cell(opregaddrn(C,N)))

#define cell_operand1(C)	(((pb)(C))[1])
#define cell_operand2(C)	(((pb)(C))[2])
#define cell_operand3(C)	(((pb)(C))[3])
#define cell_operandn(C,N)	(((pb)(C))[N])

/* Unused right now, but may come handy in future. */

/*
#define get_code_cell(pc)			(*(pc)++)
*/

/* bit fields */

/*
#define get_last__8bits(cell)		(((Cell)(cell))&0xff)
#define get_last_16bits(cell)		(((Cell)(cell))&0xffff)
#define get_last_32bits(cell)		(((Cell)(cell))&0xffffffff)

#define get_first__8bits(cell)		(((Cell)(cell))>>(sizeof(Cell)*8-8))
#define get_first_16bits(cell)		(((Cell)(cell))>>(sizeof(Cell)*8-16))
#define get_first_32bits(cell)		(((Cell)(cell))>>(sizeof(Cell)*8-32))

#define get__8bit_field(cell, pos)	\
		(((Cell)(cell))>>((sizeof(Cell)-pos-1)*8)&0xff)
#define get_16bit_field(cell, pos)	\
		(((Cell)(cell))>>((sizeof(Cell)-pos-1)*16)&0xffff)
#define get_32bit_field(cell, pos)	\
		(((Cell)(cell))>>((sizeof(Cell)-pos-1)*32)&0xffffffff)
*/

/* opcode & operand fields */

/*
#ifdef BITS64
#define opquarter0(cell)		(get_first_16bits(cell))
#define opquarter1(cell)		(get_16bit_field(cell,1))
#define opquarter2(cell)		(get_16bit_field(cell,2))
#define opquarter3(cell)		(get_last_16bits(cell))
#define opquartern(cell,N)		(get_16bit_field(cell,N))
#else
#define opquarter0(cell)		(get_first__8bits(cell))
#define opquarter1(cell)		(get__8bit_field(cell,1))
#define opquarter2(cell)		(get__8bit_field(cell,2))
#define opquarter3(cell)		(get_last__8bits(cell))
#define opquartern(cell,N)		(get__8bit_field(cell,N))
#endif
#define opcell(cell)			((Cell)(cell))
#define opint(cell)			((Integer)(cell))

#define opcode(cell)			((short)opquarter0(cell))

#define opregaddr1(cell)		(rreg+opquarter1(cell))
#define opregaddr2(cell)		(rreg+opquarter2(cell))
#define opregaddr3(cell)		(rreg+opquarter3(cell))
#define opregaddrn(cell,N)		(rreg+opquartern(cell,N))

#define opreg1(cell)			(cell(opregaddr1(cell)))
#define opreg2(cell)			(cell(opregaddr2(cell)))
#define opreg3(cell)			(cell(opregaddr3(cell)))
#define opregn(cell)			(cell(opregaddrn(cell,N)))

#define operand1(cell)			((short)opquarter1(cell))
#define operand2(cell)			((short)opquarter2(cell))
#define operand3(cell)			((short)opquarter3(cell))
#define operandn(cell,N)		((short)opquartern(cell,N))
*/


/*
#define opvaraddr(cell)			(ereg+(-(Cell)(cell)))
*/
/* 64 bit could benefit from change in compiler/loader for the V operands */

/*
#define opvaraddr1(cell)	(ereg+(-(unsigned int)(opquarter1(cell))))
#define opvaraddr2(cell)	(ereg+(-(unsigned int)(opquarter2(cell))))
#define opvaraddr3(cell)	(ereg+(-(unsigned int)(opquarter3(cell))))
#define opvaraddrn(cell,N)	(ereg+(-(unsigned int)(opquartern(cell,n))))

#define opvar1(cell)			(cell(opvaraddr1(cell)))
#define opvar2(cell)			(cell(opvaraddr2(cell)))
#define opvar3(cell)			(cell(opvaraddr3(cell)))
#define opvarn(cell,N)			(cell(opvaraddrn(cell,N)))
*/


#define BUILTIN_TBL_SZ 256

#ifdef PROFILE
extern Cell inst_table[BUILTIN_TBL_SZ][6];
extern UInteger num_switch_envs;
extern UInteger num_switch_envs_iter;
#else
extern Cell inst_table[BUILTIN_TBL_SZ][5];
#endif

/************************************************************************/
/*	The following is the set of all instructions.			*/
/************************************************************************/

/* Basic term instructions */

#define getpvar         0x00
#define getpval         0x01
#define getstrv         0x02
#define gettval         0x03
#define getcon          0x04
#define getnil          0x05
#define getstr          0x06
#define getlist         0x07
#define unipvar         0x08
#define unipval         0x09
#define unitvar         0x0a
#define unitval         0x0b
#define unicon          0x0c
#define uninil          0x0d
#define getnumcon	0x0e
#define putnumcon	0x0f
#define putpvar         0x10
#define putpval         0x11
#define puttvar         0x12
#define putstrv         0x13
#define putcon          0x14
#define putnil          0x15
#define putstr          0x16
#define putlist         0x17
#define bldpvar         0x18
#define bldpval         0x19
#define bldtvar         0x1a
#define bldtval         0x1b
#define bldcon          0x1c
#define bldnil          0x1d
#define uninumcon	0x1e
#define bldnumcon	0x1f

#define no_inst         0x20 
/* the above number is used for invalid opcodes in the threading engine */

#define uniavar		0x21  /* for single occurrence variables */
#define bldavar		0x22
#define unitvar_getlist_uninumcon 0x23  /* combined, same reg, 16-bit int */
#define bldtval_putlist_bldnumcon 0x24  /* combined, same reg, 16-bit int */
#define bldtvar_list_numcon 0x25  /* combined, same reg, 16-bit int */
#define getkpvars	0x26 /* get k pvars, k>=2 */

#define getinternstr    0x27 /* has ptr to interned str (or list) */
#define uniinternstr	0x28
#define bldinternstr	0x29

#define cmpreg		0x2c /* numeric compare op1 to op2 and put -1,0,1 in op2, op2 cmp op1 */
#define addintfastuni	0x2d /* add int to t/p var and unify with t/p var */
#define addintfastasgn	0x2e /* add int to t/p var and assign to t/p var */
#define xorreg          0x2f

#define getattv		0x30
#define putattv		0x31

#define getlist_tvar_tvar	0x48

/*----- Instructions for tries as code (Do NOT change the numbers) -----*/

#define trie_no_cp_attv		0x5c
#define trie_trust_attv		0x5d
#define trie_try_attv		0x5e
#define trie_retry_attv		0x5f

#define trie_no_cp_str		0x60
#define trie_trust_str		0x61
#define trie_try_str		0x62
#define trie_retry_str		0x63

#define trie_no_cp_list		0x64
#define trie_trust_list		0x65
#define trie_try_list		0x66
#define trie_retry_list		0x67

#define trie_no_cp_var		0x68
#define trie_trust_var		0x69
#define trie_try_var		0x6a
#define trie_retry_var		0x6b

#define trie_no_cp_val		0x6c
#define trie_trust_val		0x6d
#define trie_try_val		0x6e
#define trie_retry_val		0x6f

#define trie_no_cp_numcon	0x70
#define trie_trust_numcon	0x71
#define trie_try_numcon		0x72
#define trie_retry_numcon	0x73

#define trie_no_cp_numcon_succ	0x74
#define trie_trust_numcon_succ	0x75
#define trie_try_numcon_succ	0x76
#define trie_retry_numcon_succ	0x77

#define trie_proceed		0x78
#define hash_opcode 		0x79
#define hash_handle 		0x7a
#define trie_assert_inst	0x7c
#define trie_root		0x7d

#define variant_trie_no_cp_val        0x88 /* moved so start on mod 4 */
#define variant_trie_trust_val        0x89
#define variant_trie_try_val          0x8a
#define variant_trie_retry_val        0x8b

/* to reclaim deleted returns at completion */
#define trie_no_cp_fail         0x90
#define trie_trust_fail         0x91
#define trie_try_fail           0x92
#define trie_retry_fail         0x93

#define trie_fail       0x94

#define completed_trie_member   0x99

#define getfloat	0x80
#define putfloat	0x81
#define unifloat	0x82
#define bldfloat	0x83
#define getdfloat	0x84
#define putdfloat	0x85

/*----------------------------------------------------------------------*/

/* Non-determinism instructions */

#define dynfail		0x9e
#define dyntrymeelse    0x9f
#define trymeelse       0xa0
#define retrymeelse     0xa1
#define trustmeelsefail 0xa2
#define try             0xa3
#define retry           0xa4
#define trust           0xa5
#define getpbreg        0xa6
#define gettbreg	0xa7
#define putpbreg	0xa8
#define puttbreg	0xa9
#define jumptbreg	0xaa

#define getVn	        0xab      /* for tabled predicates */
#define test_heap       0xac      /* for heap overflow testing */
#define putpbreg_ci	0xad
#define puttbreg_ci	0xae

/* Indexing instructions */

#define switchonterm    0xb0
#define switchonbound	0xb3
#define switchon3bound	0xb4
#define switchonthread  0xb5

/* Instructions to compile body ors	*/

#define trymeorelse		0xb7
#define retrymeorelse		0xb8
#define trustmeorelsefail	0xb9

#define dyntrustmeelsefail	0xba	/* Dynamic trust instruction */
#define dynretrymeelse      	0xbb	/* Dynamic retry inst (for gc) */

#define restore_dealloc_proceed 0xbc    /* for special interrupt handling */

#define fdivreg			0xbd    /* for div operation, floor toward -inf */
/* Tabling instructions --- they should really be changed to be as shown */

#define tabletrysingle		0xc0
#define tabletry		0xc1
#define tableretry		0xc2
#define tabletrust		0xc3
#define check_complete		0xc4
#define answer_return		0xc5
#define resume_compl_suspension 0xc6
#define tabletrysinglenoanswers	0xc7 /* incremental evaluations */
#define check_interrupt		0xce
#define new_answer_dealloc	0xcf

/* Term comparison inst */

#define term_comp	0xd0

/* Numeric instructions */

#define movreg          0xd1
#define negate		0xd2
#define and 		0xd3
#define or 		0xd4
#define logshiftl	0xd5
#define logshiftr	0xd6
#define addreg          0xd7
#define subreg          0xd8
#define mulreg          0xd9
#define divreg          0xda
#define idivreg		0xdb
#define int_test_z	0xdc
#define int_test_nz	0xdd
#define fun_test_ne	0xde
#define powreg          0xdf
/* Unsafe term instructions */

#define putdval         0xe0
#define putuval         0xe1

/* Other Numeric instructions */
#define minreg               0xe2
#define maxreg               0xe3

#define dynnoop         0xe4

/* Procedure instructions */

#define call_forn       0xe5
#define load_pred       0xe6
#define allocate_gc     0xe7
#define call            0xe8
#define allocate        0xe9
#define deallocate      0xea
#define proceed         0xeb
#define xsb_execute         0xec
#define deallocate_gc	0xed
#define proceed_gc      0xee
#define calld           0xef

/* Branching instructions */

#define jump            0xf0
#define jumpz           0xf1
#define jumpnz          0xf2
#define jumplt          0xf3
#define jumple          0xf4
#define jumpgt          0xf5
#define jumpge          0xf6

/* Miscellaneous instructions */

//  #define cases		0xf7 %??
#define sob_jump_out 	0xf7
#define fail            0xf8
#define noop            0xf9
#define halt            0xfa
#define builtin         0xfb
#define unifunc		0xfc
#define jumpcof		0xfe
#define endfile         0xff
   /* virtual instruction, used for disassembler to link different segs */

#endif
