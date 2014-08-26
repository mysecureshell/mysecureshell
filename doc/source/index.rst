.. MySecureShell documentation master file, created by
   sphinx-quickstart on Thu Jul 31 09:16:59 2014.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to MySecureShell's documentation!
=========================================

.. image:: images/logo_large.png
    :align: center

MySecureShell is a solution made to brings **more features to sftp/scp protocol** given by `OpenSSH <http://www.openssh.com/>`_. As by default, OpenSSH brings a lot of liberty to connected users which imply to thrust in your users. The goal of MySecureShell is to **offer the power and security of OpenSSH, with enhanced features** (like `ACL <http://en.wikipedia.org/wiki/Access_control_list>`_) to restrict connected users.

MySecureShell was created because of the lack of file transfer features in OpenSSH. OpenSSH was not designed as a file transfer tool at first.

**MySecureShell is not a patch for OpenSSH, it's a shell for users**. This to avoid including security holes in OpenSSH, not to be depedent of a version of OpenSSH and to avoid to recompile OpenSSH with a patch on each new versions. So **MySecureShell remains easy to install, secure and easy to configure**.

Regarding MySecureShell projects, here are usefull links you need to know:

* `The online documentation <https://mysecureshell.readthedocs.org>`_ can be found on `ReadTheDoc <https://readthedocs.org/>`_. You can **download the PDF,HTML or Epub** version directly from it.
* `The source code <https://github.com/deimosfr/mysecureshell>`_ is hosted on `GitHub <https://github.com>`_.
* If you encounter a bug or have any kind of issue related to MySecureShell, fell free to `open an issue <https://github.com/deimosfr/mysecureshell/issues>`_ on GitHub.

.. toctree::
    :maxdepth: 1
    :numbered:

    introduction
    quick_try
    installation
    configuration
    usages
    faq

