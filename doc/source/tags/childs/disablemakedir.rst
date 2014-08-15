DisableMakeDir
==============

Synopsis
--------
This tag, deny to the connected user to create folders.

Options
-------

============== ========= ======== ============= =======
Name           Default   Values   Since version Context
============== ========= ======== ============= =======
DisableMakeDir false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                        true
============== ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to create directories::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableMakeDir true
    </User>

