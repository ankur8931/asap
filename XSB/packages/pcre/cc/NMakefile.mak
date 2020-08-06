## File:      packages/pcre/cc/NMakefile.mak
## Author(s): Mandar Pathak
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

# Make file for pcre4pl.dll

XSBDIR=..\..\..
MYPROGRAM=pcre4pl

CPP=cl.exe
LINKER=link.exe

OUTDIR     = bin
ARCHDIR    =$(XSBDIR)\config\x86-pc-windows
ARCHBINDIR =$(ARCHDIR)\bin
ARCHOBJDIR =$(ARCHDIR)\saved.o
INTDIR=.

ALL : "$(OUTDIR)\$(MYPROGRAM).dll"
	nmake /f NMakefile.mak clean

CLEAN :
	-@if exist "$(INTDIR)\$(MYPROGRAM).obj" erase "$(INTDIR)\$(MYPROGRAM).obj"
	-@if exist "$(INTDIR)\$(MYPROGRAM).dll" erase "$(INTDIR)\$(MYPROGRAM).dll"
	-@if exist "$(INTDIR)\$(MYPROGRAM).exp" erase "$(INTDIR)\$(MYPROGRAM).exp"


CPP_PROJ=/nologo /MT /W3 /EHsc /O2 /I "$(ARCHDIR)" \
		 /I "$(XSBDIR)\emu" /I "$(XSBDIR)\prolog_includes" \
		 /I "$(XSBDIR)\packages\pcre\cc\pcre" \
		 /D "WIN_NT" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" \
		 /Fo"$(ARCHOBJDIR)\\" /Fd"$(ARCHOBJDIR)\\" /c 
	
SOURCE=$(MYPROGRAM).c
"$(ARCHOBJDIR)\$(MYPROGRAM).obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

LINK_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
		advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
		odbc32.lib odbccp32.lib xsb.lib pcre.lib \
		/nologo /dll \
		/machine:I386 /out:"$(OUTDIR)\$(MYPROGRAM).dll" \
		/libpath:"$(ARCHBINDIR)" \
		/libpath:"$(XSBDIR)\prolog_includes" \
		/libpath:.\bin

LINK_OBJS=  "$(ARCHOBJDIR)\$(MYPROGRAM).obj"

"$(OUTDIR)\$(MYPROGRAM).dll" : "$(ARCHBINDIR)" $(LINK_OBJS)
    $(LINKER) $(LINK_FLAGS) $(LINK_OBJS)
