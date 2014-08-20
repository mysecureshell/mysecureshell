CanChangeRights
===============

.. highlight:: apache

Synopsis
--------
If enabled, a user or set of users, won't be able to change permissions on files and directories.

Options
-------

=============== ========= ======== ============= =======
Name            Default   Values   Since version Context
=============== ========= ======== ============= =======
CanChangeRights true      false    1.1           Default,Group,IpRange,User,VirtualHost
\                         true
=============== ========= ======== ============= =======

Examples
--------
Here, only users in the admins group are able to make changes on files and directories::

    <Default>
        CanChangeRights false
    </Default>

    <Group admins>
        CanChangeRights true
    </Group>

