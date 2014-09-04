DisableRemoveDir
================

.. highlight:: apache

Synopsis
--------
This tag, deny to the connected user to remove directories.

Options
-------

================= ========= ======== ============= =======
Name              Default   Values   Since version Context
================= ========= ======== ============= =======
DisableRemoveDir  false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                           true
================= ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to remove directories::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableRemoveDir true
    </User>

