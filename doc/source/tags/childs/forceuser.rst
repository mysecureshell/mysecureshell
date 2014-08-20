ForceUser
==========

.. highlight:: apache

Synopsis
--------
You can force the usage of a username for the connected user. That mean when a user will connect, even if his username is not the desired one, it will force this group to be used when a file or folder are created. This allows multiple accounts to access the same account but each one with its own password.

Options
-------

========= ========= ========== ============= =======
Name      Default   Values     Since version Context
========= ========= ========== ============= =======
ForceUser null      group name 1.20          Default,Group,IpRange,User,VirtualHost
========= ========= ========== ============= =======

Examples
--------
In this example, we want to force the user *sftp_user* to be used by a *username* user::

    <Default>
        Home /home
    </Default>

    <User username>
        ForceUser sftp_user
    </User>

The *sftp_user* user will be used when the user *username* will create a file or a folder.
