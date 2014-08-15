DisableReadFile
===============

Synopsis
--------
This tag, deny to the connected user to read files content.

Options
-------

=============== ========= ======== ============= =======
Name            Default   Values   Since version Context
=============== ========= ======== ============= =======
DisableReadFile false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                         true
=============== ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to read files content::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableReadFile true
    </User>

