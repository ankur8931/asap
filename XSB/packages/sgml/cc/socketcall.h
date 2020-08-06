/******************************************************************************
 *                             socketcall.h
 *  Files to include for socket handling for windows and unix
 *
 *****************************************************************************/


#ifdef WIN_NT
#include <winsock2.h>
#else 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif

