# Microsoft Developer Studio Generated NMAKE File, Based on cmain.dsp
!IF "$(CFG)" == ""
CFG=cmain - Win32 Debug
!MESSAGE No configuration specified. Defaulting to cmain - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "cmain - Win32 Release" && "$(CFG)" != "cmain - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cmain.mak" CFG="cmain - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cmain - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "cmain - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cmain - Win32 Release"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\cmain.exe"


CLEAN :
	-@erase "$(INTDIR)\cmain.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cmain.exe"

CPP_PROJ=/nologo /ML /W3 /EHsc /O2 /I "..\..\emu" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XSB_DLL" /Fp"$(INTDIR)\cmain.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cmain.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xsb.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cmain.pdb" /machine:I386 /out:"$(OUTDIR)\cmain.exe" /libpath:"..\..\config\x86-pc-windows\bin" 
LINK32_OBJS= \
	"$(INTDIR)\cmain.obj"

"$(OUTDIR)\cmain.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cmain - Win32 Debug"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\cmain.exe"


CLEAN :
	-@erase "$(INTDIR)\cmain.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cmain.exe"
	-@erase "$(OUTDIR)\cmain.ilk"
	-@erase "$(OUTDIR)\cmain.pdb"

CPP_PROJ=/nologo /MLd /W3 /Gm /EHsc /ZI /Od /I "..\..\emu" /I "..\..\config\x64-pc-windows" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "XSB_DLL" /Fp"$(INTDIR)\cmain.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cmain.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xsb.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\cmain.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cmain.exe" /pdbtype:sept /libpath:"..\..\config\x64-pc-windows\bin" 
LINK32_OBJS= \
	"$(INTDIR)\cmain.obj"

"$(OUTDIR)\cmain.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("cmain.dep")
!INCLUDE "cmain.dep"
!ELSE 
!MESSAGE Warning: cannot find "cmain.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cmain - Win32 Release" || "$(CFG)" == "cmain - Win32 Debug"
SOURCE=cmain.c

"$(INTDIR)\cmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

