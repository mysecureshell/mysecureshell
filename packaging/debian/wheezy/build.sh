#!/bin/bash -x

# Args:
# * version to build
# * packaging folder path

# Todo
# Verif dans le changelog que la version existe
# Sinon proposer de l'override ?

#version=$(echo $1 | sed 's/^v//')
pkg_folder=$2

# cd in the correct folder if requested in arg
if [ $pkg_folder ] ; then
    if [ -d $pkg_folder ] ; then
        cd $pkg_folder
    fi
elif [ ! -d ../debian ] ; then
    echo "Please be in the package folder"
    exit 1
fi

current_dir=$(basename "$PWD")

echo "[+] Loading vars"
. ./vars
version=$VERSION_BUILD

echo "[+] Copying sources"
rm -Rf package/debian/mysecureshell_$version
mkdir -p package/debian/mysecureshell_$version || exit 1
cd ..
cp -Rf config* Core install.sh.in LICENSE locales_* Makefile* man README* SftpAdmin sftp_config SftpServer SftpState SftpWho utils uninstaller.sh.in debian packaging/package/debian/mysecureshell_$version || exit 1
cd $pkg_folder/package/debian/mysecureshell_$version || exit 1

echo "[+] Flushing unwanted files"
debuild clean || exit 1
rm -Rf *.git doc debian/mysecureshell deployment-tools third-apps packaging
find . -name *.o -exec rm -f {} \;

echo "[+] Creating orig archive"
current_dir=$(basename "$PWD")
echo $current_dir
pwd
cd ..
tar -czf mysecureshell_$version.orig.tar.gz $current_dir || exit 1
cd $current_dir

echo "[+] Generating package"
if [ $SIGN_PACKAGES = 'yes' ] ; then
    if [ -d ~/.gnupg ] ; then
        dpkg-buildpackage -k$DEBSIGN_KEYID || exit 1
    else
        echo "[FAIL] Can't sign if no gpg keys are available (~/.gnupg)"
	exit 1
    fi
else
    debuild -us -uc --lintian-opts -i || exit 1
fi

echo "[+] Lintian package"
lintian -EviIL +pedantic ../*.deb

if [ $PUSH_UPSTREAM = 'yes' ] && [ $SIGN_PACKAGES = 'yes' ] ;then
    if [ ! -f ~/.dput.cf ] ; then
        cp ../.dput.cf ~
    fi
    # http://mentors.debian.net/package/mysecureshell
    echo "[+] Pushing to Debian mentors"
    cd ..
    rm *.upload
    dput mentors *.changes
fi
