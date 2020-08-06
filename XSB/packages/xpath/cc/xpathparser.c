/*****************************************************************************
 *                       xpathparser.c 
 * This is the main file. It provides the bridge between xsb and libxml2 
 * xml, xpath processing library. This file includes wrappers for the libxml2 
 * library.
 *
 ***************************************************************************/

#include "nodeprecate.h"

#include "xsb_config.h"
#ifdef WIN_NT
#define XSB_DLL
#endif

#include <assert.h>
#include <libxml/xpathInternals.h>
#include "cinterf.h"
#include "auxlry.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#define MY_ENCODING "ISO-8859-1"

#define MAXSTRLEN 256
#define MAXSTRINGLEN 32000


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

#include "fetch_file.c"

typedef enum {
  ERR_ERRNO,				/* , int */
					/* ENOMEM */
					/* EACCES --> file, action */
					/* ENOENT --> file */
  ERR_TYPE,				/* char *expected, term_t actual */
  ERR_DOMAIN,				/* char *expected, term_t actual */
  ERR_EXISTENCE,			/* char *expected, term_t actual */
  
  ERR_FAIL,				/* term_t goal */
  
  ERR_LIMIT,				/* char *limit, long max */
  ERR_MISC				/* char *fmt, ... */
} plerrorid;

int	xpath_error(plerrorid, ...);



prolog_term xpath_error_term;

int execute_xpath_expression(const char* xmlsource, const xmlChar* xpathExpr, const xmlChar* nsList, prolog_term output_term, char flag);
int register_namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) ;
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output);


/**
 * Allocate the global error term. This term is used to throw the exception
 * on the prolog side.
 * Input : none
 * Output : none
 **/
DllExport int call_conv allocate_xpath_error_term__()
{
  check_thread_context
  xpath_error_term = reg_term(CTXTc 1);
  return TRUE;
}

/**
 * This is the main function used to evaluate the xpath expression
 * Input : Xml source, xpath expression, namespace prefix specifications
 * Output : Resultant xml after evaluating xpath
 **/


DllExport int call_conv parse_xpath__()
{
  /*Temporary prolog terms to handle the input*/
  prolog_term source_term, xpath_expr_term, output_term, tmp_term, ns_term;
  char *source = NULL, *tmpsource = NULL, *tmp = NULL, flag = 0, server[MAXSTRLEN], fname[MAXSTRLEN];
  xmlChar *xpath_expr = NULL, *namespace = NULL;
  int ret = 0,  n=0;

  SQUASH_LINUX_COMPILER_WARN(n)

  /*Initialize the xpath parser*/  
  xmlInitParser();
  
  output_term = reg_term(CTXTc 3);
  source_term = reg_term(CTXTc 1);
  ns_term = reg_term(CTXTc 4);
  
  /*Parse the xml source term*/
  if (is_functor(source_term)){

    tmp = p2c_functor(source_term);

    tmp_term = p2p_arg(source_term, 1);
    /*Source is a file*/ 
    if (!strcmp(tmp, "file")){
      source = p2c_string(tmp_term);
      flag = 1;
    }
    /*Source is a string*/
    else if (!strcmp(tmp, "string")){
      source = p2c_string(tmp_term);
      flag = 0;
    }
    /*Source is a url*/
    else if (!strcmp(tmp, "url")){

      tmpsource = p2c_string(tmp_term);
      source = malloc(strlen(tmpsource)+1);
      strcpy(source, tmpsource);
      if (parse_url(source, server, fname) != FALSE){

	/*Source is a url is of the form file:// */
	if (!strcmp(server, "file")){
	  strcpy(source, fname);
	  flag = 1;
	}
	else{
	  n = 0;
	  /*Fetch file from remote location*/
	  if (get_file_www(server, fname, &source) == FALSE){
	    return xpath_error(ERR_DOMAIN, "url", tmp_term);
	  }
	  else{
	    n = strlen(source);
	  }
	}
      }
      else{
	return xpath_error(ERR_DOMAIN, "url", tmp_term);
      }
    }
  }

  /*Extract the xpath expression from prolog input*/
  xpath_expr_term = reg_term(CTXTc 2);
  if (is_nil(xpath_expr_term)){
    return xpath_error(ERR_DOMAIN, "xpath expression", xpath_expr_term);
  }
  xpath_expr = (xmlChar *)p2c_string(xpath_expr_term);
  if (!xpath_expr){
    return xpath_error(ERR_DOMAIN, "xpath expression", xpath_expr_term);
  }

  /* Takes care of the bug in libxml2. Converts the '/' input expression 
   * to '/\*'
   */
  if (!strcmp((char *)xpath_expr, "/")) {
      free(xpath_expr);
      xpath_expr = (xmlChar *)malloc(3); // 3 = length("/*")+1
      strcpy((char *)xpath_expr, "/*");
    }		     

  /*Extract the namespace prefix list from the prolog input*/
  ns_term = reg_term(CTXTc 4);
  if (is_string(ns_term)) {
    namespace = (xmlChar *)p2c_string(ns_term);
    }
  /*This is the function which evaluates the xpath expression on xml input*/
  ret = execute_xpath_expression(source, xpath_expr, namespace, output_term, flag);
  if (ret == FALSE){
    return xpath_error(ERR_MISC, "xpath", "Unable to parse the xpath expression");
  }
  xmlCleanupParser();
  return TRUE;
}


/**
 * This is the main function which evaluates the xpath query on the xml input
 * Input : xml source, xpath query, namespace prefix, type of xml source
 * (file or buffer)
 * Output : Resultant xml 
 */
int 
execute_xpath_expression(const char * xmlsource, const xmlChar* xpathExpr, const xmlChar* nsList, prolog_term output_term, char flag) {
  xmlDocPtr doc;
  xmlXPathContextPtr xpathCtx=NULL; 
  xmlXPathObjectPtr xpathObj=NULL;
  xmlBufferPtr *buf=NULL;
  int size=0, i=0, j=0, bufsize = 0;

  char *output_buffer=NULL, *ptr=NULL;
    
  /* Load XML document */
  if (flag == 1){
    doc = xmlParseFile(xmlsource);
    if (doc == NULL){
      return FALSE;
    }
  }
  else{
    doc = xmlParseMemory(xmlsource, strlen(xmlsource));
    if (doc == NULL){
      return FALSE;
    }
  }

  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == NULL) {
    xmlFreeDoc(doc); 
    return FALSE;
  }
    
  /* Register namespaces from list (if any) */
  if ((nsList != NULL) && (register_namespaces(xpathCtx, nsList) < 0)) {
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc); 
    return FALSE;
  }

  /* Evaluate xpath expression */
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if (xpathObj == NULL) {
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc); 
    return FALSE;
  }
   
  size = xpathObj->nodesetval->nodeNr;

  buf = malloc(size * sizeof(xmlBufferPtr)+1);
  if (!buf){
    return FALSE;
  }
  /*Store the resultant xml in buffer*/
  xmlSetBufferAllocationScheme(XML_BUFFER_ALLOC_EXACT);
    
  for (i = 0; i < size; i++){
    buf[i]=xmlBufferCreate();
    xmlNodeDump(buf[i], doc, xpathObj->nodesetval->nodeTab[i],0,0);
    bufsize+=strlen((char *)buf[i]->content); 
  }

  output_buffer = malloc(bufsize+1);
  if (!output_buffer){
    return FALSE;
  }

  ptr = output_buffer;
  for (j=0;j<i;j++){
    strcpy(ptr, (char *)buf[j]->content);
    ptr+=strlen((char *)buf[j]->content);
  }
  *ptr='\0';

  /*Store the resultant xml in output term*/
  if (is_var(output_term)) {
    c2p_string(CTXTc output_buffer, output_term); 
    }
  else
    {
      return FALSE;
    }
  
  /* Cleanup */
  free(output_buffer);
  for (j=0;j<size;j++){
    xmlBufferFree(buf[j]);
  }
  free(buf);
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx); 
  xmlFreeDoc(doc); 
    
  return TRUE;
}

/**
 * Register the namespaces prefixes to be used in xpath expressions while
 * handling namespaces
 * Input : pointer to context object, list of prefix = naamespace url
 * Output : TRUE on success/ FALSE on failure
 **/
int 
register_namespaces(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) {
  xmlChar* nsListDup;
  xmlChar* prefix;
  xmlChar* href;
  xmlChar* next;
    
  assert(xpathCtx);
  assert(nsList);
	
  nsListDup = xmlStrdup(nsList);
  if (nsListDup == NULL) {
    return FALSE;	
  }
    
  next = nsListDup; 
  while(next != NULL) {
    /* skip spaces */
    while((*next) == ' ') next++;
    if ((*next) == '\0') break;

    /* find prefix */
    prefix = next;
    next = (xmlChar*)xmlStrchr(next, '=');
    if (next == NULL) {
      xmlFree(nsListDup);
      return FALSE;	
    }
    *(next++) = '\0';	

    /* find href */
    href = next;
    next = (xmlChar*)xmlStrchr(next, ' ');
    if (next != NULL) {
      *(next++) = '\0';	
    }

    /* do register namespace */
    if (xmlXPathRegisterNs(xpathCtx, prefix, href) != 0) {
      xmlFree(nsListDup);
      return FALSE;	
    }
  }
   xmlFree(nsListDup);
  return TRUE;
}

/**
 * Error handling term. Creates error term to throw on the prolog side
 * Input : error type
 * Output : TRUE on success/ FALSE on failure
 **/ 
int
xpath_error(plerrorid id, ...)
{ prolog_term except = p2p_new(CTXT);
  prolog_term formal = p2p_new(CTXT);
  prolog_term swi = p2p_new(CTXT);
  prolog_term tmp1 = p2p_new(CTXT);
  prolog_term tmp;

  va_list args;
  char msgbuf[1024];
  char *msg = NULL;

  va_start(args, id);
  /*Create the error term based on the type of error*/
  switch(id)
    { case ERR_ERRNO:
	{ int err = va_arg(args, int);
      
	  msg = strerror(err);

	  switch(err)
	    { 
	      /*Not enough memory*/
	    case ENOMEM:
	  
	      c2p_functor(CTXTc "xpath", 1, tmp1); 	
	      tmp = p2p_arg(tmp1, 1);
	      c2p_functor(CTXTc "resource_error", 1, tmp);
	      
	      c2p_string(CTXTc "no_memory", p2p_arg(tmp, 1));
	      p2p_unify(CTXTc tmp1, formal); 
	      break;
	      /*Permission denied error*/
	    case EACCES:
	      { 
		const char *file = va_arg(args,   const char *);
		const char *action = va_arg(args, const char *);

		c2p_functor(CTXTc "xpath", 1, tmp1);
		tmp = p2p_arg(tmp1, 1);

		c2p_functor(CTXTc "permission_error", 3, tmp);
		c2p_string(CTXTc (char*)action, p2p_arg(tmp, 1));
		c2p_string(CTXTc "file", p2p_arg(tmp, 2));
		c2p_string(CTXTc (char*)file, p2p_arg(tmp, 3));

		p2p_unify(CTXTc tmp1, formal);
		break;
	      }
	      /*Entity not found*/
	    case ENOENT:
	      { 
		const char *file = va_arg(args, const char *);
		c2p_functor(CTXTc "xpath", 1, tmp1);
		tmp = p2p_arg(tmp1, 1);

		c2p_functor(CTXTc "permission_error", 2, tmp);
	  		  
		c2p_string(CTXTc "file", p2p_arg(tmp, 1));
		c2p_string(CTXTc (char*)file, p2p_arg(tmp, 2));

		p2p_unify(CTXTc tmp1, formal); 

		break;
	      }
	      /*Defaults to system error*/
	    default:
	      {
	        c2p_functor(CTXTc "xpath", 1, tmp1);
	        tmp = p2p_arg(tmp1, 1);

		c2p_string(CTXTc "system_error", tmp);
		p2p_unify(CTXTc tmp1, formal);
		break;
	      }
	    }
	  break;
	}
    case ERR_TYPE:
      { 
	/*Type error*/
	const char *expected = va_arg(args, const char*);
	prolog_term actual        = va_arg(args, prolog_term);


	c2p_functor(CTXTc "xpath", 1, tmp1);
	tmp = p2p_arg(tmp1, 1);

	if (is_attv(actual) && strcmp(expected, "variable") != 0 ) {
	    c2p_string(CTXTc "instantiation_error", tmp);
	    p2p_unify(CTXTc tmp1, formal);
	} else {

	    c2p_functor(CTXTc "type_error", 2, tmp);
	    c2p_string(CTXTc (char*)expected, p2p_arg(tmp, 1));
	    p2p_unify(CTXTc actual, p2p_arg(tmp, 2));
	    
	    p2p_unify(CTXTc tmp1, formal);
	  }
	break;
      }	
    case ERR_DOMAIN:
      { 
	/*Functor domain error*/
	const char *expected = va_arg(args, const char*);
	prolog_term actual        = va_arg(args, prolog_term);

	c2p_functor(CTXTc "xpath", 1, tmp1);
	tmp = p2p_arg(tmp1, 1);
	
	if (is_attv(actual) && strcmp(expected, "variable") != 0 )
	  {
	    c2p_string(CTXTc "instantiation_error", tmp);
	    p2p_unify(CTXTc tmp1, formal);
	  }
	else
	  {
	    c2p_functor(CTXTc "domain_error", 2, tmp);
	    c2p_string(CTXTc (char*)expected, p2p_arg(tmp, 1));
	    p2p_unify(CTXTc actual, p2p_arg(tmp, 2));
	    p2p_unify(CTXTc tmp1, formal);
	  }	
	break;
      }
    case ERR_EXISTENCE:
      { 
	/*Resource not in existence error*/
	const char *type = va_arg(args, const char *);
	prolog_term obj  = va_arg(args, prolog_term);

	c2p_functor(CTXTc "xpath", 1, tmp1);
	tmp = p2p_arg(tmp1, 1);

	c2p_functor(CTXTc "existence_error", 2, tmp);
	
	c2p_string(CTXTc (char*)type, p2p_arg(tmp, 1));
	p2p_unify(CTXTc obj, p2p_arg(tmp, 2));
	
	p2p_unify(CTXTc tmp1, formal);
	break;
      }
    case ERR_FAIL:
      {
	/*Goal fail error*/ 
	prolog_term goal  = va_arg(args, prolog_term);

	c2p_functor(CTXTc "xpath", 1, tmp1);
	tmp = p2p_arg(tmp1, 1);

	c2p_functor(CTXTc "goal_failed", 1, tmp);

	p2p_unify(CTXTc p2p_arg(tmp,1), goal);	
      
	p2p_unify(CTXTc tmp1, formal);
	break;
      }
    case ERR_LIMIT:
      { 
	/*Limit exceeded error*/
	const char *limit = va_arg(args, const char *);
	long maxval  = va_arg(args, long);

	c2p_functor(CTXTc "xpath", 1, tmp1);
	tmp = p2p_arg(tmp1, 1);
	
	c2p_functor(CTXTc "limit_exceeded", 2, tmp);
	c2p_string(CTXTc (char*)limit, p2p_arg(tmp,1));
	c2p_int(CTXTc maxval, p2p_arg(tmp, 2));

	
	p2p_unify(CTXTc tmp1, formal);
	break;
      }
    case ERR_MISC:
      { 
	/*Miscellaneous error*/
	const char *id = va_arg(args, const char *);
      
	const char *fmt = va_arg(args, const char *);

	vsprintf(msgbuf, fmt, args);
	msg = msgbuf;

	c2p_functor(CTXTc "xpath", 1, tmp1);
	tmp = p2p_arg(tmp1, 1);

	
	c2p_functor(CTXTc "miscellaneous", 1, tmp);
	c2p_string(CTXTc (char*)id, p2p_arg(tmp, 1));
	p2p_unify(CTXTc tmp1, formal);
	break; 
      }
    default:
      assert(0);
    }

  va_end(args);

  if (msg)
    { 
      prolog_term msgterm  = p2p_new(CTXT);

      if (msg)
	{ 
	  c2p_string(CTXTc msg, msgterm);
	}

      tmp = p2p_new(CTXT);

      c2p_functor(CTXTc "xpath_context", 1, tmp);
      p2p_unify(CTXTc p2p_arg(tmp, 1), msgterm);	
      p2p_unify(CTXTc tmp, swi);
    }
  /*Unify the created term with the error term*/
  tmp = p2p_new(CTXT);
  c2p_functor(CTXTc "xpath_error", 2, tmp);
  p2p_unify(CTXTc p2p_arg(tmp, 1), formal);
  p2p_unify(CTXTc p2p_arg(tmp, 2), swi);
  p2p_unify(CTXTc tmp, except);

  return  p2p_unify(CTXTc xpath_error_term, except);
}


