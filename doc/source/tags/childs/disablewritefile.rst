DisableWriteFile
================

Synopsis
--------
This tag, deny to the connected user to write files.

Options
-------

================ ========= ======== ============= =======
Name             Default   Values   Since version Context
================ ========= ======== ============= =======
DisableWriteFile false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                          true
================ ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to write files::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableWriteFile true
    </User>

