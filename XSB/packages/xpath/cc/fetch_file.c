/*****************************************************************************
 *                        fetch_file.c
 * This file contains functions which are used to download remote files. If 
 * the input is a url, then the functions defined below are used to parse the
 * url and download the remote file.
 *
 ****************************************************************************/

#include "nodeprecate.h"

#include "xsb_config.h"
#include "socketcall.h"
#include "auxlry.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "load_page.h"


#define SA      struct sockaddr

#define MAXSTRLEN 256

extern char * load_page (char *source, curl_opt options, curl_ret *ret_vals);
extern curl_opt init_options();

int parse_url( const char * url, char * server, char *fname);
int get_file_www(char *server, char * fname, char **buf);


/**
 * Function invoked if the input is a url.
 * Parse the url to extract the protocol, server, file path, port
 * Input : url
 * Output : server, port, file path
 **/
int parse_url( const char * url, char * server, char *fname)
{
  int i,j;
  int len = strlen(url);
  char temp[MAXSTRLEN];
  
  int flag = 0, flag_file = 0;

  SQUASH_LINUX_COMPILER_WARN(flag)
  
  for(i = 0; i<MAXSTRLEN; i++) {
    *(server+i) = 0;
    *(fname+i) = 0;
    *(temp+i) = 0;
  }
  
  
  for( i=0;i<len;i++) {
    *(temp+i)=url[i];
    
    if( url[i] == ':') {
      flag = 1;
      i++;
      /*If the protocol is not file:// or http:// its an error*/
      if(strcmp( temp , "http:") && strcmp( temp, "file:"))
	return FALSE;
      
      if(!strcmp( temp, "http:")){
	flag_file = 1;
      }
      else if(!strcmp( temp, "file:")){
	flag_file = 2;
      }
      
      if( url[i] == '/' && url[i+1] == '/') {
	*(temp+i) = url[i];
	i = i+ 1;		
	*(temp+i) = url[i];
	i = i+1;
	break;
      }
      else
	return FALSE;
    }
  }
  
  if( flag_file == 2){
    strcpy( server, "file");
    strcpy( fname, url+i);
    return TRUE;
  }
  
  /*Extract the server*/
  for(j=0;i<len;i++,j++) {
    if(url[i] == '/')
      break;
    
    *(server+j) = url[i];
  }
  /*Extract the filename*/
  for(j=0;i<len;i++,j++) {
    *(fname+j) = url[i];
  }
  return TRUE;
}

/**
 * Download the file from the specified url. Invoked only if the source is a 
 * url
 * Input :  server, port, file name
 * Output : The downloaded file
 **/
int get_file_www(char *server, char *fname, char **source)
{

  curl_opt options = init_options();
  curl_ret ret_vals;
  
  if(*source == NULL) {
    *source = (char*)malloc(strlen(server)+strlen(fname)+1);
    strcpy(*source, server);
    strcat(*source, fname);
  }
  
  *source = load_page(*source, options, &ret_vals);
  
  if(*source == NULL)
    return FALSE;
  return TRUE;
}
