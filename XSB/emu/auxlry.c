/* File:      auxlry.c
** Author(s): Warren, Sagonas, Xu
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
** $Id: auxlry.c,v 1.24 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


#include "xsb_config.h"

#include <stdio.h>

/* take care of the time.h problems */
#include "xsb_time.h"

#ifndef WIN_NT
#include <sys/resource.h>

#ifdef SOLARIS
/*--- Include the following to bypass header file inconcistencies ---*/
extern int getrusage();
extern int gettimeofday();
#endif

#ifdef HP700
#include <sys/syscall.h>
extern int syscall();
#define getrusage(T, USAGE)	syscall(SYS_getrusage, T, USAGE);
#endif

#endif

#ifdef WIN_NT
#include <windows.h>
#include <winbase.h>
// vanished: #include <versionhelpers.h>
#include "windows.h"
#endif

/*----------------------------------------------------------------------*/

double cpu_time(void)
{
  double time_sec;

#if defined(WIN_NT)
#ifndef _MSC_VER
#define ULONGLONG unsigned long long
#else
#define ULONGLONG __int64
#endif

  /**  static int win_version = -1;

  if (win_version == -1) {
    OSVERSIONINFO winv;
    winv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&winv);
    win_version = winv.dwPlatformId;
    } ***/

  //  if (win_version == VER_PLATFORM_WIN32_NT) {
  if (1 /* IsWindowsXPOrGreater() seems obsolete */) {
    HANDLE thisproc;
    FILETIME creation, exit, kernel, user;
    ULONGLONG lkernel, luser;
    double stime, utime;

    thisproc = GetCurrentProcess();
    GetProcessTimes(thisproc,&creation,&exit,&kernel,&user);

    lkernel = ((ULONGLONG) kernel.dwHighDateTime << 32) + 
      kernel.dwLowDateTime;
    luser = ((ULONGLONG) user.dwHighDateTime << 32) + 
      user.dwLowDateTime;

    stime = lkernel / 1.0e7;
    utime = luser / 1.0e7;

    time_sec =  stime + utime;

  } else {
    time_sec = ( clock() / CLOCKS_PER_SEC);
  }

#else
  struct rusage usage;

  getrusage(RUSAGE_SELF, &usage);
  time_sec = (float)usage.ru_utime.tv_sec +
	     (float)usage.ru_utime.tv_usec / 1000000.0;
#endif

  return time_sec;
}

/*----------------------------------------------------------------------*/

/* local = TRUE, if local time is requested */
void get_date(int local, int *year, int *month, int *day,
	     int *hour, int *minute, int *second)
{
#ifdef WIN_NT
    SYSTEMTIME SystemTime;
    if (local)
      GetLocalTime(&SystemTime);
    else
      GetSystemTime(&SystemTime);
    *year = SystemTime.wYear;
    *month = SystemTime.wMonth;
    *day = SystemTime.wDay;
    *hour = SystemTime.wHour;
    *minute = SystemTime.wMinute;
    *second = SystemTime.wSecond;
#else
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv,NULL);
    if (local)
      tm = localtime(&tv.tv_sec);
    else
      tm = gmtime(&tv.tv_sec);
    *year = tm->tm_year;
    if (*year < 1900)
      *year += 1900;
    *month = tm->tm_mon + 1;
    *day = tm->tm_mday;
    *hour = tm->tm_hour;
    *minute = tm->tm_min;
    *second = tm->tm_sec;
#endif
#endif
}

/*----------------------------------------------------------------------*/

double real_time(void)
{
#if defined(WIN_NT)
  double value = ((float) clock() / CLOCKS_PER_SEC);
#else
  double value;
  struct timeval tvs;

  gettimeofday(&tvs, 0);
  value = tvs.tv_sec + 0.000001 * tvs.tv_usec;
#endif
  return value;
}

/*----------------------------------------------------------------------*/

/* My version of gdb gets confused when I set a breakpoint in include
   files within emuloop.  Thus the use of gdb_dummy() */

void gdb_dummy(void) 
  {
  }
