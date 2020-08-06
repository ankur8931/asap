/* File:      socket_defs_xsb.h
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: socket_defs_xsb.h,v 1.9 2010-08-19 15:03:37 spyrosh Exp $
** 
*/



/* socket macros */
#define SOCKET_ROOT        0
#define SOCKET_BIND        1
#define SOCKET_LISTEN      2
#define SOCKET_ACCEPT      3
#define SOCKET_CONNECT     4
/* #define SOCKET_FLUSH       5  deleted, use file_function */
#define SOCKET_CLOSE       6
#define SOCKET_RECV	   7
#define SOCKET_SEND	   8
#define SOCKET_SEND_EOF	   9
#define SOCKET_SEND_ASCI   10
#define SOCKET_GET0        11
#define SOCKET_PUT         12
#define SOCKET_SET_OPTION  13
#define SOCKET_SET_SELECT     14
#define SOCKET_SELECT         15
#define SOCKET_SELECT_DESTROY 16


#define SOCK_NOLINGER -1     /* the no-linger socket option */

/* Some typical error codes for socket ops.
   Positive codes are used for socket failures. 
   They are returned by errno.
   The other typical error code is TIMEOUT_ERR */
#define SOCK_OK       0      /* indicates sucessful return from socket      */
#define SOCK_EOF     -1      /* end of file in socket_recv, socket_get0     */
