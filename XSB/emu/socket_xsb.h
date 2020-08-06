/* File:      socket_xsb.h
** Author(s): juliana, davulcu, kifer
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
** $Id: socket_xsb.h,v 1.11 2011-05-16 01:03:30 kifer Exp $
** 
*/

#ifndef __SOCKET_XSB_H__

#define __SOCKET_XSB_H__

#include "socket_defs_xsb.h"




#ifdef WIN_NT

#define BAD_SOCKET(sockfd)         sockfd==INVALID_SOCKET
#define SOCKET_OP_FAILED(sockfd)   sockfd==SOCKET_ERROR
#define IS_IP_ADDR(string)    	   inet_addr(string) != INADDR_NONE
#define XSB_SOCKET_ERRORCODE	   WSAGetLastError()
#define XSB_SOCKET_ERRORCODE_RESET WSASetLastError(0)
#define FillWithZeros(addr)    	  ZeroMemory(&addr, sizeof(addr));
#ifndef EINPROGRESS
#define EINPROGRESS               WSAEINPROGRESS
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK               WSAEWOULDBLOCK
#endif
#define SET_SOCKET_BLOCKING(fd, val) (ioctlsocket(fd, FIONBIO, (u_long FAR *)&val) == 0)
#define GETSOCKOPT(fd,lvl,optname,optval,optlen) getsockopt(fd, lvl, optname, (char *)optval, optlen)
#define SETSOCKOPT(fd,lvl,optname,optval,optlen) setsockopt(fd, lvl, optname, (char *)optval, optlen)


#else /* UNIX */

#define SOCKET 	        int
#define SOCKADDR_IN 	struct sockaddr_in /* in windows, but not Unix */
#define PSOCKADDR       struct sockaddr *  /* in windows, but not Unix */
#define closesocket    	       	   close
#define XSB_SOCKET_ERRORCODE   	   errno
#define XSB_SOCKET_ERRORCODE_RESET errno = 0
#define BAD_SOCKET(sockfd)         sockfd<0
#define SOCKET_OP_FAILED(sockfd)   sockfd<0
#define IS_IP_ADDR(string)    	   inet_addr(string) != -1
#define FillWithZeros(addr)    	   memset((char *)&addr, (int) 0, sizeof(addr));
#define SET_SOCKET_BLOCKING(fd, val) (val \
                                      ? (fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) != -1) \
                                      : (fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL, 0) ^ O_NONBLOCK)) != -1))
#define GETSOCKOPT(fd,lvl,optname,optval,optlen) getsockopt(fd, lvl, optname, (void *)optval, optlen)
#define SETSOCKOPT(fd,lvl,optname,optval,optlen) setsockopt(fd, lvl, optname, (void *)optval, optlen)

#endif



#define MAXCONNECT 50	     /* max number of connections per socket_select */

/* the length of the XSB header that contains the info on the size of the
   subsequent message. Used by socket_send/socket_recv */
#define XSB_MSG_HEADER_LENGTH  sizeof(int)

/* These are used only by the readmsg function */
#define SOCK_READMSG_FAILED  -1      /* failed socket call     	      	    */
#define SOCK_READMSG_EOF     -2	     /* when EOF is reached    	       	    */
#define SOCK_HEADER_LEN_MISMATCH -3  /* when hdr length != message length   */

#endif /* __SOCKET_XSB_H__ */








