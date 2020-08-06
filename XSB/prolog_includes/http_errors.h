
/*
  Return Codes for Libwww Protocol Modules and Streams.
  Success codes are (>=0) and failure are (<0)
*/

/* we disable the constant definitions for standard HTTP errors inside the XSB
   libwww code, because these defs are included from Libwww itself */
#ifndef XSB_LIBWWW_PACKAGE

#define HT_OK			0	/* Generic success */
#define HT_ALL			1	/* Used by Net Manager */

#define HT_CONTINUE             100     /* Continue an operation */
#define HT_UPGRADE              101     /* Switching protocols */

#define HT_LOADED		200  	/* Everything's OK */
#define HT_CREATED  	        201     /* New object is created */
#define HT_ACCEPTED  	        202     /* Accepted */
#define HT_NON_AUTHORITATIVE    203     /* Non-authoritative Information */
#define HT_NO_DATA		204  	/* OK but no data was loaded */
#define HT_RESET_CONTENT        205     /* Reset content */
#define HT_PARTIAL_CONTENT	206  	/* Partial Content */
#define HT_PARTIAL_OK	     	207     /* Partial Update OK */

#define HT_MULTIPLE_CHOICES     300     /* Multiple choices */
#define HT_PERM_REDIRECT	301  	/* Permanent redirection */
#define HT_FOUND        	302  	/* Found */
#define HT_SEE_OTHER            303     /* See other */
#define HT_NOT_MODIFIED         304     /* Not Modified */
#define HT_USE_PROXY            305     /* Use Proxy */
#define HT_PROXY_REDIRECT       306     /* Proxy Redirect */
#define HT_TEMP_REDIRECT        307     /* Temporary redirect */


#define HT_IGNORE		900  	/* Ignore this in the Net manager */
#define HT_CLOSED		901  	/* The socket was closed */
#define HT_PENDING		902  	/* Wait for connection */
#define HT_RELOAD		903  	/* If we must reload the document */

#define HT_ERROR		-1	/* Generic failure */

#define HT_BAD_REQUEST	        -400    /* Bad request */
#define HT_NO_ACCESS		-401	/* Unauthorized */
#define HT_FORBIDDEN		-403	/* Access forbidden */
#define HT_NOT_FOUND		-404	/* Not found */
#define HT_NOT_ALLOWED	        -405    /* Method Not Allowed */
#define HT_NOT_ACCEPTABLE	-406	/* Not Acceptable */
#define HT_NO_PROXY_ACCESS      -407    /* Proxy Authentication Failed */
#define HT_CONFLICT             -409    /* Conflict */
#define HT_LENGTH_REQUIRED      -411    /* Length required */
#define HT_PRECONDITION_FAILED  -412    /* Precondition failed */
#define HT_TOO_BIG              -413    /* Request entity too large */
#define HT_URI_TOO_BIG          -414    /* Request-URI too long */
#define HT_UNSUPPORTED          -415    /* Unsupported */
#define HT_BAD_RANGE            -416    /* Request Range not satisfiable */
#define HT_EXPECTATION_FAILED   -417    /* Expectation Failed */
#define HT_REAUTH               -418    /* Reauthentication required */
#define HT_PROXY_REAUTH         -419    /* Proxy Reauthentication required */

#define HT_SERVER_ERROR	        -500    /* Internal server error */
#define HT_NOT_IMPLEMENTED      -501    /* Not implemented */
#define HT_BAD_GATEWAY	        -502	/* Bad gateway */
#define HT_RETRY		-503	/* If service isn't available */
#define HT_GATEWAY_TIMEOUT      -504    /* Gateway timeout */
#define HT_BAD_VERSION		-505	/* Bad protocol version */
#define HT_PARTIAL_NOT_IMPLEMENTED -506

#define HT_INTERNAL		-900    /* Weird -- should never happen. */
#define HT_WOULD_BLOCK		-901    /* If we are in a select */
#define HT_INTERRUPTED 		-902    /* Note the negative value! */
#define HT_PAUSE                -903    /* If we want to pause a stream */
#define HT_RECOVER_PIPE         -904    /* Recover pipe line */
#define HT_TIMEOUT              -905    /* Connection timeout */
#define HT_NO_HOST              -906    /* Can't locate host */

#endif /* XSB_LIBWWW_PACKAGE */


/* ERROR CODES SPECIFIC TO THE XSB LIBWWW PACKAGE  -- NOT http errors */
#define WWW_DOC_SYNTAX	     	-2001	/* Bad document syntax or nesting
					   limit exceeded. */
#define WWW_EXTERNAL_ENTITY     -2002   /* XML external entity not found */
#define WWW_EXPIRED_DOC	        -2003   /* when document's last modified time
					   is older than user-specified time */
#define WWW_URI_SYNTAX	        -2004   /* when uri has bad syntax */

