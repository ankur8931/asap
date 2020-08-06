/*
** File: packages/curl/cc/curl2pl.c
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

#include "nodeprecate.h"


#include "xsb_config.h"
#ifdef WIN_NT
#define XSB_DLL
#endif
#include "cinterf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "load_page.h"
#include "error_term.h"

#ifndef WIN_NT
#include <sys/stat.h>
#endif

#ifdef MULTI_THREAD
#define xsb_get_main_thread_macro xsb_get_main_thread()
#define  check_thread_context  th = xsb_get_main_thread();
#else
#define xsb_get_main_thread_macro
#define check_thread_context
#endif

#ifdef MULTI_THREAD
static th_context *th = NULL;
#endif

#include "error.c"

extern char * load_page (char *source, curl_opt options, curl_ret *ret_vals);
extern void * encode (char *url, char **dir, char **file, char **suffix);
extern curl_opt init_options();

/**
 * Allocate error term on C side
 * Input : Prolog variable
 * Output : none
 **/ 

DllExport int call_conv curl_allocate_error_term()
{
  check_thread_context
  global_error_term = reg_term(CTXTc 1);
  global_warning_term = reg_term(CTXTc 2);
  return TRUE;
}

/**
 * Remove uninstantiated terms in the warning list at end
 * Input : Warning term
 **/
DllExport int call_conv curl_finalize_warn()
{
  /*Temporary prolog term to iterate over the warnings list*/
  prolog_term tmp;

  check_thread_context
  tmp = reg_term(CTXTc 1);
  while( is_list(tmp)){
    tmp = p2p_cdr(tmp);
  }
  if( is_var(tmp)){
    c2p_nil(CTXTc tmp);
  }
  return TRUE;
}

DllExport int call_conv pl_load_page()
{

  prolog_term head, tail, result = 0;

  char *functor, *url = NULL, *data = NULL;
  char *username = NULL, *password = NULL;

  curl_opt options = init_options();
  curl_ret ret_vals;

	
  check_thread_context
  tail = reg_term(CTXTc 1);
  
  if(!is_list(tail))
    return curl2pl_error(ERR_DOMAIN, "source", tail);

  while(is_list(tail)){
    
    head = p2p_car(tail); 
    tail = p2p_cdr(tail);

    if(is_functor(head)){
      
      functor = p2c_functor(head);
   
      if(!strcmp(functor,"source")){

	prolog_term term_url_func, term_url = 0;

	term_url_func = p2p_arg(head, 1);
     
	if(is_functor(term_url_func)){
		
	  if(!strcmp(p2c_functor(term_url_func), "url")){
	    
	    term_url = p2p_arg(term_url_func, 1);
	    url = p2c_string(term_url);
	    data = load_page(url, options, &ret_vals);
	  }
	  else{
	    return curl2pl_error(ERR_MISC, "source", term_url);
	  }
	}
	else{
	  return curl2pl_error(ERR_MISC, "source", "Improper input format");
	}
      }
      else if(!strcmp(functor,"options")){

	prolog_term term_options = p2p_arg(head, 1);
	prolog_term term_option;

	while(is_list(term_options)){

		term_option = p2p_car(term_options);
		if(!strcmp(p2c_functor(term_option), "redirect")) {
			if(!strcmp(p2c_string(p2p_arg(term_option, 1)), "true"))
				options.redir_flag = 1;
			else
				options.redir_flag = 0;
		}
		else if(!strcmp(p2c_functor(term_option), "secure")){
			if(!strcmp(p2c_string(p2p_arg(term_option, 1)), "false"))
				options.secure.flag = 0;
			else
				options.secure.crt_name = p2c_string(p2p_arg(term_option, 1));
		}
		else if(!strcmp(p2c_functor(term_option), "auth")){
			username = p2c_string(p2p_arg(term_option, 1));
			password = p2c_string(p2p_arg(term_option, 2));
			options.auth.usr_pwd = (char *) malloc ((strlen(username) + strlen(password) + 2) * sizeof(char));
			strcpy(options.auth.usr_pwd, username);
			strcat(options.auth.usr_pwd, ":");
			strcat(options.auth.usr_pwd, password);			
		}
		else if(!strcmp(p2c_functor(term_option), "timeout")){
		  options.timeout = (int)p2c_int(p2p_arg(term_option, 1));
		}
		else if(!strcmp(p2c_functor(term_option), "url_prop")){
			options.url_prop = options.redir_flag;
		}
		else if(!strcmp(p2c_functor(term_option), "user_agent")){
			options.user_agent = p2c_string(p2p_arg(term_option, 1));
		}
		else if(!strcmp(p2c_functor(term_option), "post")){
			options.post_data = p2c_string(p2p_arg(term_option, 1));
		}
		term_options = p2p_cdr(term_options);
	}

      }
      else if(!strcmp(functor,"document")){

	result = p2p_arg(head, 1);
      }
      else if(!strcmp(functor,"properties")){

	c2p_int(CTXTc (int) ret_vals.size, p2p_arg(head, 1));
	/* the following code can be used to convert to local/UTC time, if
	    necessary. Note: XSB uses local time, and ret_vals.modify_time
	    is local, too.

	    struct tm * timeinfo;
	    timeinfo = gmtime(&(ret_vals.modify_time)); // UTC time
	    timeinfo = localtime(&(ret_vals.modify_time)); // local time
	    c2p_int(CTXTc (int) mktime(timeinfo), p2p_arg(head,2));
	*/
	/* return modification time as an integer */
	c2p_int(CTXTc (int) ret_vals.modify_time, p2p_arg(head,2));
	/*
	  The following converts time to string - not useful

	  if (ctime(&ret_vals.modify_time) == NULL)
	      c2p_string("", p2p_arg(head, 2));
	  else
	      c2p_string(CTXTc (char *) ctime(&ret_vals.modify_time),
		         p2p_arg(head, 2));
	*/
      }
    }
    else{
      return curl2pl_error(ERR_DOMAIN, "source", head);
    }
  }

  c2p_string(CTXTc data, result);

return TRUE;
}

DllExport int call_conv pl_encode_url()
{

  char	*url;
  char *dir, *file_base, *suffix;

  check_thread_context
  url = (char *) extern_ptoc_string(1);
 
  encode(url, &dir, &file_base, &suffix);

  c2p_string(CTXTc dir, p2p_car(reg_term(CTXTc 2)));
  c2p_string(CTXTc file_base, p2p_car(p2p_cdr(reg_term(CTXTc 2))));
  c2p_string(CTXTc suffix, p2p_car(p2p_cdr(p2p_cdr(reg_term(CTXTc 2)))));

  return TRUE;
}
