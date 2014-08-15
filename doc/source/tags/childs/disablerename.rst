DisableRename
=============

Synopsis
--------
This tag, deny to the connected user to rename files or folders.

Options
-------

============= ========= ======== ============= =======
Name          Default   Values   Since version Context
============= ========= ======== ============= =======
DisableRename false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                       true
============= ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to rename files and folders::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableRename true
    </User>

