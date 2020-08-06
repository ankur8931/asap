@echo off
REM   makexsb64.bat
REM   Script for compiling XSB under Windows64 using VC++

set XSBCONFIGdir=..\config\x64-pc-windows

IF NOT EXIST %XSBCONFIGdir%\saved.o MKDIR %XSBCONFIGdir%\saved.o
IF NOT EXIST %XSBCONFIGdir%\bin mkdir %XSBCONFIGdir%\bin
IF NOT EXIST %XSBCONFIGdir%\lib mkdir %XSBCONFIGdir%\lib

IF NOT EXIST ..\emu\private_builtin.c  copy private_builtin.in ..\emu\private_builtin.c

@copy odbc\* %XSBCONFIGdir%
@copy windows64\banner.msg %XSBCONFIGdir%
@copy windows64\xsb_configuration.P %XSBCONFIGdir%\lib
@copy windows64\xsb_config.h %XSBCONFIGdir%
@copy windows64\xsb_config_aux.h %XSBCONFIGdir%
@copy windows64\xsb_debug.h %XSBCONFIGdir%

@cd ..\emu

REM Concatenate MSVC_mkfile.mak & MSVC.dep into emu\MSVC_mkfile.mak
@copy ..\build\windows64\MSVC_mkfile.mak+..\build\MSVC.dep MSVC_mkfile.mak


@nmake /nologo /f "MSVC_mkfile.mak" %1 %2 %3 %4 %5 %6 %7

@if exist MSVC_mkfile.mak del MSVC_mkfile.mak

@cd ..\gpp
@nmake /nologo /s /f "MSVC_mkfile64.mak" %1 %2 %3 %4 %5 %6 %7

@cd ..\packages

@cd dbdrivers
@nmake /nologo /s /f NMakefile64.mak %1 %2 %3 %4 %5 %6 %7
@cd ..

REM Must build curl before sgml and xpath
@cd curl
@nmake /nologo /f NMakefile64.mak %1 %2 %3 %4 %5 %6 %7
@cd ..

@cd sgml\cc
@nmake /nologo /f NMakefile64.mak %1 %2 %3 %4 %5 %6 %7
@cd ..\..

REM We don't have win64 binaries for libxml2.dll and iconv.dll
REM @cd xpath\cc
REM @nmake /nologo /f NMakefile64.mak %1 %2 %3 %4 %5 %6 %7
REM @cd ..\..

@cd pcre
@nmake /nologo /f NMakefile64.mak %1 %2 %3 %4 %5 %6 %7
@cd ..

@cd ..\build

