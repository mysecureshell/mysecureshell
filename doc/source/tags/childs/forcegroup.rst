ForceGroup
==========

.. highlight:: apache

Synopsis
--------
You can force the group of the connected user. That mean when a user will connect, even if his group is not the desired one, it will force this group to be used when a file or folder are created.

Options
-------

========== ========= ========== ============= =======
Name       Default   Values     Since version Context
========== ========= ========== ============= =======
ForceGroup null      group name 1.20          Default,Group,IpRange,User,VirtualHost
========== ========= ========== ============= =======

Examples
--------
In this example, we want to force the group sftp to be used by a *user*::

    <Default>
        Home /home
    </Default>

    <User username>
        ForceGroup sftp
    </User>

The *sftp* group will be used when the user *username* will create a file or a folder.
