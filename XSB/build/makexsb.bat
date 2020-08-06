@echo off
REM   makexsb.bat
REM   Script for compiling XSB under Windows using VC++

set XSBCONFIGdir=..\config\x86-pc-windows

IF NOT EXIST %XSBCONFIGdir%\saved.o MKDIR %XSBCONFIGdir%\saved.o
IF NOT EXIST %XSBCONFIGdir%\bin mkdir %XSBCONFIGdir%\bin
IF NOT EXIST %XSBCONFIGdir%\lib mkdir %XSBCONFIGdir%\lib

IF NOT EXIST ..\emu\private_builtin.c  copy private_builtin.in ..\emu\private_builtin.c

@copy odbc\* %XSBCONFIGdir%
@copy windows\banner.msg %XSBCONFIGdir%
@copy windows\xsb_configuration.P %XSBCONFIGdir%\lib
@copy windows\xsb_config.h %XSBCONFIGdir%
@copy windows\xsb_config_aux.h %XSBCONFIGdir%
@copy windows\xsb_debug.h %XSBCONFIGdir%

@cd ..\emu

REM Concatenate MSVC_mkfile.mak & MSVC.dep into emu\MSVC_mkfile.mak
@copy ..\build\windows\MSVC_mkfile.mak+..\build\MSVC.dep MSVC_mkfile.mak


@nmake /nologo /f "MSVC_mkfile.mak" %1 %2 %3 %4 %5 %6 %7

@if exist MSVC_mkfile.mak del MSVC_mkfile.mak

@cd ..\gpp
@nmake /nologo /s /f "MSVC_mkfile.mak" %1 %2 %3 %4 %5 %6 %7

@cd ..\packages

@cd .\dbdrivers
@nmake /nologo /s /f NMakefile.mak %1 %2 %3 %4 %5 %6 %7
@cd ..

REM Must build curl before sgml and xpath
@cd .\curl
@nmake /nologo /f NMakefile.mak %1 %2 %3 %4 %5 %6 %7
@cd ..

@cd .\sgml\cc
@nmake /nologo /f NMakefile.mak %1 %2 %3 %4 %5 %6 %7
@cd ..\..

@cd .\xpath\cc
@nmake /nologo /f NMakefile.mak %1 %2 %3 %4 %5 %6 %7
@cd ..\..

@cd .\pcre
@nmake /nologo /f NMakefile.mak %1 %2 %3 %4 %5 %6 %7
@cd ..

@cd ..\build

