# Make file for driver_manager.dll

XSBDIR=..\..\..
MYPROGRAM=driver_manager

CPP=cl.exe
LINKER=link.exe

OUTDIR     = windows
ARCHDIR    =$(XSBDIR)\config\x86-pc-windows
ARCHBINDIR =$(ARCHDIR)\bin
ARCHOBJDIR =$(ARCHDIR)\saved.o
INTDIR=.

ALL : "$(OUTDIR)\$(MYPROGRAM).dll"

CLEAN :
	-@if exist "$(INTDIR)\*.obj" erase "$(INTDIR)\*.obj"
	-@if exist "$(INTDIR)\*.dll" erase "$(INTDIR)\*.dll"
	-@if exist "$(INTDIR)\*.exp" erase "$(INTDIR)\*.exp"


CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "$(ARCHDIR)" \
		 /I "$(XSBDIR)\emu" /I "$(XSBDIR)\prolog_includes" \
		 /D "WIN32" /D "WIN_NT" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" \
		 /Fo"$(ARCHOBJDIR)\\" /Fd"$(ARCHOBJDIR)\\" /c 
	
SOURCE=$(MYPROGRAM).c
"$(ARCHOBJDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

LINK_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
		odbc32.lib odbccp32.lib xsb.lib \
		/nologo /dll \
		/machine:I386 /out:"$(OUTDIR)\$(MYPROGRAM).dll" \
		/libpath:"$(ARCHBINDIR)"
LINK_OBJS=  "$(ARCHOBJDIR)\$(MYPROGRAM).obj"

"$(OUTDIR)\$(MYPROGRAM).dll" : "$(ARCHBINDIR)" $(LINK_OBJS)
    $(LINKER) @<<
  $(LINK_FLAGS) $(LINK_OBJS)
<<
