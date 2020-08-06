# Microsoft Developer Studio Generated NMAKE File, Based on second_foreign.dsp
!IF "$(CFG)" == ""
CFG=second_foreign - Win32 Debug
!MESSAGE No configuration specified. Defaulting to second_foreign - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "second_foreign - Win32 Release" && "$(CFG)" != "second_foreign - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "second_foreign.mak" CFG="second_foreign - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "second_foreign - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "second_foreign - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "second_foreign - Win32 Release"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\second_foreign.dll"


CLEAN :
	-@erase "$(INTDIR)\second_foreign.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xsb_wrap_second_foreign.obj"
	-@erase "$(OUTDIR)\second_foreign.dll"
	-@erase "$(OUTDIR)\second_foreign.exp"
	-@erase "$(OUTDIR)\second_foreign.lib"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "..\..\emu" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECOND_FOREIGN_EXPORTS" /D "XSB_DLL" /Fp"$(INTDIR)\second_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\second_foreign.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xsb.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\second_foreign.pdb" /machine:I386 /out:"$(OUTDIR)\second_foreign.dll" /implib:"$(OUTDIR)\second_foreign.lib" /libpath:"..\..\config\x86-pc-windows\bin" 
LINK32_OBJS= \
	"$(INTDIR)\second_foreign.obj" \
	"$(INTDIR)\xsb_wrap_second_foreign.obj"

"$(OUTDIR)\second_foreign.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "second_foreign - Win32 Debug"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\second_foreign.dll"


CLEAN :
	-@erase "$(INTDIR)\second_foreign.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xsb_wrap_second_foreign.obj"
	-@erase "$(OUTDIR)\second_foreign.dll"
	-@erase "$(OUTDIR)\second_foreign.exp"
	-@erase "$(OUTDIR)\second_foreign.ilk"
	-@erase "$(OUTDIR)\second_foreign.lib"
	-@erase "$(OUTDIR)\second_foreign.pdb"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /EHsc /Zi /Od /I "..\..\config\x86-pc-windows" /I "..\..\emu" /I "..\..\prolog_includes"  /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECOND_FOREIGN_EXPORTS" /D "XSB_DLL" /Fp"$(INTDIR)\second_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\second_foreign.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xsb.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\second_foreign.pdb" /debug /machine:I386 /out:"$(OUTDIR)\second_foreign.dll" /implib:"$(OUTDIR)\second_foreign.lib" /libpath:"..\..\config\x86-pc-windows\bin" 
LINK32_OBJS= \
	"$(INTDIR)\second_foreign.obj" \
	"$(INTDIR)\xsb_wrap_second_foreign.obj"

"$(OUTDIR)\second_foreign.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("second_foreign.dep")
!INCLUDE "second_foreign.dep"
!ELSE 
!MESSAGE Warning: cannot find "second_foreign.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "second_foreign - Win32 Release" || "$(CFG)" == "second_foreign - Win32 Debug"
SOURCE=second_foreign.c

!IF  "$(CFG)" == "second_foreign - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /EHsc /O2 /I "..\..\emu" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECOND_FOREIGN_EXPORTS" /D "XSB_DLL" /Fp"$(INTDIR)\second_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\second_foreign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "second_foreign - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /EHsc /Zi /Od /I "..\..\emu" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECOND_FOREIGN_EXPORTS" /D "XSB_DLL" /Fp"$(INTDIR)\second_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\second_foreign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=xsb_wrap_second_foreign.c

!IF  "$(CFG)" == "second_foreign - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /EHsc /O2 /I "..\..\emu" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECOND_FOREIGN_EXPORTS" /D "XSB_DLL" /Fp"$(INTDIR)\second_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\xsb_wrap_second_foreign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "second_foreign - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /EHsc /Zi /Od /I "..\..\emu" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECOND_FOREIGN_EXPORTS" /D "XSB_DLL" /Fp"$(INTDIR)\second_foreign.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\xsb_wrap_second_foreign.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

