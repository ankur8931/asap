/* Categories of permanent space use: */
#define ATOM_SPACE		0
#define STRING_SPACE		1
#define ASSERT_SPACE		2
#define COMPILED_SPACE		3
#define FOR_CODE_SPACE		4
#define TABLE_SPACE		5
#define FINDALL_SPACE		6
#define PROFILE_SPACE		7
#define MT_PRIVATE_SPACE	8
#define BUFF_SPACE		9
#define GC_SPACE		10
#define HASH_SPACE		11
#define INTERPROLOG_SPACE	12
#define THREAD_SPACE		13
#define READ_CAN_SPACE		14
#define LEAK_SPACE		15
#define SPECIAL_SPACE		16
#define OTHER_SPACE		17
#define INCR_TABLE_SPACE	18
#define ODBC_SPACE		19
// The next two are for printing out memory errors
#define TCP_SPACE		20
#define COMPL_SPACE		21
#define GL_SPACE                22
// VARSTRING_SPACE??  some other to thread?

#define NUM_CATS_SPACE		20
