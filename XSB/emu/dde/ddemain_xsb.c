/* File:      ddemain_xsb.c (for windows)
** Author(s): Warren
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1998
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
** $Id: ddemain_xsb.c,v 1.6 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#define INCLUDE_DDEML_H
#include <windows.h>
/** #include <ddeml.h> **/
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "cinterf.h"

#define WM_USER_INITIATE (WM_USER+1)
DWORD idInst;
HWND hwnd;
char szAppName[] = "XSBWin";
char szTopic[] = "square";
HSZ hszService;

char tempstring[100];
long int rcode;
char szBuffer[256];
char szBuff1[256];
char szBuff2[256];
char *szQuery;
char *szBuff3;
#define initsizeBuff3 500
long int sizeBuff3 = 0;
  
long FAR PASCAL _export xsbProc(HWND,UINT,UINT,LONG);
HDDEDATA FAR PASCAL _export DdeCallback(
        UINT,UINT,HCONV,HSZ,HSZ,HDDEDATA,DWORD,DWORD);

/* quick test to see whether atom must be quoted */
int mustquote(char *atom)
{
    int i;

    if (!(atom[0] >= 'a' && atom[0] <= 'z')) return TRUE;
    for (i=1; atom[i] != '\0'; i++) {
        if (!((atom[i] >= 'a' && atom[i] <= 'z') ||
             (atom[i] >= 'A' && atom[i] <= 'Z') ||
             (atom[i] == '_') ||
             (atom[i] >= '0' && atom[i] <= '9')
             )) return TRUE;
    }
    return FALSE;
}

/* copy a string (quoted if !toplevel and necessary) into a buffer.  */
void printpstring(char *atom, int toplevel, char *straddr, long int *ind)
{
    int i;
   
    if (toplevel || !mustquote(atom)) {
        strcpy(straddr+*ind,atom);
        *ind += strlen(atom);
    } else {
        straddr[(*ind)++] = '\'';
        for (i = 0; atom[i] != '\0'; i++) {
            straddr[(*ind)++] = atom[i];
            if (atom[i] == '\'') straddr[(*ind)++] = '\'';
        }
        straddr[(*ind)++] = '\'';
    }
}
    
/* calculate approximate length of a printed term.  For space alloc. */
DWORD clenpterm(prolog_term term)
{
  int i, clen;

  if (is_var(term)) return 11;
  else if (is_int(term)) return 12;
  else if (is_float(term)) return 12;
  else if (is_nil(term)) return 2;
  else if (is_string(term)) return strlen(p2c_string(term))+5;
  else if (is_list(term)) {
      clen = 1;
      clen += clenpterm(p2p_car(term)) + 1;
      while (is_list(term)) {
          clen += clenpterm(p2p_car(term)) + 1;
          term = p2p_cdr(term);
      }
      if (!is_nil(term)) {
          clen += clenpterm(term) + 1;
      }
      return clen+1;
  } else if (is_functor(term)) {
      clen = strlen(p2c_functor(term))+5;
      if (p2c_arity(term) > 0) {
          clen += clenpterm(p2p_arg(term,1)) + 1;
          for (i = 2; i <= p2c_arity(term); i++) {
              clen += clenpterm(p2p_arg(term,i)) + 1;
          }
          return clen + 1;
      } else return clen;
  } else {
      fprintf(stderr,"error, unrecognized type");
      return 0;
  }
}

/* print a prolog_term into a buffer.
(Atoms are quoted if !toplevel and it's necessary for Prolog reading) */
void printpterm(prolog_term term, int toplevel, char *straddr, long int *ind)
{
  int i;

  if (is_var(term)) {
      sprintf(tempstring,"_%p",term);
      strcpy(straddr+*ind,tempstring);
      *ind += strlen(tempstring);
  } else if (is_int(term)) {
      sprintf(tempstring,"%d",p2c_int(term));
      strcpy(straddr+*ind,tempstring);
      *ind += strlen(tempstring);
  } else if (is_float(term)) {
      sprintf(tempstring,"%f",p2c_float(term));
      strcpy(straddr+*ind,tempstring);
      *ind += strlen(tempstring);
  } else if (is_nil(term)) {
      strcpy(straddr+*ind,"[]");
      *ind += 2;
  } else if (is_string(term)) {
      printpstring(p2c_string(term),toplevel,straddr,ind);
  } else if (is_list(term)) {
      strcpy(straddr+*ind,"[");
      *ind += 1;
      printpterm(p2p_car(term),FALSE,straddr,ind);
      term = p2p_cdr(term);
      while (is_list(term)) {
          strcpy(straddr+*ind,",");
          *ind += 1;
          printpterm(p2p_car(term),FALSE,straddr,ind);
          term = p2p_cdr(term);
      }
      if (!is_nil(term)) {
          strcpy(straddr+*ind,"|");
          *ind += 1;
          printpterm(term,FALSE,straddr,ind);
      }
      strcpy(straddr+*ind,"]");
      *ind += 1;
  } else if (is_functor(term)) {
      printpstring(p2c_functor(term),FALSE,straddr,ind);
      if (p2c_arity(term) > 0) {
          strcpy(straddr+*ind,"(");
          *ind += 1;
          printpterm(p2p_arg(term,1),FALSE,straddr,ind);
          for (i = 2; i <= p2c_arity(term); i++) {
              strcpy(straddr+*ind,",");
              *ind += 1;
              printpterm(p2p_arg(term,i),FALSE,straddr,ind);
          }
          strcpy(straddr+*ind,")");
          *ind += 1;
      }
  } else fprintf(stderr,"error, unrecognized type");
}

/* Windows main routine */
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdParam, int nCmdShow)
{
    MSG msg;
    WNDCLASS wndclass;
    FARPROC pfnDdeCallback;
    UINT ddeerror;
    int argc = 3;
    char *argv[] = {"xsb","-i","-n"};

    if (hPrevInstance) return FALSE;

    wndclass.style = 0;
    wndclass.lpfnWndProc = xsbProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    RegisterClass(&wndclass);

    hwnd = CreateWindow(szAppName,"XSB DDE Server",WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,CW_USEDEFAULT,    // x,y position
                        CW_USEDEFAULT,CW_USEDEFAULT,    // 200,100,    // x,y size
                        NULL,NULL,hInstance,NULL);

    ShowWindow(hwnd,nCmdShow);  // nCmdShow
    UpdateWindow(hwnd);

    /* Initialize for DDE */
    pfnDdeCallback = MakeProcInstance((FARPROC)DdeCallback,hInstance);
    idInst = 0;
    ddeerror = DdeInitialize(&idInst,
                        (PFNCALLBACK)pfnDdeCallback,
                        CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS,
                        0L);
    if (ddeerror) {
        sprintf(tempstring,"Could not initialize server!\n  rc=%x, idInst=%x",ddeerror,idInst);
        MessageBox(hwnd,tempstring,szAppName, MB_ICONEXCLAMATION|MB_OK);
        DestroyWindow(hwnd);
        return FALSE;
    }
    
    freopen("xsblog","w",stdout);
    freopen("xsblog","a",stderr);
    
    /* Initialize xsb */
    xsb_init(argc,argv);

        /* This seems necessary??? huh???     
    rcode = xsb_query_string("true.");
    while (!rcode) {rcode = xsb_next();} */

    hszService = DdeCreateStringHandle(idInst,szAppName,0);
    DdeNameService(idInst,hszService,NULL,DNS_REGISTER);
    
    /* sprintf(tempstring,"XSB INITIALIZED!\n  rc=%x, idInst=%x",ddeerror,idInst);
    MessageBox(hwnd,tempstring,szAppName, MB_ICONEXCLAMATION|MB_OK); */

    while (GetMessage(&msg,NULL,0,0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    
    /* Close XSB connection */
    xsb_close();

    /* terminate DDE */
    (void) DdeFreeStringHandle(idInst,hszService);
    FreeProcInstance(pfnDdeCallback);
    DdeUninitialize(idInst);

    return msg.wParam;
}

long FAR PASCAL _export xsbProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (message) {
      case WM_PAINT:
        
        hdc = BeginPaint(hwnd,&ps);
        GetClientRect(hwnd,&rect);
        DrawText(hdc,"XSB running",-1,&rect,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        EndPaint(hwnd,&ps);
        return 0;

      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);
}

HDDEDATA FAR PASCAL _export DdeCallback(UINT type, UINT fmt,
                        HCONV hConv, HSZ hsz1, HSZ hsz2,
                        HDDEDATA data, DWORD data1,
                        DWORD data2)
{
  long int ind, i, spaceneeded, sizeQuery;
  DWORD Qlen, QSegLen;
  static HCONV handConv;
  static HDDEDATA hdDataHandle;

    /*DdeQueryString(idInst,hsz1,szBuff1,sizeof(szBuff1),0);
    DdeQueryString(idInst,hsz2,szBuff2,sizeof(szBuff2),0);
    
    fprintf(stderr,"DDE Callback, type=%d, fmt=%d, hConv=%d, hsz1=%s, hsz2=%s,\n d1=%x, d2=%x\n",
        type,fmt,hConv,szBuff1,szBuff2,data1,data2);*/

    
    switch (type) {
      case XTYP_ERROR:
        fprintf(stderr,"error: xtyp_error\n");
        return NULL;
      case XTYP_ADVDATA:
        fprintf(stderr,"DDE msg received ADVDATA\n");
        return DDE_FNOTPROCESSED;
      case XTYP_ADVREQ:
        fprintf(stderr,"DDE msg received ADVREQ\n");
        return NULL;
      case XTYP_ADVSTART:
        fprintf(stderr,"DDE msg received ADVSTART\n");
        return NULL;
      case XTYP_ADVSTOP:
        fprintf(stderr,"DDE msg received ADVSTOP\n");
        return NULL;
      
      case XTYP_CONNECT:
        DdeQueryString(idInst,hsz2,szBuffer,sizeof(szBuffer),0);
        if (strcmp(szBuffer,szAppName)) return FALSE;
        Qlen = DdeQueryString(idInst,hsz1,NULL,0,0);
        szQuery = (char *)malloc(Qlen+1);
        (void)DdeQueryString(idInst,hsz1,szQuery,Qlen+1,0);
        if (!strcmp(szQuery,"XSB")) {
            free(szQuery);
            szQuery = NULL;
        }
        return TRUE;

      case XTYP_CONNECT_CONFIRM:
        handConv = hConv;
        return TRUE;

      case XTYP_DISCONNECT:
        return NULL;
      case XTYP_EXECUTE:
        fprintf(stderr,"DDE msg received EXECUTE\n");
        return DDE_FNOTPROCESSED;

      case XTYP_POKE:
        QSegLen = DdeGetData(data,NULL,100000,0L);
        if (!szQuery) {
            szQuery = (char *)malloc(QSegLen);
            QSegLen = DdeGetData(data,szQuery,100000,0L);
            sizeQuery = QSegLen;
        } else {
            szQuery = (char *)realloc(szQuery,sizeQuery+QSegLen+1);
            QSegLen = DdeGetData(data,szQuery+sizeQuery,100000,0L);
            sizeQuery =+ QSegLen;
        }
        return DDE_FACK;
        
      case XTYP_REGISTER:
        fprintf(stderr,"DDE msg received REGISTER\n");
        return NULL;

      case XTYP_REQUEST:
        /*fprintf(stderr,"DDE msg received REQUEST:\n");*/
        if (!szQuery) return NULL;
        if (sizeBuff3 < 10) {
            szBuff3 = (char *)malloc(initsizeBuff3);
            sizeBuff3 = initsizeBuff3;
        }
        ind = 0;
        rcode = xsb_query_string(szQuery);      /* call the query */
        if (rcode) {
            strcpy(szBuff3+ind,"no\r");
            ind += 3;
        } else if (is_string(reg_term(2)) || p2c_arity(reg_term(2))==0) {
            strcpy(szBuff3+ind,"yes\r");
            ind += 4;
            while (!rcode) rcode = xsb_next();
        } else while (!rcode) {
            spaceneeded = ind + clenpterm(reg_term(2)) + 20;  /* fudge factor */
            if (spaceneeded > sizeBuff3) {
                while (spaceneeded > sizeBuff3) {sizeBuff3 = 2*sizeBuff3;}
                szBuff3 = realloc(szBuff3,sizeBuff3);
            }
            for (i=1; i<p2c_arity(reg_term(2)); i++) {
                printpterm(p2p_arg(reg_term(2),i),TRUE,szBuff3,&ind);
                strcpy(szBuff3+ind,"\t");
                ind += 1;
            }
            printpterm(p2p_arg(reg_term(2),p2c_arity(reg_term(2))),TRUE,szBuff3,&ind);
            strcpy(szBuff3+ind,"\r");
            ind += 1;
            rcode = xsb_next();
        }
        hdDataHandle = DdeCreateDataHandle(idInst,szBuff3,ind+1,0,hsz2,CF_TEXT,0);
        free(szQuery);
        szQuery = NULL;
        return hdDataHandle;

      case XTYP_WILDCONNECT:
        fprintf(stderr,"DDE msg received WILDCONNECT\n");
        return NULL;
      default:
        fprintf(stderr,"DDE msg received: %d\n",type);        
    }
    return NULL;
}

