MySecureShell
=============

[![Build Status](https://travis-ci.org/mysecureshell/mysecureshell.svg?branch=master)](https://travis-ci.org/mysecureshell/mysecureshell)  [![Coverity Scan](https://scan.coverity.com/projects/8386/badge.svg)](https://scan.coverity.com/projects/mysecureshell-mysecureshell)

MySecureShell is a solution which has been made to bring **more features to sftp/scp protocol** given by OpenSSH. By default, OpenSSH brings a lot of liberty to connected users which imply to trust in your users. The goal of MySecureShell is to **offer the power and security of OpenSSH, with enhanced features** (like ACL) to restrict connected users.

MySecureShell was created because of the lack of file transfer features in OpenSSH. OpenSSH was not designed as a file transfer solution, that’s why we made MySecureShell.

**MySecureShell is not a patch for OpenSSH, it’s a shell for users**. It has the advantage to:

* Avoid including security holes in OpenSSH
* No dependency on against an OpenSSH version
* No OpenSSH recompilation is required

**So MySecureShell remains easy to install, secure and easy to configure.**

You can find the official [documentation here](https://mysecureshell.readthedocs.io/).
