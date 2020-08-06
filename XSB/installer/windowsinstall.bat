
REM Arg 1 is the path where InstallXSB.jar is located
cd %1%
@ shift

@ set vspath=%1%
@ set vsdisk=%2%
@ set xsbdisk=%3%
@ set currpath=%4%
@ set makexsb=%5%
@ set setvcvar=%6%
@ %vsdisk%
@ cd %vspath%\VC\bin
@ call %setvcvar%
@ %xsbdisk%
cd %currpath%
@ cd build
@ call %makexsb% clean
@ call %makexsb%
cd ..
echo "======= done ======="
