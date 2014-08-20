CanChangeTime
=============

.. highlight:: apache

Synopsis
--------
If enabled, a user or set of users, won't be able to change timestamps on files and directories.

Options
-------

============= ========= ======== ============= =======
Name          Default   Values   Since version Context
============= ========= ======== ============= =======
CanChangeTime true      false    1.1           Default,Group,IpRange,User,VirtualHost
\                       true
============= ========= ======== ============= =======

Examples
--------
Here, only users in the admins group are able to make changes on files and directories::

    <Default>
        CanChangeTime false
    </Default>

    <Group admins>
        CanChangeTime true
    </Group>

