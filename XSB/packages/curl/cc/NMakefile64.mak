## File:      packages/curl/NMakefile64.mak
## Author(s): Aneesh Ali
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 2010 - 2017
## 
## XSB is free software; you can redistribute it and/or modify it under the
## terms of the GNU Library General Public License as published by the Free
## Software Foundation; either version 2 of the License, or (at your option)
## any later version.
## 
## XSB is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
## more details.
## 
## You should have received a copy of the GNU Library General Public License
## along with XSB; if not, write to the Free Software Foundation,
## Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##

# Make file for curl2pl.dll

#DEBUG_FLAG=/D "DEBUG"

XSBDIR=..\..\..
MYPROGRAM=curl2pl

CPP=cl.exe
LINKER=link.exe

OUTDIR     = bin64
ARCHDIR    =$(XSBDIR)\config\x64-pc-windows
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
	/I "$(XSBDIR)\packages\curl\cc"\
	/D "WIN64" /D "WIN_NT" $(DEBUG_FLAG) /D "_WINDOWS" /D "_MBCS" \
	/Fo"$(ARCHOBJDIR)\\" /Fd"$(ARCHOBJDIR)\\" /c 
	
SOURCE=load_page.c
"$(ARCHOBJDIR)\load_page.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=error.c curl2pl.c
"$(ARCHOBJDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

LINK_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
		odbc32.lib odbccp32.lib xsb.lib wsock32.lib libcurl.lib \
		/nologo /dll \
		/machine:x64 /out:"$(OUTDIR)\$(MYPROGRAM).dll" \
		/libpath:"$(ARCHBINDIR)"	\
		/libpath:.\bin64

LINK_OBJS=  "$(ARCHOBJDIR)\load_page.obj" "$(ARCHOBJDIR)\$(MYPROGRAM).obj"

"$(OUTDIR)\$(MYPROGRAM).dll" : "$(ARCHBINDIR)" $(LINK_OBJS)
    $(LINKER) @<<
  $(LINK_FLAGS) $(LINK_OBJS)
<<
