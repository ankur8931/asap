/* /home/ubuntu/asap/XSB/config/x86_64-unknown-linux-gnu/xsb_config.h.  Generated from def_config.in by configure.  */
/* def_config.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
#define _ALL_SOURCE 1
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* #ifdef ___ALWAYS_TRUE___ is a hack to force autoheader copy the relevant
   statements into config.h */
#define ___ALWAYS_TRUE___ 1

#define CC "gcc"

#define RELEASE_DATE 2016-07-06
#define RELEASE_MONTH 7
#define RELEASE_DAY 6
#define RELEASE_YEAR 2016

/* sometimes needed for C preprocessor under Linux */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

/* Single precision floats */
/* #undef FAST_FLOATS */

/* sometimes needed for C preprocessor under HP-UX */
/* #undef _HPUX_SOURCE */

/* Defined if 64 bit machine */
#define BITS64 1

/* Defined if cilk compile */
/* #undef CILK */

/* this is used by many to check if config.h was included in proper order */
#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED 1
#endif

/* HP700 running HP-UX */
/* #undef HP700 */

/* Actually, AIX unix of IBM */
/* #undef IBM */

/* Defined under Linux */
#define LINUX 1

/* Guess tagging strategy for newer versions of Linux */
/* #undef GENERAL_TAGGING */

/* Defined under OS X */
/* #undef DARWIN */

/* Defined under FreeBSD */
/* #undef FREEBSD */

/* Older Linux using a.out format */
/* #undef LINUX_AOUT */

/* Newer Linux using ELF format */
#define LINUX_ELF 1

/* Use local eval strategy. This is the default. */
#define LOCAL_EVAL 1

/* MIPS based machines, such as old SGI's */
/* #undef MIPS_BASED */

/* MK Linux on Power PC */
/* #undef MKLINUX_PPC */

/* CYGWIN */
/* #undef CYGWIN */

/* Defined, if XSB is built with support for ORACLE DB */
/* #undef ORACLE */
/* #undef ORACLE_DEBUG */

/* Defined on SGI machines */
/* #undef SGI */
/* #undef SGI64 */

/* Sun Solaris OS */
/* #undef SOLARIS */
/* #undef BIG_MEM */

/* Sun Solaris on Intel */
/* #undef SOLARIS_x86 */

/* Old, pre-solaris SunOS */
/* #undef SUN */

/* The version number of the XSB release */
#define XSB_VERSION "3.7. (Clan MacGregor)"

/* XSB is built with support for ODBC */
/* #undef XSB_ODBC */

/* XSB is built with support for InterProlog */
/* #undef XSB_INTERPROLOG */

/* XSB's FOREIGN_ELF is designed in a way such that it supports
   any system using the high level dlopen... functions.
*/

#define FOREIGN_ELF 1
/* #undef FOREIGN_AOUT */

#ifdef ___ALWAYS_TRUE___
#if (defined(FOREIGN_AOUT) || defined(FOREIGN_ELF) || defined(FOREIGN_WIN32))
#define FOREIGN
#endif
#endif

#ifdef MKLINUX_PPC
#define FOREIGN_ELF 1
/* #undef FOREIGN */
#endif

/* Sets runtime library search path for the dynamic loader */
/* #undef RUNTIME_LD_PATH */

#define JUMPTABLE_EMULOOP 1
#define INSN_BLOCKS 1

#ifdef ___ALWAYS_TRUE___
#if ( defined(SUN) )
#define memmove(A,B,L) bcopy(B,A,L)
#endif
#endif

/* Debugging variables are set in debug.h, not here.
   We include these dummy definitions only to avoid the autoheader warnings */
#if 0
#define DEBUG
#define CP_DEBUG
#define PROFILE
#define DEBUG_ORACLE
#define DEBUG_VERBOSE
#define DEBUG_VM
#define DEBUG_ASSERTIONS
#endif

/* #undef NON_OPT_COMPILE */

/*  File needed for XSB to get the configuration information */
#define CONFIGURATION "x86_64-unknown-linux-gnu"
#define FULL_CONFIG_NAME "x86_64-unknown-linux-gnu"

#define GC 1

/* #undef GC_TEST */

/* #undef MULTI_THREAD */

/* #undef SHARED_COMPL_TABLES */

/* #undef CONC_COMPL */

/* The number of bytes in a long.  */
/* #undef SIZEOF_LONG */

/* Define if you have the gethostbyname function.  */
#define HAVE_GETHOSTBYNAME 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the mkdir function.  */
#define HAVE_MKDIR 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have readline library */
/* #undef HAVE_READLINE */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the <SQL.H> header file.  */
/* #undef HAVE_SQL_H */

/* Define if you have the <SQLEXT.H> header file.  */
/* #undef HAVE_SQLEXT_H */

/* Define if you have the <malloc.h> header file.  */
/* #undef HAVE_MALLOC_H */

/* Define if you have the <odbcinst.h> header file.  */
/* #undef HAVE_ODBCINST_H */

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <windows.h> header file.  */
/* #undef HAVE_WINDOWS_H */

/* Define if you have the c3v6 library (-lc3v6).  */
/* #undef HAVE_LIBC3V6 */

/* Define if you have the client library (-lclient).  */
/* #undef HAVE_LIBCLIENT */

/* Define if you have the clntsh library (-lclntsh).  */
/* #undef HAVE_LIBCLNTSH */

/* Define if you have the common library (-lcommon).  */
/* #undef HAVE_LIBCOMMON */

/* Define if you have the core library (-lcore).  */
/* #undef HAVE_LIBCORE */

/* Define if you have the core3 library (-lcore3).  */
/* #undef HAVE_LIBCORE3 */

/* Define if you have the cv6 library (-lcv6).  */
/* #undef HAVE_LIBCV6 */

/* Define if you have the dl library (-ldl).  */
#define HAVE_LIBDL 1

/* Define if you have the epc library (-lepc).  */
/* #undef HAVE_LIBEPC */

/* Define if you have the generic library (-lgeneric).  */
/* #undef HAVE_LIBGENERIC */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the ncr library (-lncr).  */
/* #undef HAVE_LIBNCR */

/* Define if you have the nlsrtl library (-lnlsrtl).  */
/* #undef HAVE_LIBNLSRTL */

/* Define if you have the nlsrtl3 library (-lnlsrtl3).  */
/* #undef HAVE_LIBNLSRTL3 */

/* Define if you have the nsl library (-lnsl).  */
#define HAVE_LIBNSL 1

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Define if you have the sql library (-lsql).  */
/* #undef HAVE_LIBSQL */

/* Define if you have the sqlnet library (-lsqlnet).  */
/* #undef HAVE_LIBSQLNET */

/* Define if you have the thread library (-lthread).  */
/* #undef HAVE_LIBTHREAD */

/* Define if you have execvp */
#define HAVE_EXECVP 1

/* On x86/linux/gcc, we use bp as the register that contains
the local program counter */
/* #undef USE_BP_LPCREG */

/* GC on SLG-WAM! ;) */
#define SLG_GC 1

/* SimpleScalar emulator */
/* #undef SIMPLESCALAR */

/* electric fence needs a different tagging scheme */
/* #undef ELECTRIC_FENCE */

/* demand support */
/* #undef DEMAND */
