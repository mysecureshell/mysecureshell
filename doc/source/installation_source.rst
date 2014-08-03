From sources installation
=========================

.. contents:: Topics

.. highlight:: bash

This method is generally used because you need need to build the latest version from VCS repository, need to make pull request for patching purpose or because it is not packaged on your Operating System.

The first thing to do is to get the sources and you will need git for that::

    $ git clone https://github.com/deimosfr/mysecureshell
    $ cd mysecureshell

Then prepare the sources with the configure command::

    $ ./configure --with-logcolor=yes

If everyhtings went fine, you should have something like this::

    MySecureShell has been configured with the following options:
    Log file: /var/log/sftp-server.log (color: yes)
    User binaries: /usr/bin
    Shutdown file: /etc/sftp.shut
    Configuration file: /etc/ssh/sftp_config
    
    Supported:
    Remote Admin : yes
    UTF-8 Encoding : yes

You can now launch the compilation::

    $ make all

And finally install it (with root user)::

    # make install

