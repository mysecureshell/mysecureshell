CentOS/RedHat installation
==========================

.. contents:: Topics

.. highlight:: bash

Dedicated MySecureShell repository
----------------------------------

If you want to use the latest version of MySecureShell, the simplest way is to use the official dedicated repository.

CentOS 6
^^^^^^^^

First, add the repository lines in */etc/yum.repos.d/mysecureshell.repo*::

    # echo "[mysecureshell]
    name=MySecureShell
    baseurl=http://mysecureshell.free.fr/repository/index.php/centos/6.4/
    enabled=1
    gpgcheck=0" > /etc/yum.repos.d/mysecureshell.repo

You're now ready to install MySecureShell::

    # yum install mysecureshell

CentOS 5
^^^^^^^^

First, add the repository lines in */etc/yum.repos.d/mysecureshell.repo*::

    # echo "[mysecureshell]
    name=MySecureShell
    baseurl=http://mysecureshell.free.fr/repository/index.php/centos/5.5/
    enabled=1
    gpgcheck=0" > /etc/yum.repos.d/mysecureshell.repo

You're now ready to install MySecureShell::

    # yum install mysecureshell

