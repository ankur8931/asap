/*  Part of XSB-Prolog SGML/XML parser

    Author:  Rohan Shirwaikar
    WWW:     www.xsb.org
    Copying: LGPL-2.  See the file COPYING or http://www.gnu.org

*/

#include "cinterf.h"
#include <stdlib.h>
#ifdef HAVE_MALLOC_H
#include HAVE_MALLOC_H
#endif
#include "error.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "dtd.h"

#define CHARSET 256

static int
do_quote(prolog_term in, prolog_term quoted, char **map)
{ char *ins;
  unsigned len;
  unsigned  char *s;
  char outbuf[1024];
  char *out = outbuf;
  int outlen = sizeof(outbuf);
  int o = 0;
  int changes = 0;

  prolog_term tmp;

  ins = p2c_string( in);
  
  len = strlen( ins);

  if ( len == 0 )
    return p2p_unify(in, quoted);

  for(s = (unsigned char*)ins ; len-- > 0; s++ )
  { int c = *s;
    
    if ( map[c] )
    { int l = strlen(map[c]);
      if ( o+l >= outlen )
      { outlen *= 2;
	  
	if ( out == outbuf )
	{ out = malloc(outlen);
	  memcpy(out, outbuf, sizeof(outbuf));
	} else
	{ out = realloc(out, outlen);
	}
      }
      memcpy(&out[o], map[c], l);
      o += l;
      changes++;
    } else
    { if ( o >= outlen-1 )
      { outlen *= 2;
	  
	if ( out == outbuf )
	{ out = malloc(outlen);
	  memcpy(out, outbuf, sizeof(outbuf));
	} else
	{ out = realloc(out, outlen);
	}
      }
      out[o++] = c;
    }
  }
  out[o]= 0;
  
  if ( changes > 0 )
	{
		c2p_string( out, tmp);
		return p2p_unify( quoted, tmp);
	}
  else
    return p2p_unify(in, quoted);
}


static int
pl_xml_quote_attribute()
{ 
	prolog_term in = reg_term(1);
	prolog_term out = reg_term(2);
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


static int
pl_xml_quote_cdata()
{ 
	prolog_term in = reg_term(1);
	prolog_term out = reg_term(2);
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


static int
pl_xml_name()
{ char *ins;
  unsigned len;
  static dtd_charclass *map;
  unsigned int i;
  prolog_term in = reg_term(1);


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
