/*****************************************************************************
 *                        socketcall.h
 * Header files to be included for socket api in windows and unix
 *
 ****************************************************************************/

#ifdef WIN_NT
#include <winsock2.h>
#include <windows.h>
#else 
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<unistd.h>
#endif
