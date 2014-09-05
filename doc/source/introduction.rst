Introduction
============

.. _my-reference-label:

What is MySecureShell?
----------------------

.. include:: mysecureshell_description.rst

Why SFTP and not FTP?
---------------------

If you're wondering why you should take MySecureShell as an SFTP server instead of a classical FTP, there are several reasons:

#. You do not have to open some dedicated firewall ports for file transfers
#. You are using one of the most used and secure protocol (SSH)
#. You do not have to manage SSL certificates to guaranty the security
#. As easy to use as a classical FTP server
#. As many or more features that you can find on any classical FTP servers
#. Easy to install, configure, maintain and use

What are MySecureShell's SFTP new features to OpenSSH
-----------------------------------------------------

Is MySecureShell really add enhanced features to OpenSSH sftp? Yes! Here is a sample of features of what MySecureShell is able to give:

* Bandwidth control
* Only authorized files and folders can be shown
* Advanced logging information
* ACL can be made with IP/Usernam/Groups/VirtualHost/...
* Confined environments (chroot, which is also available in the latests version of OpenSSH)
* Restrict users to have sftp only (shell access is disabled by default)
* And more...

Which Operating Systems are compatible with MySecureShell?
----------------------------------------------------------

MySecureShell has been developed to work on several types of Operating Systems like Linux, Mac OS X, Solaris, BSD and Windows (with `Cygwin <https://www.cygwin.com/>`_).

We unfortunately do not package MySecureShell for all platforms but we do our best to bring binary packages to most used/common Operating System for production usage like:

* Linux (`Debian <https://www.debian.org/>`_/`Ubuntu <http://www.ubuntu.com/>`_)
* Linux (`RedHat <http://www.redhat.com/>`_/`CentOS <http://www.centos.org/>`_/`Fedora <http://fedoraproject.org/>`_)
* Mac `OS X <https://www.apple.com>`_

If binary packages are not available for your system, we do our best to make them easily installable with scripts shell and Makefiles.
