/*
** File: packages/curl/cc/load_page.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "load_page.h"

struct result_t
{
  size_t size;
  size_t len;
  char *data;
};

/*
 * Write data callback function (called within the context of 
 * curl_easy_perform.
 */
static size_t
write_data (void *buffer, size_t size, size_t nmemb, void *userp)
{
  struct result_t *result = userp;

  while (result->len + (size * nmemb) >= result->size)
    {

      result->data = realloc (result->data, result->size * 2);
      result->size *= 2;
    }

  memcpy (result->data + result->len, buffer, size * nmemb);
  result->len += size * nmemb;

  result->data[result->len] = 0;

  return size * nmemb;
}

char *
load_page (char *source, curl_opt options, curl_ret *ret_vals)
{
  CURL *curl;
  char * EMPTY_STRING = "";
  char * source_enc;
  char * url = NULL;
  struct result_t result;

  /* First step, init curl */
  curl = curl_easy_init ();
  if (!curl) {
    fprintf(stderr,"curl: couldn't initialize\n");
    return NULL;
  }
  
  memset (&result, 0, sizeof (result));
  result.size = 1024;
  result.data = (char *) calloc (result.size, sizeof (char));

  source_enc = curl_easy_unescape(curl , source , 0, 0);
  strcpy(source, source_enc);
  curl_free(source_enc);

  /* Tell curl the URL of the file we're going to retrieve */
  curl_easy_setopt (curl, CURLOPT_URL, source);

  /* Tell curl to retrieve the filetime */
  curl_easy_setopt(curl, CURLOPT_FILETIME, 1);

  /* Tell curl that we'll receive data to the function write_data, and
   * also provide it with a context pointer for our error return.
   */
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, &result);

  /* Redirect */
  curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, options.redir_flag);

  /* Verify the certificate */
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, options.secure.flag);
  curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, options.secure.flag * 2);
  curl_easy_setopt (curl, CURLOPT_CAINFO, options.secure.crt_name);

  /* Authentication */
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(curl, CURLOPT_USERPWD, options.auth.usr_pwd);

  /* Timeout */
  if(options.timeout > 0)
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, options.timeout);

  /* Retreive only properties */ 
  curl_easy_setopt (curl, CURLOPT_HEADER, options.url_prop); 
  curl_easy_setopt (curl, CURLOPT_NOBODY, options.url_prop);

  /* User Agent */
  curl_easy_setopt(curl, CURLOPT_USERAGENT, options.user_agent);

  /* Post Data */
  if(strlen(options.post_data)>0)
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options.post_data);

  /* Allow curl to perform the action */
  curl_easy_perform (curl);

  curl_easy_getinfo (curl, CURLINFO_EFFECTIVE_URL, &url);
  ret_vals->url_final = (char *) malloc ((strlen(url) + 1) * sizeof(char));
  strcpy(ret_vals->url_final, url);

  curl_easy_getinfo (curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &(ret_vals->size));
  curl_easy_getinfo(curl, CURLINFO_FILETIME, &(ret_vals->modify_time));

  curl_easy_cleanup (curl);

  if (result.len > 0)
    return result.data;

  free (result.data);
  return EMPTY_STRING;
}

void *
encode (char *url, char **dir, char **file, char **suffix)
{
  size_t dir_len = 0;
  char *dir_enc = NULL, *file_enc = NULL, *suff_enc = NULL, *ptr;

  ptr = strrchr (url, '/');
  if (ptr == NULL)
  {
    *file = (char *) malloc ((strlen(url)+1)*sizeof (char));
    strcpy(*file, url);
  }
  else 
  {
    ptr++;
    *file = (char *) malloc ((strlen(ptr)+1)*sizeof (char));
    strcpy(*file, ptr);  
    dir_len = strlen(url) - strlen(*file) - 1;
  }

  ptr = strrchr (*file, '.');
  if (ptr == NULL)
  {
    *suffix = (char *) malloc (sizeof (char));
    (*suffix)[0] = '\0';
  }
  else
  {
    ptr++;
    *suffix = (char *) malloc ((strlen(ptr)+1)*sizeof (char));
    strcpy(*suffix, ptr);  
    (*file)[strlen(*file) - strlen(*suffix) - 1] = '\0';
  }

  *dir = (char *) malloc ((dir_len + 1) * sizeof (char));
  strncpy(*dir, url, dir_len);
  (*dir)[dir_len] = '\0';

  dir_enc = curl_easy_escape (NULL, *dir, 0);
  *dir = realloc (*dir, (strlen(dir_enc) + 1) * sizeof (char));
  strcpy (*dir, dir_enc);
  curl_free (dir_enc);

  file_enc = curl_easy_escape (NULL, *file, 0);
  *file = realloc (*file, (strlen(file_enc) + 1) * sizeof (char));
  strcpy (*file, file_enc);
  curl_free (file_enc);

  suff_enc = curl_easy_escape (NULL, *suffix, 0);
  *suffix = realloc (*suffix, (strlen(suff_enc) + 1) * sizeof (char));
  strcpy (*suffix, suff_enc);
  curl_free (suff_enc);

  return 0;
}

curl_opt init_options() {

  curl_opt options;
  options.redir_flag = 1;
  options.secure.flag = 1;
  options.secure.crt_name = "";
  options.auth.usr_pwd = "";
  options.timeout = 0;
  options.url_prop = 0;
  options.user_agent = "http://xsb.sourceforge.net/";
  options.post_data = "";

  return options;
}
