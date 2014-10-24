#!/bin/sh
current_dir=$(basename "$PWD")

if [ ! -d ../debian ] ; then
    echo "Please be in the package folder"
    exit 1
fi

echo "[+] Please choose a tag to build"
cd ..
git tag | sort -r
read tag
git checkout $tag
if [ $? -ne 0 ] ; then
    echo "[FAIL] can't switch to this tag/branch"
    exit 1
fi
version=$(echo $tag | sed 's/^v//g')

echo "[+] Loading vars"
. ./package/vars

echo "[+] Copying sources"
rm -Rf package/debian/mysecureshell_$version
mkdir -p package/debian/mysecureshell_$version || exit 1
cp -Rf config* Core install.sh.in LICENSE locales_* Makefile* man README* SftpAdmin sftp_config SftpServer SftpState SftpWho utils uninstaller.sh.in debian package/debian/mysecureshell_$version || exit 1
cd package/debian/mysecureshell_$version || exit 1

echo "[+] Flushing unwanted files"
debuild clean || exit 1
rm -Rf *.git doc debian/mysecureshell deployment-tools third-apps
find . -name *.o -exec rm -f {} \;

echo "[+] Creating orig archive"
current_dir=$(basename "$PWD")
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
