#!/bin/sh

## Installation Script v0.7 - Made by Pierre
## MySecureShell Team <teka2nerdman@users.sourceforge.net>

## Language local initialising

LANG=

## Functions Looking for available languages

MyGetLocale()
{
	if [ "$LANG" == "" ] ; then
		echo $1
	else
		tmp=`grep -F "$1=" locales_$LANG | cut -d= -f2-`
		if [ "$tmp" == "" ] ; then
			echo $1
		else
			echo $tmp
		fi
	fi
}

MyListLocale()
{
	echo "The available languages are:"
	grep -F 'DESCRIPTION=' locales_* | cut -d= -f2-
	echo "Usage: ./install.sh xx(language)"
}

if [ "$1" == "" ] ; then
	MyListLocale
else
	if [ -f "locales_$1" ] ; then
		LANG=$1
	fi

## Root detection

if [ `whoami` == "root" ] ; then
	echo ""
else
	echo -e "\n###################################################################"
	tmp=`MyGetLocale 'sorry'`
	echo -e "\t\t\t"$tmp `whoami`
	MyGetLocale 'Warning root ask'
	echo -e "###################################################################\n"
	exit 1
fi

## Functions
detecfiles() {
	fileufund=`MyGetLocale 'Existing file'`" $filedetec\t\t\t`MyGetLocale 'failed'`
	echo -e "$fileufund"
	instend=`MyGetLocale 'installation'`"\t\t\t\t"`MyGetLocale 'failed'`"
	echo -e "$instend"
        exit 1
}

filefound() {
	filefund=`MyGetLocale 'Existing file'`" $filedetec\t\t\t`MyGetLocale 'ok'`"
	echo -e "$filefund"
}

sftpfunc() {
	if [ -f $sftpsrv/sftp-server_MSS ] ; then
		mv -f $sftpsrv/sftp-server_MSS $sftpsrv/sftp-server_MSS.bak
		cp -f ./sftp-server_MSS $sftpsrv/sftp-server_MSS
		sftpinst=$sftpinst`MyGetLocale 'savesftps_mss'`"\t\t\t"`MyGetLocale 'ok'`"\n"
	else
		cp -f ./sftp-server_MSS $sftpsrv/sftp-server_MSS
	fi
	sftpinst=$sftpinst`MyGetLocale 'addsftps_mss'`"\t\t\t"`MyGetLocale 'ok'`"\n"
	chmod -f 4755 $sftpsrv/sftp-server_MSS
}

shellfunc() {
grepshell=`grep /bin/MySecureShell /etc/shells`
if [ $? == 0 ] ; then
	sftpinst=$sftpinst`MyGetLocale 'shellalreadyvd'`"\t\t"`MyGetLocale 'ok'`"\n"
else
	MyGetLocale 'validshellask'
	read rep3
	if [ $rep3 = "y" ] ; then
		echo "/bin/MySecureShell" >>/etc/shells
		sftpinst=$sftpinst`MyGetLocale 'shellvalid'`"\t\t"`MyGetLocale 'ok'`"\n"
	else
		if [ $rep3 = "n" ] ; then
			sftpinst=$sftpinst`MyGetLocale 'novalidshell'`"\t"`MyGetLocale 'ok'`"\n"
		else
			clear  	    
			instend=`MyGetLocale 'installation'`"\t\t\t"`MyGetLocale 'failed'`"\t\n"
			echo -e "$instend"
			MyGetLocale 'answers'
			exit 1
		fi
	fi
fi
}

## Starting script
## Welcome and files detection

clear
echo "#########################################"
echo -e "#\t\tMySecureShell\t\t#"
echo -e "#########################################\n"
tmp=`MyGetLocale 'Welcome'`
echo -e $tmp "\n"

MyGetLocale 'Needed installation files'
filedetec="MySecureShell"
if [ -f ./$filedetec ] ; then
	filefound
	filedetec="sftp_config"
	if [ -f ./$filedetec ] ; then
		filefound
		filedetec="sftp-server_MSS"
		if [ -f ./$filedetec ] ; then
	    		filefound
		else
	    		detecfiles
		fi
     	else
		detecfiles
   	fi
else
 	detecfiles
fi

## Test system

echo -e "\n"
tmp=`MyGetLocale 'TestSystem?'`
echo -n $tmp
read rep7

if [ $rep7 = "y" ] ; then
    MyGetLocale 'LaunchMSS'
    ./MySecureShell --configtest > /dev/null
    MyGetLocale 'LaunchSftpMSS'
    ./sftp-server_MSS < /dev/null
    MyGetLocale 'Testsuccess'
fi

## Introduction text

echo -e "\n"
MyGetLocale 'text1'
MyGetLocale 'text2'
MyGetLocale 'text3'
MyGetLocale 'text4'
MyGetLocale 'text5'
MyGetLocale 'text6'
MyGetLocale 'text7'

## Starting or ending installation

read rep1
if [ $rep1 = "y" ] ; then
	tmp=`MyGetLocale 'installation'`
	echo -e "$tmp\n"
else
	clear
	tmp2=`MyGetLocale 'installation'`"\t\t\t\t"`MyGetLocale 'failed'`"\n"
	if [ $rep1 = "n" ] ; then
		break
	else
		MyGetLocale 'answers'
	fi
	echo -e "$tmp2"
exit 1
fi

## If MSS is present, stop server

if [ -f /usr/bin/sftp-state ] ; then
	MyGetLocale 'statestopquest'
	/usr/bin/sftp-state stop
fi

## Existing ssh or sshd folder

if [ -d /etc/sshd ] ; then
	sshfolder=/etc/sshd
else
	if [ -d /etc/ssh ] ; then
		sshfolder=/etc/ssh
	else
		MyGetLocale 'mksshfolder'
		read repssh
		if [ $repssh = 'y' ] ; then
			sshfolder=/etc/ssh
			mkdir $sshfolder
			sftpinst=`MyGetLocale 'lgsshfolder'`"\t\t\t\t"`MyGetLocale 'ok'`"\t\n"
		else
			clear
			MyGetLocale 'stopinstssh'
			echo -e "$tmp2"
			exit 1
		fi
	fi
fi

## Config file

if [ -f $sshfolder/sftp_config ] ; then
	echo -e "\n"
	MyGetLocale 'warnconf'
	MyGetLocale 'warnerase'
	read rep2
	if [ $rep2 = "y" ] ; then
		cp -f ./sftp_config $sshfolder/sftp_config
		chmod 644 $sshfolder/sftp_config
		sftpinst=`MyGetLocale 'conffilerep1'`"\t\t"`MyGetLocale 'ok'`"\t\n"
	else
		if [ $rep2 = "n" ] ; then
			sftpinst=$sftpinst`MyGetLocale 'conffilerep2'`"\t"`MyGetLocale 'ok'`"\t\n"
		else
			clear  	    
			instend=`MyGetLocale 'installation'`"\t\t\t\t"`MyGetLocale 'failed'`"\t\n"
			echo -e "$instend"
			MyGetLocale 'answers'
			exit 1
		fi
	fi
else
	cp -f ./sftp_config $sshfolder/sftp_config
	chmod 644 $sshfolder/sftp_config
	sftpinst=$sftpinst`MyGetLocale 'mkconffile'`"\t\t\t\t"`MyGetLocale 'ok'`"\t\n"
fi

## Updating Detection

if [ -f /bin/MySecureShell ] ; then
	sftpinst=$sftpinst`MyGetLocale 'upconffile'`"\t\t\t"`MyGetLocale 'ok'`"\t\n"
else
	sftpinst=$sftpinst`MyGetLocale 'mkconffile'`"\t\t\t\t"`MyGetLocale 'ok'`"\t\n"
fi
cp -f ./MySecureShell /bin
chmod 755 /bin/MySecureShell

## /etc/shells detection

if [ -f /etc/shells ] ; then
	shellfunc
else
	MyGetLocale 'mkshells'
	read repshells
	if [ $repshells = 'y' ] ; then
		touch /etc/shells
		chmod 644 /etc/shells
		shellfunc
	else
		clear  	    
		instend=`MyGetLocale 'installation'`"\t\t\t"`MyGetLocale 'failed'`"\t\n"
		echo -e "$instend"
		MyGetLocale 'answers'
	fi
fi

## sftp-server_MSS Installation

sftpservernum="0"
for sftp_server_detect in "/usr/libexec" "/usr/lib" "/usr/lib/ssh" "/usr/lib/openssh" "/usr/libexec/openssh" ; do
	if [ -f $sftp_server_detect/sftp-server ] ; then
		sftpsrv=$sftp_server_detect
		sftpfunc
		sftpservernum="1"
	fi
done
if [ $sftpservernum = "0" ] ; then
	sftploctmp=`/usr/bin/locate sftp-server`
	if [ $? == 1 ] ; then
		clear
		MyGetLocale 'nosftpfund'
		instend=`MyGetLocale 'installation'`"\t\t\t"`MyGetLocale 'failed'`"\t\n"
		echo -e "$instend"
		exit 1
	else
		MyGetLocale 'sftpsrvloc1'
		MyGetLocale 'sftpsrvloc2'
		read rep4
		if [ $rep4 = 'y' ] ; then
			MyGetLocale 'locfund1'
			MyGetLocale 'locfund2'
			echo -e ""
			for file in `locate sftp-server` ; do
				if [ -x $file ] ; then
					echo "[$file]"
				fi
			done
			echo -e "\n"
			MyGetLocale 'locfund3'
			MyGetLocale 'locfund4'
			read rep5
			sftpsrv=$rep5
			sftpfunc
			echo -e "\n"
		else
			clear
			MyGetLocale 'sftp-s_unlocated'
			tmp=`MyGetLocale 'installation'`"\t\t\t\t"`MyGetLocale 'failed'`"\n"
			echo -e $tmp
			exit 1
		fi
	fi
fi

## Utilities installation

if [ -d ./utils ] ; then
	MyGetLocale 'utilities?'
	read rep6
	if [ $rep6 = 'y' ] ; then
# sftp-who file
		if [ -f ./utils/sftp-who ] ; then
			cp -f ./utils/sftp-who /usr/bin
			sftpinst=$sftpinst`MyGetLocale 'sftp-who'`"\t\t\t"`MyGetLocale 'ok'`"\n"
		else
			sftpinst=$sftpinst`MyGetLocale 'sftp-who'`"\t\t\t"`MyGetLocale 'failed'`"\n"`MyGetLocale 'unfound'`"\n"
		fi
# sftp-kill file
		if [ -f ./utils/sftp-kill ] ; then
			cp -f ./utils/sftp-kill /usr/bin
			sftpinst=$sftpinst`MyGetLocale 'sftp-kill'`"\t\t\t"`MyGetLocale 'ok'`"\n"
		else
			sftpinst=$sftpinst`MyGetLocale 'sftp-kill'`"\t\t\t"`MyGetLocale 'failed'`"\n"`MyGetLocale 'unfound'`"\n"
		fi
# sftp-state file
		if [ -f ./utils/sftp-state ] ; then
			cp -f ./utils/sftp-state /usr/bin
			sftpinst=$sftpinst`MyGetLocale 'sftp-state'`"\t\t\t"`MyGetLocale 'ok'`"\n"
			/usr/bin/sftp-state start
		else
			sftpinst=$sftpinst`MyGetLocale 'sftp-state'`"\t\t\t"`MyGetLocale 'failed'`"\n"`MyGetLocale 'unfound'`"\n"
		fi
	else
		sftpinst=$sftpinst`MyGetLocale 'noutilities'`"\t"`MyGetLocale 'ok'`"\n"
	fi
else
	sftpinst=$sftpinst`MyGetLocale 'utilsnodetec'`"\t\t"`MyGetLocale 'failed'`"\n"
fi

# Fin d'installation

sftpinst=$sftpinst"\n"`MyGetLocale 'finishedinst'`"\n\n"`MyGetLocale 'osxreminder'`

clear
echo -e "$sftpinst\n"

fi

