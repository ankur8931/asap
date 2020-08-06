#!/bin/bash
make clean
make 
jar cfm ../InstallXSB.jar manifest.mf *.class system.properties link.properties
make clean
chmod 700 ../InstallXSB.jar

echo ""
echo "*** To install XSB using this installer, please do the following:"
echo ""
echo "cd .."
echo "java -jar InstallXSB.jar"
echo ""
echo "*** or execute InstallXSB.jar by clicking on its icon in the file manager."
