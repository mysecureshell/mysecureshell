FAQ
===

.. contents:: Topics

.. highlight:: bash

What can I do if I have a problem?
----------------------------------

#. Be sure it's not a miss-configuration problem first.
#. Check out the FAQ to be sure the problem was not already listed.
#. If the problem still persist, you can look at the `list of issues <https://github.com/deimosfr/mysecureshell/issues>`_ to see if the problem have not already corrected.
#. If not, create an `issue on GitHub <https://github.com/deimosfr/mysecureshell/issues>`_.

On issues, we'll do our best to reply as soon as possible.

I can't open symbolic link folders
----------------------------------

There are several reasons why it may not work. Check those bullet points:

* If VirtualChroot is enabled and the symbolic link is outside the chroot, it is not accessible and is this normal
* If StayAtHome is enabled, it may be the same reason.

If the problem still persist, you can use ``--bind`` option to ``mount`` command. For example if you need to access to an external folder (here /mnt/external) from the Home value (here /var/sftp)::

    # mkdir /var/sftp/external
    # mount --bind /mnt/external /var/sftp/external

Then you'll be able to access to the external folder inside the Home folder.

Connection refused after connection
-----------------------------------

When you have *connection refused* message when you're trying to login, this is generally a problem of firewalling.

Check that you're not behind a firewall or your SSH port (default is 22) is correctly open::

    # nc -v your_sftp_server 22
    Connection to localhost 22 port [tcp/ssh] succeeded!

If it's still not ok, please check that your SSH service is properly running::

    # service ssh status

Why do I got random errors on download or file access?
------------------------------------------------------

This is generally due to SFTP clients *cache*. Some clients like *FileZilla* have a cache enabled by default which can indicates wrong informations (connected even if you're not, showing files that do not exists anymore on the server, etc...).

The solution consist in disabling the cache on the client size.

Why uploaded files have 0 size on the server?
---------------------------------------------

When this problem occurs, it's usually because of a disc full on the server side.

Clients do not see all folders or files on the server, why?
-----------------------------------------------------------

Check if the ``HideNoAccess`` child tag is enabled. This tag hides folders and files to your users with no access to them.

If you want your users to be able to see (but do not have access) to any files or folders without having rights, you should consider disabling ``HideNoAccess`` value.

Only allow MySecureShell users for SFTP connections
---------------------------------------------------

If you want to only allow MySecureShell users to use sftp connections on your server, you need to change/adapt this line (OpenSSH version > 4.5) in */etc/ssh/sshd_config*::

    Subsystem sftp /bin/MySecureShell -c sftp-server

.. warning::

    Changing this parameters will block users who do not have MySecureShell as a shell!

How can I enforce security like blocking bruteforce attacks?
------------------------------------------------------------

You can use a solution called `Fail2ban <http://www.fail2ban.org>`_ which will check at the authentication logs in real time. If it detects a certain amount of failed login in a given time, it will block an IP address using iptables rules.

How coud I limit the number of simultaneous unauthenticated connections?
------------------------------------------------------------------------

If you want to limit the number of simultaneous unauthenticated connections by changing the ``MaxStartups`` value in */etc/ssh/sshd_config* file::

    MaxStartups 5

In this example, we do not allow more than 5 unauthenticated connections at the same time.

How can I make a passwordless connection?
-----------------------------------------

To get a secure connection without credentials, you can use SSH key exchange. One of the advantage of that solution is the security enforcement. To make it work, be sure that your server allow it in its configuration file */etc/ssh/sshd_config*::

    PubkeyAuthentication yes

If you only want to allow this user using SSH key, you can disable it's password account::

    # passwd -d useraccount

.. note::
    
    Note that it will also disable local password account

How to use another authentication system?
-----------------------------------------

Of course you can use another authentication system like:
* OpenLDAP
* Kerberos
* ...
You have to configure PAM (Pluggable Authentication Module) to be able to authenticate trought another backend. You can check that your backend connection is working fine like this::

    $ getent passwd username
    username:x:1000:1000:username,,,:/home/username:/usr/bin/zsh

We can see here a user called username, with id 1000. You now have to change his shell to make it work properly.
