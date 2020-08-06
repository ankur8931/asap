/*
** File: packages/curl/cc/common.h
** Author: Aneesh Ali
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2010
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
*/

//#ifdef WIN_NT
#include <time.h>
//#endif

typedef struct opt
{

  int redir_flag;

  struct sec
  {
    int flag;
    char *crt_name;
  }secure;

  struct au
  {
    char *usr_pwd;
  }auth;

  int timeout;

  int url_prop;

  char *user_agent;

  char *post_data;

}curl_opt;

typedef struct ret
{

  char *url_final;

  double size;

  time_t modify_time;

}curl_ret;

