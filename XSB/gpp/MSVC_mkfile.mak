# Microsoft Developer Studio Generated NMAKE File, Based on gpp.dsp


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

OUTDIR=..\config\x86-pc-windows\bin
INTDIR=..\config\x86-pc-windows\saved.o

ALL : "$(OUTDIR)\gpp.exe"


CLEAN :
	-@if exist "$(INTDIR)\gpp.obj" erase "$(INTDIR)\gpp.obj"
	-@if exist "$(INTDIR)\vc60.idb" erase "$(INTDIR)\vc60.idb"
	-@if exist "$(OUTDIR)\gpp.exe" erase "$(OUTDIR)\gpp.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /W3 /EHsc /O2 /D "WIN32" /D "WIN_NT" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gpp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\gpp.pdb" /machine:I386 /out:"$(OUTDIR)\gpp.exe" 
LINK32_OBJS= \
	"$(INTDIR)\gpp.obj"

"$(OUTDIR)\gpp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


.c{$(INTDIR)}.obj:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


SOURCE=.\gpp.c

"$(INTDIR)\gpp.obj" : $(SOURCE) "$(INTDIR)"

