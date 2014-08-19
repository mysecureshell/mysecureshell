Ubuntu installation
===================

.. contents:: Topics

.. highlight:: bash

Dedicated MySecureShell repository
----------------------------------

If you want to use the latest version of MySecureShell, the simplest way is to use the official dedicated repository.

Ubuntu 12.04
^^^^^^^^^^^^

First import the GPG repository key::

    # apt-key adv --keyserver pool.sks-keyservers.net --recv-keys E328F22B

Then, add the MySecureShell repository lines::

    # add-apt-repository 'deb http://mysecureshell.free.fr/repository/index.php/ubuntu/12.04 testing main'

You're now ready to install MySecureShell::

    # apt-get update
    # apt-get install mysecureshell

Ubuntu 10.10
^^^^^^^^^^^^

First import the GPG repository key::

    # apt-key adv --keyserver pool.sks-keyservers.net --recv-keys E328F22B

Then, add the MySecureShell repository lines::

    # add-apt-repository 'deb http://mysecureshell.free.fr/repository/index.php/ubuntu/10.10 testing main'

You're now ready to install MySecureShell::

    # apt-get update
    # apt-get install mysecureshell

