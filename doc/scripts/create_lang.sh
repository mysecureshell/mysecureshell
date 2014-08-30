#!/bin/bash

if [[ -z $1 ]] ; then
    echo 'Please enter a language: scripts/create_lang.sh <lang>'
    echo 'scripts/create_lang.sh fr'
    exit 1
fi

lang=$1
if [ ! -d build/locale ] ; then
    make gettext
fi

# Create sphinxdoc lang
sphinx-intl update --locale-dir source -p build/locale -l ../locale/$lang
sphinx-intl build --locale-dir source
make -e SPHINXOPTS="-D language='$lang'" html

# Create transifex lang
sphinx-intl update-txconfig-resources --pot-dir build/locale --transifex-project-name MySecureShell --locale-dir source

