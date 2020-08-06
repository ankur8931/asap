/* File:      builtin.h
** Author(s): Warren, Xu, Swift, Sagonas, Freire, Rao
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: builtin.h,v 1.116 2013-01-04 21:34:45 dwarren Exp $
** 
*/

#define BUILTIN_TBL_SZ 256

/************************************************************************/
/*	The following is the set of all primitive predicates.		*/
/************************************************************************/

#define PSC_NAME	 1
#define PSC_ARITY	 2
#define PSC_TYPE	 3
#define PSC_PROP	 4
#define PSC_SET_TYPE	 5
#define PSC_SET_PROP	 6
#define PSC_SET_SPY      7
#define PSC_EP   	 8
#define PSC_SET_EP	 9

#define TERM_NEW_MOD    10
#define TERM_PSC	11
#define TERM_TYPE	12
#define TERM_COMPARE	13
#define TERM_NEW	14
#define TERM_ARG	15
#define TERM_SET_ARG	16
#define STAT_FLAG	17
#define STAT_SET_FLAG	18
#define BUFF_ALLOC	19
#define BUFF_WORD	20
#define BUFF_SET_WORD	21
#define BUFF_BYTE	22
#define BUFF_SET_BYTE	23
#define CODE_CALL	24

#define STR_LEN		25
#define SUBSTRING       26
#define STR_CAT		27
#define STR_CMP		28
#define STRING_SUBSTITUTE 29
#define STR_MATCH       30

#define CALL0		31
/* some other builtins that might need hard implementation */
#define STAT_STA	32
#define STAT_CPUTIME	33
#define CODE_LOAD	34
#define BUFF_SET_VAR	35
#define BUFF_DEALLOC	36
#define BUFF_CELL	37
#define BUFF_SET_CELL	38
#define COPY_TERM	39
#define XWAM_STATE	40
/* check for substring */
#define DIRNAME_CANONIC 41
/* for efficiency reasons, the following predicates are also implemented */
#define PSC_INSERT	42
#define PSC_IMPORT	43
#define PSC_DATA	44
#define CONPSC		45

#define PSC_INSERTMOD	46
#define CALLN		47

#define FILE_GETTOKEN	48
#define FILE_PUTTOKEN	49
#define TERM_HASH	50
#define UNLOAD_SEG	51
#define LOAD_OBJ	52

#define WH_RANDOM	53

#define GETENV			 54
#define SYS_SYSCALL		 55
#define SYS_SYSTEM		 56
#define SYS_GETHOST		 57
#define SYS_ERRNO		 58
#define PUTENV                   59

#define PSC_MOD		 	 60

#define FILE_WRITEQUOTED	 61
#define GROUND  		 62
#define PSC_IMPORT_AS		 63
#define GROUND_CYC               64
#define INTERN_STRING            65
#define EXPAND_FILENAME 	 66
#define TILDE_EXPAND_FILENAME    67
#define IS_ABSOLUTE_FILENAME     68
#define PARSE_FILENAME        	 69
#define ALMOST_SEARCH_MODULE     70
#define EXISTING_FILE_EXTENSION  71
#define DO_ONCE                  72
#define INTERN_TERM		 73

#define CRYPTO_HASH		 74

/* for efficiency, implemented these in C */
#define CONGET_TERM	         75
#define CONSET_TERM	         76

#define STORAGE_BUILTIN	       	 77

/* incremental evaluation */
#define INCR_EVAL_BUILTIN        78

#define GET_DATE                 80
#define STAT_WALLTIME            81

#define PSC_INIT_INFO		 98
#define PSC_GET_SET_ENV_BYTE	 99
#define PSC_ENV		        100
#define PSC_SPY		        101
#define PSC_TABLED	        102
#define PSC_SET_TABLED		103

#define IS_INCOMPLETE           104
#define ANSWER_COMPLETION_OPS	105

#define GET_PTCP	        107
#define GET_PRODUCER_CALL	108
#define DEREFERENCE_THE_BUCKET	109
#define PAIR_PSC		110
#define PAIR_NEXT		111
#define NEXT_BUCKET		112

#define SLG_NOT			114
#define LRD_SUCCESS		115

#define IS_XWAMMODE             117
#define CLOSE_OPEN_TABLES       118

#define FILE_FUNCTION           123
#define SLASH_BUILTIN           124

#define ABOLISH_ALL_TABLES      126
#define ZERO_OUT_PROFILE        127
#define WRITE_OUT_PROFILE       128
#define ASSERT_CODE_TO_BUFF	129
#define ASSERT_BUFF_TO_CLREF	130

#define GET_TRIE_LEAF           132
#define FILE_READ_CANONICAL	133
#define GEN_RETRACT_ALL		134
#define DB_GET_LAST_CLAUSE	135
#define DB_RETRACT0		136
#define DB_GET_CLAUSE		137
#define DB_BUILD_PRREF		138
#define DB_ABOLISH0		139
#define DB_RECLAIM0		140
#define DB_GET_PRREF		141
#define FORMATTED_IO            142
#define TABLE_STATUS            143
#define GET_DELAY_LISTS		144
#define ABOLISH_TABLE_CALL      145
#define ABOLISH_TABLE_PREDICATE 146
#define TRIE_ASSERT		147
#define TRIE_RETRACT		148
#define TRIE_DELETE_RETURN	149
#define TRIE_GET_RETURN		150
#define TRIE_UNIFY_CALL		151	/* get_calls/1 */
#define GET_LASTNODE_CS_RETSKEL 152
#define TRIE_GET_CALL		153	/* get_call/3 */
#define BREG_RETSKEL		154
#define TRIE_RETRACT_SAFE	155
#define ABOLISH_MODULE_TABLES   156
#define TRIE_ASSERT_HDR_INFO	157
#define TRIMCORE		158
#define NEWTRIE                 159
#define TRIE_INTERN             160
#define TRIE_INTERNED           161
#define TRIE_UNINTERN           162
#define BOTTOM_UP_UNIFY         163
#define TRIE_TRUNCATE           164
#define TRIE_DISPOSE_NR         165
#define TRIE_UNDISPOSE          166
#define RECLAIM_UNINTERNED_NR   167
#define GLOBALVAR               168
#define CCALL_STORE_ERROR       169
#define SET_TABLED_EVAL		170
#define UNIFY_WITH_OCCURS_CHECK  171
#define PUT_ATTRIBUTES		172
#define GET_ATTRIBUTES		173
#define DELETE_ATTRIBUTES	174
#define ATTV_UNIFY		175
#define SLEEPER_THREAD_OPERATION 176
#define MARK_TERM_CYCLIC        177

/* This is the builtin where people should put their private, experimental
   builtin code. SEE THE EXAMPLE IN private_builtin.c to UNDERSTAND HOW TO DO
   IT. Note: even though this is a single builtin, YOU CAN SIMULATE ANY NUMBER
   OF BUILTINS WITH IT.  */
#define PRIVATE_BUILTIN	        180

#define SEGFAULT_HANDLER	182
#define GET_BREG                183

#define FLOAT_OP        188
#define IS_ATTV			189 /* similar to IS_LIST */
#define VAR             190
#define NONVAR			191
#define ATOM			192
#define INTEGER			193
#define REAL			194
#define NUMBER			195
#define ATOMIC			196
#define COMPOUND		197
#define CALLABLE		198
#define IS_LIST			199
#define FUNCTOR			200
#define ARG                     201
#define UNIV			202
#define IS_MOST_GENERAL_TERM    203
#define HiLog_ARG		204
#define HiLog_UNIV		205
#define EXCESS_VARS		206
#define ATOM_CODES		207
#define ATOM_CHARS		208
#define NUMBER_CHARS		209
#define PUT			210
#define TAB			211
#define NUMBER_CODES		212
#define IS_CHARLIST		213
#define NUMBER_DIGITS		214
#define IS_NUMBER_ATOM		215
#define TERM_DEPTH              216
#define IS_CYCLIC               217
#define TERM_SIZE               218
#define TERM_SIZE_LIMIT         219

#define SORT			220
#define KEYSORT			221
#define PARSORT			222

/* This is the place to put new functions involving dynamic code 
   assertion and retraction -- e.g. gc */
#define DYNAMIC_CODE_FUNCTION     223
/* This is the place to put new functions that inspect tables, such as 
   answer completion */
#define TABLE_INSPECTION_FUNCTION 224

#define URL_ENCODE_DECODE    	  225
#define CHECK_CYCLIC              226
#define IS_ACYCLIC                227
/*#define GROUND_CYC              228 */

#define FINDALL_FREE    	229

#define ORACLE_QUERY		230
#define ODBC_EXEC_QUERY		231

#define SET_SCOPE_MARKER        232
#define UNWIND_STACK            233
#define CLEAN_UP_BLOCK          234

/* This is the place to put new MT functions -- for thread_create, 
   user mutexes, message queues, etc. */
#define THREAD_REQUEST		235

#define MT_RANDOM_REQUEST       236

/* added by dsw to support profiling, and backtracing */
#define XSB_PROFILE             237
#define XSB_BACKTRACE		238

#define COPY_TERM_3             240

#define EXP_HEAP                246
#define MARK_HEAP               247
#define GC_STUFF                248
#define FINDALL_INIT		249
#define FINDALL_ADD		250
#define FINDALL_GET_SOLS	251
#define SOCKET_REQUEST          252
#define JAVA_INTERRUPT          253
#define FORCE_TRUTH_VALUE	254

/* This hook is for InterProlog */
#define INTERPROLOG_CALLBACK    255

/* cons for jumpcof calls */
#define ATOM_TEST		1
#define INTEGER_TEST		2
#define REAL_TEST		3
#define NUMBER_TEST		4
#define ATOMIC_TEST		5
#define COMPOUND_TEST		6
#define CALLABLE_TEST		7
#define IS_LIST_TEST		8
#define IS_MOST_GENERAL_TERM_TEST 9
#define IS_ATTV_TEST		10
#define VAR_TEST		11
#define NONVAR_TEST		12
#define DIRECTLY_CALLABLE_TEST  13
#define IS_NUMBER_ATOM_TEST	14
#define GROUND_TEST		15

#define PLUS_FUNCT 1
#define MINUS_FUNCT 2
#define TIMES_FUNCT 3
#define DIV_FUNCT 4
#define BITAND_FUNCT 5
#define BITOR_FUNCT 6

#define BITNOT_FUNCT 7
#define IDIV_FUNCT 8

#define SIN_FUNCT 9
#define COS_FUNCT 10
#define TAN_FUNCT 11

#define FLOAT_FUNCT 13
#define FLOOR_FUNCT 14
#define EXP_FUNCT 15
#define LOG_FUNCT 16
#define LOG10_FUNCT 17
#define SQRT_FUNCT 18
#define ASIN_FUNCT 19
#define ACOS_FUNCT 20
#define ATAN_FUNCT 21
#define ABS_FUNCT 22
#define TRUNC_FUNCT 23
#define ROUND_FUNCT 24
#define CEIL_FUNCT 25

#define BITSHIFTL_FUNCT 26
#define BITSHIFTR_FUNCT 27
#define UMINUS_FUNCT 28


/* URL Encode and Decode sub-builtins */
#define URL_ENCODE    	  1
#define URL_DECODE    	  2

#define MEMORY_ERROR      1
#define NON_MEMORY_ERROR  2

#define ABOLISH_INCOMPLETES  1
#define ERROR_ON_INCOMPLETES 2

#define CYCLIC_SUCCEED 1
#define CYCLIC_FAIL    2

#define START_SLEEPER_THREAD 1
#define CANCEL_SLEEPER_THREAD 2

#define MD5  1
#define SHA1 2
