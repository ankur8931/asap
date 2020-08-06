/* File:      orastuff.h
** Author(s): Ernie Johnson
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
** $Id: orastuff.h,v 1.11 2010-08-19 15:03:36 spyrosh Exp $
** 
*/



/*
 *                   M I S C E L L A N E O U S
 *                   =========================
 */

/* SQL Communications Area
   ----------------------- */
 
#define SQLCA_INIT
EXEC SQL INCLUDE sqlca;

/* ------------------------------------------------------------------------ */

/*
 *  Number of cursors we can handle at once.
 */
#define NUM_CURSORS   21


/*
 *  Statuses returned to Prolog from the Oracle Interace Primitives.
 */
#define ORACLE_EXCEPTION     2
#define INTERFACE_EXCEPTION  1
#define SUCCESS              0
#define INTERFACE_ERROR     -1
#define ORACLE_ERROR        -2

#define IsExceptionStatus(Status)   (Status > SUCCESS)
#define IsSuccessStatus(Status)     (Status == SUCCESS)
#define IsFailureStatus(Status)     (Status < SUCCESS)


/* ======================================================================== */

/*
 *                   T A L K I N G   T O   O R A C L E
 *                   =================================
 */

/*
 *  Oracle Descriptors
 *  ==================
 *  The use of DB cursors center around the use of so-called descriptors.
 *  These are data structures defined by Oracle which allow certain
 *  information to be conveyed, between the program and the DB, about the
 *  query.  A descriptor can be dedicated to either input or output.  Input
 *  descriptors are only necessary when using SQL query templates: an SQL
 *  statement with placeholders where values must be inserted.  The use of
 *  templates allows for re-use of DB parsing info, but it requires the
 *  overhead of descriptor manipulation.  SQL statements which are passed
 *  with values intact as a string to Oracle can be executed immediately.
 *  All SQL SELECT statements require a descriptor to handle DB output.
 */


/* SQL Descriptor Area (SQLDA)
   --------------------------- */

EXEC SQL INCLUDE sqlda;


/* SQLDA Access Macros
   ------------------- */

#define SQLDA_NumEntriesAlloced(SQLDA)             ((SQLDA)->N)
#define SQLDA_ItemValueArrayBase(SQLDA,Index)      ((SQLDA)->V[Index])
#define SQLDA_ItemValueArrayWidth(SQLDA,Index)     ((SQLDA)->L[Index])
#define SQLDA_ItemValueArrayType(SQLDA,Index)      ((SQLDA)->T[Index])
#define SQLDA_IndValueArrayBase(SQLDA,Index)       ((SQLDA)->I[Index])
#define SQLDA_NumEntriesFound(SQLDA)               ((SQLDA)->F)
#define SQLDA_ItemNameBuffer(SQLDA,Index)          ((SQLDA)->S[Index])
#define SQLDA_ItemNameBufLen(SQLDA,Index)          ((SQLDA)->M[Index])
#define SQLDA_ItemNameLength(SQLDA,Index)          ((SQLDA)->C[Index])
#define SQLDA_IndNameBuffer(SQLDA,Index)           ((SQLDA)->X[Index])
#define SQLDA_IndNameBufLen(SQLDA,Index)           ((SQLDA)->Y[Index])
#define SQLDA_IndNameLength(SQLDA,Index)           ((SQLDA)->Z[Index])


/* SQLDA Methods        (taken from sqlcpr.h)
   ------------- */

/* Allocates SQL Descriptor Area */
extern SQLDA *sqlald( int, unsigned int, unsigned int );

/* Deallcates SQL Descriptor Area */
extern void sqlclu( SQLDA * );

/* Checks and resets the high order bit of an SQLDA type field after a
   DESCRIBE of the SLIs  This bit indicates whether a column has been
   specified as NON NULL: 1 = allows NULLs, 0 = disallows NULLs. */
extern void sqlnul( short*, short*, int* );

/* Extracts precision (number of significant digits) and scale (where
   rounding occurs) from the length field of an SQLDA associated with a
   NUMBER datatype. */
extern void sqlprc( long*, int*, int* );


/* Descriptor Constants
   -------------------- */

/*
 * Maximum number of input placeholders or output columns allowed.
 * (There's a real limit set by Oracle; find this constant.)
 */

#define MAX_NUM_IOVALUES  100


/*
 * Limits on the maximum length of the *names* of the select-list
 * items and bind variables.
 * Bind variables are called "bindX" where X is a counter.
 * SLI names depend on the table's column names, or the computation
 * to be performed.
 */

#define BINDVAR_NAME_BUFFER_SIZE  8
#define SLI_NAME_BUFFER_SIZE     30


/*
 * ORACLE recognizes two classes of datatypes: internal and external.
 * Internal datatypes specify how ORACLE (a) stores data in database
 * columns, and (b) represents database pseudocolumns.  External datatypes
 * specify how data is stored in host variables.  External datatypes
 * include the internal datatypes plus other language-specific types.
 * Below, a description of all the Internal and the relevant External
 * datatypes are given.
 *
 * A description containing the term "binary" means that the data would be
 * received in some non-native-language format, either because it is a
 * representation used internally by Oracle (as in the case of NUMBER and
 * DATE) or because the data has a structure imposed by some external
 * influence.  ORACLE, therefore, does not know the format of this data
 * and hence does not attempt to interpret it, as in the case of the RAW
 * and LONG RAW datatypes.
 *
 * A description containing the term "fixed-length" means that every byte
 * of the input value or column field is significant.  In particular, a
 * column value of this type contains blank padding when extracted, even
 * if the host variable is declared to be of a "variable length" type.
 */

enum OracleDataTypes {

  /* Internal (and External) DataTypes */

  VARCHAR2_ODT = 1,         /* Variable-length character string */
  NUMBER_ODT = 2,           /* Binary Number */
  LONG_ODT = 8,             /* Fixed-length character string */
  ROWID_ODT = 11,           /* Binary value */
  DATE_ODT = 12,            /* Fixed-length date/time value */
  RAW_ODT = 23,             /* Fixed-length binary data */
  LONGRAW_ODT = 24,         /* Fixed-length binary data */
  CHAR_ODT = 96,            /* Fixed-length character string */

  /* External DataTypes (cont'd) */

  INTEGER_ODT = 3,          /* Signed Integer */
  FLOAT_ODT = 4,            /* Floating point number */
  STRING_ODT = 5,           /* NULL-terminated character string */
  VARNUM_ODT = 6,           /* Variable-length binary number */
  VARCHAR_ODT = 9,          /* Variable-length character string */
  VARRAW_ODT = 15,          /* Variable-length binary data */
  UNSIGNED_ODT = 68,        /* Unsigned integer */
  LONGVARCHAR_ODT = 94,     /* Variable-length character string */
  LONGVARRAW_ODT = 95,      /* Variable-length binary data */
  CHARZ_ODT = 97            /* C fixed-length, NULL-terminated char string */
};

/* Some data types have a fixed or limited representation as strings... */
#define ROWID_TO_STRING_BUFSIZE    20
#define DATE_TO_STRING_BUFSIZE     10

/* Others give no hint as to its size, so we set our own limit */
#define LONG_TO_STRING_BUFSIZE    100

/* ------------------------------------------------------------------------ */

/*
 *  Local Extensions for Host Variable Arrays
 *  =========================================
 *  Constants, Types, and Data Structures for managing Oracle descriptors
 *  which allow for the use of multi-tuple input and output.  The size of
 *  these arrays are obtained from XSB's global flags.
 *  (In the literature, input placeholders are referred to as "bind
 *   variables" and elements of an output tuple are referred to as
 *   "select list items".)
 */

typedef unsigned int  uint;


typedef struct Bind_Variable_Specification {
  SQLDA *descriptor;       /* Oracle Bind Descriptor */
  uint array_length;       /* number of elements in the input arrays */
  uint cur_entry_index;    /* which element of the BV arrays are to be filled;
			      valid range: [0..(array_length-1)] */
} BindVarSpec;


/*
 *  Original details about a table's columns, as reported by
 *  DESCRIBE SELECT LIST.
 */
typedef struct Table_Column_Attributes {
  short datatype;    /* after the null-bit has been cleared */
  short not_null;    /* TRUE = NULLs not allowed; FALSE = NULLs allowed */
  long size;         /* width of column, may be 0 if none given at creation */
} ColumnSpec;

typedef struct Select_List_Item_Specification {
  SQLDA *descriptor;       /* Oracle Select Descriptor */
  uint array_length;       /* number of elements in the output arrays */
  ColumnSpec *column_specs;   /* array of details about the selected columns */
  uint cur_row_index;      /* indicates the element across all arrays to be
			      read; valid range: [0..(array_length-1)] */
  uint cur_col_index;      /* denotes which SLI should next be read by Prolog;
			      valid range: [0..(numSLIs-1)] */
  int cur_row_number;      /* RowID of next tuple to return to Prolog */
  uint total_rows_recvd;   /* total num rows received from Oracle so far */
  xsbBool end_of_active_set;  /* flag indicating whether the last of the active
				 set has been returned from Oracle. */
} SLI_Spec;  


/*
 *  Default array widths in bytes for select list items (output) and bind
 *  variables (input).  Actual values can be set by the user and are
 *  stored in flags ORA_INPUTARRAY_LENGTH and ORA_OUTPUTARRAY_LENGTH.
 *  Whenever the values of these flags are <= 0, the defaults are used.
 */
#define DEFAULT_INPUTARRAY_LENGTH    200
#define DEFAULT_INPUTARRAY_WIDTH     30
#define DEFAULT_OUTPUTARRAY_LENGTH   200


/* ======================================================================== */

/*
 *                S T A T E M E N T   P R O P E R T I E S
 *                =======================================
 *
 *
 *  Reusing a DB cursor can greatly reduce the time needed to process a
 *  statement.  An SQL statement can be considered to have a particular
 *  "template": syntactically valid SQL constructs together with
 *  placeholders for input and ouput values.  XSB determines and maintains
 *  templates for SQL statements established through many Oracle-interface
 *  predicates, and in doing so, assigns to each distinct template a unique
 *  number.  This number allows us to reuse internal and database
 *  structures created while processing a query with the same template.
 *
 *  Together with this number, the text string representing the statement
 *  itself and a tag indicating the type of statement are all maintained
 *  for an issued Oracle query.
 */


typedef unsigned int TemplateNumber;


/* SQL Statement Type
 *  ------------------
 *  We enumerate only classes of statments, where each one requires unique
 *  handling.  (Explicit numbering for coordination with Prolog.)
 */
typedef enum SQL_Stmt_Type_Classes {
  SELECT_SQL_STMT = 0,      /* Uses output arrays */
  INSERT_SQL_STMT = 1,      /* Uses input arrays */
  CURSOR_DAMAGING_SQL_STMT = 2,   /* Transaction Control and Data Def'n SQL
			    stmts (which trigger COMMITs) invalidate cursors */
  OTHER_SQL_STMT = 3        /* Other Data Manipulation, and Session and System
			   Control SQL stmts which are benign... we think */
} SqlStmtType;


typedef struct SQL_Statement_Specification {
  TemplateNumber template;
  SqlStmtType type;
  char *string;
} SqlStmtSpec;


/* ======================================================================== */

/*
 *                    C U R S O R   C O M P O N E N T S
 *                    =================================
 *
 *
 *  The following data structure tracks the allocation of Oracle cursors, as
 *  well as local structures, to submitted SQL statements.  Handles for
 *  these resources, represented as indices into this array, are maintained
 *  for the last NUM_CURSORS active statements.
 *
 *  In Oracle, cursors are destroyed when a scheme-altering operation is
 *  executed.  Therefore, locally we must break ties between the templates
 *  and the now defunct cursors (subsequent statement calls will have to
 *  rebuild these structures).  We do so by invalidating all active cursor
 *  handles and resetting all inactive cursor ones to the unused state.  We
 *  must also establish tests to ensure that operations are performed only
 *  on valid cursors.
 */


typedef int CursorHandle;

typedef enum Cursor_Status {
  UNUSED_CURSOR_STATUS,       /* Has no allocated internal structures */
  ACTIVE_CURSOR_STATUS,       /* Currently in use */
  INACTIVE_CURSOR_STATUS,     /* Not in use; maintains internal structures */
  INVALID_CURSOR_STATUS       /* ACTIVE cursor destroyed by execution of a
			         Transaction Control SQL statement */
} CursorStatus;

#define INVALID_CURSOR_HANDLE -1


typedef struct Cursor_Components {
  CursorStatus status;
  SqlStmtSpec stmt;
  BindVarSpec bv;
  SLI_Spec sli;
} Cursor;
