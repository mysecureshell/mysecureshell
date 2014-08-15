DisableSetAttribute
===================

Synopsis
--------
This tag, deny to the connected user to change the current files or folders attributes.

Options
-------

=================== ========= ======== ============= =======
Name                Default   Values   Since version Context
=================== ========= ======== ============= =======
DisableSetAttribute false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                   true
=================== ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to modify files and folders attributes::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableSetAttribute true
    </User>

