# Make file for xsb_re_match.dll

XSBDIR=..\..\..
MYPROGRAM=xsb_re_match
# This probably needs to be linked against a POSIX match library something.lib
REGMATCH_LIB= 
# The directory where the something.lib library is found
REGMATCH_LIB_DIR=

CPP=cl.exe
OUTDIR=.
INTDIR=.

ALL : "$(OUTDIR)\$(MYPROGRAM).dll"

CLEAN :
	-@erase "$(INTDIR)\$(MYPROGRAM).obj"
	-@erase "$(INTDIR)\$(MYPROGRAM).dll"
	-@erase "$(INTDIR)\$(MYPROGRAM).exp"


CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "$(XSBDIR)\config\x64-pc-windows" \
		 /I "$(XSBDIR)\emu" /I "$(XSBDIR)\prolog_includes" \
		 /D "WIN64" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" \
		 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 
	

SOURCE=$(MYPROGRAM).c

"$(INTDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
		odbc32.lib odbccp32.lib xsb.lib \
		$(REGMATCH_LIB) \
		/nologo /dll \
		/machine:x64 /out:"$(OUTDIR)\$(MYPROGRAM).dll" \
		/libpath:"$(XSBDIR)\config\x64-pc-windows\bin" \
		/libpath:"$(REGMATCH_LIB_DIR)
LINK32_OBJS=  "$(INTDIR)\$(MYPROGRAM).obj"

"$(OUTDIR)\$(MYPROGRAM).dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
