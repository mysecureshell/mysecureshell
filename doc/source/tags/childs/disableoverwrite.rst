DisableOverwrite
================

.. highlight:: apache

Synopsis
--------
This tag, deny to the connected user to overwrite onto an existing file.

Options
-------

================ ========= ======== ============= =======
Name             Default   Values   Since version Context
================ ========= ======== ============= =======
DisableOverwrite false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                          true
================ ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to overwrite existing files::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableOverwrite true
    </User>

