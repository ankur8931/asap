#   MSVC_mkfile.mak:  Makefile to compile XSB on Windows using VC++ NMAKE
#   windows/MSVC_mkfile.mak.  Generated from MSVC_mkfile.in by configure.
#
# Usage:
#   NMAKE /f "MSVC_Mfile.mak"
#

### Custom compilation: the file custom_settings.mak
###
### To compile with native Oracle support, put this in custom_settings.mak:
### ORACLE=yes
### In this case, also add the appropriate
###  /LIBPATH:"oracle_library1" /LIBPATH:"oracle_library2" ...
### to MY_LIBRARY_DIRS (see below), where Oracle libraries should be taken
### from the Oracle manual.
###
### To compile with debugging options, add this:
### DEBUG=yes
###
### If you have custom directories to use for #include commands or DLLs,
### create the file custom_settings.mak and put these lines there
### MY_INCLUDE_DIRS=/I"incdir1" /I"incdir2" ...
### MY_LIBRARY_DIRS=/LIBPATH:"libdir1" /LIBPATH:"libdir2" ...
###     For instance, to configure with support for threads, add
###     /LIBPATH:"pthreadVC1.lib" to MY_LIBRARY_DIRS in custom_settings.mak
###     and configure with --enable-mt
### XSB_INTERPROLOG=yes/true/1 can also be specified here, if needed
###                 In this case, MY_INCLUDE_DIRS should point to the
###                 directories that contain jni.h and jni_md.h

!IF EXIST("..\build\windows\custom_settings.mak")
!INCLUDE "..\build\windows\custom_settings.mak"
!ENDIF

CONFIGDIR=..\config\x86-pc-windows
CONFIG_INCLUDE_FLAG=/I"$(CONFIGDIR)" $(MY_INCLUDE_DIRS)
SITE_LIBS= $(MY_LIBRARY_DIRS)

!IF "$(DLL)" != "no"
DLL=yes
!ENDIF

# $(CALLCONV) can be "cdecl"  or "stdcal"
!IF "$(CALLCONV)" != "cdecl"
CALLCONV=stdcal
!ENDIF

!IF "$(ORACLE)" != "yes"
ORACLE=no
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

################################################################################
# Begin Project
CPP=cl.exe

# Assume we are running NMAKE in the emu directory
#!IF  "$(DLL)" == "yes"
# Put DLL in the bin directory
DLLDIR=$(CONFIGDIR)\bin
OUTDIR=$(CONFIGDIR)\bin
#!ELSE
#OUTDIR=$(CONFIGDIR)\bin
#!ENDIF

OBJDIR=$(CONFIGDIR)\saved.o
LIBDIR=$(CONFIGDIR)\lib

INTDIR=.

# MSVC_emumkfile.mac must be in the config directory and define LINK_OTHEROBJS
# as a list of .obj files to be linked with XSB statically. These .obj files
# are intended to be outside of the XSB tree.
!IF EXIST("$(CONFIGDIR)\MSVC_emumkfile.mac")
!INCLUDE "$(CONFIGDIR)\MSVC_emumkfile.mac"
!ENDIF

!IF "$(CALLCONV)" == "cdecl"
CALL_CONV=XSB_DLL_C
!ELSE
CALL_CONV=XSB_DLL
!ENDIF

!IF  "$(ORACLE)" == "yes"
ORACLE_FLAG=/D "ORACLE"
ORACLE_MSG=with Oracle support
!IF  "$(SITE_LIBS)" == ""
!ERROR Oracle libraries must be specified, if building XSB with native interface to Oracle
!ENDIF
!ENDIF


!IF  "$(XSB_INTERPROLOG)" == "yes" || "$(XSB_INTERPROLOG)" == "true" || "$(XSB_INTERPROLOG)" == "1"
INTERPROLOG_FLAG=/D "XSB_INTERPROLOG"
INTERPROLOG_MSG=with support for InterProlog's native engine
!ENDIF

!IF "$(DLL)" == "yes"
ALL:: "$(DLLDIR)\xsb.dll" "$(OUTDIR)\xsb.exe"
!ELSE
ALL:: "$(OUTDIR)\xsb.exe"
!ENDIF

SOCKET_LIBRARY=wsock32.lib

"$(OUTDIR)" :
	if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_OBJS=$(OBJDIR)/
CPP_SBRS=
EXECUTABLE=link.exe
DLL_LINK_OBJS= \
	"$(OBJDIR)/auxlry.obj" \
	"$(OBJDIR)/builtin.obj" \
	"$(OBJDIR)/biassert.obj" \
	"$(OBJDIR)/call_graph_xsb.obj" \
	"$(OBJDIR)/call_xsb.obj" \
	"$(OBJDIR)/cinterf.obj" \
	"$(OBJDIR)/deadlock.obj" \
	"$(OBJDIR)/debug_xsb.obj" \
	"$(OBJDIR)/dis.obj" \
	"$(OBJDIR)/dynload.obj" \
	"$(OBJDIR)/dynamic_stack.obj" \
	"$(OBJDIR)/emuloop.obj" \
	"$(OBJDIR)/error_xsb.obj" \
	"$(OBJDIR)/findall.obj" \
	"$(OBJDIR)/function.obj" \
	"$(OBJDIR)/hash_xsb.obj" \
	"$(OBJDIR)/hashtable_xsb.obj" \
	"$(OBJDIR)/hashtable.obj" \
	"$(OBJDIR)/hashtable_itr.obj" \
	"$(OBJDIR)/heap_xsb.obj" \
	"$(OBJDIR)/url_encode.obj" \
	"$(OBJDIR)/sha1.obj" \
	"$(OBJDIR)/md5.obj" \
	"$(OBJDIR)/incr_xsb.obj" \
	"$(OBJDIR)/init_xsb.obj" \
	"$(OBJDIR)/inst_xsb.obj" \
	"$(OBJDIR)/io_builtins_xsb.obj" \
	"$(OBJDIR)/loader_xsb.obj" \
	"$(OBJDIR)/memory_xsb.obj" \
	"$(OBJDIR)/odbc_xsb.obj" \
	"$(OBJDIR)/orient_xsb.obj" \
	"$(OBJDIR)/pathname_xsb.obj" \
	"$(OBJDIR)/psc_xsb.obj" \
	"$(OBJDIR)/private_builtin.obj" \
	"$(OBJDIR)/random_xsb.obj" \
	"$(OBJDIR)/remove_unf.obj" \
	"$(OBJDIR)/residual.obj" \
	"$(OBJDIR)/rw_lock.obj" \
	"$(OBJDIR)/scc_xsb.obj" \
	"$(OBJDIR)/slgdelay.obj" \
	"$(OBJDIR)/struct_manager.obj" \
	"$(OBJDIR)/struct_intern.obj" \
	"$(OBJDIR)/storage_xsb.obj" \
	"$(OBJDIR)/sub_delete.obj" \
	"$(OBJDIR)/subp.obj" \
	"$(OBJDIR)/system_xsb.obj" \
	"$(OBJDIR)/table_stats.obj" \
	"$(OBJDIR)/tables.obj" \
	"$(OBJDIR)/thread_xsb.obj" \
	"$(OBJDIR)/timer_xsb.obj" \
	"$(OBJDIR)/token_xsb.obj" \
	"$(OBJDIR)/trace_xsb.obj" \
	"$(OBJDIR)/trie_lookup.obj" \
	"$(OBJDIR)/trie_search.obj" \
	"$(OBJDIR)/tries.obj" \
	"$(OBJDIR)/tr_utils.obj" \
	"$(OBJDIR)/tst_insert.obj" \
	"$(OBJDIR)/tst_retrv.obj" \
	"$(OBJDIR)/tst_unify.obj" \
	"$(OBJDIR)/tst_utils.obj" \
	"$(OBJDIR)/socket_xsb.obj" \
	"$(OBJDIR)/string_xsb.obj" \
	"$(OBJDIR)/varstring.obj" \
	"$(OBJDIR)/ubi_BinTree.obj" \
	"$(OBJDIR)/ubi_SplayTree.obj"\
	"$(OBJDIR)/getMemorySize.obj"

# DLLs don't use main_xsb.c
!IF  "$(DLL)" == "no"
LINK_OBJS=$(DLL_LINK_OBJS) $(OBJDIR)/main_xsb.obj
!ELSE
LINK_OBJS=$(OBJDIR)/main_xsb.obj
!ENDIF

# Oracle requires one additional file
!IF "$(ORACLE)" == "yes"
!IF "$(DLL)" == "yes"
DLL_LINK_OBJS=$(DLL_LINK_OBJS) $(OBJDIR)/orastuff.obj
!ELSE
LINK_OBJS=$(LINK_OBJS) $(OBJDIR)/orastuff.obj
!ENDIF
!ENDIF

# InterProlog's native engine requires one additional file
!IF "$(XSB_INTERPROLOG)" != ""
!IF "$(DLL)" == "yes"
DLL_LINK_OBJS=$(DLL_LINK_OBJS) $(OBJDIR)/interprolog_callback.obj
!ELSE
LINK_OBJS=$(LINK_OBJS) $(OBJDIR)/interprolog_callback.obj
!ENDIF
!ENDIF

!IF  "$(DEBUG)" != "yes"  &&  "$(DLL)" == "no"
!MESSAGE Building XSB without DLL in Release mode $(ORACLE_MSG) $(INTERPROLOG_MSG)

DEBUG_CPP_FLAGS=/MT /O2 /D "NDEBUG"
LINK_DEBUG_FLAGS=/incremental:no /debug

!ELSEIF  "$(DEBUG)" == "yes"  &&  "$(DLL)" == "no"
!MESSAGE Building XSB executable in Debug mode $(ORACLE_MSG) $(INTERPROLOG_MSG)

DEBUG_CPP_FLAGS=/MTd /Gm /Zi /Od /D "_DEBUG" /D "DEBUG" /Fd"$(INTDIR)/"
LINK_DEBUG_FLAGS=/debug

!ELSEIF "$(DEBUG)" != "yes" && "$(DLL)" == "yes"
!MESSAGE Building XSB with DLL in Release mode $(ORACLE_MSG) $(INTERPROLOG_MSG)

DEBUG_CPP_FLAGS=/MT /O2 /D "NDEBUG"
DLL_CPP_FLAGS=/D "$(CALL_CONV)"
LINK_DEBUG_FLAGS=$(DLLDIR)"\xsb.lib" /incremental:no
DLL_LINK_DEBUG_FLAGS=/incremental:no

!ELSEIF "$(DEBUG)" == "yes" &&  "$(DLL)" == "yes"
!MESSAGE Building XSB with DLL in Debug mode $(ORACLE_MSG) $(INTERPROLOG_MSG)

DEBUG_CPP_FLAGS=/MTd /Gm /Zi /Od /D "_DEBUG" /D "DEBUG" /Fd"$(INTDIR)/"
DLL_CPP_FLAGS=/D "$(CALL_CONV)"
LINK_DEBUG_FLAGS=/debug $(DLLDIR)"\xsb.lib" 
DLL_LINK_DEBUG_FLAGS=/debug

!ENDIF

CPP_PROJ=\
	/nologo /W3 /EHsc /D "WIN32" /D "_CONSOLE" /c /Fp"$(OBJDIR)/xsb.pch" \
	/D "_CRT_SECURE_NO_WARNINGS" /Fo"$(OBJDIR)/" \
	$(CONFIG_INCLUDE_FLAG) $(ORACLE_FLAG) \
	$(INTERPROLOG_FLAG) $(DEBUG_CPP_FLAGS) \
	$(DLL_CPP_FLAGS)

LINK_FLAGS=\
	/nologo \
	/STACK:8388608 \
	/subsystem:console \
	/machine:I386 \
	$(LINK_DEBUG_FLAGS) \
	$(SOCKET_LIBRARY) \
	$(SITE_LIBS) \
	kernel32.lib \
	user32.lib \
	gdi32.lib \
	winspool.lib \
	comdlg32.lib \
	advapi32.lib \
	shell32.lib \
	ole32.lib \
	oleaut32.lib \
	uuid.lib \
	odbc32.lib \
	odbccp32.lib \
	/pdb:"$(OUTDIR)/xsb.pdb" \
	/out:"$(OUTDIR)/xsb.exe"

DLL_CPP_PROJ=$(CPP_PROJ) /D "_WINDOWS"
DLL_LINK_FLAGS=\
	/nologo \
	/STACK:8388608 \
	/subsystem:windows \
	/dll \
	/machine:I386 \
	$(SOCKET_LIBRARY) \
	$(SITE_LIBS) \
	kernel32.lib \
	user32.lib \
	gdi32.lib \
	winspool.lib \
	comdlg32.lib \
	advapi32.lib \
	shell32.lib \
	ole32.lib \
	oleaut32.lib \
	uuid.lib \
	odbc32.lib \
	odbccp32.lib \
	$(DLL_LINK_DEBUG_FLAGS) \
	/implib:"$(DLLDIR)/xsb.lib" \
	/pdb:"$(DLLDIR)/xsbdll.pdb" \
	/out:"$(DLLDIR)/xsb.dll"

"$(OUTDIR)\xsb.exe" : "$(OBJDIR)" $(DEF_FILE) $(LINK_OBJS) $(LINK_OTHEROBJS)
	@echo Linking to create xsb.exe: $(LINK_OBJS) $(LINK_OTHEROBJS)
	-@$(EXECUTABLE) @<<
  $(LINK_FLAGS) $(LINK_OBJS) $(LINK_OTHEROBJS)
<<

"$(DLLDIR)\xsb.dll": "$(DLLDIR)" $(DEF_FILE) $(DLL_LINK_OBJS) $(LINK_OTHEROBJS)
	@echo Linking to create xsb.dll: $(DLL_LINK_OBJS) $(LINK_OTHEROBJS)
	-@$(EXECUTABLE) @<<
  $(DLL_LINK_FLAGS) $(DLL_LINK_OBJS) $(LINK_OTHEROBJS)
<<

.c{$(CPP_OBJS)}.obj:
   -@$(CPP) $(CPP_PROJ) $<

.cpp{$(CPP_OBJS)}.obj:
   -@$(CPP) $(CPP_PROJ) $<

.cxx{$(CPP_OBJS)}.obj:
   -@$(CPP) $(CPP_PROJ) $<

.c{$(CPP_SBRS)}.sbr:
   -@$(CPP) $(CPP_PROJ) $<

.cpp{$(CPP_SBRS)}.sbr:
   -@$(CPP) $(CPP_PROJ) $<

.cxx{$(CPP_SBRS)}.sbr:
   -@$(CPP) $(CPP_PROJ) $<

CLEAN :
	-@if exist "$(OBJDIR)\*.obj" erase "$(OBJDIR)\*.obj"
	-@if exist "$(LIBDIR)\*.xwam" erase "$(LIBDIR)\*.xwam"
	-@if exist "$(OUTDIR)\*.exe" erase "$(OUTDIR)\*.exe"
	-@if exist "$(OUTDIR)\*.lib" erase "$(OUTDIR)\*.lib"
	-@if exist "$(OUTDIR)\*.dll" erase "$(OUTDIR)\*.dll"
	-@if exist "$(OUTDIR)\*.exp" erase "$(OUTDIR)\*.exp"


# Local Variables:
# coding-system-for-write: iso-2022-7bit-dos
# End:
