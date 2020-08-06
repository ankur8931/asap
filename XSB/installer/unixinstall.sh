#!/bin/sh
############################################################################
#			XSB Installation
#File name: 		unixinstall.sh
#Author(s): 			Dongli Zhang
#Brief description: 	This shell is used to run some command.
		
#sh unixinstall.sh ubuntu configure1  [option0-jdk-location]
#sh unixinstall.sh ubuntu configure2  [option0-jdk-location]
#sh unixinstall.sh ubuntu checkJava
#sh unixinstall.sh ubuntu installFeatures your-password feature1 feature2....
#sh unixinstall.sh ubuntu checkhome
#sh unixinstall.sh ubuntu checkhomearg your-jdk-path

############################################################################


#global variables for all supported Unix systems
export osUbuntu="ubuntu"
export osDebian="debian"
export osFedora="fedora"
export osSuse="suse"
export osMandriva="mandriva"

#global variable for all packages
export dbName
export curlName
export xmlName
export pcreName
export makedependName

# $1 is the current directory of the installer
cd $1
shift

#store package's name in its corresponding global variable
getDBName()
{
    case "$osType" in
	"$osUbuntu")
		dbName="unixodbc-dev"
		export dbName ;;
	"$osDebian")
		dbName="unixodbc-dev"
		export dbName ;;
	"$osFedora")
		dbName="unixODBC-devel"
		export dbName ;;
	"$osSuse")
		dbName="unixODBC-devel"
		export dbName ;;
	"$osMandriva")
		dbName="libunixODBC-devel" 
		export dbName ;;
    esac
}

#store package's name in its corresponding global variable
getCurlName()
{
    case "$osType" in
	"$osUbuntu")
	    result=`echo $password | sudo -S apt-cache search ^libcurl[0-9]*-openssl-dev`	
	    curlName=`echo $result | awk '{ print $1; }'`
	    export culrName ;;
	"$osDebian")
	    result=`echo $password | sudo -S apt-cache search ^libcurl[0-9]*-openssl-dev`
	    curlName=`echo $result | awk '{ print $1; }'`
	    export curlName ;;
	"$osFedora")
	    curlName="libcurl-devel"
	    export curlName ;;
	"$osSuse")
	    curlName="libcurl-devel"
	    export curlName ;;
	"$osMandriva")
	    curlName="libcurl-devel"
	    export curlName ;;
    esac	
}

#store package's name in its corresponding global variable
getXmlName()
{
    case "$osType" in
	"$osUbuntu")
	    result=`echo $password | sudo -S apt-cache search ^libxml[0-9]*-dev`
	    xmlName=`echo $result | awk '{ print $1; }'`
	    export xmlName ;;
	"$osDebian")
	    result=`echo $password | sudo -S apt-cache search ^libxml[0-9]*.-dev`
	    xmlName=`echo $result | awk '{ print $1; }'`
	    export xmlName ;;
	"$osFedora")
	    result=`yum list | grep ^libxml.-devel`
	    xmlName=`echo $result | awk '{ print $1; }'`
	    export xmlName ;;
	"$osSuse")
	    result=`zypper search libxml?-devel | grep libxml`
	    xmlName=`echo $result | awk '{ print $3; }'`
	    export xmlName ;;
	"$osMandriva")
	    xmlName=`urpmq --list | grep libxml.-devel | tail -n1`
	    export xmlName ;;
    esac
}

#store package's name in its corresponding global variable
getPcreName()
{
    case "$osType" in
	"$osUbuntu")
	    result=`echo $password | sudo -S apt-cache search ^libpcre[0-9]*-dev`
	    pcreName=`echo $result | awk '{ print $1; }'`
	    export pcreName ;;
	"$osDebian")
	    result=`echo $password | sudo -S apt-cache search ^libpcre[0-9]*-dev`
	    pcreName=`echo $result | awk '{ print $1; }'`
	    export pcreName ;;
	"$osFedora")
	    pcreName="pcre-devel" 
	    export pcreName ;;
	"$osSuse") 
	    pcreName="pcre-devel" 
	    export pcreName ;;
	"$osMandriva")
	    pcreName="libpcre-devel"
	    export pcreName ;;
    esac
}

getMakedependName()
{
    case "$osType" in
	"$osUbuntu")
		makedependName="xutils-dev"
		export makedependName ;;
	"$osDebian")
		makedependName="xutils-dev"
		export makedependName ;;
	"$osFedora")
		makedependName="imake"
		export makedependName ;;
	"$osSuse")
		makedependName="xorg-x11-util-devel"
		export makedependName ;;
	"$osMandriva")
		makedependName="makedepend"
		export makedependName ;;
    esac	
}

#function to install dbdrivers
installLibDB()
{
    getDBName

    case "$osType" in
	"$osUbuntu")
	    if dpkg -l | grep -q $dbName ; then
		    echo "\n$dbName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $dbName
	    fi ;;
	"$osDebian")
	    if dpkg -l | grep -q $dbName ;  then
		    echo "\n$dbName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $dbName
	    fi ;;
	"$osFedora")
	    if rpm -q $dbName ; then 
		    echo ""
		    echo "$dbName - package already installed."
	    else
		    echo $password | sudo -S yum install -y $dbName
	    fi ;;
	"$osSuse")
	    if rpm -q $dbName | grep "is not installed" ; then
		    echo $password | sudo -S zypper install -y $dbName 
	    else
		    echo "$dbName - package already installed."
	    fi ;;
	"$osMandriva")
	    echo $password | sudo -S urpmi --auto $dbName ;;
    esac
}

#function to install curl
installLibcurl()
{
    getCurlName

    case "$osType" in
	"$osUbuntu")
	    if dpkg -l | grep -q $curlName ; then
		    echo "\n$curlName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $curlName
	    fi ;;
	"$osDebian")
	    if dpkg -l | grep -q $curlName ; then
		    echo "\n$curlName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $curlName
	    fi ;;
	"$osFedora")
	    if rpm -q $curlName ; then 
		    echo ""
		    echo "$curlName - package already installed."
	    else
		    echo $password | sudo -S yum install -y $curlName
	    fi ;;
	"$osSuse")
	    if rpm -q $curlName | grep "is not installed" ; then
		    echo $password | sudo -S zypper install -y $curlName
	    else
		    echo "$curlName - package already installed."
	    fi ;;
	"$osMandriva")
	    echo $password | sudo -S urpmi --auto $curlName ;;
    esac
}

#function to install libxml-dev
installLibxmldev()
{
    getXmlName
    case "$osType" in
	"$osUbuntu")
	    if dpkg -l | grep -q $xmlName ; then 
		    echo "\n$xmlName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $xmlName
	    fi ;;
	"$osDebian")
	    if dpkg -l | grep -q $xmlName ; then
		    echo "\n$xmlName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $xmlName
	    fi ;;
	"$osFedora")
	    if rpm -q $xmlName ; then 
		    echo ""
		    echo "$xmlName - package already installed."
	    else
		    echo $password | sudo -S yum install -y $xmlName
	    fi ;;
	"$osSuse")
	    if rpm -q $xmlName | grep "is not installed" ; then
		    echo $password | sudo -S zypper install -y $xmlName
	    else
		    echo "$xmlName - package already installed."
	    fi ;;
	"$osMandriva")
	    echo $password | sudo -S urpmi --auto $xmlName ;;
    esac
}

#function to install libpcre-dev
installLibpcredev()
{
    getPcreName
    case "$osType" in
	"$osUbuntu")
	    if dpkg -l | grep -q $pcreName ; then 
		    echo "\n$pcreName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $pcreName
	    fi ;;
	"$osDebian")
	    if dpkg -l | grep -q $pcreName ; then
		    echo "\n$pcreName - package already installed."
	    else
		    echo $password | sudo -S apt-get install -y $pcreName
	    fi ;;
	"$osFedora")
	    if rpm -q $pcreName ; then 
		    echo ""
		    echo "$pcreName - package already installed."
	    else
		    echo $password | sudo -S yum  install -y $pcreName
	    fi ;;
	"$osSuse")
	    if rpm -q $pcreName | grep "is not installed" ; then
		    echo $password | sudo -S zypper install -y $pcreName
	    else
		    echo "$pcreName - package already installed."
	    fi ;;
	"$osMandriva")
	    echo $password | sudo -S urpmi --auto $pcreName ;;
    esac
}

#function to install makedepend
installMakedepend()
{
    getMakedependName
    echo "\n installing package makedepend\n"
    case "$osType" in
	"$osUbuntu")
	    echo $password | sudo -S apt-get install -y $makedependName ;;
	"$osDebian")
	    echo $password | sudo -S apt-get install -y $makedependName ;;
	"$osFedora")
	    echo $password | sudo -S yum install -y $makedependName ;;
	"$osSuse")
	    echo $password | sudo -S zypper install -y $makedependName ;;
	"$osMandriva")
	    echo $password | sudo -S urpmi --auto $makedependName ;;
    esac
}

#install required packages for db feature
installDB()
{
    installLibDB
}

#install required packages for http feature
installHTTP()
{
    installLibcurl
}

#install required packages for xml feature
installXML()
{
    installLibxmldev
}

#install required packages for regular expression feature
installReg()
{
    installLibpcredev
}

#function to install features with arg $1, which is the name of feature
installFeature()
{
    featureName=$1

    if [ "$featureName"x = "db"x ] ; then
	    installDB
    fi

    if [ "$featureName"x = "http"x ] ; then
	    installHTTP
    fi

    if [ "$featureName"x = "xml"x ] ; then
	    installXML
    fi

    if [ "$featureName"x = "reg"x ] ; then
	    installReg
    fi
}

#global variable to store the type of os
osType=$1
#global variable to store the command
command=$2

export osType
export command

#command to run ./configure
#sh unixinstall.sh ubuntu configure1
if [ "$command"x = "configure1"x ] ; then
	javapath=$3
	if [ "$javapath"x != ""x ] ; then
		export JAVA_HOME=$javapath
	fi

	current=`pwd`
	cd build
	sh ./configure

	echo "\n" | sh makexsb clean
	echo "\n" | sh makexsb

	echo "The compilation process has finished"
	echo "\n"
	echo "======= done ======="
fi

#command to run ./configure --with-dbdrivers
#sh unixinstall.sh ubuntu comfigure2
if [ "$command"x = "configure2"x ] ; then
	javapath=$3
	if [ "$javapath"x != ""x ] ; then
		export JAVA_HOME=$javapath
	fi

	current=`pwd`
	cd build
	sh ./configure --with-dbdrivers

	echo "\n" | sh makexsb clean
	echo "\n" | sh makexsb

	echo "The compilation process has finished"
	echo "\n"
	echo "======= done ======="
fi

#command to check whether jdk is installed 
#sh unixinstall.sh ubuntu checkJava
if [ "$command"x = "checkJava"x ] ; then
	str1=`which javac | grep  "javac"`
	str2=`which javac | grep  "no javac"`
	
	if [ "$str1"x = ""x ] || [ "$str2"x = "no javac"x ] ; then
		echo "no"
	else
		echo "yes"
	fi
fi

#command to install features
#sh unixinstall.sh ubuntu installFeatures your-password feature1 feature2....
if [ "$command"x = "installFeatures"x ] ; then
    if [ $# -le 2 ] ; then
	echo "No extra features requested.\n"	
	echo "======= done ======="
	exit
    fi

    password=$3
    export password

    shift 
    shift 
    shift

    while [ $# -ne 0 ]
    do
	featureName=$1
	echo ""
	echo "Installing $featureName ..."

	installFeature $featureName

	shift
    done

    installMakedepend

    echo "======= done ======="
fi

#command to check whether java home path in JAVA_HOME variable is the right one
#sh unixinstall.sh ubuntu checkhome
if [ "$command"x = "checkhome"x ]; then
    javahome=`echo $JAVA_HOME`
	
    if [ "$javahome"x = ""x ] ; then
	echo "no"
    else
	str1=`ls $javahome/include | grep jni.h`
	if [ "$str1"x = ""x ] ; then 
	    echo "no"
	    exit
	fi
	echo "yes"
    fi
fi

#command to check whether the jdk path in arg is the right one
#sh unixinstall.sh ubuntu checkhomearg your-jdk-path
if [ "$command"x = "checkhomearg"x ] ; then
    javahome=$3
    str1=`ls $javahome/include | grep jni.h`
    if [ "$str1"x = ""x ] ; then
	echo "no"
	exit
    fi

    str2=`ls $javahome/bin | grep javac`
    if [ "$str2"x = ""x ] ; then
	echo "no"
	exit
    fi

    echo "yes"
fi


if [ "$command"x = "checkpassword"x ] ; then
	password=$3

	test=`echo $password | sudo -S echo "xsb"`
	
	if [ "$test"x = "xsb"x ] ; then
		echo "granted"
	else
		echo "access denied"
	fi
fi
