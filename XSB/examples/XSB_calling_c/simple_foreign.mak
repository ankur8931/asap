# Microsoft Developer Studio Generated NMAKE File, Based on simple_foreign.dsp
!IF "$(CFG)" == ""
CFG=simple_foreign - Win32 Debug
!MESSAGE No configuration specified. Defaulting to simple_foreign - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "simple_foreign - Win32 Release" && "$(CFG)" != "simple_foreign - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simple_foreign.mak" CFG="simple_foreign - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simple_foreign - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simple_foreign - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "simple_foreign - Win32 Release"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\simple_foreign.dll"


CLEAN :
	-@erase "$(INTDIR)\simple_foreign.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\simple_foreign.dll"
	-@erase "$(OUTDIR)\simple_foreign.exp"
	-@erase "$(OUTDIR)\simple_foreign.lib"

CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "..\..\config\x86-pc-windows"  /I "..\..\emu" /I "..\..\prolog_includes"  /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIMPLE_FOREIGN_EXPORTS" /Fp"$(INTDIR)\simple_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple_foreign.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xsb.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\simple_foreign.pdb" /machine:I386 /out:"$(OUTDIR)\simple_foreign.dll" /implib:"$(OUTDIR)\simple_foreign.lib" /libpath:"..\..\config\x86-pc-windows\bin" 
LINK32_OBJS= \
	"$(INTDIR)\simple_foreign.obj"

"$(OUTDIR)\simple_foreign.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simple_foreign - Win32 Debug"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\simple_foreign.dll"


CLEAN :
	-@erase "$(INTDIR)\simple_foreign.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\simple_foreign.dll"
	-@erase "$(OUTDIR)\simple_foreign.exp"
	-@erase "$(OUTDIR)\simple_foreign.ilk"
	-@erase "$(OUTDIR)\simple_foreign.lib"
	-@erase "$(OUTDIR)\simple_foreign.pdb"

CPP_PROJ=/nologo /MTd /W3 /Gm /EHsc /Zi /Od  /I "..\..\config\x86-pc-windows"  /I "..\..\emu" /I "..\..\prolog_includes" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SIMPLE_FOREIGN_EXPORTS" /Fp"$(INTDIR)\simple_foreign.pch"  /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /RTC1  /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple_foreign.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xsb.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\simple_foreign.pdb" /debug /machine:I386 /out:"$(OUTDIR)\simple_foreign.dll" /implib:"$(OUTDIR)\simple_foreign.lib" /libpath:"..\..\config\x86-pc-windows\bin" 
LINK32_OBJS= \
	"$(INTDIR)\simple_foreign.obj"

"$(OUTDIR)\simple_foreign.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("simple_foreign.dep")
!INCLUDE "simple_foreign.dep"
!ELSE 
!MESSAGE Warning: cannot find "simple_foreign.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "simple_foreign - Win32 Release" || "$(CFG)" == "simple_foreign - Win32 Debug"
SOURCE=simple_foreign.c

"$(INTDIR)\simple_foreign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

