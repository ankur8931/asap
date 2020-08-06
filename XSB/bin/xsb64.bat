@echo off
REM Script for running XSB when built natively on Windows x64

set BINDIR=%0\..\..

%BINDIR%\config\x64-pc-windows\bin\xsb %1 %2 %3 %4 %5 %6 %7
