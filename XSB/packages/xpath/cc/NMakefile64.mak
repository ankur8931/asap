 # Make file for xpathparserdll

XSBDIR=..\..\..
MYPROGRAM=xpathparser
CURLDIR=..\..\curl\cc

CPP=cl.exe
OUTDIR=$(XSBDIR)\config\x64-pc-windows
OUTBINDIR=$(OUTDIR)\bin
OUTOBJDIR=$(OUTDIR)\saved.o
INTDIR=.

ALL : "$(OUTBINDIR)\$(MYPROGRAM).dll"

CLEAN :
	-@if exist "$(INTDIR)\*.obj" erase "$(INTDIR)\*.obj"
	-@if exist "$(INTDIR)\*.dll" erase "$(INTDIR)\*.dll"
	-@if exist "$(INTDIR)\*.exp" erase "$(INTDIR)\*.exp"


CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "$(OUTDIR)" \
	/I "$(XSBDIR)\emu" /I "$(XSBDIR)\prolog_includes" \
	/I "$(XSBDIR)\packages\curl\cc" \
	/I "$(XSBDIR)\packages\xpath\cc"\
	/I ".\include" \
	 /D "WIN_NT" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" \
	 /Fo"$(OUTOBJDIR)\\" /Fd"$(OUTOBJDIR)\\" /c 
	
SOURCE="$(CURLDIR)\load_page.c"
"$(OUTOBJDIR)\load_page.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=fetch_file.c xpathparser.c 
"$(OUTOBJDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
	advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
	odbc32.lib odbccp32.lib xsb.lib wsock32.lib libxml2.lib \
	libcurl.lib curl2pl.lib\
	/nologo /dll \
	/machine:x64 /out:"$(OUTBINDIR)\$(MYPROGRAM).dll" \
	/libpath:"$(OUTBINDIR)"	\
	/libpath:.\bin64 \
	/libpath:"$(CURLDIR)\bin64"
LINK32_OBJS=  "$(OUTOBJDIR)\load_page.obj" "$(OUTOBJDIR)\$(MYPROGRAM).obj"

copy_misc :
	  del ..\xpath_init.P
	  copy ..\Misc\xpath_init-wind.P ..\xpath_init.P

"$(OUTBINDIR)\$(MYPROGRAM).dll" : "$(OUTBINDIR)" $(LINK32_OBJS) copy_misc
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
