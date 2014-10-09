Fedora installation
===================

.. image:: images/logo_fedora.png
    :align: center
    :width: 300px

.. contents:: Topics

.. highlight:: bash

Dedicated MySecureShell repository
----------------------------------

If you want to use the latest version of MySecureShell, the simplest way is to use the official dedicated repository.

Fedora 19
^^^^^^^^^^^^^^^^^

First, add the repository lines in */etc/yum.repos.d/mysecureshell.repo*::

    > echo "[mysecureshell]
    name=MySecureShell
    baseurl=http://mysecureshell.free.fr/repository/index.php/fedora/19/
    enabled=1
    gpgcheck=0" > /etc/yum.repos.d/mysecureshell.repo

You're now ready to install MySecureShell::

    > yum install mysecureshell

Fedora 17
^^^^^^^^^^^^^^^^^

First, add the repository lines in */etc/yum.repos.d/mysecureshell.repo*::

    > echo "[mysecureshell]
    name=MySecureShell
    baseurl=http://mysecureshell.free.fr/repository/index.php/fedora/17/
    enabled=1
    gpgcheck=0" > /etc/yum.repos.d/mysecureshell.repo

You're now ready to install MySecureShell::

    > yum install mysecureshell

