/***************************************************************************
 *                           sgml2pl.c
 * This is the main file. It contains the interface to the parser internal
 * functions. It also contains the callback functions which are invoked
 * when certain parts of the xml document are encountered.
 *
 *************************************************************************/

#include "deprecation.h"

#include "xsb_config.h"

#ifdef WIN_NT
#define XSB_DLL
#endif
#include "cinterf.h"

#ifdef MULTI_THREAD
#define xsb_get_main_thread_macro xsb_get_main_thread()
#define  check_thread_context \
	th = xsb_get_main_thread();
#else
#define xsb_get_main_thread_macro
#define check_thread_context
#endif

#include <stdlib.h>
#include "dtd.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "fetch_file.c"
#include "parser.c"
#include "charmap.c"
#include "sgmlutil.c"
#include "xmlns.c"
#include "model.c"
#include "error_term.h"
#include "sgmlutil.h"
#include "basic_defs.h"

#ifndef WIN_NT
#include <sys/stat.h>
#endif

/*
#include "socketcall.h"
*/

#define PD_MAGIC        0x36472ba1      /* just a number */

#define MAX_ERRORS      50
#define MAX_WARNINGS    50
#define MAXSTRLEN 256


typedef enum
  { EM_QUIET = 0,                         /* Suppress messages */
    EM_PRINT,                             /* Print message */
    EM_STYLE                              /* include style-messages */
  } errormode;

typedef enum
  { SA_FILE = 0,                          /* Stop at end-of-file */
    SA_INPUT,                             /* Do not complete input */
    SA_ELEMENT,                           /* Stop after first element */
    SA_CONTENT,                           /* Stop after close */
    SA_DECL                               /* Stop after declaration */
  } stopat;

typedef struct _env
{ prolog_term        tail;
  struct _env *parent;
} env;

typedef struct _parser_data
{ int         magic;                    /* PD_MAGIC */
  dtd_parser *parser;                   /* parser itself */

  int         warnings;                 /* #warnings seen */
  int         errors;                   /* #errors seen */
  int         max_errors;               /* error limit */
  int         max_warnings;             /* warning limit */
  errormode   error_mode;               /* how to handle errors */
  int         positions;                /* report file-positions */

  predicate_t on_begin;                 /* begin element */
  predicate_t on_end;                   /* end element */
  predicate_t on_cdata;                 /* cdata */
  predicate_t on_entity;                /* entity */
  predicate_t on_pi;                    /* processing instruction */
  predicate_t on_urlns;                 /* url --> namespace */
  predicate_t on_error;                 /* errors */
  predicate_t on_decl;                  /* declarations */

  stopat      stopat;                   /* Where to stop */
  int         stopped;                  /* Environment is complete */

  void*   source;                  /* Where we are reading from */
  int its_a_url;

  prolog_term      list;                     /* output term (if any) */
  prolog_term      tail;                     /* tail of the list */
  env        *stack;                    /* environment stack */
  int         free_on_close;            /* sgml_free parser on close */
} parser_data;


dtd_parser * parser_error = NULL;

#include "error.c"


dtd *
new_dtd(const ichar *doctype);

static int
get_dtd(prolog_term t, dtd **dtdp);

dtd_parser *
new_dtd_parser(dtd *dtd);

int unify_dtd( prolog_term t, dtd * dtd);

int unify_parser( prolog_term t, dtd_parser * p);


static int
on_begin(dtd_parser *p, dtd_element *e, int argc, sgml_attribute *argv);

static int
on_end(dtd_parser *p, dtd_element *e);

static int
on_entity(dtd_parser *p, dtd_entity *e, int chr);

static int
on_pi(dtd_parser *p, const ichar *pi);

static int
on_cdata(dtd_parser *p, data_type type, size_t len, const ochar *data);

static void
put_element_name(dtd_parser *p, prolog_term t, dtd_element *e);

static int
unify_attribute_list(dtd_parser *p, prolog_term alist,
                     int argc, sgml_attribute *argv);
static parser_data *
new_parser_data(dtd_parser *p);

static void
put_url(dtd_parser *p, prolog_term t, const ichar *url);

static int
on_error(dtd_parser *p, dtd_error *error);

static int
on_decl(dtd_parser *p, const ichar *decl);

static void
put_attribute_name(dtd_parser *p, prolog_term t, dtd_symbol *nm);

static void
put_attribute_value(dtd_parser *p, prolog_term t, sgml_attribute *a);


static ichar *
istrblank(const ichar *s);

static int
unify_listval(dtd_parser *p,  prolog_term t, attrtype type, Integer len, const char *text);

static dtd_srcloc *
file_location(dtd_parser *p, dtd_srcloc *l);

static int
can_end_omitted(dtd_parser *p);

static int
set_option_dtd( dtd *dtd, dtd_option option, char *set);



/**
 * Create a new parser object on the C side and return pointer to prolog
 * Input  : Var prolog term, Options list
 * Output : Pointer to parser object
 **/

DllExport int call_conv pl_new_sgml_parser()
{
  /*Temporary terms to parse the input from prolog side*/
  prolog_term head, tail, tmp, ref, tmp1;

  /*Pointer to dtd and parser objects*/
  dtd *dtd = NULL;
  dtd_parser *p;

  char *str;

  check_thread_context
  tail = reg_term(CTXTc 2);

  /*Parsing the options list*/
  while(is_list(tail))
    {
      head = p2p_car(tail);
      tmp1 = p2p_cdr(tail);
      tail = tmp1;
      if(is_functor( head))
	{

	  /*Extract the dtd pointer if present. Otherwise create the
	    dtd object*/
	  str = p2c_functor( head);
	  if(strcmp( str, "dtd_struct"))
	    {
	      return FALSE;
	    }
	  tmp = p2p_arg(head, 1);
	  if( is_var( tmp))
	    {
	      dtd = new_dtd(NULL);
	      dtd->references++;
	      c2p_int(CTXTc (Integer)dtd, tmp);
	    }
	  else
	    {
	      if( !get_dtd( head,  &dtd))
		return FALSE;

	    }
	}
    }

  ref = reg_term(CTXTc 1);
  p = new_dtd_parser(dtd);

  parser_error = p;
  return unify_parser(ref, p);

}

/**
 * Create a representative sgml_parser(...) prolog term for the parser object
 * Input : Parser object pointer
 * Output : Prolog parser term
 **/

int unify_parser( prolog_term t, dtd_parser *p)
{
  prolog_term tmp1;

  /*Temporary prolog terms to create the output terms*/
  tmp1 = p2p_new(CTXT);

  /*Create the prolog term*/
  c2p_functor(CTXTc "sgml_parser", 1, tmp1);
  c2p_int(CTXTc (Integer) p, p2p_arg( tmp1, 1));

  return p2p_unify(CTXTc t, tmp1);
}


/**
 * Create a representative dtd_struct(...) prolog term for the dtd object
 * Input: Dtd object pointer
 * Output: dtd prolog term
 **/

int unify_dtd( prolog_term t, dtd * d)
{
  /*Temporary prolog term to create the output term*/
  prolog_term tmp, tmp1;

  tmp = p2p_new(CTXT);
  tmp1 = p2p_new(CTXT);

  /*dtd_struct/2 if doctype is specified*/

  if(d->doctype)
    {
      c2p_functor(CTXTc "dtd_struct", 2, tmp1);
      c2p_int(CTXTc (Integer) d, p2p_arg( tmp1, 1));
      c2p_string(CTXTc (char *) d->doctype, p2p_arg( tmp1, 2));
    }
  /* dtd_struct/1 if no doctype is specified */
  else
    {
      c2p_functor(CTXTc "dtd_struct", 1, tmp1);
      c2p_int(CTXTc (Integer) d, tmp);
      p2p_unify(CTXTc p2p_arg( tmp1, 1), tmp);
    }

  return p2p_unify(CTXTc t, tmp1);
}

/**
 * Create a new dtd object on the C side and return pointer to prolog
 * Input: prolog var term,
 * Output: pointer to dtd object
 **/

DllExport int call_conv pl_new_dtd()
{ char *dt;
  dtd *dtd;
  prolog_term doctype;
  prolog_term ref;

  check_thread_context
  doctype = reg_term(CTXTc 1);
  ref = reg_term(CTXTc 2);

  /*Extract the doctype*/
  if ( !(dt = p2c_string( doctype) ))
    return sgml2pl_error(ERR_TYPE, "atom", doctype);

  /*Create the dtd*/
  if ( !(dtd=new_dtd((ichar *) dt)) )
    return FALSE;

  dtd->references++;

  return unify_dtd(ref, dtd);
}

/**
 * Dtd object constructor for the C side dtd object
 * Input : doctype string
 * Output : Initialized dtd object pointer
 **/
dtd *
new_dtd(const ichar *doctype)
{ dtd *dtd = calloc(1, sizeof(*dtd));

  dtd->magic     = SGML_DTD_MAGIC;
  dtd->implicit  = TRUE;
  dtd->dialect   = DL_SGML;
  if ( doctype )
    dtd->doctype = istrdup(doctype);
  dtd->symbols   = new_symbol_table();
  dtd->charclass = new_charclass();
  dtd->charfunc  = new_charfunc();
  dtd->charmap   = new_charmap();
  dtd->space_mode = SP_SGML;
  dtd->ent_case_sensitive = TRUE;
  dtd->shorttag    = TRUE;
  dtd->number_mode = NU_TOKEN;
  return dtd;
}

/**
 * Create a new parser for the specified dtd.
 * This dtd is used to validate the xml document which is parsed by the parser.
 * Input: dtd object
 * Output: Parser object
 **/


dtd_parser *
new_dtd_parser(dtd *dtd)
{
  dtd_parser *p = calloc(1, sizeof(*p));

  if ( !dtd )
    dtd = new_dtd(NULL);
  dtd->references++;

  p->magic       = SGML_PARSER_MAGIC;
  p->dtd         = dtd;
  p->state       = S_PCDATA;
  p->mark_state  = MS_INCLUDE;
  p->dmode       = DM_DTD;
  p->encoding    = ENC_ISO_LATIN1;
  p->buffer      = new_icharbuf();
  p->cdata       = new_ocharbuf();
  p->event_class = EV_EXPLICIT;
  set_src_dtd_parser(p, IN_NONE, NULL);

  return p;
}

/**
 * Extract the C side dtd object pointer from the prolog term
 * Input : Dtd prolog term
 * Output : Dtd object pointer
 */

static int
get_dtd(prolog_term t, dtd **dtdp)
{
  char * str;

  if ( is_functor(t))
    {
      /*Temporary prolog terms to parse the inputs*/
      prolog_term temp_term;
      void *ptr;

      str = p2c_functor(t);

      if(strcmp( str, "dtd_struct"))
	return FALSE;

      temp_term = p2p_arg(t, 1);
      /*Extract the dtd object pointer from prolog term*/
      if ((ptr = (void *) p2c_int(temp_term) ))
	{
	  dtd *tmp = ptr;
	  if ( tmp->magic == SGML_DTD_MAGIC )
	    {
	      *dtdp = tmp;

	      return TRUE;
	    }
	  return sgml2pl_error(ERR_EXISTENCE, "dtd_struct", t);
	}
    }

  return sgml2pl_error(ERR_TYPE, "dtd_struct", t);
}

/**
 * Extract C side parser object pointer from the prolog term
 * Input : parser prolog term
 * Output : Pointer to C side parser object
 **/

static int
get_parser(prolog_term parser, dtd_parser **p)
{
  /*Temporary terms to parse the prolog input*/
  prolog_term temp_term;
  void *ptr;
  char *str = NULL;

  if(is_functor(parser))
    {

      /*Extract the parser object pointer from prolog term*/
      str = p2c_functor( parser);

      if(strcmp(str,"sgml_parser"))
	{
	  return FALSE;
	}
      temp_term = p2p_arg( parser, 1);

      if( (ptr = (void *) p2c_int(temp_term)))
	{
	  dtd_parser *tmp = ptr;
	  if ( tmp->magic == SGML_PARSER_MAGIC )
	    {
	      *p = tmp;
	      return TRUE;
	    }
	  return sgml2pl_error(ERR_EXISTENCE, "sgml_parser", parser);
	}
    }

  return sgml2pl_error(ERR_TYPE, "sgml_parser", parser);
}

/**
 * Create a doctype object on the C side
 * Input : doctype string, parser prolog term
 * Output : prolog doctype term
 **/

DllExport int call_conv pl_doctype()
{
  dtd_parser *p = NULL;
  prolog_term parser, doctype;
  dtd * dtd;

  check_thread_context
  parser = reg_term(CTXTc 1);
  doctype = reg_term(CTXTc 2);


  /*Extract parser from the parser prolog term*/
  if ( !get_parser(parser, &p) )
    return FALSE;
  dtd = p->dtd;

  if(is_var(doctype) && dtd->doctype)
    {
      c2p_string(CTXTc (char *) dtd->doctype, doctype);
    }
  return TRUE;
}

/**
 * Set parse options in the parser object
 * Input : parser object, options list
 **/

DllExport int call_conv pl_set_sgml_parser()
{
  dtd_parser *p = NULL;
  prolog_term parser, options, temp_term;
  check_thread_context

  parser = reg_term(CTXTc 1);
  options = reg_term(CTXTc 2);

  /*Extract the parser object pointer from the prolog term*/
  if ( !get_parser(parser, &p) )
    return FALSE;

  if( is_functor(options))
    {
      char *funcname;

      funcname = p2c_functor( options);

      /*Set the dialect in the parser. Dialect may be xml, xmlns or sgml*/
      if( streq( funcname, "dialect"))
	{
	  char *s;
	  temp_term = p2p_arg(options, 1);
	  s=p2c_string( temp_term);
	  if ( streq(s, "xml") )
	    set_dialect_dtd(p->dtd, DL_XML);
	  else if ( streq(s, "xmlns") )
	    set_dialect_dtd(p->dtd, DL_XMLNS);
	  else if ( streq(s, "sgml") )
	    set_dialect_dtd(p->dtd, DL_SGML);
	  else
	    return sgml2pl_error(ERR_DOMAIN, "sgml_dialect", temp_term);
	}
      /*Sets the shorttag handling option to FALSE or true*/
      else if( streq( funcname, "shorttag"))
	{
	  char *booleanstring=NULL;

	  temp_term = p2p_arg( options, 1);

	  booleanstring = p2c_string( temp_term);
	  if( !booleanstring){
	    return sgml2pl_error(ERR_TYPE, "boolen", temp_term);
	  }
	  if( strcmp( booleanstring, "false") &&
	      strcmp( booleanstring, "true") &&
	      strcmp( booleanstring, "FALSE") &&
	      strcmp( booleanstring, "TRUE"))
	    {
	      return sgml2pl_error( ERR_TYPE, "boolean", temp_term);
	    }

	  set_option_dtd( p->dtd, OPT_SHORTTAG, booleanstring);

	}
      /*Set the file name which is displayed as the source of errors*/
      else if( streq( funcname, "file"))
	{
	  char * file;

	  temp_term = p2p_arg( options, 1);
	  file=p2c_string( temp_term);
	  set_src_dtd_parser( p, IN_FILE, file);
	}
      /*Set the current line to parse*/
      else if ( streq( funcname, "line"))
	{
	  temp_term = p2p_arg( options, 1);

	  (p->location.line = (int)p2c_int( temp_term));
	}
      /*Set the current character position to parse*/
      else if ( streq( funcname, "charpos"))
	{
	  temp_term = p2p_arg( options, 1);

	  p->location.charpos = p2c_int( temp_term);

	}
      /*Set the space handling*/
      else if( streq( funcname, "space"))
	{
	  char *s;
	  temp_term =p2p_arg(options, 1);
	  s=p2c_string( temp_term);

	  if ( streq(s, "preserve") )
	    p->dtd->space_mode = SP_PRESERVE;
	  else if ( streq(s, "default") )
	    p->dtd->space_mode = SP_DEFAULT;
	  else if ( streq(s, "remove") )
	    p->dtd->space_mode = SP_REMOVE;
	  else if ( streq(s, "sgml") )
	    p->dtd->space_mode = SP_SGML;
	  else
	    return FALSE;

	}
      /*Set the defaults*/
      else if( streq( funcname, "defaults"))
	{
	  int val;

	  temp_term =p2p_arg(options, 1);

	  val=(int)p2c_int( temp_term);

	  if ( val )
	    p->flags &= ~SGML_PARSER_NODEFS;
	  else
	    p->flags |= SGML_PARSER_NODEFS;

	}
      /*Set the number option*/
      else if( streq( funcname, "number"))
	{
	  char *s;
	  temp_term = p2p_arg(options, 1);
	  s=p2c_string( temp_term);

	  if ( streq(s, "token") )
	    p->dtd->number_mode = NU_TOKEN;
	  else if ( streq(s, "integer") )
	    p->dtd->number_mode = NU_INTEGER;

	  else
	    return FALSE;

	}
      /*Set the doctype*/
      else if( streq( funcname, "doctype"))
	{
	  char *s;
	  temp_term = p2p_arg(options, 1);

	  if( is_var( temp_term))
	    p->enforce_outer_element = NULL;
	  else
	    {
	      if( !(s=p2c_string(temp_term) ))
		return FALSE;
	      p->enforce_outer_element = dtd_add_symbol(p->dtd, (ichar *) s);

	    }

	}

    }
  return TRUE;

}

/**
 * Allocate error term on C side
 * Input : Prolog variable
 * Output : none
  This doesn't work with XSB garbage colleciton, since the location of
  a variable might change!  Must statically allocate enough memory
  with -m ???
 **/

DllExport int call_conv pl_allocate_error_term()
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
DllExport int call_conv pl_finalize_warn()
{
  /*Temporary prolog terms to iterate the warnings list*/
  prolog_term tmp, tmp1;
  check_thread_context

  tmp = reg_term(CTXTc 1);
  while( is_list( tmp)){
    tmp1 = p2p_cdr( tmp);
    tmp = tmp1;
  }
  if( is_var( tmp)){
    c2p_nil(CTXTc tmp);
  }
  return TRUE;
}


/**
 * This is the main starting function. This function parses the input
 * options and invokes the parser.
 * Input : Parser, Options
 * Output : The parsed prolog term
 */

DllExport int call_conv pl_sgml_parse()
{
  dtd_parser *p = NULL;
  parser_data *pd;
  parser_data *oldpd;
  /*Temporary prolog terms to parse the options list*/
  prolog_term head , parser, options, tail, tmp1;
  FILE *in = NULL;
  struct stat stbuf;

  int  recursive, has_content_length = FALSE, its_a_url = 0;
  size_t source_len = 0;
  size_t content_length = 0;

  char *str, *source=NULL, fname[MAXSTRLEN], *tmpsource=NULL;

  check_thread_context


  parser = reg_term(CTXTc 1);
  options = reg_term(CTXTc 2);
  tail = options;

  /*Extract the parser from input prolog term*/
  if ( !get_parser(parser, &p) )
    return FALSE;


  if( p->closure) {
      recursive = TRUE;
      oldpd = p->closure;

      if ( oldpd->magic != PD_MAGIC || oldpd->parser != p )
	return sgml2pl_error(ERR_MISC, "sgml", "Parser associated with illegal data");

      pd = calloc(1, sizeof(*pd));
      *pd = *oldpd;
      p->closure = pd;

      its_a_url = pd->its_a_url;
      if(its_a_url == 1)
	source = (char *) pd->source;
      else if( its_a_url == 0)
	in = pd->source;

  } else {
      recursive = FALSE;
      oldpd = NULL;

      set_mode_dtd_parser(p, DM_DATA);

      /*Set the call back functions in the parser*/
      p->on_begin_element = on_begin;
      p->on_end_element   = on_end;
      p->on_entity        = on_entity;
      p->on_pi            = on_pi;
      p->on_data          = on_cdata;
      p->on_error         = on_error;
      p->on_decl          = on_decl;
      pd = new_parser_data(p);
  }


  /*Validate the options list*/
  if(!is_list(tail))
    return sgml2pl_error( ERR_DOMAIN, "source", tail);

  while (is_list(tail)) {
    head = p2p_car(tail);
    tmp1 = p2p_cdr(tail);
    tail = tmp1;


    if(is_functor( head)){
      str = p2c_functor( head);

      /*Assign the output prolog term to the parser object. The parser creates the output in this term*/

      if (!strcmp(str,"document")) { 
	pd->list = p2p_arg( head, 1);
	pd->tail = pd->list;
	pd->stack = NULL;
      }
      /*Set the source in the relevant field of the parser*/
      else if (!strcmp(str,"source")) {
	/*Temporary terms used to parse the prolog input*/
	prolog_term temp_term1, temp_term2 = 0;
	char server[MAXSTRLEN], * tmpstr=NULL;

	temp_term1 = p2p_arg( head, 1);


	if ( is_functor( temp_term1)) {

	  tmpstr = p2c_functor( temp_term1);

	  /*Source is a url*/
	  if ( !strcmp("url", tmpstr)){
	    temp_term2 = p2p_arg(temp_term1, 1);
	    tmpsource = p2c_string(temp_term2);
	    source = malloc( strlen(tmpsource));
	    strcpy( source, tmpsource);

	    /*Validate the url*/
	    if (parse_url( source, server, fname) != FALSE)
	      {
		/*Url is of the form file:// */
		if ( !strcmp( server, "file")){
		  if(!(in = fopen( fname, "rb"))){
		    return sgml2pl_error(ERR_EXISTENCE, "file", temp_term2);
		  }
		  its_a_url = 0;
		  fstat( fileno( in), &stbuf);
		  source_len = stbuf.st_size;
		}

		else{
		  /*Url is of the from http://  */
		  if(get_file_www( server, fname, &source) == FALSE){
		    return sgml2pl_error(ERR_MISC, "url", source);
		  }
		  else{
		    source_len = strlen( source);
		    its_a_url = 1;
		  }
		}
	      }
	    else
	      {
		return sgml2pl_error(ERR_DOMAIN, "url", temp_term2);
	      }
	  }
	  /*Source is a file*/
	  else if ( !strcmp( "file", tmpstr)){

	    temp_term2 = p2p_arg(temp_term1, 1);
	    source = p2c_string(temp_term2);
	    if(!(in = fopen( source, "rb"))){
	      return sgml2pl_error(ERR_EXISTENCE, "file", temp_term2);
	    }
	    its_a_url = 0;
	    fstat( fileno( in), &stbuf);
	    source_len = stbuf.st_size;
	    set_src_dtd_parser(p, IN_FILE, source);
	  }
	  /*Input is a string*/
	  else if ( !strcmp( "string", tmpstr)){

	    temp_term2 = p2p_arg(temp_term1, 1);
	    source = p2c_string( temp_term2);
	    source_len = strlen( source);
	    its_a_url = 1;
	  } else{
	    return sgml2pl_error( ERR_MISC, "source", temp_term2);
	  }
	} else{
	  return sgml2pl_error( ERR_MISC, "source", "Invalid input argument 1");
	}
      }
      /*Set the content length to parse*/
      else if ( !strcmp(str,"content_length")) {
	/*Temporary prolog term to parse the options list*/
	prolog_term temp_term1, temp_term2;

	temp_term1 = p2p_arg( head, 1);
	temp_term2 = p2p_arg( temp_term1, 1);
	content_length = p2c_int( temp_term2);
	has_content_length = TRUE;
      }
      /*Sets how much of the current input should be parsed*/
      else if( !strcmp(str,"parse")) {
	  char *s;
	  /*Temporary prolog terms to parse the options list*/
	  prolog_term temp_term;

	  temp_term = p2p_arg( head, 1);

	  s = p2c_string(temp_term);

	  if(streq(s,"element"))
	    pd->stopat = SA_ELEMENT;
	  else if ( streq(s, "content") )
	    pd->stopat = SA_CONTENT;
	  else if ( streq(s, "file") )
	    pd->stopat = SA_FILE;
	  else if ( streq(s, "input") )
	    pd->stopat = SA_INPUT;
	  else if ( streq(s, "declaration") )
	    pd->stopat = SA_DECL;
	  else {
	      return sgml2pl_error(ERR_DOMAIN, "parse", temp_term);
	  }

      }
      /*Set how the syntax errors should be handled*/
      else if ( !strcmp( str, "syntax_errors")) {
	char *s;
	/*Temporary prolog term to parse the options list*/
	prolog_term temp_term;

	temp_term = p2p_arg( head, 1);

	s = p2c_string(temp_term);

	if ( streq(s, "quiet") )
	  pd->error_mode = EM_QUIET;
	else if ( streq(s, "print") )
	  pd->error_mode = EM_PRINT;
	else if ( streq(s, "style") )
	  pd->error_mode = EM_STYLE;
	else
	  return sgml2pl_error(ERR_DOMAIN, "syntax_error", temp_term);

      }
      /*Set the positions option*/
      else if( !strcmp( str, "positions")){
	char *s=NULL;
	/*Temporary prolog terms to parse the options list*/
	prolog_term temp_term = 0;

	temp_term = p2p_arg( head, 1);

	s = p2c_string(temp_term);

	if ( streq(s, "true") )
	  pd->positions = TRUE;
	else if ( streq(s, "false") )
	  pd->positions = FALSE;
	else
	  return sgml2pl_error(ERR_DOMAIN, "positions", temp_term);
      }

    } else {
      return sgml2pl_error(ERR_DOMAIN, "source", head);
    }
  }

#define CHECKERROR							\
  if ( pd->errors > pd->max_errors && pd->max_errors >= 0 )		\
    return sgml2pl_error(ERR_LIMIT, "max_errors", (long)pd->max_errors);

  if ( pd->stopat == SA_CONTENT && p->empty_element )
    goto out;

  if (in || its_a_url) {
      int eof = FALSE;
      int i = 0;

      if(!recursive)
	{
	  pd->its_a_url = its_a_url;
	  if ( its_a_url ==1)
	    {
	      pd->source = source;
	    }
	  else if( its_a_url ==0)
	    pd->source = in;
	}
      /*Read the source character by character and parse xml*/
      while( !eof) {
	  char c=0;
	  char ateof = FALSE;

	  if ( has_content_length ) {
	    if ( content_length <= 0 )
	      c = EOF;
	    else {
	      if (its_a_url == 1) {
		c = source[i++];
		if (i == source_len) {
		  ateof = TRUE;
		}
	      } else if(its_a_url == 0) {
		c = fgetc(in);
		source_len=source_len -1;
		if( source_len <= 0)
		  ateof = TRUE;
	      }
	    }
	    
	    if(!ateof)
	      ateof = (--content_length <= 0);

	  } else {
	    if (its_a_url == 1) {
	      c = source[i++];
	      if (i == source_len) {
		ateof = TRUE;
	      }
	    } else if( its_a_url ==0) {
	      c = fgetc(in);
	      source_len=source_len -1;
	      if( source_len <= 0)
		ateof = TRUE;
	    }
	  }

	  if (ateof) {
	      eof = TRUE;
	      if ( c == LINEFEED )  {        /* file ends in LINEFEED */
		c = CARRIAGERETURN;
	      } else if ( c != CARRIAGERETURN )  {        /* file ends in normal char */
		putchar_dtd_parser(p, c);
		if ( pd->stopped )
		  goto stopped;
		c = CARRIAGERETURN;
	      }
	  }
	  putchar_dtd_parser( p, c);
	  if ( pd->stopped ) {
	  stopped:
	    pd->stopped = FALSE;
	    if ( pd->stopat != SA_CONTENT )
	      reset_document_dtd_parser(p); /* ensure a clean start */
	    goto out;
	  }

      }
      
      if ( !recursive && pd->stopat != SA_INPUT )
	end_document_dtd_parser(p);
      
  out:
      /*Remove the ununified portions of the output prolog term*/
      if( !is_nil( pd->tail)) {
	c2p_nil(CTXTc pd->tail);
      }
      if ( recursive ) {
	p->closure = oldpd;
      } else {
	p->closure = NULL;
      }

      pd->magic = 0;                      /* invalidate */
      free(pd);

      if (its_a_url == 0)
	fclose(in);
      return TRUE;
  }

  return TRUE;
}

/**
 * Open the Dtd specified. The parser uses this dtd to validate the xml
 * while parsing
 * Input : dtd object pointer, parser object pointer
 **/

DllExport int call_conv pl_open_dtd()
{ dtd *dtd = NULL;
  dtd_parser *p;
  parser_data *pd;

  /*Prolog terms to parse the options list*/
  prolog_term ref, options, tail, head, tmp1;

  FILE * in = NULL;

  char *str, file[MAXSTRLEN], server[MAXSTRLEN], *fname=NULL, *tmpfname=NULL;
  int its_a_url = 0;
  struct stat stbuf;
  size_t source_len = 0;
  check_thread_context

  ref = reg_term(CTXTc 1);
  options = reg_term(CTXTc 2);

  /*Extract the Dtd*/
  if ( !get_dtd(ref, &dtd) )
    return FALSE;

  /*Create a new parser object*/
  p = new_dtd_parser(dtd);
  p->dmode = DM_DTD;
  pd = new_parser_data(p);
  pd->free_on_close = TRUE;

  tail = options;

  while(is_list(tail))
    {
      head = p2p_car(tail);
      tmp1 = p2p_cdr(tail);
      tail = tmp1;

      /*Go through the list of options*/
      if(is_functor( head)){
	str = p2c_functor( head);


	if(!strcmp(str,"source")){
	  /*Temporary prolog terms to parse the options list*/
	  prolog_term temp_term1, temp_term2;
	  char * tmpstr = NULL;
	  temp_term1 = p2p_arg( head, 1);
	  tmpstr = p2c_functor(temp_term1);


	  /*The source is a url*/
	  if(!strcmp( tmpstr, "url")){
	    temp_term2 = p2p_arg(temp_term1, 1);
	    tmpfname = p2c_string(temp_term2);
	    fname = malloc( strlen(tmpfname));
	    strcpy( fname, tmpfname);
	    if( parse_url( fname, server, file) != FALSE) {

	      source_len = 0;

	      /*The url is of the form file:// */
	      if( !strcmp( server, "file")){
		if(!(in = fopen( file, "rb"))){
		  return sgml2pl_error(ERR_EXISTENCE, "file", temp_term2);
		}
		its_a_url = 0;
		fstat( fileno( in), &stbuf);
		source_len = stbuf.st_size;
	      }

	      else{
		/*Source is a url of the form http://...*/
		if( get_file_www( server, file, &fname) == FALSE){
		  return sgml2pl_error( ERR_MISC, "url", fname);
		}
		else{
		  its_a_url = 1;
		  source_len = strlen( fname);
		}
	      }
	    }
	    else
	      {
		return sgml2pl_error( ERR_DOMAIN, "url", temp_term2);
	      }
	  }
	  /*Source is a file*/
	  else if( !strcmp( tmpstr, "file")){
	    temp_term2 = p2p_arg( temp_term1, 1);
	    fname = p2c_string( temp_term2);
	    its_a_url = 0;
	    if(!(in = fopen( fname, "r"))){
	      return sgml2pl_error(ERR_EXISTENCE, "File", temp_term2);

	    }
	    fstat( fileno( in), &stbuf);
	    source_len = stbuf.st_size;
	  }
	  /*Source is a string*/
	  else if(!strcmp( tmpstr, "string")){
	    its_a_url = 1;
	    temp_term2 = p2p_arg( temp_term1, 1);
	    fname = p2c_string( temp_term2);
	    source_len = strlen( fname );
	  }
	  else{
	    return FALSE;
	  }
	}
      }
    }



  if ( !pd->parser || pd->parser->magic != SGML_PARSER_MAGIC ){
    errno = EINVAL;
    return FALSE;
  }

  if ( (pd->errors > pd->max_errors && pd->max_errors >= 0) || pd->stopped ){
    errno = EIO;
    return FALSE;
  }

  /*Parse the dtd contents*/
  if (its_a_url == 1) {
    int i = 0;
    source_len = strlen(fname);

    for( i=0; (size_t)i<source_len ; i++){
      putchar_dtd_parser(pd->parser, fname[i]);
    }
  }

  else if( its_a_url == 0)
    {
      char c;
      int i = 0;

      for( i=0;(size_t)i<source_len;i++)
	{
	  c = fgetc(in);
	  putchar_dtd_parser(pd->parser, c);
	}
      fclose(in);
    }
  return TRUE;
}

/**
 * Free the allocated parser object
 * Input : parser object
 */

DllExport int call_conv pl_free_sgml_parser()
{
  dtd_parser *p = NULL;
  prolog_term parser;
  check_thread_context

  parser = reg_term(CTXTc 1);

  if ( get_parser(parser, &p) )
    {
      free_dtd_parser(p);
      return TRUE;
    }

  return FALSE;
}

/**
 * Free the dtd object
 * Input : dtd object
 */
DllExport int call_conv pl_free_dtd()
{ dtd *dtd = NULL;

  prolog_term dtd_term;
  check_thread_context

  dtd_term  = reg_term(CTXTc 1);

  if ( get_dtd(dtd_term, &dtd) )
    {
      free_dtd(dtd);
      return TRUE;
    }

  return FALSE;
}


/**
 * Internal function to set the parser internal data
 * Input : parser object pointer
 **/

static parser_data *
new_parser_data(dtd_parser *p)
{
  parser_data *pd;

  pd = calloc(1, sizeof(*pd));
  pd->magic = PD_MAGIC;
  pd->parser = p;
  pd->max_errors = MAX_ERRORS;
  pd->max_warnings = MAX_WARNINGS;
  pd->error_mode = EM_PRINT;
  p->closure = pd;

  return pd;
}

/**
 * Internal functions to aid in parsing the xml
 **/
static dtd_srcloc *
file_location(dtd_parser *p, dtd_srcloc *l)
{
  while(l->parent && l->type != IN_FILE)
    l = l->parent;

  return l;
}

static int
can_end_omitted(dtd_parser *p)
{ sgml_environment *env;

  for(env=p->environments; env; env = env->parent)
    {
      dtd_element *e = env->element;

      if ( !(e->structure && e->structure->omit_close) )
	return FALSE;
    }

  return TRUE;
}

/**
 * Error handler. This function is called when an error is encountered
 * while parsing
 * Input : Parser object pointer, error string
 * Output : None, unifies the allocated error term with the error
 **/

static int
on_error(dtd_parser *p, dtd_error *error)
{
  parser_data *pd = p->closure;
  const char *severity;

  if ( pd->stopped )
    return TRUE;

  if ( pd->stopat == SA_ELEMENT &&
       (error->minor == ERC_NOT_OPEN || error->minor == ERC_NOT_ALLOWED) && can_end_omitted(p) )
    {
      end_document_dtd_parser(p);
      sgml_cplocation(&p->location, &p->startloc);
      pd->stopped = TRUE;
      return TRUE;
    }

  switch(error->severity)
    {
    case ERS_STYLE:
      if ( pd->error_mode != EM_STYLE )
	return TRUE;
      severity = "informational";
      break;
    case ERS_WARNING:
      pd->warnings++;
      severity = "warning";
      break;
    case ERS_ERROR:
    default:                            /* make compiler happy */
      pd->errors++;
      severity = "error";
      break;
    }

  /*Create the error(...) term in the allocated error term*/
  if ( pd->error_mode != EM_QUIET )
    {

      /*Temporary prolog variables to create the error term*/
      prolog_term temp_term1 = p2p_new(CTXT);
      prolog_term temp_term2 = p2p_new(CTXT);
      prolog_term tmptail, tmp;
      dtd_srcloc *l = file_location(p, &p->startloc);


      /*Create the error term*/
      c2p_functor(CTXTc "sgml", 4, temp_term1);
      unify_parser(p2p_arg(temp_term1, 1), p);
      c2p_string(CTXTc (l->name ? (char*) l->name : "[]"), p2p_arg( temp_term1, 2));
      c2p_int(CTXTc l->line, p2p_arg( temp_term1, 3));
      c2p_string(CTXTc error->plain_message, p2p_arg( temp_term1, 4));

      c2p_functor(CTXTc (char*)severity, 1, temp_term2);
      p2p_unify(CTXTc temp_term1, p2p_arg( temp_term2, 1));

      /*Generate an error or a warning based on severity*/
      if(!strcmp(severity, "error")){
	p2p_unify(CTXTc global_error_term, temp_term2);
      }
      else
	{
	  tmptail = global_warning_term;
	  while( is_list( tmptail))
	    {
	      tmp = p2p_cdr(tmptail);
	      tmptail = tmp;
	    }
	  c2p_list(CTXTc tmptail);
	  p2p_unify(CTXTc p2p_car(tmptail), temp_term2);
	}
    }

  return TRUE;
}

/**
 * This function is invoked when a decalaration is encountered
 * Input : Parser object, string containing declaration
 * Output : none
 */
static int
on_decl(dtd_parser *p, const ichar *decl)
{
  parser_data *pd = p->closure;

  if ( pd->stopped )
    return TRUE;

  if ( pd->stopat == SA_DECL )
    pd->stopped = TRUE;

  return TRUE;

}

/**
 * This function is invoked when an xml element is opened
 * Input : parser object pointer, xml element which is opened, options
 * Output : none
 **/

static int
on_begin(dtd_parser *p, dtd_element *e, int argc, sgml_attribute *argv)
{
  parser_data *pd = p->closure;
  env *env1;


  if ( pd->stopped )
    return TRUE;

  if(pd->tail)
    {
      /*Prolog term representing the element term created*/
      prolog_term et = p2p_new(CTXT);

      /*Temporary prolog terms to create the output terms*/
      prolog_term tmp, content;

      tmp = p2p_new(CTXT);


      /*Create an element(...) term in the output*/
      c2p_functor(CTXTc "element", 3, et);

      put_element_name(p, p2p_arg( et, 1) , e);


      /*Create the attribute list for the element*/
      unify_attribute_list( p, p2p_arg( et, 2), argc, argv);

      c2p_list(CTXTc tmp);

      if(!p2p_unify(CTXTc pd->tail, tmp))
	return FALSE;

      tmp = p2p_car( pd->tail);

      if(!p2p_unify(CTXTc tmp, et))
	return FALSE;

      content = p2p_arg( tmp, 3);

      tmp = p2p_cdr( pd->tail);
      pd->tail = tmp;

      /*Adjust the output term to handle the recursive nature of an xml document*/
      env1 = sgml_calloc(1, sizeof(struct _env *));
      env1->tail   = pd->tail;
      env1->parent = pd->stack;
      pd->stack   = env1;


      pd->tail = content;
    }
  return TRUE;
}


/**
 * Internal functions to create the attribute list in the element(...) term
 * Input : parser object pointer,  attribute list, options
 * Output : none
 **/
static int
unify_attribute_list(dtd_parser *p, prolog_term alist,
                     int argc, sgml_attribute *argv)
{
  int i;

  /*Temporary prolog terms*/
  prolog_term tail = alist;
  prolog_term temp_term[2];
  prolog_term tmp, tmp1;

  for( i = 0 ; i<argc;  i++)
    {
      tmp = p2p_new(CTXT);
      temp_term[0] = p2p_new(CTXT);
      temp_term[1] = p2p_new(CTXT);
      tmp1 = p2p_new(CTXT);

      put_attribute_name(p, temp_term[0], argv[i].definition->name);
      put_attribute_value(p, temp_term[1], &argv[i]);

      /*Create a list of attributes with '=' as functor*/
      c2p_functor(CTXTc "=", 2, tmp);
      p2p_unify(CTXTc p2p_arg( tmp, 1), temp_term[0]);
      p2p_unify(CTXTc p2p_arg( tmp, 2), temp_term[1]);

      c2p_list(CTXTc  tmp1);

      if( !p2p_unify(CTXTc tail, tmp1))
	return FALSE;

      tmp1 = p2p_car( tail);

      if(!p2p_unify(CTXTc tmp1, tmp))
	return FALSE;

      tmp1 = p2p_cdr(tail);
      tail = tmp1;
    }

  tmp1 = p2p_new(CTXT);
  c2p_nil(CTXTc tmp1);

  if(!p2p_unify(CTXTc tail, tmp1))
    return FALSE;

  return TRUE;
}

/**
 * Internal function to resolve xml namespaces for attributes
 **/

static void
put_attribute_name(dtd_parser *p, prolog_term t, dtd_symbol *nm)
{

  const ichar *url, *local;

  if ( p->dtd->dialect == DL_XMLNS )
    {
      xmlns_resolve_attribute(p, nm, &local, &url);
      if(url)
	{

	  c2p_functor(CTXTc ":", 2, t);
	  put_url(p, p2p_arg( t, 1), url);
	  c2p_string(CTXTc (char*)local, p2p_arg( t, 2));
	}
      else
	{
	  c2p_string(CTXTc (char*)local, t);
	}

    }
  else
    {
      c2p_string(CTXTc (char *) nm->name, t);
    }
}

/**
 * Internal function to check if string is blank
 * Input : string
 **/
static ichar *
istrblank(const ichar *s)
{ for( ; *s; s++ )
    { if ( isspace(*s) )
	return (ichar *)s;
    }

  return NULL;
}

/**
 * Internal function to create the attribute list
 */
static void
put_attribute_value(dtd_parser *p, prolog_term t, sgml_attribute *a)
{
  switch(a->definition->type)
    { case AT_CDATA:
	c2p_string(CTXTc (char *) (a->value.cdata), t);
	break;
    case AT_NUMBER:
      {
	if ( a->value.text )
	  c2p_string(CTXTc (char *) (a->value.text), t);
	else
	  c2p_int(CTXTc a->value.number, t);
	break;
      }
    default:
      {
	const ichar *val = a->value.text;
	const ichar *e;
	prolog_term tmp;

	if ( a->definition->islist )	/* multi-valued attribute */
	  {
	    prolog_term tail, head;

	    tail = t;
	    for(e=istrblank(val); e; val = e+1, e=istrblank(val))
	      {
		if ( e == val )
		  continue;			/* skip spaces */

		tmp = p2p_new(CTXT);
		c2p_list(CTXTc  tmp);

		p2p_unify(CTXTc tail, tmp);

		head = p2p_car( tail);
		tmp = p2p_cdr( tail);
		tail = tmp;
		unify_listval(p, head, a->definition->type, e-val, (char *) val);
	      }

	    tmp = p2p_new(CTXT);
	    c2p_list(CTXTc  tmp);

	    p2p_unify(CTXTc tail, tmp);


	    head = p2p_car( tail);
	    tmp = p2p_cdr( tail);
	    tail = tmp;
	    unify_listval(p, head, a->definition->type, e-val, (char *) val);

	    c2p_nil(CTXTc tmp);
	    p2p_unify(CTXTc tmp, tail);

	  }
	else
	  c2p_string(CTXTc (char*)val, t);

      }
    }

}

static int
unify_listval(dtd_parser *p,  prolog_term t, attrtype type, Integer len, const char *text)
{
  prolog_term tmp = p2p_new(CTXT);
  if ( type == AT_NUMBERS && p->dtd->number_mode == NU_INTEGER )
    {
      char *e;
      long v = strtol(text, &e, 10);

      if ( e-text == len && errno != ERANGE )
	{
	  c2p_int(CTXTc v, tmp);
	  return p2p_unify(CTXTc t, tmp);
	}
      /* TBD: Error!? */
    }

  c2p_string(CTXTc (char*)text, tmp);


  return p2p_unify(CTXTc t, tmp);
}

/**
 * Handler function which is invoked when an entity is encountered
 * Input : parser object, entity encountered
 * Output : none
 **/
static int
on_entity(dtd_parser *p, dtd_entity *e, int chr)
{
  parser_data *pd = p->closure;

  if ( pd->stopped )
    return TRUE;

  if(pd->tail)
    {
      /*Temporary prolog terms to parse prolog inputs*/
      prolog_term h, tmp, tmp2, tmp1;

      tmp1 = p2p_new(CTXT);
      c2p_list(CTXTc tmp1);

      /*Create a term entity(...) in the output*/
      if(p2p_unify(CTXTc pd->tail, tmp1))
 	{
	  h = p2p_car(pd->tail);
	  tmp = p2p_cdr(pd->tail);
	  pd->tail = tmp;
	  tmp2 = p2p_new(CTXT);

	  /*Creating the output term for the entity*/
	  if(e)
	    {

	      c2p_functor(CTXTc (char *) "entity", 1 , tmp2);
	      c2p_string(CTXTc (char *) (e->name->name), p2p_arg( tmp2, 1));
	      p2p_unify(CTXTc h, tmp2);

	    }
	  else
	    {
	      c2p_functor(CTXTc "entity", 1, tmp2);
	      c2p_int(CTXTc chr, p2p_arg( tmp2, 1));
	      p2p_unify(CTXTc h, tmp2);
	    }
  	}
    }
  return TRUE;
}

/**
 * Handler function which is invoked when a processing instruction
 * is encountered
 * Input : parser object pointer, processing instruction
 * Output : none
 **/

static int
on_pi(dtd_parser *p, const ichar *pi)
{
  parser_data *pd = p->closure;
  if ( pd->stopped )
    return TRUE;

  if ( pd->tail )
    {
      prolog_term head, tmp1, tmp;

      tmp = p2p_new(CTXT);
      c2p_list(CTXTc  tmp);

      /*Create a term of the form pi(...) in the output*/
      if( p2p_unify(CTXTc pd->tail, tmp))
	{
	  head = p2p_car(pd->tail);
	  tmp = p2p_cdr(pd->tail);
	  pd->tail = tmp;

	  tmp1 = p2p_new(CTXT);

	  c2p_functor(CTXTc "pi", 1, tmp1);
	  c2p_string(CTXTc (char*)pi, p2p_arg( tmp1, 1));

	  p2p_unify(CTXTc head, tmp1);
	}
    }
  return TRUE;
}

/**
 * Handler function which is invoked when cdata is encountered
 * Input : parser object pointer, encountered cdata
 * Output : none
 **/
static int
on_cdata(dtd_parser *p, data_type type, size_t len, const ochar *data)
{
  parser_data *pd = p->closure;
  int rval=0;
  if ( pd->tail && !pd->stopped )
    {
      /*Temporary prolog terms used to create the output terms*/
      prolog_term head, tmp, tmp1;

      tmp1 = p2p_new(CTXT);

      tmp = p2p_new(CTXT);
      c2p_list(CTXTc  tmp);

      /*Create cdata(...)/sdata(...)/ndata(...) terms in the output*/
      if(p2p_unify(CTXTc pd->tail, tmp))
	{
	  head = p2p_car( pd->tail);
	  tmp = p2p_cdr( pd->tail);
	  pd->tail = tmp;

	  switch(type)
	    {
	    case EC_CDATA:
	      c2p_string(CTXTc (char*)data, tmp1);
	      p2p_unify(CTXTc tmp1, head);
	      break;
	    case EC_SDATA:
	      {
		prolog_term data_term = p2p_new(CTXT);

		c2p_functor(CTXTc "sdata", 1, data_term);
		c2p_string(CTXTc (char*)data, p2p_arg( data_term, 1));

		rval =  p2p_unify(CTXTc head, data_term);
		break;
	      }
	    case EC_NDATA:
	      {
		prolog_term data_term = p2p_new(CTXT);

		c2p_functor(CTXTc "ndata", 1, data_term);
		c2p_string(CTXTc (char*)data, p2p_arg( data_term, 1));

		rval =  p2p_unify(CTXTc head, data_term);
		break;
	      }
	    default:
	      rval = FALSE;
	      assert(0);
	    }
	  if (rval)
	    {
	      return TRUE;
	    }
	}

    }
  return FALSE;
}

/**
 * Handler function which is invoked when the end of element is encountered.
 * Input : parser object pointer, the element
 * Output : none
 **/
static int
on_end(dtd_parser *p, dtd_element *e)
{
  parser_data *pd = p->closure;

  /* Temp prolog terms used to delete the ununified parts of the output term */
  prolog_term tmp;


  tmp = p2p_new(CTXT);
  c2p_nil(CTXTc tmp);

  if(pd->stopped)
    return TRUE;

  if ( pd->tail && !pd->stopped ) {
      if ( !is_nil( pd->tail)) {
	p2p_unify(CTXTc pd->tail, tmp);
      }
      if ( pd->stack ) {
	env *parent = pd->stack->parent;
	pd->tail = pd->stack->tail;
	/* win64 crashes here on examples/sgml/files/bat.sgml */
	sgml_free(pd->stack);
	pd->stack = parent;
      } else {
	if ( pd->stopat == SA_CONTENT )
	  pd->stopped = TRUE;
      }
  }
  
  if ( pd->stopat == SA_ELEMENT && !p->environments->parent )
    pd->stopped = TRUE;
  
  return TRUE;
}


/**
 * Helper functions to create the element(...) term in the output. Also
 * xml namespaces.
 **/

static void
put_element_name(dtd_parser *p, prolog_term t, dtd_element *e)
{
  const ichar *url, *local;

  if ( p->dtd->dialect == DL_XMLNS) {
      assert(p->environments->element == e);
      xmlns_resolve_element(p, &local, &url);

      if(url) {
	  c2p_functor(CTXTc ":", 2, t);
	  put_url(p, p2p_arg( t, 1), url);
	  c2p_string(CTXTc (char*)local, p2p_arg( t, 2));
      } else {
	c2p_string(CTXTc (char*)local, t);
      }
  } else
    c2p_string (CTXTc (char *) (e->name->name), t);

  return;
}


/**
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * put_url(dtd_parser *p, term_t t, const ichar *url)
 *  Store the url-part of a name-space qualifier in term.  We call
 *  xml:xmlns(-Canonical, +Full) trying to resolve the specified
 *  namespace to an internal canonical namespace.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Input : pointer to parser object, prolog term, url
 * Output : none
 **/

static void
put_url(dtd_parser *p, prolog_term t, const ichar *url)
{
  parser_data *pd = p->closure;

  if ( !pd->on_urlns ) {
    c2p_string(CTXTc (char*) url, t);
    return;
  }
}


/**
 *Functions to handle quoted strings
 **/

#define CHARSET MAXSTRLEN

static int
do_quote(prolog_term in, prolog_term quoted, char **map)
{ char *ins;
  size_t len;
  unsigned  char *s;
  char outbuf[1024];
  char *out = outbuf;
  int outlen = sizeof(outbuf);
  size_t o = 0;
  int changes = 0;

  prolog_term tmp = 0;

  ins = p2c_string( in);

  len = strlen( ins);

  if ( len == 0 )
    return p2p_unify(CTXTc in, quoted);

  for(s = (unsigned char*)ins ; len-- > 0; s++ )
    { int c = *s;

      if ( map[c] ) {
	size_t l = strlen(map[c]);
	if ( o+l >= (size_t) outlen ) {
	  outlen *= 2;
	  if ( out == outbuf ) {
	    out = malloc(outlen);
	    memcpy(out, outbuf, sizeof(outbuf));
	  } else {
	    out = realloc(out, outlen);
	  }
	}
	memcpy(&out[o], map[c], l);
	o += l;
	changes++;
      } else {
	if ( o >= (size_t)(outlen-1) ) {
	  outlen *= 2;
	  if ( out == outbuf ) {
	    out = malloc(outlen);
	    memcpy(out, outbuf, sizeof(outbuf));
	  } else {
	    out = realloc(out, outlen);
	  }
	}
	out[o++] = c;
      }
    }
  out[o]= 0;
  
  if ( changes > 0 ) {
    c2p_string(CTXTc out, tmp);
    return p2p_unify(CTXTc quoted, tmp);
  }
  else
    return p2p_unify(CTXTc in, quoted);
}

/**
 * Function to handle quoted attributes
 **/
DllExport int call_conv pl_xml_quote_attribute()
{
  check_thread_context
  prolog_term in = reg_term(CTXTc 1);
  prolog_term out = reg_term(CTXTc 2);
  static char **map;

  if ( !map )
    { int i;

      if ( !(map = malloc(CHARSET*sizeof(char*))) )
	return sgml2pl_error(ERR_ERRNO, errno);

      for(i=0; i<CHARSET; i++)
	map[i] = NULL;

      map['<']  = "&lt;";
      map['>']  = "&gt;";
      map['&']  = "&amp;";
      map['\''] = "&apos;";
      map['"']  = "&quot;";
    }

  return do_quote(in, out, map);
}

/**
 * Function to handle quoted cdata
 **/
DllExport int call_conv pl_xml_quote_cdata()
{
  check_thread_context
  prolog_term in = reg_term(CTXTc 1);
  prolog_term out = reg_term(CTXTc 2);
  static char **map;

  if ( !map )
    { int i;

      if ( !(map = malloc(CHARSET*sizeof(char*))) )
	return sgml2pl_error(ERR_ERRNO, errno);

      for(i=0; i<CHARSET; i++)
	map[i] = NULL;

      map['<']  = "&lt;";
      map['>']  = "&gt;";
      map['&']  = "&amp;";
    }

  return do_quote(in, out, map);
}

DllExport int call_conv pl_xml_name()
{ char *ins;
  size_t len;
  static dtd_charclass *map;
  unsigned int i;
  check_thread_context
  prolog_term in = reg_term(CTXTc 1);


  if ( !map )
    map = new_charclass();

  ins = p2c_string( in);

  len = strlen( ins);

  if ( len == 0 )
    return FALSE;
  if ( !(map->class[ins[0] & 0xff] & CH_NMSTART) )
    return FALSE;
  for(i=1; i<len; i++)
    {
      if ( !(map->class[ins[i] & 0xff] & CH_NAME) )
	return FALSE;
    }

  return TRUE;
}











