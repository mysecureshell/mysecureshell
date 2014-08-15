DisableRemoveFile
=================

Synopsis
--------
This tag, deny to the connected user to remove files.

Options
-------

================= ========= ======== ============= =======
Name              Default   Values   Since version Context
================= ========= ======== ============= =======
DisableRemoveFile false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                           true
================= ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to remove files::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableRemoveFile true
    </User>

