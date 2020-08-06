/* File:      io_builtins_xsb_i.h
** Author(s): davulcu, kifer, swift, zhou
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
** $Id: io_builtins_xsb_i.h,v 1.68 2013-05-06 21:10:24 dwarren Exp $
** 
*/


/* This file is separate from io_builtins.c because here we have the
   in-lined file_function (to speed up file_get/put). */



#if (defined(CYGWIN))
#include <fcntl.h>
#endif

#ifdef WIN_NT
#include <io.h>
#endif

#include "file_modes_xsb.h"

/* protected by MUTEX IO */
STRFILE *iostrs[MAXIOSTRS] = {NULL,NULL,NULL,NULL,NULL};

extern char   *expand_filename(char *filename);
//extern int xsb_intern_fileptr(FILE *, char *, char *, char *, int);

static FILE *stropen(CTXTdeclc char *str)
{
  Integer i;
  size_t  len;
  STRFILE *tmp;
  char *stringbuff;

  for (i=1; i<MAXIOSTRS; i++) { /* 0 reserved for static use from C */
    if (iostrs[i] == NULL) break;
  }
  if (i>=MAXIOSTRS) return FALSE;
  tmp = (STRFILE *)mem_alloc(sizeof(STRFILE),OTHER_SPACE);
  iostrs[i] = tmp;
  len = strlen(str);
  // new copy is needed in case string came from concatenated longstring
  stringbuff = (char *)mem_alloc(len+1,OTHER_SPACE);
  strcpy(stringbuff,str);

  tmp->strcnt = len;
  tmp->strptr = (byte *)stringbuff;
  tmp->strbase = (byte *)stringbuff;
#ifdef MULTI_THREAD
  tmp->owner = xsb_thread_id;
#endif
  return (FILE *)iostrdecode(i);
}

void strclose(int i)
{
  i = iostrdecode(i);
  if (iostrs[i] != NULL) {
    mem_dealloc(iostrs[i]->strbase,iostrs[i]->strcnt+1,OTHER_SPACE);
    mem_dealloc((byte *)iostrs[i],sizeof(STRFILE),OTHER_SPACE);
    iostrs[i] = NULL;
  }
}

int unset_fileptr(FILE *stream) {
  int i;
  for (i = 0; i <= MAX_OPEN_FILES; i++) {
    if (open_files[i].file_ptr == stream) return(i);
  }
  return(-1);
}


/* file_flush, file_pos, file_truncate, file_seek */

/* Two levels of locking: MUTEX_IO locks table iteself, and is used
   when we arent changing a given stream itself, as in file_open.  In
   addition, however, the io_locks are used to ensure atomic operation
   for all io operations.

Best to put locks AFTER SET_FILEPTR to avoid problems with mutexes
   (they should be unlocked via error_handler, but ...) */
inline static xsbBool file_function(CTXTdecl)
{
  FILE *fptr;
  int io_port, mode, charset;
  size_t size, value, offset, length;
  STRFILE *sfptr;
  XSB_StrDefine(VarBuf);
  char *addr, *tmpstr;
  prolog_term pterm;
  Cell term;
  char *strmode;

  switch (ptoc_int(CTXTc 1)) {
  case FILE_FLUSH: /* file_function(0,+IOport,-Ret,_,_) */
    /* ptoc_int(CTXTc 2) is XSB I/O port */
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);   
    value = fflush(fptr);
    XSB_STREAM_UNLOCK(io_port);
    ctop_int(CTXTc 3, (int) value);
    break;
  case FILE_SEEK: /* file_function(1,+IOport, +Offset, +Place, -Ret) */
    io_port = (int)ptoc_int(CTXTc 2);
    if (io_port < 0) {
      if (ptoc_int(CTXTc 4) != 0) 
	xsb_permission_error(CTXTc "file_seek","atom",ptoc_int(CTXTc 4),"file_seek",1); 
      XSB_STREAM_LOCK(io_port);
      sfptr = iostrs[iostrdecode(io_port)];
      value = ptoc_int(CTXTc 3);
      length = sfptr->strcnt + sfptr->strptr - sfptr->strbase ;
      if (value <= length) {
	if (sfptr->strcnt == -1) length++;
	sfptr->strptr = sfptr->strbase + value;
	sfptr->strcnt = length - value;
	ctop_int(CTXTc 5, 0);
      }
      else ctop_int(CTXTc 5,-1);
      XSB_STREAM_UNLOCK(io_port);
    }
    else {
      XSB_STREAM_LOCK(io_port);
      SET_FILEPTR(fptr, io_port);
      value = fseek(fptr, (long) ptoc_int(CTXTc 3), (int)ptoc_int(CTXTc 4));
      XSB_STREAM_UNLOCK(io_port);
      ctop_int(CTXTc 5, (int) value);
    }
    break;
  case FILE_TRUNCATE: /* file_function(2,+IOport,+Length,-Ret,_) */
    size = iso_ptoc_int_arg(CTXTc 3,"file_truncate/3", 2);
    //    size = ptoc_int(CTXTc 3);
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
#ifndef WIN_NT
    fseek(fptr, (long) size, 0);
    value = ftruncate( fileno(fptr), (off_t) size);
    ctop_int(CTXTc 4, (int) value);
#else
#ifdef BITS64
    ctop_int(CTXTc 4, (int) _chsize_s(fileno(fptr), size));
#else
    ctop_int(CTXTc 4, (int) chsize(fileno(fptr), size));
#endif
#endif
    XSB_STREAM_UNLOCK(io_port);
    break;
  case FILE_POS: /* file_function(3, +IOport, -Pos) */
    io_port = (int)ptoc_int(CTXTc 2); 
    term = ptoc_tag(CTXTc 3);
    if (io_port >= 0) {
      XSB_STREAM_LOCK(io_port);
      SET_FILEPTR(fptr, io_port);
      if (isnonvar(term)) {
	XSB_STREAM_UNLOCK(io_port);
	return ptoc_int(CTXTc 3) == ftell(fptr);
      }
      else
	ctop_int(CTXTc 3, ftell(fptr));
    } else { /* reading from string */
      XSB_STREAM_LOCK(io_port);
      sfptr = strfileptr(io_port);
      if (sfptr->strcnt == EOF) 
	offset = EOF;
      else 
	offset = sfptr->strptr - sfptr->strbase;
      if (isnonvar(term)) {
	XSB_STREAM_UNLOCK(io_port);
	return ptoc_int(CTXTc 3) == offset;
      }
      else
	ctop_int(CTXTc 3, offset);
    }
    XSB_STREAM_UNLOCK(io_port);
    break;
  case XSB_FILE_OPEN: {
    /* file_function(4, +FileName, +Mode, -IOport) TLS: changing modes
     and differentiating binaries, so its best to not allow integer
     modes any more */
    int ioport, opennew;
    int str_type = 0;
    char string_mode[3];

    tmpstr = ptoc_longstring(CTXTc 2);
    pterm = reg_term(CTXTc 3);

    SYS_MUTEX_LOCK( MUTEX_IO );

    if (isstring(pterm)) {
      strncpy(string_mode,string_val(pterm),3);

      switch ((string_mode)[0]) {
      case 'r': 
	mode = OREAD; 
	if ((string_mode)[1] == 'b')
	  str_type = BINARY_FILE_STREAM;
	else  str_type = TEXT_FILE_STREAM;
	break;
      case 'w': 
	mode = OWRITE; 
	if ((string_mode)[1] == 'b')
	  str_type = BINARY_FILE_STREAM;
	else  str_type = TEXT_FILE_STREAM;
	break;
      case 'a': 
	mode = OAPPEND; 
	if ((string_mode)[1] == 'b')
	  str_type = BINARY_FILE_STREAM;
	else  str_type = TEXT_FILE_STREAM;
	break;
      case 's':
	str_type = STRING_STREAM;
	if ((string_mode)[1] == 'r')
	  /* reading from string */
	  mode = OSTRINGR;
	else if ((string_mode)[1] == 'w')
	  /* writing to string */
	  mode = OSTRINGW;
	else
	  mode = -1;
	break;
      default: mode = -1;
      }
    } else {
      SYS_MUTEX_UNLOCK( MUTEX_IO );
      xsb_abort("[FILE_OPEN] File opening mode must be an atom.");
      mode = -1;
    } /* end mode handling code */

    switch (mode) {

      /* In UNIX the 'b" does nothing, but in Windows it
	 differentiates a binary from a text file.  If I take the 'b'
	 out, this breaks the compiler. */

    case OREAD: strmode = "rb"; break; /* READ_MODE */
    case OWRITE:  strmode = "wb"; break; /* WRITE_MODE */
    case OAPPEND: strmode = "ab"; break; /* APPEND_MODE */
    case OSTRINGR:
      if ((fptr = stropen(CTXTc tmpstr))) {
	ctop_int(CTXTc 5, (Integer)fptr);
      } else {
	ctop_int(CTXTc 5, -1000);
      }
      SYS_MUTEX_UNLOCK( MUTEX_IO );
      return TRUE;
    case OSTRINGW:
      xsb_abort("[FILE_OPEN] Output to strings has not been implemented yet");
      ctop_int(CTXTc 5, -1000);
      SYS_MUTEX_UNLOCK( MUTEX_IO );
      return TRUE;
    default:
      xsb_warn(CTXTc "FILE_OPEN: Invalid open file mode");
      ctop_int(CTXTc 5, -1000);
      SYS_MUTEX_UNLOCK( MUTEX_IO );
      return TRUE;
    }
    
    /* we reach here only if the mode is OREAD,OWRITE,OAPPEND */
    addr = expand_filename(tmpstr);

    opennew = (int)ptoc_int(CTXTc 4);
    if (!xsb_intern_file(CTXTc "FILE_OPEN",addr, &ioport,strmode,opennew)) {
      open_files[ioport].stream_type = str_type;
      ctop_int(CTXTc 5,ioport);
    }
    else ctop_int(CTXTc 5,-1);
    mem_dealloc(addr,MAXPATHLEN,OTHER_SPACE);

    SYS_MUTEX_UNLOCK( MUTEX_IO );
    break;
  }
    /* TLS: handling the case in which we are closing a flag that
       we're currently seeing or telling.  Probably bad programming
       style to mix streams w. open/close, though. */
  case FILE_CLOSE: /* file_function(5, +Stream,FORCE/NOFORCE) */
    {
      int rtrn; 
      io_port = (int)ptoc_int(CTXTc 2);
      if (io_port < 0) {
	CHECK_IOS_OWNER(io_port);
	strclose(io_port);
      }
      else {
	XSB_STREAM_LOCK(io_port);
	SET_FILEPTR(fptr, io_port);
	if ((rtrn = fclose(fptr))) {
	  if (ptoc_int(CTXTc 3) == NOFORCE_FILE_CLOSE) {
	    XSB_STREAM_UNLOCK(io_port);
	    //	    xsb_permission_error(CTXTc "fclose","file",reg[2],"file_close",1); 
	    xsb_permission_error(CTXTc strerror(errno),"file",reg[2],"file_close",1); 
	  }
	}
	open_files[io_port].file_ptr = NULL;
	open_files[io_port].file_name = NULL;
	open_files[io_port].io_mode = '\0';
	open_files[io_port].stream_type = 0;
	open_files[io_port].charset = CURRENT_CHARSET;
	if (pflags[CURRENT_INPUT] == (Cell) io_port) 
	  { pflags[CURRENT_INPUT] = STDIN;}
	if (pflags[CURRENT_OUTPUT] == (Cell) io_port) 
	  { pflags[CURRENT_OUTPUT] = STDOUT;}
        XSB_STREAM_UNLOCK(io_port);
      }
      break;
    }
  case FILE_GET_BYTE:  /* file_function(6, +IOport, -IntVal) */
    io_port = (int)ptoc_int(CTXTc 2);
    io_port_to_fptrs(io_port,fptr,sfptr,charset);
    XSB_STREAM_LOCK(io_port);
    ctop_int(CTXTc 3, GetC(fptr,sfptr));
    XSB_STREAM_UNLOCK(io_port);
    break;
  case FILE_PUT_BYTE:   /* file_function(7, +IOport, +IntVal) */
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
    /* ptoc_int(CTXTc 3) is char to write */
    value = ptoc_int(CTXTc 3);
    //    printf("val %d value wc %d\n",value,sizeof(wchar_t));
    putc((int)value, fptr);
#ifdef WIN_NT
    if (io_port==2 && value=='\n') fflush(fptr); /* hack for Java interface */
#endif
    XSB_STREAM_UNLOCK(io_port);
    break;
  case FILE_GETBUF:
    /* file_function(8, +IOport, +ByteCount (int), -String, -BytesRead)
       Read ByteCount bytes from IOport into String starting 
       at position Offset. */
    size = ptoc_int(CTXTc 3);
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
    XSB_StrSet(&VarBuf,"");
    XSB_StrEnsureSize(&VarBuf,(int)size);
    value = fread(VarBuf.string, 1, size, fptr);
    VarBuf.length = (int)value;
    XSB_StrNullTerminate(&VarBuf);
    XSB_STREAM_UNLOCK(io_port);
    ctop_string(CTXTc 4, VarBuf.string);
    ctop_int(CTXTc 5, value);
    break;
  case FILE_PUTBUF:
    /* file_function(9, +IOport, +ByteCount (int), +String, +Offset,
			-BytesWritten) */
    /* Write ByteCount bytes into IOport from String beginning with Offset in
       that string	      */
    pterm = reg_term(CTXTc 4);
    if (islist(pterm))
      addr = 
	p_charlist_to_c_string(CTXTc pterm,&VarBuf,"FILE_PUTBUF","input string");
    else if (isstring(pterm))
      addr = string_val(pterm);
    else {
      xsb_abort("[FILE_PUTBUF] Output argument must be an atom or a character list");
      addr = NULL;
    }
    size = ptoc_int(CTXTc 3);
    offset = ptoc_int(CTXTc 5);
    length = strlen(addr);
    size = ( size < length - offset ? size : length - offset);
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
    value = fwrite(addr+offset, 1, size, fptr);
    XSB_STREAM_UNLOCK(io_port);
    ctop_int(CTXTc 6, value);
    break;
  case FILE_READ_LINE: {
    /* Works like fgets(buf, size, stdin). Fails on reaching the end of file
    ** Invoke: file_function(FILE_READ_LINE, +File, -Str). Returns
    ** the string read.
    ** Prolog invocation: file_read_line(+File, -Str) */
    char buf[MAX_IO_BUFSIZE+1];
    int break_loop = FALSE;
    int eof=FALSE;

    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
    XSB_StrSet(&VarBuf,"");

    do {   /* fix for char-set! */
      if (fgets(buf, MAX_IO_BUFSIZE, fptr) == NULL && feof(fptr)) {
	eof=TRUE;
	break;
      } else {
	XSB_StrAppend(&VarBuf,buf);
	break_loop = (buf[(strlen(buf)-1)] == '\n');
      }
    } while (!break_loop);
    
    ctop_string(CTXTc 3, VarBuf.string);
    
    XSB_STREAM_UNLOCK(io_port);
    /* this complex cond takes care of incomplete lines: lines that end with
       end of file and not with end-of-line. */
    if ((VarBuf.length>0) || (!eof))
      return TRUE;
    else
      return FALSE;
  }
  case FILE_READ_LINE_LIST: {
    /* Works like FILE_READ_LINE but returns a list of codes
    ** Invoke: file_function(FILE_READ_LINE, +File, -List). Returns
    ** the list of codes read. Rewritten by DSW 5/18/04 to allow \0 in lines.
    ** Prolog invocation: file_read_line_list(+File, -Str) */
    char *line_buff = NULL;
    int line_buff_len = 0;
    int line_buff_disp;
    char *atomname;
    int c;
    Cell new_list;
    CPtr top = NULL;
    int i;

    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);

    line_buff_disp = 0;
    do {
      if (line_buff_disp >= line_buff_len-1) { // -1 in case of eol
	int old_len = line_buff_len;
	line_buff_len = line_buff_disp+MAX_IO_BUFSIZE;
	if(!(line_buff = mem_realloc(line_buff,old_len,line_buff_len,LEAK_SPACE)))
	  xsb_exit("No space for line buffer");
//	printf("frll: expand buffer line_buff(%p,%d)\n",line_buff,line_buff_len);
      }
      *(line_buff+line_buff_disp) = c = getc(fptr);  // fix for charset!!
      if (c == EOF) {
	if (feof(fptr)) break;
	else xsb_permission_error(CTXTc strerror(errno),"file read",reg[2],"file_read_line_list",2);
      }
      line_buff_disp++;
    } while (c != '\n');
    *(line_buff+line_buff_disp) = 0;
//    printf("frll: eol at %d\n",line_buff_disp);
    
    check_glstack_overflow(3, pcreg, 2*sizeof(Cell)*line_buff_disp);
    atomname = line_buff;

    if (line_buff_disp == 0) new_list = makenil;
    else {
      new_list = makelist(hreg);
      for (i = 0; i < line_buff_disp; i++) {
	follow(hreg) = makeint(*(unsigned char *)atomname);
	atomname++;
	top = hreg+1;
	hreg += 2;
	follow(top) = makelist(hreg);
      }
      follow(top) = makenil;
    }

    ctop_tag(CTXTc 3, new_list);
    
    if (line_buff) mem_dealloc(line_buff,line_buff_len,LEAK_SPACE);

    /* this complex cond takes care of incomplete lines: lines that end with
       end of file and not with end-of-line. */
    //    if ((line_buff_disp>0) || (c != EOF))
    XSB_STREAM_UNLOCK(io_port);
    if (line_buff_disp>0)
      return TRUE;
    else
      return FALSE;
  }
  /* Like FILE_PUTBUF, but ByteCount=Line length. Also, takes atoms and lists
     of characters: file_function(11, +IOport, +String, +Offset) */
  case FILE_WRITE_LINE:
    pterm = reg_term(CTXTc 3);
    if (islist(pterm))
      addr =
	p_charlist_to_c_string(CTXTc pterm,&VarBuf,"FILE_WRITE_LINE","input string");
    else if (isstring(pterm))
      addr = string_val(pterm);
    else {
      xsb_abort("[FILE_WRITE_LINE] Output arg must be an atom or a char list");
      addr = NULL;
    }
    offset = ptoc_int(CTXTc 4);
    size = strlen(addr)-offset;
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
    fwrite(addr+offset, 1, size, fptr);
    XSB_STREAM_UNLOCK(io_port);
    break;

  case FILE_REOPEN: 
    /* file_function(FILE_REOPEN, +Filename,+Mode,+IOport,-ErrorCode) */
    tmpstr = ptoc_string(CTXTc 2);
    pterm = reg_term(CTXTc 3);
    if (isointeger(pterm))
      mode = (int)oint_val(pterm);
    else if (isstring(pterm)) {
      switch ((string_val(pterm))[0]) {
      case 'r': mode = OREAD; break;
      case 'w': mode = OWRITE; break;
      case 'a': mode = OAPPEND; break;
      case 's':
	if ((string_val(pterm))[1] == 'r')
	  /* reading from string */
	  mode = OSTRINGR;
	else if ((string_val(pterm))[1] == 'w')
	  /* writing to string */
	  mode = OSTRINGW;
	else
	  mode = -1;
	break;
      default: mode = -1;
      }
    } else {
      xsb_abort("[FILE_REOPEN] Open mode must be an atom or an integer");
      mode = -1;
    }

    switch (mode) {
      /* "b" does nothing, but POSIX allows it */
    case OREAD:   strmode = "rb";  break; /* READ_MODE */
    case OWRITE:  strmode = "wb";  break; /* WRITE_MODE */
    case OAPPEND: strmode = "ab";  break; /* APPEND_MODE */
    case OSTRINGR:
      xsb_abort("[FILE_REOPEN] Reopening of strings hasn't been implemented");
      ctop_int(CTXTc 5, -1000);
      return TRUE;
    case OSTRINGW:
      xsb_abort("[FILE_REOPEN] Reopening of strings hasn't been implemented");
      ctop_int(CTXTc 5, -1000);
      return TRUE;
    default:
      xsb_warn(CTXTc "FILE_REOPEN: Invalid open file mode");
      ctop_int(CTXTc 5, -1000);
      return TRUE;
    }
    
    /* we reach here only if the mode is OREAD,OWRITE,OAPPEND */
    addr = expand_filename(tmpstr);
    io_port = (int)ptoc_int(CTXTc 4);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR(fptr, io_port);
    fflush(fptr);
    fptr = freopen(addr, string_val(pterm), fptr);
    XSB_STREAM_UNLOCK(io_port);

    if (fptr) {
      struct stat stat_buff;
      if (!stat(addr, &stat_buff) && !S_ISDIR(stat_buff.st_mode))
	/* file exists and isn't a dir */
	ctop_int(CTXTc 5, 0);
      else {
	xsb_warn(CTXTc "FILE_REOPEN: File %s is a directory, cannot open!", addr);
	ctop_int(CTXTc 5, -2);
      }
    } else
      ctop_int(CTXTc 5, -3);
    mem_dealloc(addr,MAXPATHLEN,OTHER_SPACE);

    break;

    /* TLS: I looked through this, and it seems to work with streams,
       but its possible that the file clone should move the file name
       and mode from the source to the destination when it copies or
       creates an io port? */

  case FILE_CLONE: {
    /* file_function(FILE_CLONE,SrcIOport,DestIOport,ErrorCode) */
    /* Note: when cloning (dup) streams, NT doesn't copy the buffering mode of
       the source file. So, if this will turn out to be a problem, a new
       builtin (interface to setvbuf) will have to be introduced. */
    FILE *src_fptr, *dest_fptr;
    int src_fd, dest_fd, dest_xsb_fileno, src_xsb_fileno, errcode=0;
    char *mode = NULL;
    prolog_term dest_fptr_term;

    src_xsb_fileno = (int)ptoc_int(CTXTc 2);
    dest_fptr_term = reg_term(CTXTc 3);
    XSB_STREAM_LOCK(src_xsb_fileno);
    XSB_STREAM_LOCK(int_val(dest_fptr_term));
    SET_FILEPTR(src_fptr, src_xsb_fileno);
    fflush(src_fptr);
    src_fd = fileno(src_fptr);

    if (isnonvar(dest_fptr_term)) {
      /* assume the user wants dup2-like functionality */
      SET_FILEPTR(dest_fptr, int_val(dest_fptr_term));
      dest_fd = fileno(dest_fptr);
      errcode = dup2(src_fd,dest_fd);
    } else {
      /* user wanted dup-like functionality */
      dest_fd = dup(src_fd);
      if (dest_fd >= 0) {
#if (defined (WIN_NT) && ! defined(CYGWIN))
	/* NT doesn't have fcntl(). Brain damage? But Cygwin does */
	mode = "r+";
#else /* Unix */ 
	int fd_flags;
	/* get the flags that open has set for this file descriptor */
	fd_flags = fcntl(dest_fd, F_GETFL) & (O_ACCMODE | O_APPEND); 
	switch (fd_flags) {
	case O_RDONLY:
	    mode = "rb";
	    break;

	case O_WRONLY:
	    mode = "wb";
	    break;

	case O_ACCMODE:
		/* Should not happen */
		/* Falls through */

	case O_RDWR:
	    mode = "rb+";
	    break;

	case O_RDONLY | O_APPEND:
	    mode = "rb";
	    break;

	case O_WRONLY | O_APPEND:
	    mode = "ab";
	    break;

	case O_ACCMODE | O_APPEND:
		/* Should not happen */
		/* Falls through */

	case O_RDWR | O_APPEND:
	    mode = "ab+";
	    break;

	default:
		mode = "rb+";
		break;
	}
#endif
	dest_fptr = fdopen(dest_fd, mode);
	if (dest_fptr) {
	  dest_xsb_fileno = 
	    xsb_intern_fileptr(CTXTc dest_fptr,"FILE_CLONE",
			       open_files[src_xsb_fileno].file_name,
			       &open_files[src_xsb_fileno].io_mode,
			       open_files[src_xsb_fileno].charset);
	  c2p_int(CTXTc dest_xsb_fileno, dest_fptr_term);
	} else {
	  /* error */
	  errcode = -1;
	}
      } else
	/* error */
	errcode = -1;
    }
    ctop_int(CTXTc 4, errcode);

    XSB_STREAM_UNLOCK(int_val(dest_fptr_term));
    XSB_STREAM_UNLOCK(src_xsb_fileno);
    break;
  }

  case PIPE_OPEN: { /* open_pipe(-ReadPipe, -WritePipe) */
    int pipe_fd[2];

    if (PIPE(pipe_fd) < 0) {
      ctop_int(CTXTc 2, PIPE_TO_PROC_FAILED);
      ctop_int(CTXTc 3, PIPE_FROM_PROC_FAILED);
      return TRUE;
    }
    ctop_int(CTXTc 2, pipe_fd[0]);
    ctop_int(CTXTc 3, pipe_fd[1]);
    break;
  }

  case FD2IOPORT: { /* fd2ioport(+Pipe, -IOport,+Mode) */
    /* this can take any C file descriptor and make it into an XSB I/O port.
        For backward compatability,mode may not be used -- where it is "u" */
    int pipe_fd, i;
    char *mode=NULL;
#ifndef WIN_NT /* unix */
    int fd_flags;
#endif
    pipe_fd = (int)ptoc_int(CTXTc 2); /* the C file descriptor */
    pterm = reg_term(CTXTc 4);

    if (isstring(pterm)) {
      if ((string_val(pterm))[0] == 'u') {
	/* Need to try to find mode */
#ifdef WIN_NT
    /* NT doesn't have fcntl(). Brain damage? */
    mode = "r+";
#else /* unix */
    fd_flags = fcntl(pipe_fd, F_GETFL); 
    if (fd_flags == O_RDONLY)
      mode = "rb";
    else if (fd_flags == O_WRONLY)
      mode = "wb";
    else {
      /* can't determine the mode of the C fd -- return "r+" */
      mode = "r+";
    }
#endif
      } 
      else mode = string_val(pterm);
    }
    else {
      xsb_abort("[FD2IOPORT] Opening mode must be an atom.");
      mode = "x";
    }

    fptr = fdopen(pipe_fd, mode);

    SYS_MUTEX_LOCK( MUTEX_IO );
    /* xsb_intern_file will return -1, if fdopen fails */
    i = xsb_intern_fileptr(CTXTc fptr, "FD2IOPORT","created from fd",mode,CURRENT_CHARSET);
    ctop_int(CTXTc 3, i);
    open_files[i].stream_type = PIPE_STREAM;
    SYS_MUTEX_UNLOCK( MUTEX_IO );
    break;
  }
    
    /* TLS: relying on thread-safety of library -- no mutex here */
  case FILE_CLEARERR: { /* file_function(16, +IOport) */
    io_port = (int)ptoc_int(CTXTc 2);
    if ((io_port < 0) && (io_port >= -MAXIOSTRS)) {
    }
    else {
      XSB_STREAM_LOCK(io_port);
      SET_FILEPTR(fptr, io_port);
      clearerr(fptr);
      XSB_STREAM_UNLOCK(io_port);
    }
    break;
  }

  case TMPFILE_OPEN: {
    /* file_function(17, -IOport)
       Opens a temp file in r/w mode and returns its IO port */
    SYS_MUTEX_LOCK( MUTEX_IO );
    if ((fptr = tmpfile())) 
      ctop_int(CTXTc 2, xsb_intern_fileptr(CTXTc fptr, "TMPFILE_OPEN",
					   "TMPFILE","wb+",CURRENT_CHARSET));
    else
      ctop_int(CTXTc 2, -1);
    SYS_MUTEX_UNLOCK( MUTEX_IO );
    break;
  }
    
  case STREAM_PROPERTY: {
    int stream;
    stream = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(stream);
    switch (ptoc_int(CTXTc 3)) {

      /* Type, Repos, eof_actions are all currently functions of class */
    case STREAM_EOF_ACTION:
    case STREAM_REPOSITIONABLE:
    case STREAM_TYPE: 
    case STREAM_CLASS: 
      ctop_int(CTXTc 4, open_files[stream].stream_type);
      break;
    
    case STREAM_FILE_NAME:  
      if (open_files[stream].stream_type < 3)
	ctop_string(CTXTc 4, open_files[stream].file_name);
      break;

    case STREAM_MODE: 
    case STREAM_INPUT: 
    case STREAM_OUTPUT: {

      mode = open_files[stream].io_mode; 
      if (mode == 'r' || mode == 's') {
	ctop_int(CTXTc 4,READ_MODE);
      } else if (mode == 'w' || mode == 'x') {
	ctop_int(CTXTc 4,WRITE_MODE);
      } else if (mode == 'a' || mode == 'b') {
	ctop_int(CTXTc 4,APPEND_MODE);
      }
      break;
    }
    }
    XSB_STREAM_UNLOCK(stream);
    break;
  }

  case IS_VALID_STREAM: {
    int stream;
    char iomode;

    stream = (int)ptoc_int(CTXTc 2);
    if (stream >= MAX_OPEN_FILES)
	return FALSE;
    XSB_STREAM_LOCK(stream);
    if ((stream < 0) && (stream >= -MAXIOSTRS)) {
      /* port for reading from string */
      sfptr = strfileptr(stream);
      XSB_STREAM_UNLOCK(stream);
      if (sfptr == NULL) {
	return FALSE;
      }
      else {
	ctop_int(CTXTc 3,READ_MODE);
	return TRUE;
      }
    }
    if (stream < -MAXIOSTRS) {
      XSB_STREAM_UNLOCK(stream);
      return FALSE;
    }
    fptr = fileptr(stream); \
    if ((fptr==NULL) && (stream != 0)) {
      XSB_STREAM_UNLOCK(stream);
      return FALSE;
    }
    else {
	iomode = open_files[stream].io_mode; 
	XSB_STREAM_UNLOCK(stream);
	if (iomode == 'r' || iomode == 's') {
	  ctop_int(CTXTc 3,READ_MODE);
	} else ctop_int(CTXTc 3,WRITE_MODE);
	return TRUE;
      }  
  }

  case PRINT_OPENFILES: { /* no args */
    int i; 
    for (i= 0 ; i < MAX_OPEN_FILES ; i++) {
      if (open_files[i].file_name == NULL) {
 	printf("i: %d File Ptr %p Mode %c Type %d \n",
 	        i,open_files[i].file_ptr,open_files[i].io_mode,
	        open_files[i].stream_type);
      } else {
	printf("i; %d File Ptr %p Name %s Mode %c Type %d\n",i,
	       open_files[i].file_ptr, open_files[i].file_name,open_files[i].io_mode,
	       open_files[i].stream_type);
      }
    }
    break;
  }

    /* TLS: range checking for streams done by is_valid_stream */
  case FILE_END_OF_FILE: {

    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    if (io_port < 0) {
      sfptr = strfileptr(io_port);
      XSB_STREAM_UNLOCK(io_port);
      if (sfptr->strcnt == EOF) {
	XSB_STREAM_UNLOCK(io_port);
	return TRUE;
      } else {
	XSB_STREAM_UNLOCK(io_port);
	return FALSE;
      }
    }
    else {
      if (feof(open_files[ptoc_int(CTXTc 2)].file_ptr) != 0) {
	XSB_STREAM_UNLOCK(io_port);
	return TRUE;
      } else {
	XSB_STREAM_UNLOCK(io_port);
	return FALSE;
      }
    }
  }

  case FILE_PEEK_BYTE: {
    int bufchar;

    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    if ((io_port < 0) && (io_port >= -MAXIOSTRS)) {
      sfptr = strfileptr(io_port);
      ctop_int(CTXTc 3, strpeekc(sfptr));
    } else {
      SET_FILEPTR(fptr, io_port);
      bufchar = getc(fptr);
      ctop_int(CTXTc 3, bufchar);
      if (bufchar >= 0) 
	ungetc(bufchar, fptr);
    }
    XSB_STREAM_UNLOCK(io_port);
    break;
  }

  case XSB_STREAM_LOCK_B: {
#ifdef MULTI_THREAD
    XSB_STREAM_LOCK(ptoc_int(CTXTc 2));
#else
    return TRUE;
#endif
    break;
  }

  case XSB_STREAM_UNLOCK_B: {
#ifdef MULTI_THREAD
    XSB_STREAM_UNLOCK(ptoc_int(CTXTc 2));
#else
    return TRUE;
#endif
    break;
  }

  case FILE_NL: 
    io_port = (int)ptoc_int(CTXTc 2);
    SET_FILEPTR(fptr, io_port);
#ifdef WIN_NT
    XSB_STREAM_LOCK(io_port);
    putc(CH_RETURN,fptr); putc(CH_NEWLINE,fptr);
    XSB_STREAM_UNLOCK(io_port);
#else
    putc(CH_NEWLINE,fptr);
#endif
    break;

  case FILE_GET_CODE: {	/* file_function(6, +IOport, -IntVal) */
    int code;
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    code = GetCodeP(io_port);
    ctop_int(CTXTc 3, code);
    XSB_STREAM_UNLOCK(io_port);
    break;
  }
  case FILE_PUT_CODE:   /* file_function(7, +IOport, +IntVal) */
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR_CHARSET(fptr, charset,io_port);
    value = ptoc_int(CTXTc 3);
    PutCode((int)value,charset,fptr);
#ifdef WIN_NT
    if (io_port==2 && value=='\n') fflush(fptr); /* hack for Java interface */
#endif
    XSB_STREAM_UNLOCK(io_port);
    break;
  case FILE_GET_CHAR:	{
    int read_codepoint;
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    read_codepoint = GetCodeP(io_port);
    if (read_codepoint == EOF) 
      ctop_string(CTXTc 3, "end_of_file");
    else {
      char s[5],*ch_ptr;
      ch_ptr = (char *) utf8_codepoint_to_str(read_codepoint, (byte *)s); /* internal is always utf8 */
      *ch_ptr = '\0';
      ctop_string(CTXTc 3,s);
    }
    XSB_STREAM_UNLOCK(io_port);
    break;
  }
  case FILE_PUT_CHAR: {
    byte *ch_ptr;

    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    SET_FILEPTR_CHARSET(fptr, charset, io_port);
    ch_ptr = (byte *)iso_ptoc_string(CTXTc 3,"put_char/[1,2]");
    PutCode(utf8_char_to_codepoint(&ch_ptr),charset,fptr);
    break; 
  }

  case FILE_PEEK_CODE: {
    int bufcode;
    io_port = (int)ptoc_int(CTXTc 2);
    io_port_to_fptrs(io_port,fptr,sfptr,charset);
    XSB_STREAM_LOCK(io_port);
    bufcode = GetCodeP(io_port);
    if (bufcode == EOF) 
      bufcode = -1;
    else {
      byte s[5],*ch_ptr;
      ch_ptr = codepoint_to_str(bufcode, charset, s);
      while (ch_ptr>s){
	ch_ptr--;
	unGetC(*ch_ptr,fptr,sfptr); 
      }
    }
    ctop_int(CTXTc 3, bufcode);
    XSB_STREAM_UNLOCK(io_port);
    break;
  }

 case FILE_PEEK_CHAR:	{  // these 2 cases can be consolidated! (and sim above)
    int read_codepoint;
    io_port = (int)ptoc_int(CTXTc 2);
    XSB_STREAM_LOCK(io_port);
    read_codepoint = GetCodeP(io_port);
    if (read_codepoint == EOF) 
      ctop_string(CTXTc 3, "end_of_file");
    else {
      char s[5],*ch_ptr,ss[5];
      ch_ptr = (char *)utf8_codepoint_to_str(read_codepoint,(byte *) s); /* internal char always utf8 */
      *ch_ptr = '\0';
      io_port_to_fptrs(io_port,fptr,sfptr,charset);
      if (charset == UTF_8) {
	while (ch_ptr>s){
	  ch_ptr--;                   
	  unGetC(*ch_ptr,fptr,sfptr); 
	}
      } else {
	ch_ptr = (char *) codepoint_to_str(read_codepoint, charset, (byte *)ss);
	while (ch_ptr>ss) {
	  ch_ptr--;
	  unGetC(*ch_ptr,fptr,sfptr);
	}
      }
      ctop_string(CTXTc 3,s);
    }
    XSB_STREAM_UNLOCK(io_port);
    break;
 }

  case ATOM_LENGTH: {
    Integer len;
    Cell term = ptoc_tag(CTXTc 2);
    Cell lenpar = ptoc_tag(CTXTc 3);
    if (isstring(term)) {
      len = utf8_nchars((byte *)string_val(term));
      if (isref(lenpar)) ctop_int(CTXTc 3,len);
      else if (isointeger(lenpar)) {
	if (oint_val(lenpar) == len) return TRUE;
	if (oint_val(lenpar) < 0) 
	  xsb_domain_error(CTXTc "not_less_than_zero",lenpar,"atom_length/2",2);
	else return FALSE;
      }
      else xsb_type_error(CTXTc "integer",lenpar,"atom_length/2",2);
    }
    else if (isref(term)) {
      xsb_instantiation_error(CTXTc "atom_length/2",1);
    } else {
      xsb_type_error(CTXTc "atom",term,"atom_length/2",1);
    }
    break;
  }

  default:
    xsb_abort("[FILE_FUNCTION]: Invalid file operation, %d\n", ptoc_int(CTXTc 1));
  }
  
  return TRUE;
}

