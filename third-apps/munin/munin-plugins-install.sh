#!/bin/sh

## MySecureShell Munin Graphs Installation Tool v0.1 - Made by MySecureShell Team
## MySecureShell Team <teka2nerdman@users.sourceforge.net>

## Vars
mssbin=/usr/bin/mysecureshell

### NO NEED TO MODIFY NOW ###

## Root detection
if [ "$USER" != "root" ] ; then
    echo ""
    echo "#################################################################"
    echo "#	Sorry $USER but you must be root to continue		#"
    echo "#################################################################"
    echo ""
    exit 1
fi

## Test if MySecureShell exist
if [ ! -x $ssbin ] ; then
	echo "Sorry but $mssbin isn't executable or not detected, please install MSS first"
	exit 1
fi

## Munin Directory Detection
muninetc="/etc/munin/plugins"
if [ ! -d /etc/munin/plugins ] ; then
	echo "Sorry but I didn't find your /etc/munin/plugins/ directory for Munin. Could you please enter yours :"
	read muninetc
	test -d $muninetc || mkdir -p $muninetc
fi

muninusr="/usr/share/munin/plugins"
if [ ! -d /usr/share/munin/plugins ] ; then
	echo "Sorry but I didn't find your /usr/share/munin/plugins/ directory for Munin. Could you please enter yours :"
	read muninusr
	test -d $muninusr || mkdir -p $muninusr
fi

# Copy MySecureShell munin files and creating symbolic links
for i in mysecureshell mysecureshell_bandwith mysecureshell_users ; do
	echo "Do you want to install this kind of script : $i ? (y/n)"
	read ans
	if [ $ans = "y" ] ; then
		cp $i $muninusr && ln -s $muninusr/$i $muninetc/$i && chmod 755 $muninusr/$i && echo "$i [ INSTALLED ]"
	else
		echo "$i [ NOT INSTALLED ]"
	fi
done

if [ -x /etc/init.d/munin-node ] ; then
	echo "Would you like to restart munin for changes to take effect ? (y/n)"
	read muninrestart
	if [ $muninrestart = "y" ] ; then
		/etc/init.d/munin-node restart
	fi
else
	echo "If you run munin node, please restart the service for changes to take effect"
fi

echo "Installation finished"
