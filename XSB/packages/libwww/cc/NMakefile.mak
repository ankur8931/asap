# Make file for libwww_request.dll
# One might need to play with it somewhat to get it quite right

XSBDIR=..\..\..
MYPROGRAM=libwww_request
LIBWWW_LIBS = wwwapp.lib wwwcache.lib wwwcore.lib wwwdir.lib wwwdll.lib wwwfile.lib wwwftp.lib wwwgophe.lib wwwhtml.lib wwwhttp.lib wwwinit.lib wwwmime.lib wwwmux.lib wwwnews.lib wwwstream.lib wwwtelnt.lib wwwtrans.lib wwwutils.lib wwwwais.lib wwwxml.lib wwwzip.lib pics.lib

# Replace with the right directory where the libwww's *.lib files are found
LIBWWW_LIBS_DIR = ..\..\..\..\libwww\bin

CPP=cl.exe
OUTDIR=.
INTDIR=.

ALL : "$(OUTDIR)\$(MYPROGRAM).dll"

CLEAN :
	-@erase "$(INTDIR)\$(MYPROGRAM).obj"
	-@erase "$(INTDIR)\$(MYPROGRAM).dll"
	-@erase "$(INTDIR)\$(MYPROGRAM).exp"


CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "$(XSBDIR)\config\x86-pc-windows" \
		 /I "$(XSBDIR)\emu" /I "$(XSBDIR)\prolog_includes" \
		 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" \
		 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 
	

SOURCE=$(MYPROGRAM).c
"$(INTDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=libwww_parse_html.c
"$(INTDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=libwww_parse_xml.c
"$(INTDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=libwww_parse_rdf.c
"$(INTDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
		odbc32.lib odbccp32.lib xsb.lib \
		$(LIBWWW_LIBS) \
		/nologo /dll \
		/machine:I386 /out:"$(OUTDIR)\$(MYPROGRAM).dll" \
		/libpath:"$(XSBDIR)\config\x86-pc-windows\bin" \
		/libpath:"$(LIBWWW_LIBS_DIR)
LINK32_OBJS=  "$(INTDIR)\$(MYPROGRAM).obj"

"$(OUTDIR)\$(MYPROGRAM).dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
