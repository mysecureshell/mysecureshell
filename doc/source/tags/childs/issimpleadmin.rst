IsSimpleAdmin
=============

Synopsis
--------
IsSimpleAdmin tag grants to a user or a set of users, restricted admins permissions. That means they can administrate MySecureShell throught ``sftp-admin`` CLI or with MySecureShell GUI. This tag is less permissive than :doc:`IsAdmin tag<isadmin>`. IsSimpleAdmin permissions are limited to:

* See online users
* Disconnect users
* Start / shutdown MySecureShell server
* See statistics

Options
-------

============= ========= ======== ============= =======
Name          Default   Values   Since version Context
============= ========= ======== ============= =======
IsSimpleAdmin false     false    1.0           Default,Group,IpRange,User,VirtualHost
\                       true
============= ========= ======== ============= =======

Examples
--------
If you want to grant to *simpleadmins* group the administrative rights::

    <Default>
        Home /home
    </Default>

    <Group simpleadmins>
        IsSimpleAdmin true
    </Group>
